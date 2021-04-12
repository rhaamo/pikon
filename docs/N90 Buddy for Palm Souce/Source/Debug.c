/*
	File:		Debug.c

	Contains:	Debugging routines

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

         <5>     2/16/99    ksh     Add support for logging to memo pad
         <4>     6/30/98    ksh     Revise serial code
         <3>     2/15/98    KSH     Update ResetMemoHolder to handle N90 saved setting
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/



#include "N90sBuddy.h"

#ifdef DEBUGBUILD

MemoDebug	gMemoDebug;

const char testdata[] = { 0x14, 0x00, 
0x38, 0x24, 0xE0, 0x5C, 0x00, 0x00, 
0x25, 0x24, 0xE1, 0x5C, 0x00, 0x00, 
0x38, 0x56, 0xE2, 0x5C, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x5C, 0x00, 0x00, 
0x2D, 0x18, 0xE8, 0x5C, 0x00, 0x00, 
0x2D, 0x18, 0xEE, 0x5C, 0x00, 0x00, 
0x30, 0x12, 0xED, 0x5C, 0x00, 0x00, 
0x2B, 0x2A, 0xEC, 0x5C, 0x00, 0x00, 
0x25, 0x24, 0xEB, 0x5C, 0x00, 0x00, 
0x20, 0x2A, 0xEA, 0x5C, 0x00, 0x00, 
0x2C, 0x18, 0xE9, 0x5C, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x5C, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x58, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x51, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x4E, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x4B, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x48, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x48, 0x00, 0x00, 
0x2F, 0x18, 0xE3, 0x44, 0x00, 0x00, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0x12, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0x10, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0x02, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0x00, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0xFA, 
0x30, 0x18, 0x23, 0x5C, 0x00, 0xFC, 
0xFF, 0x0C};



/***********************************************************************
 *
 * FUNCTION:     DebugStartLog, DebugStopLog
 *
 * DESCRIPTION:  Starts logging to memo holder
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/

void DebugStartLog(void)
{
	Char			logString[80];
	DateTimeType	dateTimeRec;
	#define kMemoHolderString	"NBdy "
	
	MemSet(&gMemoDebug, sizeof(MemoDebug), 0);
	
	gMemoDebug.memoRef = DmOpenDatabaseByTypeCreator('DATA', sysFileCMemo, dmModeReadWrite);
	if (gMemoDebug.memoRef)
	{
		gMemoDebug.memoIndex = DmNumRecords(gMemoDebug.memoRef);
		gMemoDebug.memoRecH = DmNewRecord (gMemoDebug.memoRef, &(gMemoDebug.memoIndex), 4096);
		if (gMemoDebug.memoRecH)
		{
			gMemoDebug.memoRecP = MemHandleLock(gMemoDebug.memoRecH);
			
			DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, kMemoHolderString);
			gMemoDebug.memoOffset += StrLen(kMemoHolderString);

			TimSecondsToDateTime(TimGetSeconds(), &dateTimeRec);
			
			DateToAscii(dateTimeRec.month, dateTimeRec.day, dateTimeRec.year, dfYMDWithDots, logString);
			DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, logString);
			gMemoDebug.memoOffset += StrLen(logString);

			DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, " ");
			gMemoDebug.memoOffset++;

			TimeToAscii(dateTimeRec.hour, dateTimeRec.minute, tfDot24h, logString);
			DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, logString);
			gMemoDebug.memoOffset += StrLen(logString);

			DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, "\n");
			gMemoDebug.memoOffset++;
		}
	}
}

void DebugStopLog(void)
{
	if (gMemoDebug.memoRef)
	{
		if (gMemoDebug.memoIndex)
			DmReleaseRecord(gMemoDebug.memoRef, gMemoDebug.memoIndex, true);
		DmCloseDatabase(gMemoDebug.memoRef);
	}
	MemSet(&gMemoDebug, sizeof(MemoDebug), 0);
}


Boolean DebugFormHandleEvent(EventPtr event)
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
   			case debugPeekBtn:
   				Peek();
   				handled = true;
   				break;
   			case debugPokeBtn:
   				Poke();
   				handled = true;
   				break;
   			case debugExitBtn:
 				FrmReturnToForm(0);
				handled = true;
 				break;
   		}
			break;
	
		}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:     AddSampleRolls
 *
 * DESCRIPTION:  Downloads all data from the Nikon Memo holder
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err AddSampleRolls()
{
	Err			err = 0;
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
	VoidPtr		recordP;
	Char			captionName[] = "";
	RollHeaderPackedRec	rollHeader;
	Word			byteCount;
	Word			rollCount;
	static Word	rollNumber = 0;

	bytesPerFrame = 6;
	byteCount = 0x9A;
	
	nFrames = (byteCount - 4) / bytesPerFrame;
	cameraSize = StrLen(gCameraName) + 1;
	captionSize = StrLen(captionName) + 1;
	dbSize = (nFrames * (kFrameHeaderPackedSize + 1)) + 
				kRollHeaderPackedSize +
				cameraSize + captionSize;

	for (rollCount = 0; rollCount < 5; rollCount++)
	{
		ddata = (UChar *) testdata;
		rollH = DmNewHandle(gMemoDatabaseRef, dbSize);
		if (rollH == NULL)
		{
			err = memErrNotEnoughSpace;
			goto ERROR;
		}
		
		rollHeader.timeDate = TimGetSeconds();
		rollHeader.rollNumber = rollNumber;
		rollHeader.iso = ddata[byteCount-1];
		rollHeader.storageLevel = bytesPerFrame/2 - 1;
		rollHeader.frameCount = nFrames;

		offset = 0;
		
		recordP = (RollHeaderPackedPtr) MemHandleLock(rollH);

		err = DmWriteCheck(recordP, offset, dbSize);
		if (err)
			goto ERROR;
		
		DmWrite(recordP, offset, &rollHeader, kRollHeaderPackedSize);
		offset += kRollHeaderPackedSize;
		
		DmWrite(recordP, offset, gCameraName, cameraSize);
		offset += cameraSize;

		DmWrite(recordP, offset, captionName, captionSize);
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
		
		MemHandleUnlock(rollH);
		id = rollNumber;

		DmAttachRecord(gMemoDatabaseRef, &id, rollH, NULL);
		DmReleaseRecord(gMemoDatabaseRef, id, true);

		rollNumber++;
	}	
		
ERROR:

	if (err)
	{
		if (rollH) MemHandleFree(rollH);
	}

	if (err) PostError(err);
	return err;
}

void DeleteAllRolls()
{
	UInt	i;
	UInt	numRecords;
	Err	err;
	
	numRecords = DmNumRecords(gMemoDatabaseRef);
	for (i = 0; i < numRecords; i++)
		err = DmRemoveRecord(gMemoDatabaseRef, 0);
	
}


/***********************************************************************
 *
 * FUNCTION:     ResetMemoHolder
 *
 * DESCRIPTION:  Resets the memo holder to the last successful download point
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err ResetMemoHolder()
{
	BuddyPreferencesType prefs;
	Err err = 0;
	
	GetPreferences(&prefs);
	
	if (prefs.lastDownloadMark != 0)
	{
		StartSession();

		if (gCameraType == cameraN90s)
			SendCommand(kWriteDataMode,0x0000FD44,&(prefs.lastDownloadMark),2);
		else if (gCameraType == cameraN90)
		{
			UChar locationFD42 = prefs.lastDownloadMark;
			
			SendCommand(kWriteDataMode,0x0000FD42,&locationFD42,1);
		}
		
		err = EndSession();

		if (err)
			PostError(err);
			
	}
	
	return(err);
}

Err Peek()
{
	Err		err = 0;
	UChar		input[100];
	Char		output[400];
	Long		address;
	Word		byteCount;
	CharPtr	addrFld,byteFld;
	
	addrFld = FldGetTextPtr(GetObjectPtr(debugAddressFld));
	byteFld = FldGetTextPtr(GetObjectPtr(debugBytesFld));
	
	if (addrFld && byteFld)
	{
		address = TextToNum(addrFld);
		byteCount = TextToNum(byteFld);
		StartSession();
		SendCommand(kReadDataMode, address, input, byteCount);
		err = EndSession();
		
		if (err) goto ERROR;
		
		DataToHex(input,byteCount,output, NULL);
		SetFieldText(debugDataFld,output);
	}
	
ERROR:	
	if (err) PostError(err);	
	return(err);
}

Err Poke()
{
	Err		err = 0;
	UChar		output[100];
	Long		address;
	Word		byteCount;
	CharPtr	addrFld,dataFld;
	
	addrFld = FldGetTextPtr(GetObjectPtr(debugAddressFld));
	dataFld = FldGetTextPtr(GetObjectPtr(debugDataFld));
	if (addrFld && dataFld)
	{
		address = TextToNum(addrFld);
		HexToData(dataFld, output, &byteCount);
	
		StartSession();
		SendCommand(kWriteDataMode, address, output, byteCount);
		err = EndSession();
	}
	
ERROR:	
	if (err) PostError(err);	
	return(err);
}

void HexToData(CharPtr input, UChar *output, Word *byteCount)
{
	UChar	*p;
	UChar	byte;
	Char	c;
	Boolean hiNib = true;
	
	p = output;
	StrToLower(input,input);
	while ((c = *input++) != 0)
	{
		if ((c >= '0') && (c <= '9'))
			c -= '0';
		else if ((c >= 'a') && (c <= 'f'))
			c -= ('a' - 0x0A);
		else
			continue;
			
		if (hiNib)
			byte = c << 4;
		else
		{
			byte += c;
			*p++ = byte;
		}
		hiNib = !hiNib;
	}
	
	*byteCount = (p - output);
}

void DataToHex(UChar *input, Word count, CharPtr output, CharPtr prefix)
{
	UChar	byte;
	UChar nibble;
	Word	wrap = 1;
	
	if (prefix)
	{
		StrCopy(output, prefix);
		output += StrLen(prefix);
	}
	
	while (count--)
	{
		byte = *input++;
		nibble = byte >> 4;
		*output++ = nibble + ((nibble < 0x0A) ? '0' : 'A' - 0x0A);
		nibble = byte & 0x0F;	
		*output++ = nibble + ((nibble < 0x0A) ? '0' : 'A' - 0x0A);
		if ((wrap % 8) == 0)
		{
			*output++ = 0x0A;
			if (prefix && (count > 1))
			{
				StrCopy(output, prefix);
				output += StrLen(prefix);
			}
		}
		else
			*output++ = ' ';
		wrap++;
	}
	output[-1] = '\0';
}

Long TextToNum(CharPtr text)
{
	Char	c;
	Long	num;
	
	StrToLower(text,text);
	num = 0;
	while ((c = *text++) != 0)
	{
		if ((c >= '0') && (c <= '9'))
			num = (num << 4) + (c - '0');
		else if ((c >= 'a') && (c <= 'f'))
			num = (num << 4) + (c - 'a') + 0x0A;
	}

	return(num);	
}

void DebugToggleSerial(void)
{	
	gSerialEnabled = !gSerialEnabled;
	FixDebugMenu();
}

void DebugToggleBaud(void)
{
	gBaudChangeEnabled = !gBaudChangeEnabled;
	FixDebugMenu();
}

void FixDebugMenu(void)
{
	MenuBarPtr 			mbp;
	MenuPullDownPtr	mp;
	CharPtr				strPtr;

	mbp = MenuGetActiveMenu();
	if (mbp)
	{
		mp = &(mbp->menus[mbp->numMenus - 1]);
		if (mp->items[0].id == DebugMenu)
		{
			strPtr = (mp->items[DebugDisableSerial - DebugAddSampleRolls]).itemStr;
			GetTemplateString(strPtr, gSerialEnabled ? debugDisableString : debugEnableString);

			strPtr = (mp->items[DebugDisableBaudChange - DebugAddSampleRolls]).itemStr;
			GetTemplateString(strPtr, gBaudChangeEnabled ? debugDisableBaudString : debugEnableBaudString);
		}
		if (mp->items[0].id == CDebugMenu)
		{
			strPtr = (mp->items[CDebugDisableSerial - CDebugPeekPoke]).itemStr;
			GetTemplateString(strPtr, gSerialEnabled ? debugDisableString : debugEnableString);

			strPtr = (mp->items[CDebugDisableBaudChange - CDebugPeekPoke]).itemStr;
			GetTemplateString(strPtr, gBaudChangeEnabled ? debugDisableBaudString : debugEnableBaudString);
		}
	}
}

#endif
