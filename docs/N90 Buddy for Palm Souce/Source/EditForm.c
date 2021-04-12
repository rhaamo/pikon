/*
	File:		EditForm.c

	Contains:	Routines for handling the edit form

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <2+>    7/5/2001    ksh     Update for PalmOS 3.5 headers
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/



#include "N90sBuddy.h"


/***********************************************************************
 *
 * FUNCTION:		EditRollFormInit
 *
 * DESCRIPTION:	Initializes all objects
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err EditRollFormInit()
{
	Err			err = 0;
	Char		rollStr[kRollDigits + 1];
	Word		roll;
	VoidHand					recordH = NULL;
	RollHeaderPackedPtr		recordP;
	
	recordH = DmQueryRecord(gMemoDatabaseRef, gCurrentRoll);
	if (recordH)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);

		gRollTime = recordP->timeDate;
		DateTimeSetDateCtl(EditRollDateSelTrigger, gRollTime);
		DateTimeSetTimeCtl(EditRollTimeSelTrigger, gRollTime);
		
		roll = recordP->rollNumber;
		GetNumberString(rollStr, roll, kRollDigits);
		
		SetTemplateString((char *) FrmGetTitle(FrmGetActiveForm()), EditRollTemplateString, rollStr);
		SetTemplateLabel(EditRollISOLabel, ISOTemplateString, GetStringTable(kISOTable, recordP->iso));
		SetTemplateLabel(EditRollCameraLabel, CameraTemplateString, (CharPtr) recordP+GetCameraOffset(recordP));
		
		SetFieldText(EditRollRollcaptionField, (CharPtr) recordP+GetCaptionOffset(recordP));

		MemHandleUnlock(recordH);
	}
		
	EditRollInitFrameItems();
	
ERROR:	
	if (err) PostError(err);
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:		EditRollFormSave
 *
 * DESCRIPTION:	Initializes all objects from camera settings
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err from any serial handling
 *
 ***********************************************************************/
Err EditRollFormSave()
{
	Err						err = 0;
	VoidHand					recordH = NULL;
	RollHeaderPackedPtr		recordP;
	
	EditRollClearEditState();
	recordH = DmQueryRecord(gMemoDatabaseRef, gCurrentRoll);
	if (recordH)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);

		if (recordP->timeDate != gRollTime)
			DmWrite(recordP, 0, &gRollTime, sizeof(recordP->timeDate));
	
		MemHandleUnlock(recordH);
	}
	
ERROR:
	return err;
}

/***********************************************************************
 *
 * FUNCTION:		EditRollFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒeditÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean EditRollFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;
	
	switch (event->eType)
	{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
			gCurrentFrame = 0;
			if (EditRollFormInit())
				FrmGotoForm(MainForm);
			else
				FrmDrawForm(FrmGetActiveForm());
			handled = true;
			break;
		
		case frmGotoEvent:
			EditRollGoto(event);
			handled = true;
			break;
			
   	case ctlSelectEvent:  // A control button was pressed and released.
   		switch (event->data.ctlEnter.controlID)
   		{
   			case EditRollDoneButton:
				FrmGotoForm(MainForm);
				handled = true;
 				break;
				
			case EditRollDateSelTrigger:
				DateTimeEditDate(&gRollTime);
				DateTimeSetDateCtl(EditRollDateSelTrigger, gRollTime);
				handled = true;
				break;
				
			case EditRollTimeSelTrigger:
				DateTimeEditTime(&gRollTime);	
				DateTimeSetTimeCtl(EditRollTimeSelTrigger, gRollTime);
				handled = true;
				break;

			default:
				gRecordDirty = true;
				break;
  			
   		}
			break;
		
		case keyDownEvent:
			if (event->data.keyDown.chr == pageUpChr)
			{
				EditRollScrollRecord(winUp);
				handled = true;
			}
			else if (event->data.keyDown.chr == pageDownChr)
			{
				EditRollScrollRecord(winDown);
				handled = true;
			}
			else
			{
				FrmHandleEvent(FrmGetActiveForm(), event);
				EditRollUpdateScrollers();
				handled = true;
			}
			break;
				
		case ctlRepeatEvent:
			if (event->data.ctlRepeat.controlID == EditRollScrollUpRepeating)
				EditRollScroll(winUp);
			else if (event->data.ctlRepeat.controlID == EditRollScrollDownRepeating)
				EditRollScroll(winDown);
			break;
		
		case fldEnterEvent:
			EditRollUpdateScrollers();
			break;
			
		case frmCloseEvent:
			EditRollFormSave();
			break;
	
		}
	return(handled);
}

void EditRollGoto(EventPtr event)
{
	FieldPtr		fld;
	VoidHand 	recordH;
	Word			offset, frameOffset;
	Int			item,frame;
	RollHeaderPackedPtr recordP;
	
	// Get a pointer to the to do record.
	gCurrentRoll = event->data.frmGoto.recordNum;
	recordH = DmQueryRecord(gMemoDatabaseRef, gCurrentRoll);
	if (recordH)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);
	
		offset = event->data.frmGoto.matchPos;
		frame = GetFrameByOffset(recordP, offset);
		if (frame < 0)
		{
			offset -= GetCaptionOffset(recordP);
			gCurrentFrame = 0;
			item = EditRollRollcaptionField;
		}
		else
		{
			gCurrentFrame = frame;
			frameOffset = GetFrameOffset(recordP, frame);
			offset -= frameOffset;
			offset -= GetFrameCaptionOffset((FrameHeaderPackedPtr) ((CharPtr) recordP + frameOffset));
			item = EditRollFrameCaptionField;
		}
		MemHandleUnlock(recordH);
		
		EditRollFormInit();
		fld = GetObjectPtr(item);
		FldSetScrollPosition(fld, offset);
		FldSetSelection(fld, offset, offset + event->data.frmGoto.matchLen);
		FrmDrawForm(FrmGetActiveForm());
		FrmSetFocus(FrmGetActiveForm(),GetObjectIndex(item));
	}
}


/***********************************************************************
 *
 * FUNCTION:    EditRollUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the edit roll scroll arrow
 *              buttons.
 *
 * PARAMETERS:  NONE
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void EditRollUpdateScrollers()
{
	FormPtr		frm;
	FieldPtr	fld;
	Boolean		scrollableUp, scrollableDown;
	Word 		upIndex, downIndex;
	
	scrollableUp = false;
	scrollableDown = false;
	
	frm = FrmGetActiveForm();
	fld = GetFocusObjectPtr();
	
	if (fld)
	{
		scrollableUp = FldScrollable(fld, winUp);	
		scrollableDown = FldScrollable(fld, winDown);	
	}	

	// Update the scroll button.
	upIndex = FrmGetObjectIndex(frm, EditRollScrollUpRepeating);
	downIndex = FrmGetObjectIndex(frm, EditRollScrollDownRepeating);
	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);
}

/***********************************************************************
 *
 * FUNCTION:    EditRollScroll
 *
 * DESCRIPTION: This routine scrolls the notes text
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void EditRollScroll(WinDirectionType direction)
{
	FieldPtr	fld;
	
	fld = GetFocusObjectPtr();

	if (fld && FldScrollable(fld, direction))
	{
		FldScrollField(fld, 1, direction);
		EditRollUpdateScrollers();
	}
}


/***********************************************************************
 *
 * FUNCTION:    EditRollScrollRecord
 *
 * DESCRIPTION: This routine scrolls the list of frames
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void EditRollScrollRecord(WinDirectionType direction)
{
	VoidHand					recordH = NULL;
	RollHeaderPackedPtr	recordP;
	SWord						maxFrame;
	SWord						newFrame;
	
	EditRollClearEditState();
	
	newFrame = gCurrentFrame;
	if ((recordH = DmQueryRecord(gMemoDatabaseRef, gCurrentRoll)) != NULL)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);
		maxFrame = recordP->frameCount - 1;
		MemHandleUnlock(recordH);	

		// Scroll the table down.
		if (direction == winDown)
		{
			newFrame++;
			if (newFrame > maxFrame)
				newFrame = maxFrame;
		}

		// Scroll the table up.
		else
		{
			newFrame--;
			if (newFrame < 0)
				newFrame = 0;
		}
	}
	if (newFrame != gCurrentFrame)
	{
		gCurrentFrame = newFrame;
		EditRollInitFrameItems();
	}
}

/***********************************************************************
 *
 * FUNCTION:    EditRollInitFrameItems
 *
 * DESCRIPTION: Initializes all the frame-specific items in the form
 *
 * PARAMETERS:  none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/

void EditRollInitFrameItems()
{
	FormPtr					frm;
	VoidHand					recordH = NULL;
	RollHeaderPackedPtr	recordP;
	FrameHeaderPackedPtr	frameP;
	Word						frameOffset;
	
	frm = FrmGetActiveForm();
	
	recordH = DmQueryRecord(gMemoDatabaseRef, gCurrentRoll);
	if (recordH)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);
		frameOffset = GetFrameOffset(recordP, gCurrentFrame);
		frameP = (FrameHeaderPackedPtr) ((CharPtr) recordP + frameOffset);
		
		SetFrameLabel(EditRollFrameLabel, FrameNNTemplateString, 
							gCurrentFrame+1, recordP->frameCount);
		
		ClearLabels(EditRollSpeedLabel, EditRollFlashCompLabel);

		switch (recordP->storageLevel)
		{
			case 2:
				if (frameP->expCompIndex)
					CopyLabel(EditRollExpCompLabel, 
						GetStringTable(kExpCompTable, frameP->expCompIndex));
				if ((frameP->flashIndex != kNoFlashIndex) && (frameP->flashCompIndex))
					CopyLabel(EditRollFlashCompLabel,
						GetStringTable(kExpCompTable, frameP->flashCompIndex));
			case 1:
				SetTemplateLabel(EditRollFocalLengthLabel, FocalLengthTempateString, 
					GetStringTable(kFocalTable, frameP->focalIndex));
				CopyLabel(EditRollExpModeLabel, 
					GetStringTable(kExposureModeTable, frameP->exposureIndex));
				CopyLabel(EditRollMeteringLabel,
					GetStringTable(kMeterTable, frameP->meterIndex));
				if (frameP->flashIndex != kNoFlashIndex)
					CopyLabel(EditRollFlashSyncLabel,
						GetStringTable(kFlashSyncTable, frameP->flashIndex));
			case 0:
				SetTemplateLabel(EditRollSpeedLabel, ShutterSpeedTemplateString,
					GetStringTable(kShutterSpeedTable, frameP->shutterIndex));
				SetTemplateLabel(EditRollApertureLabel, ApertureTemplateString, 
					GetStringTable(kApertureTable,frameP->apertureIndex));
				break;
		}

		SetFieldText(EditRollFrameCaptionField, (CharPtr) recordP + 
						frameOffset + GetFrameCaptionOffset(frameP));

		
		MemHandleUnlock(recordH);
	}
	EditRollUpdateScrollers();
}

/***********************************************************************
 *
 * FUNCTION:    EditRollClearEditState
 *
 * DESCRIPTION: This routine clears the edit state of the list.
 *              It is caled whenever a table item is selected.
 *
 *              If the new item selected is in a different row than
 *              the current record, the edit state is cleared.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the current record is deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void EditRollClearEditState()
{
	VoidHand	text;
	FieldPtr	rollFld, frameFld;
	Err		err = 0;

	FrmSetFocus(FrmGetActiveForm(), noFocus);
	
	rollFld = GetObjectPtr(EditRollRollcaptionField);
	frameFld = GetObjectPtr(EditRollFrameCaptionField);
	
	if (FldDirty(rollFld) || FldDirty(frameFld))
	{
		if (FldDirty(rollFld))
		{
			text = FldGetTextHandle(rollFld);
			if (text) 
				err = InsertCaptionHandle(gMemoDatabaseRef, gCurrentRoll, kNoRecordSelected, text);
			FldSetDirty(rollFld, false);
		}
		
		if (FldDirty(frameFld))
		{
			text = FldGetTextHandle(frameFld);
			if (text)
				err = InsertCaptionHandle(gMemoDatabaseRef, gCurrentRoll, gCurrentFrame, text);
			FldSetDirty(frameFld, false);
		}
	}
	if (err)
		PostError(err);
}

