/*
	File:		MiscForms.c

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):
         <1>     1/15/98    ksh     Initial Projector check-in from N90Buddy.c
*/

#include "N90sBuddy.h"

/***********************************************************************
 *
 * FUNCTION:     DownloadN90SMemoHolder
 *
 * DESCRIPTION:  Downloads all data from the Nikon Memo holder
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err DownloadN90SMemoHolder(Boolean firstRoll)
{
	Err			err = 0;
	struct		params {
		Word	locationFD00;		// 4
		Word	locationFD02;		
		UChar	locationFD40;		//	8
		UChar	locationFD41;
		Word	locationFD42;
		Word	locationFD44;
		Word	locationFD46;
	} mem;
	Word			readBytes;
	Word			dbSize;
	Word			nFrames;
	Word			bytesPerFrame;
	Word			i;
	Word			bytesFree;
	Word			id;
	Word			cameraSize;
	Word			captionSize;
	ULong			offset;
	Handle		rollH = NULL;
	UChar			*ddata = NULL;
	UChar			*p;
	Char			captionName[] = "";
	RollHeaderPackedPtr	recordP;
	RollHeaderPackedRec	rollHeader;
	MemoHolderResponse	memoData;
	
	err = SendCommand(kReadDataMode, 0x0000FD00, &(mem.locationFD00), 4);
	err = SendCommand(kReadDataMode, 0x0000FD40, &(mem.locationFD40), 8);
	
	if (err)
		goto ERROR;
	
	//	If the current start pointer is the same as the current roll start pointer
	//	then there's nothing in the buffer to read
	if (mem.locationFD44 == mem.locationFD46)
	{
		err = kNoMemoDataErr;
		goto ERROR;
	}
	
	//	Get the memo holder info.
	err = SendCommand(kMemoHolderMode, 0, &memoData, kMemoHolderResponseSize);
	if (err)
		goto ERROR;

	SwapShort(&mem.locationFD00);	// mh start
	SwapShort(&mem.locationFD02);	// mh end
	SwapShort(&mem.locationFD44);	// memo holder start
	SwapShort(&mem.locationFD46);	// current roll start
	SwapShort(&memoData.byteCount);
	SwapShort(&memoData.rollNumber);
		
	bytesPerFrame = MemoSettings2Bytes(mem.locationFD40);
	if (bytesPerFrame == 0)
	{
		err = kUnknownDataErr;
		goto ERROR;
	}
		
	nFrames = (memoData.byteCount - 4) / bytesPerFrame;
	
	cameraSize = StrLen(gCameraName) + 1;
	captionSize = StrLen(captionName) + 1;
	dbSize = (nFrames * (kFrameHeaderPackedSize + 1)) + 
				kRollHeaderPackedSize +
				cameraSize + captionSize;

	ddata = (UChar *) MemPtrNew(memoData.byteCount);
	rollH = DmNewHandle(gMemoDatabaseRef, dbSize);
	if ((ddata == NULL) || (rollH == NULL))
	{
		err = memErrNotEnoughSpace;
		goto ERROR;
	}
			
	//	if the buffer has wrapped around, read the roll data in two passes
	if (mem.locationFD44 + memoData.byteCount > mem.locationFD02)
	{
		readBytes = mem.locationFD02 - mem.locationFD44;
		err = SendCommand(kReadDataMode, 0x00010000 + mem.locationFD44, ddata, readBytes);
		err = SendCommand(kReadDataMode, 0x00010000 + mem.locationFD00, ddata+readBytes, memoData.byteCount - readBytes);
	}
	else
	{
		err = SendCommand(kReadDataMode, 0x00010000 + mem.locationFD44, ddata, memoData.byteCount);
	}
	
	if (err)
		goto ERROR;
		
	rollHeader.timeDate = TimGetSeconds();
	rollHeader.rollNumber = BCD2Short(memoData.rollNumber);
	rollHeader.iso = ddata[memoData.byteCount-1];
	rollHeader.storageLevel = bytesPerFrame/2 - 1;
	rollHeader.frameCount = nFrames;

	offset = 0;
	
	recordP = (RollHeaderPackedPtr) MemHandleLock(rollH);
	if (recordP == NULL)
	{
		err = memErrNotEnoughSpace;
		goto ERROR;
	}
		
	err = DmWriteCheck(recordP, offset, dbSize);
	if (err)
		goto ERROR;
		
	err = DmWrite(recordP, offset, &rollHeader, kRollHeaderPackedSize);
	offset += kRollHeaderPackedSize;

	err = DmWrite(recordP, offset, gCameraName, cameraSize);
	offset += cameraSize;

	err = DmWrite(recordP, offset, captionName, captionSize);
	offset += captionSize;

	p = ddata + 2;	// skip over roll number
	bytesFree = kFrameHeaderPackedSize - bytesPerFrame + 1;
	for (i = 0; i < nFrames; i++)
	{
		err = DmWrite(recordP, offset, p, bytesPerFrame);
		offset += bytesPerFrame;
		err = DmSet(recordP, offset, bytesFree, 0);
		offset += bytesFree;
		p += bytesPerFrame;
	}

	if (err)
		goto ERROR;
		
	MemPtrFree(ddata);
	ddata = NULL;
	
	MemHandleUnlock(rollH);
	id = 0;
	
	//	If there weren't any frames for this roll, skip the roll
	
	if (nFrames)
	{	
		err = DmAttachRecord(gMemoDatabaseRef, &id, rollH, NULL);
		if (err)
			goto ERROR;

		DmReleaseRecord(gMemoDatabaseRef, id, true);
		rollH = NULL;
	}
	else
	{
		MemHandleFree(rollH);
		rollH = NULL;
	}

	// Save the first successful download point of this session
	if (firstRoll == false)
	{
		BuddyPreferencesType	prefs;

		GetPreferences(&prefs);
		prefs.lastDownloadMark = mem.locationFD44;
		SwapShort(&(prefs.lastDownloadMark));
		SetPreferences(&prefs);
	}

	// Increment the next roll pointer by the number of bytes read
	mem.locationFD44 += memoData.byteCount;
	
	if (mem.locationFD44 >= mem.locationFD02)
		mem.locationFD44 = mem.locationFD44 - mem.locationFD02 + mem.locationFD00;

	// if we're not at the current roll, BOZO -- is it +6 or + bytesPerFrame
	if (mem.locationFD46 != mem.locationFD44)
		mem.locationFD44 += bytesPerFrame;
			
	if (mem.locationFD44 >= mem.locationFD02)
		mem.locationFD44 = mem.locationFD44 - mem.locationFD02 + mem.locationFD00;

	SwapShort(&mem.locationFD44);
	err = SendCommand(kWriteDataMode, 0x0000FD44, &(mem.locationFD44), 2);
	
	if (err)
		goto ERROR;
	
ERROR:

	if (ddata) MemPtrFree(ddata);
	if (rollH) MemHandleFree(rollH);
		
	return err;
}

/***********************************************************************
 *
 * FUNCTION:     DownloadN90MemoHolder
 *
 * DESCRIPTION:  Downloads all data from the Nikon Memo holder
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err DownloadN90MemoHolder(Boolean foundRoll)
{
	Err			err = 0;
	struct		params {
		UChar	locationFD40;		//	8
		UChar	locationFD41;
		UChar	locationFD42;
		UChar	locationFD43;
	} mem;
	Word			memoPtr;
	Word			readBytes;
	Word			dbSize;
	Word			nFrames;
	Word			bytesPerFrame;
	Word			i;
	Word			bytesFree;
	Word			id;
	Word			cameraSize;
	Word			captionSize;
	ULong			offset;
	Handle		rollH = NULL;
	UChar			*ddata = NULL;
	UChar			*p;
	Char			captionName[] = "";
	RollHeaderPackedPtr	recordP;
	RollHeaderPackedRec	rollHeader;
	MemoHolderResponse	memoData;
	
	err = SendCommand(kReadDataMode, 0x0000FD40, &(mem.locationFD40), 4);
	
	if (err)
		goto ERROR;
	
	//	If the current start pointer is the same as the current roll start pointer
	//	then there's nothing in the buffer to read
	if (mem.locationFD42 == mem.locationFD43)
	{
		err = kNoMemoDataErr;
		goto ERROR;
	}
	
	memoData.byteCount = 0xFF;
	
	bytesPerFrame = MemoSettings2Bytes(mem.locationFD40);
	if (bytesPerFrame == 0)
	{
		err = kUnknownDataErr;
		goto ERROR;
	}

	memoData.byteCount = 0xFF;
	ddata = (UChar *) MemPtrNew(memoData.byteCount);
	if (ddata == NULL)
	{
		err = memErrNotEnoughSpace;
		goto ERROR;
	}
			
	//	if the buffer has wrapped around, read the roll data in two passes
	memoPtr = mem.locationFD42 * 2;
	if (memoPtr + memoData.byteCount > 0x0200)
	{
		readBytes = 0x200 - memoPtr;
		err = SendCommand(kReadDataMode, 0x00010000 + memoPtr, ddata, readBytes);
		err = SendCommand(kReadDataMode, 0x00010000 + 0x0044, ddata+readBytes, memoData.byteCount - readBytes);
	}
	else
	{
		err = SendCommand(kReadDataMode, 0x00010000 + memoPtr, ddata, memoData.byteCount);
	}
	
	if (err)
		goto ERROR;

	p = (ddata + 2);
	nFrames = 0;
	while (*p != 0xFF)
	{
		nFrames++;
		p += bytesPerFrame;
	}

	memoData.rollNumber = *((Word *) ddata);
	rollHeader.timeDate = TimGetSeconds();
	SwapShort(&memoData.rollNumber);
	rollHeader.rollNumber = BCD2Short(memoData.rollNumber);
	rollHeader.iso = p[1];
	rollHeader.storageLevel = bytesPerFrame/2 - 1;
	rollHeader.frameCount = nFrames;
	memoData.byteCount = (p - ddata) + 2;
	
	cameraSize = StrLen(gCameraName) + 1;
	captionSize = StrLen(captionName) + 1;
	dbSize = (nFrames * (kFrameHeaderPackedSize + 1)) + 
				kRollHeaderPackedSize +
				cameraSize + captionSize;

	rollH = DmNewHandle(gMemoDatabaseRef, dbSize);
	if (rollH == NULL)
	{
		err = memErrNotEnoughSpace;
		goto ERROR;
	}

	offset = 0;
	
	recordP = (RollHeaderPackedPtr) MemHandleLock(rollH);
	if (recordP == NULL)
	{
		err = memErrNotEnoughSpace;
		goto ERROR;
	}
		
	err = DmWriteCheck(recordP, offset, dbSize);
	if (err)
		goto ERROR;
		
	err = DmWrite(recordP, offset, &rollHeader, kRollHeaderPackedSize);
	offset += kRollHeaderPackedSize;

	err = DmWrite(recordP, offset, gCameraName, cameraSize);
	offset += cameraSize;

	err = DmWrite(recordP, offset, captionName, captionSize);
	offset += captionSize;

	p = ddata + 2;	// skip over roll number
	bytesFree = kFrameHeaderPackedSize - bytesPerFrame + 1;
	for (i = 0; i < nFrames; i++)
	{
		err = DmWrite(recordP, offset, p, bytesPerFrame);
		offset += bytesPerFrame;
		err = DmSet(recordP, offset, bytesFree, 0);
		offset += bytesFree;
		p += bytesPerFrame;
	}

	if (err)
		goto ERROR;
		
	MemPtrFree(ddata);
	ddata = NULL;
	
	MemHandleUnlock(rollH);
	id = 0;
	
	if (nFrames)
	{	
		err = DmAttachRecord(gMemoDatabaseRef, &id, rollH, NULL);
		if (err)
			goto ERROR;

		DmReleaseRecord(gMemoDatabaseRef, id, true);
		rollH = NULL;
	}
	else
	{
		MemHandleFree(rollH);
		rollH = NULL;
	}


	// Save the first successful download point of this session
	if (foundRoll == false)
	{
		BuddyPreferencesType	prefs;

		GetPreferences(&prefs);
		prefs.lastDownloadMark = mem.locationFD42;
		SetPreferences(&prefs);
	}

	// Increment the next roll pointer by the number of bytes read
	memoPtr += memoData.byteCount;
	
	if (memoPtr >= 0x200)
		memoPtr = memoPtr - 0x0200 + 0x0044;

	mem.locationFD42 = memoPtr / 2;
	err = SendCommand(kWriteDataMode, 0x0000FD42, &(mem.locationFD42), 1);
	
ERROR:

	if (ddata) MemPtrFree(ddata);
	if (rollH) MemHandleFree(rollH);
		
	return err;
}


/***********************************************************************
 *
 * FUNCTION:     DownloadMemoHolder
 *
 * DESCRIPTION:  Downloads all data from the Nikon Memo holder
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err DownloadMemoHolder()
{
	Err			err = 0, endErr;
	
	Boolean		foundRoll = false;

#if DEBUGBUILD
	DebugStartLog();
#endif

	while (err == 0)
	{

		if ((err = StartSession()) != 0) goto ERROR;
		
		if (gCameraType == cameraN90s)
			err = DownloadN90SMemoHolder(foundRoll);
		else
			err = DownloadN90MemoHolder(foundRoll);
			
		if (err == 0)
			foundRoll = true;
		
		endErr = EndSession();
		if (err == 0) err = endErr;
	}

ERROR:
	
	if ((foundRoll == true) && (err == kNoMemoDataErr))
		err = 0;

	if (err)
		PostError(err);
	else
		SndPlaySystemSound(sndConfirmation);

#if DEBUGBUILD
	DebugStopLog();
#endif
		
	return err;
}

/***********************************************************************
 *
 * FUNCTION:		MemoFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒmemoÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean MemoFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;
	
	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (MemoFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
   		case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case MemoSettingsSaveButton:
	   				MemoFormSave();
	   			case MemoSettingsCancelButton:
	 				FrmReturnToForm(0);
					handled = true;
	 				break;
	 			default:
	 				gRecordDirty = true;
					break;
	  			
	   		}
			break;
		
		case fldEnterEvent:
			gRecordDirty = true;
			break;
	
		case keyDownEvent:
			if ((FrmGetFocus(FrmGetActiveForm()) == GetObjectIndex(MemoSettingsNextRollField)) &&
				 (FilterKey(event->data.keyDown.chr, (UChar *) kNumericFilter)))
				 handled = true;
			break;
		
		}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		MemoFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err MemoFormInit()
{
	Err			err = 0,endErr;
	union params
	{
		struct {
			UChar	locationFD21;		// 1
			UChar	locationFD34;		// 1
			Word	locationFD3E;		// 2
			UChar	locationFD40;		// 8
			UChar	locationFD41;
			Word	locationFD42;
			Word	locationFD44;
			Word	locationFD46;
		} n90smem;
		struct {
			UChar	locationFD21;		// 1
			UChar	locationFD34;		// 1
			Word	locationFD3E;		// 2
			UChar	locationFD40;		// 4
			UChar	locationFD41;
			UChar	locationFD42;
			UChar	locationFD43;
		} n90mem;
	} mem;

	Char			roll_text[kRollDigits + 1];
	Word			disable_memo = 0;
	SWord			index = 0;

	gRecordDirty = false;
	
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD21, &(mem.n90smem.locationFD21), 1);
	SendCommand(kReadDataMode, 0x0000FD34, &(mem.n90smem.locationFD34), 1);
	SendCommand(kReadDataMode, 0x0000FD3E, &(mem.n90smem.locationFD3E), 2);
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD40, &(mem.n90smem.locationFD40), 8);
	else if (gCameraType == cameraN90)
		SendCommand(kReadDataMode, 0x0000FD40, &(mem.n90mem.locationFD40), 4);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	Value2Index(kMemoList,mem.n90smem.locationFD40,&index);
	if (index < 0)
	{
		Value2Index(kMemoDisableList,mem.n90smem.locationFD40,&index);
		if (index < 0)
		{
			err = kUnknownDataErr;
			goto ERROR;
		}
		//	If the value was in the disabled list, we're not recording anymore
		index = 0;
	}
	
	SetPopupList(MemoSettingsMemoList, MemoSettingsMemoPopTrigger, index);

	HideObject(MemoSettingsDataStoredInCameraLabel);
	HideObject(MemoSettingsFilminCameraLabel);
	HideObject(MemoSettingsAlertBitMap);
	if (mem.n90smem.locationFD21 != kMemoNoFilm)
	{
		//	there's film in camera so disable control
		
		CtlSetEnabled(GetObjectPtr(MemoSettingsMemoPopTrigger),false);
		((ListPtr) GetObjectPtr(MemoSettingsMemoList))->attr.enabled = false;
		ShowObject(MemoSettingsAlertBitMap);
		ShowObject(MemoSettingsFilminCameraLabel);
		
	}
	else if (((gCameraType == cameraN90s) && (mem.n90smem.locationFD44 != mem.n90smem.locationFD46)) ||
			((gCameraType == cameraN90) && (mem.n90mem.locationFD42 != mem.n90mem.locationFD43)))
	{
		//	there's data in camera so disable control
		CtlSetEnabled(GetObjectPtr(MemoSettingsMemoPopTrigger),false);
		((ListPtr) GetObjectPtr(MemoSettingsMemoList))->attr.enabled = false;
		ShowObject(MemoSettingsAlertBitMap);
		ShowObject(MemoSettingsDataStoredInCameraLabel);
	}

	index = (mem.n90smem.locationFD34 & kMemoFullDownloadFlag) ? 1 : 0;
	SetPopupList(MemoSettingsFullList, MemoSettingsFullPopTrigger, index);
	
	if (mem.n90smem.locationFD34 & kMemoImprintFrameFlag)
		CtlSetValue(GetObjectPtr(MemoSettingsImprintCheckbox), 1);

	SwapShort(&mem.n90smem.locationFD3E);
	StrIToA(roll_text, BCD2Short(mem.n90smem.locationFD3E));
	SetFieldText(MemoSettingsNextRollField, roll_text);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;
	
	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		MemoFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err MemoFormSave()
{
	Err				err = 0, endErr;
	struct		params {
		UChar	locationFD34;		//	1
		Word	locationFD3E;		// 3
		UChar	locationFD40;
	} mem;
	Word				roll_number;
	UChar				index;
	UChar				value;
	Word				bytesOut;

	if (gRecordDirty == false)
		return err;
		
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD34, &(mem.locationFD34), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;
	
	index = LstGetSelection(GetObjectPtr(MemoSettingsMemoList));
	Index2Value(kMemoList,index,&value);
	mem.locationFD40 = value;

	CLEAR_FLAGS(mem.locationFD34, kMemoFullDownloadFlag | kMemoImprintFrameFlag);

	index = LstGetSelection(GetObjectPtr(MemoSettingsFullList));
	if (index)
		SET_FLAGS(mem.locationFD34, kMemoFullDownloadFlag);
	
	index = CtlGetValue(GetObjectPtr(MemoSettingsImprintCheckbox));
	if (index)
		SET_FLAGS(mem.locationFD34, kMemoImprintFrameFlag);

	roll_number = StrAToI(FldGetTextPtr(GetObjectPtr(MemoSettingsNextRollField)));
	mem.locationFD3E = Short2BCD(roll_number);
	SwapShort(&mem.locationFD3E);
	
	/*
	 *	if the MemoSettingsMemoList is disabled, then we can't change
	 *	the values as there is either film or data in the camera
	 */
	 
	bytesOut = CtlEnabled(GetObjectPtr(MemoSettingsMemoPopTrigger)) ? 3 : 2;
	
	SendCommand(kWriteDataMode, 0x0000FD34, &(mem.locationFD34), 1);
	SendCommand(kWriteDataMode, 0x0000FD3E, &(mem.locationFD3E), bytesOut);

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;
		
	if (err) PostError(err);	
	return err;
}

/***********************************************************************
 *
 * FUNCTION:		CamOptionFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒoptionÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean CamOptionFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (CamOptionFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
		
		case fldEnterEvent:
			gRecordDirty = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case CameraOptionsSaveButton:
	   				CamOptionFormSave();
	   			case CameraOptionsCancelButton:
	 					FrmReturnToForm(0);
						handled = true;
	 					break;
	 				default:
	 					gRecordDirty = true;
	 					break;
	   		}
			break;
	
	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		CamOptionFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err CamOptionFormInit()
{
	struct		params {
		UChar	locationFD30;
		UChar	locationFD31;
		UChar	locationFD33;
		UChar	locationFD34;
		UChar	locationFD35;
	} mem;
	Err			err, endErr;
	Char		meter_text[] = "00";
	CharPtr		fstr;
	SWord		index;

	gRecordDirty = false;

	StartSession();
	SendCommand(kReadDataMode, 0x0000FD33, &(mem.locationFD33), 3);
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD30, &(mem.locationFD30), 2);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if (gCameraType == cameraN90s)
	{
		index = (mem.locationFD31 & kReverseCommandFlag) ? 1 : 0;
		SetPopupList(CameraOptionsCommandDialList,CameraOptionsCommandDialPopTrigger,index);
		
		fstr = GetStringTable(kFlashSyncSpeedTable, mem.locationFD30 / 4);
		if (fstr[0] != '?')
			SetFieldText(CameraOptionsFlashSpeedField, fstr);
		else
			err = kUnknownDataErr;
	}
	else if (gCameraType == cameraN90)
	{
		HideObject(CameraOptionsSecondsLabel);
		HideObject(CameraOptionsFlashSpeedLabel);
		HideObject(CameraOptionsFlashSpeedField);
		HideObject(CameraOptionsCommandDialLabel);
		HideObject(CameraOptionsCommandDialPopTrigger);
	}
	
	mem.locationFD35 &= 0x0F;
	Value2Index(kDualList, mem.locationFD35, &index);
	if (index < 0)
		err = kUnknownDataErr;
	else
		SetPopupList(CameraOptionsDualReleaseList,CameraOptionsDualReleasePopTrigger,index);

	StrIToA(meter_text, mem.locationFD33);
	SetFieldText(CameraOptionsMeterField, meter_text);
		
	index = (mem.locationFD34 & kLongExpTimeFlag) ? 1 : 0;
	SetPopupList(CameraOptionsLongExposureList,CameraOptionsLongExposurePopTrigger,index);
	
	index = (mem.locationFD34 & kDXPriorityFlag) ? 1 : 0;
	SetPopupList(CameraOptionsISOPriorityList,CameraOptionsISOPriorityPopTrigger,index);
	
ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;
	
	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		CamOptionFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err CamOptionFormSave()
{
	Err			err = 0, endErr;
	struct		params {
		UChar	locationFD30;
		UChar	locationFD31;
		
		UChar	locationFD33;
		UChar	locationFD34;
		UChar	locationFD35;
	} mem;
	SWord			index;
	UChar			value;

	if (gRecordDirty == false)
		return err;

	StartSession();
	SendCommand(kReadDataMode, 0x0000FD33, &(mem.locationFD33), 3);
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD30, &(mem.locationFD30), 2);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if (gCameraType == cameraN90s)
	{
		CLEAR_FLAGS(mem.locationFD31, kReverseCommandFlag);
		if (LstGetSelection(GetObjectPtr(CameraOptionsCommandDialList)))
			SET_FLAGS(mem.locationFD31, kReverseCommandFlag);
	
		index = GetStringIndex(kFlashSyncSpeedTable, FldGetTextPtr(GetObjectPtr(CameraOptionsFlashSpeedField)));
		if (index >= 0)
			mem.locationFD30 = index * 4;
		else
			err = kConvertDataErr;
	}		
		
	if (err) goto ERROR;
	
	CLEAR_FLAGS(mem.locationFD35, 0x0F);
	index = LstGetSelection(GetObjectPtr(CameraOptionsDualReleaseList));
	Index2Value(kDualList, index, &value);
	SET_FLAGS(mem.locationFD35, value);

	mem.locationFD33 = StrAToI(FldGetTextPtr(GetObjectPtr(CameraOptionsMeterField)));
	if (mem.locationFD33 < 4)
		mem.locationFD33 = 0;
	if (mem.locationFD33 > 0x40)
		mem.locationFD33 = 0x40;
		
	CLEAR_FLAGS(mem.locationFD34, kLongExpTimeFlag | kDXPriorityFlag);
	if (LstGetSelection(GetObjectPtr(CameraOptionsLongExposureList)))
		SET_FLAGS(mem.locationFD34, kLongExpTimeFlag);
	
	if (LstGetSelection(GetObjectPtr(CameraOptionsISOPriorityList)))
		SET_FLAGS(mem.locationFD34, kDXPriorityFlag);
	
	if (gCameraType == cameraN90s)
		SendCommand(kWriteDataMode, 0x0000FD30, &(mem.locationFD30), 2);
	
	SendCommand(kWriteDataMode, 0x0000FD33, &(mem.locationFD33), 3);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		ViewOptionFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the Òview optionsÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean ViewOptionFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (ViewOptionFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case ViewFinderOptionsSaveButton:
					ViewOptionFormSave();
	   			case ViewFinderOptionsCancelButton:
	 				FrmReturnToForm(0);
					handled = true;
	 				break;
	 			default:
	 				gRecordDirty = true;
					break;
	  			
	   		}
			break;
	
	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		ViewOptionFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err ViewOptionFormInit()
{
	Err			err, endErr;
	struct		params {
		UChar	locationFD31;
		UChar	locationFD34;
	} mem;

	gRecordDirty = false;
	
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD34, &(mem.locationFD34), 1);
	
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD31, &(mem.locationFD31), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;

	if (gCameraType == cameraN90s)
	{
		if (mem.locationFD31 & kCenterMeterDeltaFlag)
			CtlSetValue(GetObjectPtr(ViewFinderOptionsMatrixDeltaCheckbox), 1);
			
		if (mem.locationFD31 & kEasyCompensationFlag)
			CtlSetValue(GetObjectPtr(ViewFinderOptionsAperatureModeCheckbox), 1);
	}
	else if (gCameraType == cameraN90)
	{
		HideObject(ViewFinderOptionsMatrixDeltaCheckbox);
		HideObject(ViewFinderOptionsAperatureModeCheckbox);
		HideObject(ViewFinderOptionsAperatureModeLabel);
	}
		
	if (mem.locationFD34 & kFrameCounterFlag)
		CtlSetValue(GetObjectPtr(ViewFinderOptionsFrameCounterCheckbox), 1);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		ViewOptionFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err ViewOptionFormSave()
{
	Err			err = 0, endErr;
	struct		params {
		UChar	locationFD31;
		UChar	locationFD34;
	} mem;

	if (gRecordDirty == false)
		return err;

	StartSession();
	SendCommand(kReadDataMode, 0x0000FD34, &(mem.locationFD34), 1);
	
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD31, &(mem.locationFD31), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;
		
	if (gCameraType == cameraN90s)
	{
		CLEAR_FLAGS(mem.locationFD31, kCenterMeterDeltaFlag | kEasyCompensationFlag);
		if (CtlGetValue(GetObjectPtr(ViewFinderOptionsMatrixDeltaCheckbox)))
			SET_FLAGS(mem.locationFD31, kCenterMeterDeltaFlag);
		if (CtlGetValue(GetObjectPtr(ViewFinderOptionsAperatureModeCheckbox)))
			SET_FLAGS(mem.locationFD31, kEasyCompensationFlag);
	}
	
	CLEAR_FLAGS(mem.locationFD34, kFrameCounterFlag);
	if (CtlGetValue(GetObjectPtr(ViewFinderOptionsFrameCounterCheckbox)))
		SET_FLAGS(mem.locationFD34, kFrameCounterFlag);
	
	SendCommand(kWriteDataMode, 0x0000FD34, &(mem.locationFD34), 1);

	if (gCameraType == cameraN90s)
		SendCommand(kWriteDataMode, 0x0000FD31, &(mem.locationFD31), 1);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		AFOptionFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the Òaf optionsÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean AFOptionFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (AFOptionFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case AFOptionsSaveButton:
	   				AFOptionFormSave();
	   			case AFOptionsCancelButton:
	 				FrmReturnToForm(0);
					handled = true;
	 				break;
	 			default:
	 				gRecordDirty = true;
	 				break;
	  			
	   		}
	   		break;

	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		AFOptionFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err AFOptionFormInit()
{
	Err			err, endErr;
	struct		params {
		UChar	locationFD35;
		UChar	locationFD3A;
	} mem;

	gRecordDirty = false;
	
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD35, &(mem.locationFD35), 1);
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if (mem.locationFD35 & kAFContFocusFlag)
		CtlSetValue(GetObjectPtr(AFOptionsContinuousPriorityCheckbox), 1);
	
	if (mem.locationFD35 & kAFSingleReleaseFlag)
		CtlSetValue(GetObjectPtr(AFOptionsSinglePriorityCheckbox), 1);
		
	if (mem.locationFD35 & kAFContAFLockFlag)
		CtlSetValue(GetObjectPtr(AFOptionsLockAFCheckbox), 1);
		
	if (mem.locationFD3A & kAFSimulAFAEFlag)
		CtlSetValue(GetObjectPtr(AFOptionsSimulAFAECheckbox), 1);

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		AFOptionFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err AFOptionFormSave()
{
	Err			err = 0, endErr;
	struct		params {
		UChar	locationFD35;
		UChar	locationFD3A;
	} mem;

	if (gRecordDirty == false)
		return err;

	StartSession();
	SendCommand(kReadDataMode, 0x0000FD35, &(mem.locationFD35), 1);
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	CLEAR_FLAGS(mem.locationFD35, kAFContFocusFlag | kAFSingleReleaseFlag | kAFContAFLockFlag);
	CLEAR_FLAGS(mem.locationFD3A, kAFSimulAFAEFlag);

	if (CtlGetValue(GetObjectPtr(AFOptionsContinuousPriorityCheckbox)))
		SET_FLAGS(mem.locationFD35, kAFContFocusFlag);
	if (CtlGetValue(GetObjectPtr(AFOptionsSinglePriorityCheckbox)))
		SET_FLAGS(mem.locationFD35, kAFSingleReleaseFlag);
	if (CtlGetValue(GetObjectPtr(AFOptionsLockAFCheckbox)))
		SET_FLAGS(mem.locationFD35, kAFContAFLockFlag);
	if (CtlGetValue(GetObjectPtr(AFOptionsSimulAFAECheckbox)))
		SET_FLAGS(mem.locationFD3A, kAFSimulAFAEFlag);

	SendCommand(kWriteDataMode, 0x0000FD35, &(mem.locationFD35), 1);
	SendCommand(kWriteDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		BeepOptionFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the Òbeep optionsÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean BeepOptionFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (BeepOptionFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case BeepOptionsSaveButton:
	   				BeepOptionFormSave();
	   			case BeepOptionsCancelButton:
	 				FrmReturnToForm(0);
					handled = true;
	 				break;
	 			default:
	 				gRecordDirty = true;
	 				break;
	  			
	   		}
			break;
	
	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		BeepOptionFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err BeepOptionFormInit()
{
	struct		params {
		UChar	locationFD32;
		UChar	locationFD34;
	} mem;
	Err			err, endErr;
	Word		i;
	
	gRecordDirty = false;
	
	StartSession();
	
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD32, &(mem.locationFD32), 1);
	else if (gCameraType == cameraN90)
		SendCommand(kReadDataMode, 0x0000FD34, &(mem.locationFD34), 1);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if (gCameraType == cameraN90s)
	{
		for (i = BeepOptionsN90PictureBlurCheckbox; i <= BeepOptionsN90InFocusCheckbox; i++)
			HideObject(i);
		
		if (mem.locationFD32 & kBeepFilmErrorFlag)
			CtlSetValue(GetObjectPtr(BeepOptionsFilmErrorCheckbox), 1);
		
		if (mem.locationFD32 & kBeepTimerFlag)
			CtlSetValue(GetObjectPtr(BeepOptionsSelfTimerCheckbox), 1);
			
		if (mem.locationFD32 & kBeepExpErrorFlag)
			CtlSetValue(GetObjectPtr(BeepOptionsExposureErrorCheckbox), 1);
			
		if (mem.locationFD32 & kBeepFocusFlag)
			CtlSetValue(GetObjectPtr(BeepOptionsFocusCheckbox), 1);
	}
	else if (gCameraType == cameraN90)
	{
		for (i = BeepOptionsFilmErrorCheckbox; i <= BeepOptionsFocusCheckbox; i++)
			HideObject(i);
			
		if ((mem.locationFD34 & kN90BeepBlurFlag) == 0)
			CtlSetValue(GetObjectPtr(BeepOptionsN90PictureBlurCheckbox), 1);
			
		if ((mem.locationFD34 & kN90FocusFlag) == 0)
			CtlSetValue(GetObjectPtr(BeepOptionsN90InFocusCheckbox), 1);
	}
	

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;
	
	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		BeepOptionFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err BeepOptionFormSave()
{
	Err			err = 0, endErr;
	struct		params {
		UChar	locationFD32;
		UChar	locationFD34;
	} mem;

	if (gRecordDirty == false)
		return err;

	StartSession();
	
	if (gCameraType == cameraN90s)
		SendCommand(kReadDataMode, 0x0000FD32, &(mem.locationFD32), 1);
	else
		SendCommand(kReadDataMode, 0x0000FD34, &(mem.locationFD34), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if (gCameraType == cameraN90s)
	{
		CLEAR_FLAGS(mem.locationFD32, kBeepFilmErrorFlag | BeepOptionsSelfTimerCheckbox | 
												kBeepExpErrorFlag | kBeepFocusFlag);

		if (CtlGetValue(GetObjectPtr(BeepOptionsFilmErrorCheckbox)))
			SET_FLAGS(mem.locationFD32, kBeepFilmErrorFlag);
		if (CtlGetValue(GetObjectPtr(BeepOptionsSelfTimerCheckbox)))
			SET_FLAGS(mem.locationFD32, kBeepTimerFlag);
		if (CtlGetValue(GetObjectPtr(BeepOptionsExposureErrorCheckbox)))
			SET_FLAGS(mem.locationFD32, kBeepExpErrorFlag);
		if (CtlGetValue(GetObjectPtr(BeepOptionsFocusCheckbox)))
			SET_FLAGS(mem.locationFD32, kBeepFocusFlag);

		err = SendCommand(kWriteDataMode, 0x0000FD32, &(mem.locationFD32), 1);
	}
	else
	{
		SET_FLAGS(mem.locationFD34, kN90BeepBlurFlag | kN90FocusFlag);

		if (CtlGetValue(GetObjectPtr(BeepOptionsN90PictureBlurCheckbox)))
			SET_FLAGS(mem.locationFD34, kN90BeepBlurFlag);
		if (CtlGetValue(GetObjectPtr(BeepOptionsN90InFocusCheckbox)))
			SET_FLAGS(mem.locationFD34, kN90FocusFlag);

		err = SendCommand(kWriteDataMode, 0x0000FD34, &(mem.locationFD34), 1);
	}
ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		MergeFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the Òbeep optionsÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean MergeFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
			FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case MergeAppendButton:
	   				MergeFormSave();
	   			case MergeCancelButton:
	 				FrmGotoForm(MainForm);
					handled = true;
	 				break;
	  			
	   		}
			break;
	
	}
	return(handled);
}


/***********************************************************************
 *
 * FUNCTION:		MergeFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err MergeFormSave()
{
	Err					err = 0;
	Word				sourceRoll;
	Word				destRoll;
	CharPtr				textp;
	Word				sourceFrameStart,destFrameStart;
	UChar				numRecords;
	Word				sourceSize, destSize, moveSize;
	VoidHand			sourceH = NULL,destH = NULL;
	RollHeaderPackedPtr	sourceP, destP;
	SWord				i;
	UInt				sourceIndex = -1, destIndex = -1;

	textp = FldGetTextPtr(GetObjectPtr(MergeSourceField));
	if (textp == NULL) goto ERROR;
	sourceRoll = StrAToI(textp);

	textp = FldGetTextPtr(GetObjectPtr(MergeDestField));
	if (textp == NULL) goto ERROR;
	destRoll = StrAToI(textp);
	
	if (sourceRoll == destRoll)
		goto ERROR;

	numRecords = DmNumRecords(gMemoDatabaseRef);
	for (i = 0; i < numRecords; i++)
	{
		sourceH = DmQueryRecord(gMemoDatabaseRef, i);
		
		if (sourceH)
		{
			sourceP = (RollHeaderPackedPtr) MemHandleLock(sourceH);

			// Find the source and destination DB Indexes
			if (sourceP->rollNumber == sourceRoll)
				sourceIndex = i;
			else if (sourceP->rollNumber == destRoll)
				destIndex = i;
			MemHandleUnlock(sourceH);
		}
	}
	
	if ((sourceIndex == destIndex) || (sourceIndex == -1) || (destIndex == -1))
		goto ERROR;
		
	sourceH = DmQueryRecord(gMemoDatabaseRef, sourceIndex);
	destH = DmQueryRecord(gMemoDatabaseRef, destIndex);
	
	if (!sourceH || !destH)
		goto ERROR;

	sourceP = (RollHeaderPackedPtr) MemHandleLock(sourceH);
	destP = (RollHeaderPackedPtr) MemHandleLock(destH);
				
	//	OK to append a larger roll onto a larger roll
	if (destP->frameCount > sourceP->frameCount)
	{
		MemHandleUnlock(destH);
		MemHandleUnlock(sourceH);
		goto ERROR;
	}
	
	numRecords = sourceP->frameCount;
	sourceSize = GetRollSize(sourceP);
	sourceFrameStart = GetFrameOffset(sourceP, destP->frameCount);
	destSize = GetRollSize(destP);
	destFrameStart = GetFrameOffset(destP, destP->frameCount);
	moveSize = sourceSize - sourceFrameStart;

	MemHandleUnlock(destH);
	
	//	Copy the remaining frames from the source roll
	err = DmInsertBytes(gMemoDatabaseRef, destIndex, destFrameStart, (CharPtr) sourceP + sourceFrameStart, moveSize, &destSize);
	MemHandleUnlock(sourceH);
	
	if (err) goto ERROR;
	
	//	Increment the destination's frameCount
	destH = DmGetRecord(gMemoDatabaseRef,destIndex);
	if (destH)
	{
		destP = (RollHeaderPackedPtr) MemHandleLock(destH);
		err = DmWrite(destP, offsetof(RollHeaderPackedRec,frameCount), &numRecords, sizeof(numRecords));
		MemHandleUnlock(destH);
		DmReleaseRecord(gMemoDatabaseRef, destIndex, true);
	}
	
ERROR:
	
DONE:
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		DeleteFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the Òbeep optionsÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean DeleteFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
			FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case DeleteDeleteButton:
	   				DeleteFormSave();
	   			case DeleteCancelButton:
	 				FrmGotoForm(MainForm);
					handled = true;
	 				break;
	  			
	   		}
			break;
	
	}
	return(handled);
}


/***********************************************************************
 *
 * FUNCTION:		DeleteFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err DeleteFormSave()
{
	Err					err = 0;
	Word				sourceRoll;
	Word				destRoll;
	CharPtr				textp;
	Word				rollNumber, numRecords;
	VoidHand			recordH = NULL;
	RollHeaderPackedPtr	recordP;
	SWord				i;

	textp = FldGetTextPtr(GetObjectPtr(DeleteSourceField));
	if ((textp == NULL) || (StrLen(textp) == 0)) goto ERROR;
	sourceRoll = StrAToI(textp);
	
	textp = FldGetTextPtr(GetObjectPtr(DeleteDestField));
	if ((textp == NULL) || (StrLen(textp) == 0))
		destRoll = sourceRoll;
	else
		destRoll = StrAToI(textp);
		
	if (destRoll < sourceRoll)
		goto ERROR;
		
	numRecords = DmNumRecords(gMemoDatabaseRef);
	for (i = numRecords; i >= 0; i--)
	{
		recordH = DmQueryRecord(gMemoDatabaseRef, i);
		if (recordH)
		{
			recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);
			
			rollNumber = recordP->rollNumber;
			
			MemHandleUnlock(recordH);
			
			if ((rollNumber >= sourceRoll) && (rollNumber <= destRoll))
				DmRemoveRecord(gMemoDatabaseRef, i);
		}
	}
	
	
ERROR:
	return(err);
}

