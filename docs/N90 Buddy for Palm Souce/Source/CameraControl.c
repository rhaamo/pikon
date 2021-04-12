/*
	File:		CameraControl.c

	Contains:	Functions for camera control/camera control forms

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

       <11+>      8/6/99    ksh     Add trap focus feature
        <11>     2/16/99    ksh     Fix SysTicksPerSecond call under OS 1.x
        <10>     6/30/98    ksh     Remove extra StartSession calls; Cleanup serial code
         <9>     4/13/98    ksh     Fix compiler warning
         <8>     3/25/98    ksh     Update EndSession to return an error
         <7>     3/25/98    ksh     Fix camera mode (FD26/FD27)
         <6>     3/23/98    ksh     Fix exposure compensation to compensate based on ISO
         <5>     3/14/98    ksh     Add AutoSequence icon
         <4>     3/14/98    ksh     Remove kFlashReadyFlag and only use kFlashAttachedFlag
         <3>     2/18/98    ksh     Update for 1.1b1; all dialogs now updated for N90
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


#include "N90sBuddy.h"

const UChar kBracketList[] = {8, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C};

#define kVariProgramStartIndex 0x04

typedef struct CameraControlGlobals {
	UChar	valid;
	UChar	locationFD21;
	
	UChar	locationFD25;
	UChar	locationFD26;
	UChar	locationFD27;
	UChar	locationFD28;
	UChar	locationFD29;
	UChar	locationFD2A;
	UChar	locationFD2B;
	UChar	locationFD2C;
	UChar	locationFD2D;
	
	UChar	locationFD3A;
	UChar	locationFD3B;
	UChar	locationFD3C;
	UChar	locationFD3D;
	
	UChar	locationFD89;
	
	UChar	locationFD8E;
	UChar	locationFD8F;
	UChar	locationFD90;
	
	UChar	locationFD9D;
	
	UChar	locationFE20;
	UChar	locationFE21;
	UChar	locationFE22;
	UChar	locationFE23;
	UChar	locationFE24;
	UChar	locationFE25;
	UChar	locationFE26;
	UChar	locationFE27;
	UChar	locationFE28;
	UChar	locationFE29;
	UChar	locationFE2A;
	UChar	locationFE2B;
	UChar	locationFE2C;
	UChar	locationFE2D;
	UChar	locationFE2E;
	UChar	locationFE2F;
	UChar	locationFE30;
	UChar	locationFE31;
	UChar	locationFE32;
	UChar	locationFE33;
	UChar	locationFE34;
	
	UChar	locationFE3A;
	
	UChar	locationFE4F;
	UChar	locationFE50;
	UChar	locationFE51;
} CameraControlGlobals;

CameraControlGlobals *sCCG = NULL;

/***********************************************************************
 *
 * FUNCTION:		CameraFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒcameraÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean CameraFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;
	
	switch (event->eType)
	{  	
 		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (CameraFormInit())
  				FrmGotoForm(MainForm);
			else
			{
				FrmDrawForm(FrmGetActiveForm());
 				FrmUpdateForm(CameraControlForm, frmUpdateEvent);
			}	
			handled = true;
			break;
		
		case frmCloseEvent:
			CameraFormClose();
			break;

		case frmUpdateEvent:
			CameraUpdateInfo();
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
				case CameraControlDoneButton:
					FrmGotoForm(MainForm);
					handled = true;
					break;
				case CameraControlUpdateButton:
					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
					handled = true;
					break;
				case CameraControlFocusButton:
					CameraFocus();
					handled = true;
					break;
				case CameraControlFireButton:
					CameraShutter();
					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
					handled = true;
					break;
				case CameraControlSetButton:
					CameraSetSettings();
					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
					handled = true;
	   		}
			break;

		case ctlRepeatEvent:
			switch (event->data.ctlRepeat.controlID)
			{
					
				case CameraControlShutterIncRepeating:
					if (gCameraType == cameraN90s)
					{
						if (sCCG->locationFD25 < kCameraShutterMax)
							sCCG->locationFD25++;
						SetTemplateLabel(CameraControlShutterSpeedLabel, ShutterSpeedTemplateString, 
										GetStringTable(kCameraShutterTable, sCCG->locationFD25));
					}
					else if (gCameraType == cameraN90)
					{
						if (sCCG->locationFD25 < kCameraN90ShutterMax)
							sCCG->locationFD25++;
						SetTemplateLabel(CameraControlShutterSpeedLabel, ShutterSpeedTemplateString, 
										GetStringTable(kCameraN90ShutterTable, sCCG->locationFD25));
					}
					break;	
				case CameraControlShutterDecRepeating:
					if (sCCG->locationFD25 > kCameraShutterMin)
						sCCG->locationFD25--;
					if (gCameraType == cameraN90s)
						SetTemplateLabel(CameraControlShutterSpeedLabel, ShutterSpeedTemplateString, 
									GetStringTable(kCameraShutterTable, sCCG->locationFD25));
					else if (gCameraType == cameraN90)
						SetTemplateLabel(CameraControlShutterSpeedLabel, ShutterSpeedTemplateString, 
									GetStringTable(kCameraN90ShutterTable, sCCG->locationFD25));
					break;
					
				case CameraControlExpCompIncRepeating:
					if (sCCG->locationFD2D > kCameraExpCompMin)
						sCCG->locationFD2D--;
					CopyLabel(CameraControlExpCompLabel, GetStringTable(kCameraExpCompTable, sCCG->locationFD2D));
					break;
				case CameraControlExpCompDecRepeating:
					if (sCCG->locationFD2D < kCameraExpCompMax)
						sCCG->locationFD2D++;
					CopyLabel(CameraControlExpCompLabel, GetStringTable(kCameraExpCompTable, sCCG->locationFD2D));
					break;	
				case CameraControlFlashCompIncRepeating:
					if (REG2FLASH_COMP(sCCG->locationFD3B) > 0)
						sCCG->locationFD3B = FLASH_COMP2REG(REG2FLASH_COMP(sCCG->locationFD3B) - 1);
					SetTemplateLabel(CameraControlFlashCompLabel, ExpCompEffTemplateString,
						GetStringTable(kCameraFlashCompTable, REG2FLASH_COMP(sCCG->locationFD3B)),
						GetStringTable(kExpCompTable, ((sCCG->locationFD8F - 0x5A) - (sCCG->locationFD90 * 2))));
					break;
				case CameraControlFlashCompDecRepeating:
					if (REG2FLASH_COMP(sCCG->locationFD3B) < kCameraFlashCompMax)
						sCCG->locationFD3B = FLASH_COMP2REG(REG2FLASH_COMP(sCCG->locationFD3B) + 1);
					SetTemplateLabel(CameraControlFlashCompLabel, ExpCompEffTemplateString,
						GetStringTable(kCameraFlashCompTable, REG2FLASH_COMP(sCCG->locationFD3B)),
						GetStringTable(kExpCompTable, ((sCCG->locationFD8F - 0x5A) - (sCCG->locationFD90 * 2))));
				break;
			}
			break;
				
		case menuEvent:
			handled = CameraHandleMenu(event->data.menu.itemID);
			break;
	
		}
	return(handled);
}


/***********************************************************************
 *
 * FUNCTION:		CameraHandleMenu
 *
 * DESCRIPTION:	Handles the standard menu items
 *
 * PARAMETERS:		itemID		- the menu ID
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/

Boolean CameraHandleMenu(Word item)
{
	Boolean	handled = false;
	
	// First clear the menu status from the display.
	MenuEraseStatus(NULL);

	switch(item)
	{

		case CreativeAutoSequenceShooting:
			FrmPopupForm(AutoSequenceForm);
			handled = true;
			break;
		case CreativeAutoBracketing:
			FrmPopupForm(BracketingForm);
			handled = true;
			break;
		case CreativeFreezeFocus:
			FrmPopupForm(FreezeFocusForm);
			handled = true;
			break;
		case CreativeMultipleExposure:
			FrmPopupForm(MultipleExposureForm);
			handled = true;
			break;
	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		CamerFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err CameraFormInit()
{
	Err			err = 0;
	
	sCCG = (CameraControlGlobals *) MemPtrNew(sizeof(CameraControlGlobals));
	MemSet(sCCG, sizeof(CameraControlGlobals), 0);
	
	if (sCCG == NULL) err = memErrNotEnoughSpace;
	
	gRecordDirty = false;
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		CameraFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err CameraFormClose()
{
	Err			err = 0;

	if (sCCG != NULL)
	{
		MemPtrFree(sCCG);
		sCCG = NULL;
	}
	return (err);	
}


/***********************************************************************
 *
 * FUNCTION:		CameraUpdateInfo
 *
 * DESCRIPTION:	Reads in the current camera settings and updates form
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/

Err CameraUpdateInfo()
{
	Err		err = 0, endErr;
	char	frameStr[kRollDigits];
	UChar	flashFlag;
	
	if (sCCG == NULL) return (err);
	
	sCCG->valid = false;
	
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD21, &(sCCG->locationFD21), 1);
	SendCommand(kReadDataMode, 0x0000FD25, &(sCCG->locationFD25), 9);
	SendCommand(kReadDataMode, 0x0000FD3A, &(sCCG->locationFD3A), 4);
	SendCommand(kReadDataMode, 0x0000FD89, &(sCCG->locationFD89), 1);
	SendCommand(kReadDataMode, 0x0000FD8E, &(sCCG->locationFD8E), 3);
	SendCommand(kReadDataMode, 0x0000FE20, &(sCCG->locationFE20), 21);
	SendCommand(kReadDataMode, 0x0000FE3A, &(sCCG->locationFE3A), 1);
	SendCommand(kReadDataMode, 0x0000FE4F, &(sCCG->locationFE4F), 3);
	SendCommand(kReadDataMode, 0x0000FD9D, &(sCCG->locationFD9D), 1);

	if ((err = GetSessionError()) != 0) goto ERROR;

	sCCG->valid = true;
	
	if (sCCG->locationFD21 == 0)
		CopyLabel(CameraControlFrameLabel, "NO FILM");
	else
		SetTemplateLabel(CameraControlFrameLabel, FrameTemplateString, 
							StrIToA(frameStr,sCCG->locationFD21 - 1));

	if (gCameraType == cameraN90s)
	{
		SetTemplateLabel(CameraControlISOLabel, ISOTemplateString, 
					GetStringTable(kISOTable, sCCG->locationFD90));
		SetLensLabel(CameraControlLensLabel, sCCG->locationFE33, sCCG->locationFE2F,
					sCCG->locationFE30, sCCG->locationFE31, sCCG->locationFE32);
			
		SetTemplateLabel(CameraControlFocalLengthLabel, FocalLengthTempateString, 
			GetStringTable(kFocalTable, sCCG->locationFE2E));
	}
	else if (gCameraType == cameraN90)
	{
		SetTemplateLabel(CameraControlISOLabel, ISOTemplateString, 
					GetStringTable(kISOTable, sCCG->locationFD9D));
		SetLensLabel(CameraControlLensLabel, sCCG->locationFE32, sCCG->locationFE2C,
					sCCG->locationFE2D, sCCG->locationFE2E, sCCG->locationFE2F);
		SetTemplateLabel(CameraControlFocalLengthLabel, FocalLengthTempateString, 
			GetStringTable(kFocalTable, sCCG->locationFE2B));
	}
		
	SetTemplateLabel(CameraControlShutterSpeedLabel, ShutterSpeedTemplateString, 
		GetStringTable(kShutterSpeedTable, sCCG->locationFE50));

	if (sCCG->locationFE51 > GetStringTableMax(kApertureTable))
		CopyLabel(CameraControlApertureLabel, "f/EE");
	else
		SetTemplateLabel(CameraControlApertureLabel, ApertureTemplateString, 
			GetStringTable(kApertureTable, sCCG->locationFE51));

	if (sCCG->locationFD26 < kVariProgramStartIndex)
		SetPopupList(CameraControlExposureModeList, CameraControlExposureModePopTrigger, sCCG->locationFD26);
	else
		SetPopupList(CameraControlExposureModeList, CameraControlExposureModePopTrigger, sCCG->locationFD27 + kVariProgramStartIndex);

	SetPopupList(CameraControlMeteringList, CameraControlMeteringPopTrigger, sCCG->locationFD28);
	SetPopupList(CameraControlFocusList, CameraControlFocusPopTrigger, sCCG->locationFD2B);
	SetPopupList(CameraControlMotorDriveList, CameraControlMotorDrivePopTrigger, sCCG->locationFD29);

	if (gCameraType == cameraN90s)
		SetTemplateLabel(CameraControlExpCompLabel, ExpCompEffTemplateString,
			GetStringTable(kCameraExpCompTable, sCCG->locationFD2D),
			GetStringTable(kExpCompTable, ((sCCG->locationFD8F - 0x5A) - (sCCG->locationFD90 * 2))));
	else if (gCameraType == cameraN90)
		CopyLabel(CameraControlExpCompLabel, GetStringTable(kCameraExpCompTable, sCCG->locationFD2D));
	
	//	HANDLE MULTIPLE EXPOSURE/AUTO BRACKETING
	
	HideObject(CameraControlMultExpBitMap);
	HideObject(CameraControlBracketBitMap);
	HideObject(CameraControlAutoSequenceBitMap);
	HideObject(CameraControlFreezeFocusBitMap);
	HideObject(CameraControlMBLabel);
	if (sCCG->locationFD3D > 0)
	{
		StrIToA(frameStr, sCCG->locationFD3D);
		if (sCCG->locationFD3C & kMultiExpFlag)
		{
			SetTemplateLabel(CameraControlMBLabel, MultExposureTemplateString, frameStr);
			ShowObject(CameraControlMultExpBitMap);
			ShowObject(CameraControlMBLabel);
		}
		else if (sCCG->locationFD3C & (kCameraBracketFlag + kFlashBracketFlag))
		{
			SetTemplateLabel(CameraControlMBLabel, BracketTemplateString, frameStr, 
				GetStringTable(kCameraBracketTable, sCCG->locationFD3C & 0x0F)); 
			ShowObject(CameraControlBracketBitMap);
			ShowObject(CameraControlMBLabel);
		}
		else if (sCCG->locationFD3C & kAutoSequenceFlag)
		{
			SetTemplateLabel(CameraControlMBLabel, MultExposureTemplateString, frameStr);
			ShowObject(CameraControlAutoSequenceBitMap);
			ShowObject(CameraControlMBLabel);
		}
		
		if (sCCG->locationFD3A & kFreezeFocusFlag)
			ShowObject(CameraControlFreezeFocusBitMap);

	}

	// HANDLE FLASH

	HideObject(CameraControlFlashBitMap);
	HideObject(CameraControlFlashSyncPopTrigger);
	HideObject(CameraControlFlashCompLabel);
	HideObject(CameraControlFlashCompIncBitMap);
	HideObject(CameraControlFlashCompDecBitMap);
	HideObject(CameraControlFlashCompIncRepeating);
	HideObject(CameraControlFlashCompDecRepeating);
	flashFlag = (gCameraType == cameraN90s) ? sCCG->locationFD89 : sCCG->locationFE34;
	if (flashFlag & kFlashAttachedFlag)
	{
		SetPopupList(CameraControlFlashSyncList, CameraControlFlashSyncPopTrigger, sCCG->locationFD2A);

		if (gCameraType == cameraN90s)
			SetTemplateLabel(CameraControlFlashCompLabel, ExpCompEffTemplateString,
				GetStringTable(kExpCompTable, sCCG->locationFE4F),
				GetStringTable(kExpCompTable, ((sCCG->locationFD8F - 0x5A) - (sCCG->locationFD90 * 2))));
		else if (gCameraType == cameraN90)
			CopyLabel(CameraControlFlashCompLabel, 
				GetStringTable(kExpCompTable, sCCG->locationFE3A));

		ShowObject(CameraControlFlashBitMap);
		ShowObject(CameraControlFlashSyncPopTrigger);
		ShowObject(CameraControlFlashCompLabel);
		ShowObject(CameraControlFlashCompIncBitMap);
		ShowObject(CameraControlFlashCompDecBitMap);
		ShowObject(CameraControlFlashCompIncRepeating);
		ShowObject(CameraControlFlashCompDecRepeating);
	}
	
ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		CameraFocus
 *
 * DESCRIPTION:	Sends the focus command
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/

Err CameraFocus()
{
	Err			err = 0,endErr;
	UChar		focus = 0x08;
	
	StartSession();
	SendCommand(kReadDataMode, 0x0000FD39, &focus, 1);
	focus = 0x08;	SendCommand(kWriteDataMode, 0x0000FD39, &focus, 1); 
	SendCommand(kFocusMode, 0, 0, 0);
	SysTaskDelay(300L * (gPalmOS2 ? SysTicksPerSecond() : 100) / 1000L);
	SendCommand(kReadDataMode, 0x0000FD39, &focus, 1); 
	focus = 0x00;	SendCommand(kWriteDataMode, 0x0000FD39, &focus, 1);

	
ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		CameraShutter
 *
 * DESCRIPTION:	Fires the shutter
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/

Err CameraShutter()
{
	Err			err = 0,endErr;
	
#if 1
	StartSession();
	SendCommand(kShutterMode, 0, 0, 0);
	
ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;
#else
	FireShutterSession()
#endif

	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		CameraSetSettings
 *
 * DESCRIPTION:		Sets the currently edited settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/

Err CameraSetSettings(void)
{
	Err			err = 0, endErr;
	UChar		item;
	UChar		value;
	
	if (sCCG == NULL) return (err);
	if (sCCG->valid == false) return (err);
	
	item = LstGetSelection(GetObjectPtr(CameraControlExposureModeList));

	StartSession();

	if (item < kVariProgramStartIndex)
		SendCommand(kWriteDataMode, 0x0000FD26, &item, 1);
	else
	{
		value = kVariProgramIndex;
		SendCommand(kWriteDataMode, 0x0000FD26, &value, 1);
		value = (item - kVariProgramStartIndex);
		SendCommand(kWriteDataMode, 0x0000FD27, &value, 1);
	}
	
	item = LstGetSelection(GetObjectPtr(CameraControlMeteringList));	
	SendCommand(kWriteDataMode, 0x0000FD28, &item, 1);					// Metering
	
	item = LstGetSelection(GetObjectPtr(CameraControlFocusList));	
	SendCommand(kWriteDataMode, 0x0000FD2B, &item, 1);					// Focus
	
	item = LstGetSelection(GetObjectPtr(CameraControlMotorDriveList));
	SendCommand(kWriteDataMode, 0x0000FD29, &item, 1);					// Motor drive
	
	SendCommand(kWriteDataMode, 0x0000FD25, &(sCCG->locationFD25), 1);	// Shutter Speed
	
	SendCommand(kWriteDataMode, 0x0000FD2D, &(sCCG->locationFD2D), 1);	// Exp Comp

	item = LstGetSelection(GetObjectPtr(CameraControlFlashSyncList));
	SendCommand(kWriteDataMode, 0x0000FD2A, &item, 1);
	
ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		SetLensLabel
 *
 * DESCRIPTION:		Sets the Lens label depending on focal/aperture values
 *
 * PARAMETERS:		labelID, lensId, focalMin, focalMax, apMin, apMax
 *
 * RETURNED:		NONE
 *
 ***********************************************************************/

void SetLensLabel(Word labelID, UChar lensId, UChar focalMin, 
						UChar focalMax, UChar apMin, UChar apMax)
{
	char		lensStr[kRollDigits];
	CharPtr	focalMinStr, focalMaxStr, apMinStr, apMaxStr;

	StrIToA(lensStr, lensId);
	focalMinStr = GetStringTable(kFocalTable, focalMin);
	focalMaxStr = GetStringTable(kFocalTable, focalMax);
	apMinStr = GetStringTable(kCameraApertureTable, apMin);
	apMaxStr = GetStringTable(kCameraApertureTable, apMax);
	
	if (focalMin == focalMax)
		SetTemplateLabel(labelID, LATemplateString, lensStr, focalMinStr, apMinStr);
	else if (apMin == apMax)
		SetTemplateLabel(labelID, LLATemplateString, lensStr, 
			focalMinStr, focalMaxStr, apMinStr);
	else
		SetTemplateLabel(labelID, LLAATemplateString, lensStr, 
							focalMinStr, focalMaxStr, apMinStr, apMaxStr);

}

/***********************************************************************
 *
 * FUNCTION:		MExpFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒcameraÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean MExpFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (MExpFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case MultipleExposureSetButton:
	   				MExpFormSave();
	   			case MultipleExposureCancelButton:
					FrmReturnToForm(0);
 					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
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
 * FUNCTION:		MExpFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err MExpFormInit()
{
	struct		params {
//		UChar	locationFD3A;			// BOZO
		UChar	locationFD20;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	Err			err = 0,endErr;
	
	gRecordDirty = false;

	StartSession();
//	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);		// BOZO
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kReadDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if ((mem.locationFD3C & kMultiExpFlag) &&
		(mem.locationFD3D >= StrAToI(LstGetSelectionText(GetObjectPtr(MultipleExposureMExpList),1))) && 
		(mem.locationFD3D <= LstGetNumberOfItems(GetObjectPtr(MultipleExposureMExpList))))
	{
		SetPopupList(MultipleExposureMExpList, MultipleExposureMExpPopTrigger, mem.locationFD3D - 1);
	}
	else
		SetPopupList(MultipleExposureMExpList, MultipleExposureMExpPopTrigger, 0);

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		MExpFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err MExpFormSave()
{
	struct		params {
		UChar	locationFD1F;
		UChar	locationFD20;
		UChar	locationFD3A;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	SWord		index;
	Err			err = 0, endErr;

	if (gRecordDirty == false)
		return err;

	index = LstGetSelection(GetObjectPtr(MultipleExposureMExpList));
	if (index == 0)
	{
		mem.locationFD3C = 0;
		mem.locationFD3D = 0;
	}
	else
	{
		mem.locationFD3C = kMultiExpFlag;
		mem.locationFD3D = StrAToI(LstGetSelectionText(GetObjectPtr(MultipleExposureMExpList),index));
	}

	StartSession();	
	
	// Read the sequence frame count
	SendCommand(kReadDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	if (mem.locationFD1F) goto ERROR;		// cannot set in middle of sequence
	mem.locationFD1F = 0;
	SendCommand(kWriteDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	
	// Don't know why Photo Secretary does this
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kWriteDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	
	// Seems to be necessary to lock in settings
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	mem.locationFD3A |= kCreativeSetFlag;
	SendCommand(kWriteDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	
	// Write creative options
	SendCommand(kWriteDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		BracketFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒcameraÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean BracketFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (BracketFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case BracketingSetButton:
					BracketFormSave();
	   			case BracketingCancelButton:
	 				FrmReturnToForm(0);
	 				FrmUpdateForm(CameraControlForm, frmUpdateEvent);
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
 * FUNCTION:		BracketFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err BracketFormInit()
{
	struct		params {
//		UChar	locationFD3A;		// BOZO
		UChar	locationFD20;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	SWord		index;
	Err			err = 0, endErr;
	
	gRecordDirty = false;

	StartSession();
//	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);		// BOZO
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kReadDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	if ((mem.locationFD3C & kCameraBracketFlag) || (mem.locationFD3C & kFlashBracketFlag))
	{
		if ((mem.locationFD3D >= StrAToI(LstGetSelectionText(GetObjectPtr(BracketingFramesList), 1))) &&
			(mem.locationFD3D <= LstGetNumberOfItems(GetObjectPtr(BracketingFramesList))))
		{
			SetPopupList(BracketingFramesList, BracketingFramesPopTrigger, mem.locationFD3D - 1);
		}
		else
			SetPopupList(BracketingFramesList, BracketingFramesPopTrigger, 0);
		
		Value2Index(kBracketList, mem.locationFD3C & 0x0F, &index);
		if (index < 0)
			index = 0;
		SetPopupList(BracketingStepsList, BracketingStepsPopTrigger, index);
		
		if (mem.locationFD3C & kCameraBracketFlag)
			CtlSetValue(GetObjectPtr(BracketingBracketCameraCheckbox), 1);
		if (mem.locationFD3C & kFlashBracketFlag)
			CtlSetValue(GetObjectPtr(BracketingBracketFlashCheckbox), 1);

	}
	else
	{
		SetPopupList(BracketingFramesList, BracketingFramesPopTrigger, 0);
		SetPopupList(BracketingStepsList, BracketingStepsPopTrigger, 0);
	}

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		BracketFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err BracketFormSave()
{
	struct		params {
		UChar	locationFD1F;
		UChar	locationFD20;
		UChar	locationFD3A;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	UChar		compValue;
	Word		frame;
	Word		comp;
	Boolean		bCamera, bFlash;
	Err			err = 0, endErr;

	if (gRecordDirty == false)
		return err;

	frame = LstGetSelection(GetObjectPtr(BracketingFramesList));
	comp = LstGetSelection(GetObjectPtr(BracketingStepsList));
	bCamera = CtlGetValue(GetObjectPtr(BracketingBracketCameraCheckbox));
	bFlash = CtlGetValue(GetObjectPtr(BracketingBracketFlashCheckbox));
	if ((frame == 0) || (comp == 0) || ((bCamera == 0) && (bFlash == 0)))
	{
		mem.locationFD3C = 0;
		mem.locationFD3D = 0;
	}
	else
	{
		mem.locationFD3C = 0;
		if (bCamera)
			mem.locationFD3C |= kCameraBracketFlag;
		if (bFlash)
			mem.locationFD3C |= kFlashBracketFlag;
		Index2Value(kBracketList,comp,&compValue);
		mem.locationFD3C |= compValue;

		mem.locationFD3D = StrAToI(LstGetSelectionText(GetObjectPtr(BracketingFramesList),frame));

	}

	StartSession();
	
	// Read the sequence frame count
	SendCommand(kReadDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	if (mem.locationFD1F) goto ERROR;		// cannot set in middle of sequence
	mem.locationFD1F = 0;
	SendCommand(kWriteDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);

	// Don't know why Photo Secretary does this
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kWriteDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	
	// Seems to be necessary to lock in settings
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	mem.locationFD3A |= kCreativeSetFlag;
	SendCommand(kWriteDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	
	// Write creative options
	SendCommand(kWriteDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		AutoSeqFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒcameraÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean AutoSeqFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (AutoSeqFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case AutoSequenceSetButton:
	   				AutoSeqFormSave();
	   			case AutoSequenceCancelButton:
					FrmReturnToForm(0);
 					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
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
 * FUNCTION:		AutoSeqFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err AutoSeqFormInit()
{
	struct		params {
//		UChar	locationFD3A;		// BOZO
		UChar	locationFD20;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	Err			err = 0, endErr;
	
	gRecordDirty = false;

	StartSession();
//	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);		// BOZO
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kReadDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	
	if ((mem.locationFD3C & kAutoSequenceFlag) &&
		(mem.locationFD3D >= StrAToI(LstGetSelectionText(GetObjectPtr(AutoSequenceFramesList),1))) && 
		(mem.locationFD3D <= LstGetNumberOfItems(GetObjectPtr(AutoSequenceFramesList))))
	{
		
		SetPopupList(AutoSequenceFramesList, AutoSequenceFramesPopTrigger, mem.locationFD3D - 1);
	}
	else
		SetPopupList(AutoSequenceFramesList, AutoSequenceFramesPopTrigger, 0);

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		AutoSeqFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err AutoSeqFormSave()
{
	struct		params {
		UChar	locationFD1F;
		UChar	locationFD20;
		UChar	locationFD3A;
		UChar	locationFD3C;
		UChar	locationFD3D;
	} mem;
	SWord		index;
	Err			err = 0, endErr;

	if (gRecordDirty == false)
		return err;

	index = LstGetSelection(GetObjectPtr(AutoSequenceFramesList));
	if (index == 0)
	{
		mem.locationFD3C = 0;
		mem.locationFD3D = 0;
	}
	else
	{
		mem.locationFD3C = kAutoSequenceFlag;
		mem.locationFD3D = StrAToI(LstGetSelectionText(GetObjectPtr(AutoSequenceFramesList),index));
	}

	StartSession();

	// Read the sequence frame count
	SendCommand(kReadDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	if (mem.locationFD1F) goto ERROR;		// cannot set in middle of sequence
	mem.locationFD1F = 0;
	SendCommand(kWriteDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	
	// Don't know why Photo Secretary does this
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kWriteDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	
	// Seems to be necessary to lock in settings
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	mem.locationFD3A |= kCreativeSetFlag;
	SendCommand(kWriteDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	
	// Write creative options
	SendCommand(kWriteDataMode, 0x0000FD3C, &(mem.locationFD3C), 2);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		FreezeFocusFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒcameraÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean FreezeFocusFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;

	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if (FreezeFocusFormInit())
  				FrmReturnToForm(0);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
			
	   	case ctlSelectEvent:  // A control button was pressed and released.
	   		switch (event->data.ctlEnter.controlID)
	   		{
	   			case FreezeFocusSetButton:
	   				FreezeFocusFormSave();
	   			case FreezeFocusCancelButton:
					FrmReturnToForm(0);
 					FrmUpdateForm(CameraControlForm, frmUpdateEvent);
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
 * FUNCTION:		FreezeFocusFormInit
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err FreezeFocusFormInit()
{
	struct		params {
		UChar	locationFD3A;
	} mem;
	Err			err = 0, endErr;
	
	gRecordDirty = false;

	StartSession();
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	
	if ((err = GetSessionError()) != 0) goto ERROR;
	
	
	if (mem.locationFD3A & kFreezeFocusFlag)
		CtlSetValue(GetObjectPtr(FreezeFocusFreezeFocusCheckbox), 1);
	else
		CtlSetValue(GetObjectPtr(FreezeFocusFreezeFocusCheckbox), 0);

ERROR:
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

/***********************************************************************
 *
 * FUNCTION:		FreezeFocusFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err FreezeFocusFormSave()
{
	struct		params {
		UChar	locationFD1F;
		UChar	locationFD20;
		UChar	locationFD3A;
	} mem;
	Err			err = 0, endErr;

	if (gRecordDirty == false)
		return err;
		
	StartSession();
	
	// Read the sequence frame count
	SendCommand(kReadDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);
	if (mem.locationFD1F) goto ERROR;		// cannot set in middle of sequence
	mem.locationFD1F = 0;
	SendCommand(kWriteDataMode, 0x0000FD1F, &(mem.locationFD1F), 1);

	// Don't know why Photo Secretary does this
	SendCommand(kReadDataMode, 0x0000FD20, &(mem.locationFD20), 1);
	SendCommand(kWriteDataMode, 0x0000FD20, &(mem.locationFD20), 1);

	// Turn on or off freeze focus
	SendCommand(kReadDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);
	CLEAR_FLAGS(mem.locationFD3A, kFreezeFocusFlag);
	if (CtlGetValue(GetObjectPtr(FreezeFocusFreezeFocusCheckbox)))
		SET_FLAGS(mem.locationFD3A, kFreezeFocusFlag);
	SendCommand(kWriteDataMode, 0x0000FD3A, &(mem.locationFD3A), 1);

ERROR:	
	endErr = EndSession();
	if (err == 0) err = endErr;

	if (err) PostError(err);	
	return(err);
}

