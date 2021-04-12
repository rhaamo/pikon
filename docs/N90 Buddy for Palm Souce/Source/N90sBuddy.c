/*
	File:		N90sBuddy.c

	Contains:	Top-level routines, misc form routines

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved

	Change History (most recent first):

       <19+>    12/29/00    ksh     Add Freeze Focus
        <19>     2/18/99    ksh     Fix wraparound on downloading data for both N90 & N90s
        <18>     2/18/99    ksh     Fix Wrong Camera Alert (should be using FrmCustomAlert)
        <17>     2/16/99    ksh     Add support for logging to memo pad
        <16>    11/14/98    ksh     Fix bug in memo holder download when numFrames = 0
        <15>     6/30/98    ksh     Remove extra StartSession; Cleaned up serial code
        <14>     5/21/98    ksh     Fix Download Memo Holder to terminate when no memo info
        <13>     4/13/98    ksh     Fix compiler warnings
        <12>     3/25/98    ksh     Add new error for open serial port.
        <11>     3/25/98    ksh     Fix checkin comment for version 10.
        <10>     3/25/98    ksh     Make sure serial port is closed if accidentally left open.
         <9>     3/25/98    ksh     Fix checking of Value2Index return result in Camera Options
         <8>     3/25/98    ksh     Fix StartSession/EndSession balancing
         <7>     3/14/98    ksh     Update Delete rolls to to work with most-to-least recent.
         <6>     2/18/98    ksh     Fix noErr def (should be 0 for PalmPilot)
         <5>     2/18/98    ksh     Update for 1.1b1; all dialogs now updated for N90
         <4>     2/15/98    KSH     Rename to N90 Buddy
         <3>     2/15/98    KSH     Update for N90/F90 memo holder download
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in for N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				N90SBuddy.c
 *
 * DESCRIPTION:	Nikon N90S Buddy Application for setting camera options
 *						and capturing downloaded data.
 *
 *
 *	RELEASED AS OPEN SOURCE 13 May 2004
 *	  
 * REVISION HISTORY:
 * 	 9/1/96	ksh		Initial version
 *
 **********************************************************************/

#include "N90sBuddy.h"

/***********************************************************************
 * Global variables for this module
 **********************************************************************/
Int				gCurrentView;
DmOpenRef		gMemoDatabaseRef;
UInt				gCurrentRoll;
Word				gCurrentFrame;
Word				gTopVisibleRecord;
Boolean			gRecordDirty = false;				// true if a record has been modified
Boolean			gPalmOS2 = false;
ULong				gRollTime;

TimeFormatType		gTimeFormat;					// system preference
DateFormatType		gShortDateFormat;			// system preference

const UChar kMemoList[] = { 4, 0x00, 0x45, 0x4E, 0x5F };
const UChar kMemoDisableList[] = { 4, 0x00, 0x05, 0x0E, 0x1F };
const UChar kDualList[] = {4, 0x00, 0x04, 0x08, 0x0C};
//const UChar kFlashList[] = {4, 0x90, 0x98, 0x9C, 0xA8};

/***********************************************************************
 * Prototypes for internal functions
 **********************************************************************/

void SwapShort(Word *x)
{
	Word temp;
	
	temp = *x;
	temp = (temp << 8) | (temp >> 8);
	*x = temp;
}

Word MemoSettings2Bytes(UChar setting)
{
	setting &= 0x0F;
	if (setting == 0x05)
		return(2);
	else if (setting == 0x0E)
		return(4);
	else if (setting == 0x0F)
		return(6);
	else
		return(0);
}

Word BCD2Short(Word bcd)
{
	Word result;
	
	result = ((bcd >> 12) * 1000) +
				(((bcd >> 8) & 0x000F) * 100) +
				(((bcd >> 4) & 0x000F) * 10) +
				(bcd & 0x000F);
				
	return(result);
}

Word Short2BCD(Word value)
{
	Word result;
	
	result =		(((value / 1000) % 10) << 12) +
					(((value / 100	) % 10) << 8) +
					(((value	/ 10	) % 10) << 4) +
					(((value / 1	) % 10) << 0);

	return(result);
}


void	Index2Value(const UChar *list, UChar index, UChar *value)
{
	*value = list[index + 1];
}

void Value2Index(const UChar *list, UChar value, SWord *index)
{
	Word	i;
	Word	size;
	UChar *p;
	
	*index = -1;	
	size = *list++;
	for (i = 0, p = (UChar *) list; i < size; i++, p++)
	{
		if (*p == value)
		{
			*index = i;
			break;
		}
	}
}

void SetFrameLabel(Word labelID, Word rsrcID, Word value1, Word value2)
{
	VoidHand	h;
	CharPtr	titleTemplateP;
	FormPtr	frm;
	CharPtr	p;
	Word		len;
	Char		string1[kRollDigits + 1], string2[kRollDigits + 1];
	
	StrIToA(string1, value1);
	StrIToA(string2, value2);
	
	h = DmGetResource(strRsc, rsrcID);
	if (h == NULL) return;
	
	titleTemplateP = MemHandleLock(h);
	len = StrLen(string1) + StrLen(string2) + StrLen(titleTemplateP) + 1;
	p = MemPtrNew(len);
	if (p == NULL) return;
	
	StrCopy(p, titleTemplateP);
	MemMove(StrChr(p,'#'), string1, StrLen(string1)+1);
	StrCat(p, StrChr(titleTemplateP, '#') + 1);
	MemMove(StrChr(p,'#'), string2, StrLen(string2)+1);
	StrCat(p, StrChr(StrChr(titleTemplateP,'#')+1,'#')+1);

	MemHandleUnlock(h);

	frm = FrmGetActiveForm();
	CopyLabel(labelID, p);
	MemPtrFree(p);
}

Boolean SeekRecord(WordPtr indexP, Word offset, Word direction)
{

	DmSeekRecordInCategory(gMemoDatabaseRef, indexP, offset, direction, dmAllCategories);
	if (DmGetLastErr()) return (false);
	
	return (true);
}

void DirtyRecord (Word index)
{
	Word		attr;

	DmRecordInfo (gMemoDatabaseRef, index, &attr, NULL, NULL);
	attr |= dmRecAttrDirty;
	DmSetRecordInfo (gMemoDatabaseRef, index, &attr, NULL);
}


void PostError(Err err)
{
	Err	errClass;
	char	errorString[9];
	
	StrIToH(errorString, (ULong) err);
	StrCopy(&(errorString[2]), &(errorString[4]));
	errorString[0] = '0';
	errorString[1] = 'x';
	errClass = (err & 0xFF00);
	switch (errClass)
	{
		case memErrorClass:
			if (err == memErrNotEnoughSpace)
				FrmCustomAlert(NoMemoryAlert, errorString, NULL, NULL);
			else
				FrmCustomAlert(MemoryErrorAlert, errorString, NULL, NULL);
			break;
		case dmErrorClass:
			FrmCustomAlert(DatabaseErrorAlert, errorString, NULL, NULL);
			break;
		case serErrorClass:
			FrmCustomAlert(SerialManagerAlert, errorString, NULL, NULL);
			break;
		case appErrorClass:
			switch (err)
			{
				case kWrongCameraErr:
					FrmCustomAlert(WrongCameraAlert, errorString, NULL, NULL);
					break;
				case kNoMemoDataErr:
					FrmAlert(NoMemoHolderAlert);
					break;
				case kUnknownDataErr:
					FrmCustomAlert(UnknownDataAlert, errorString, NULL, NULL);
					break;
				case kConvertDataErr:
					FrmCustomAlert(ConvertDataErrAlert, errorString, NULL, NULL);
					break;
				case kSerialOpenErr:
					FrmCustomAlert(SerialOpenErrAlert, errorString, NULL, NULL);
					break;
				default:
					FrmCustomAlert(CommErrorAlert, errorString, NULL, NULL);
					break;
			}
			break;
		default:
			FrmCustomAlert(SysErrorAlert, errorString, NULL, NULL);
			break;
	}

}


/***********************************************************************
 *
 * FUNCTION:    GetPreferences()
 * FUNCTION:	 SetPreferences()
 *
 * DESCRIPTION: Draws the roll number assuming there's space 
 *
 * PARAMETERS:	 BuddyPreferencesType *
 *
 * RETURNED:	 Boolean (if prefs found for GetPreferences)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/

Boolean GetPreferences(BuddyPreferencesType *prefs)
{
	return (PrefGetAppPreferencesV10(kNBuddyCreator, kN90sBuddyVersion, prefs, sizeof(BuddyPreferencesType)));
}

void SetPreferences(BuddyPreferencesType *prefs)
{
	PrefSetAppPreferencesV10(kNBuddyCreator, kN90sBuddyVersion, prefs, sizeof(BuddyPreferencesType));
}


/***********************************************************************
 *
 * FUNCTION:    DrawRollNumber
 *
 * DESCRIPTION: Draws the roll number assuming there's space 
 *
 * PARAMETERS:	 memo  - pointer to a memo
 *              x     - draw position
 *              y     - draw position
 *              width - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void DrawRollNumber (Word rollNumber, SWord x, SWord y, SWord width)
{
	Int titleLen;
	Int charsToDraw;
	SWord titleWidth;
	Boolean stringFit;
	Char	roll_text[kRollDigits + 1];

	charsToDraw = StrLen(GetNumberString(roll_text, rollNumber, kRollDigits));

	titleWidth = width;
	titleLen = charsToDraw;
	FntCharsInWidth (roll_text, &titleWidth, &titleLen, &stringFit);

	if (stringFit)
	{
		WinDrawChars (roll_text, titleLen, x, y);
	}
}

/***********************************************************************
 *
 * FUNCTION:     StartApplication
 *
 * DESCRIPTION:  This routine sets up the initial state of the application.
 *
 * PARAMETERS:   None.
 *
 * RETURNED:     Nothing.
 *
 ***********************************************************************/
Err StartApplication(void)
{
	Err		err = 0;
	Word		mode;
	UInt		cardNo;
	LocalID	dbID;
	UInt		dbAttrs;
	ULong		sysVersion;
	SystemPreferencesType	sysPrefs;
	BuddyPreferencesType		prefs;
	
	gCurrentRoll = kNoRecordSelected;
	gCurrentFrame = kNoRecordSelected;
	gCurrentView = MainForm;
	gTopVisibleRecord = 0;

	FtrGet('psys', 1, &sysVersion);
	if (sysVersion >= 0x02000000 )
		gPalmOS2 = true;
		
	PrefGetPreferences(&sysPrefs);
	gTimeFormat = sysPrefs.timeFormat;
	gShortDateFormat = sysPrefs.dateFormat;
	
	mode = dmModeReadWrite;
	gMemoDatabaseRef = DmOpenDatabaseByTypeCreator(kNBuddyDBType,kNBuddyCreator, mode);
	if (!gMemoDatabaseRef)
	{
		err = DmCreateDatabase(0, kNBuddyOldDBName, kNBuddyCreator, kNBuddyDBType, false);
		if (err) return err;
					
		gMemoDatabaseRef = DmOpenDatabaseByTypeCreator(kNBuddyDBType,kNBuddyCreator, mode);
		if (! gMemoDatabaseRef) return 1;

		err = DmOpenDatabaseInfo(gMemoDatabaseRef, &dbID, NULL, NULL, &cardNo, NULL);
		if (err) return err;
		
		err = DmDatabaseInfo(0, dbID, NULL, &dbAttrs, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		if (err) return err;
		
		dbAttrs = dbAttrs | dmHdrAttrBackup;
		err = DmSetDatabaseInfo(0, dbID, NULL, &dbAttrs, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		if (err) return err;

	}

	if (gMemoDatabaseRef)
	{
		VoidHand	r1,r2;
		
		r1 = DmQueryRecord(gMemoDatabaseRef, 0);
		r2 = DmQueryRecord(gMemoDatabaseRef, 1);

		if (r1 && r2)
		{
			RollHeaderPackedPtr	rp1, rp2;
			Word				roll1, roll2;
			
			rp1 = (RollHeaderPackedPtr) MemHandleLock(r1);
			rp2 = (RollHeaderPackedPtr) MemHandleLock(r2);
			roll1 = rp1->rollNumber;
			roll2 = rp2->rollNumber;
			MemHandleUnlock(r1);
			MemHandleUnlock(r2);
			
			if (roll2 > roll1)
				DmQuickSort(gMemoDatabaseRef, (DmComparF *) CompareRecords, 0);
		}
	}
	
	if (GetPreferences(&prefs) == false)
	{
		MemSet(&prefs, sizeof(BuddyPreferencesType), 0);
		SetPreferences(&prefs);
	}
	
	return(err);
}


Int CompareRecords(RollHeaderPackedPtr r1, RollHeaderPackedPtr r2, Int other)
{
#pragma unused(other)

	return(r2->rollNumber - r1->rollNumber);
	
}

/***********************************************************************
 *
 * FUNCTION:    StopApplication
 *
 * DESCRIPTION: This routine closes the application's database.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
void StopApplication(void)
{

	// Send a frmSave event to all the open forms.
	FrmSaveAllForms();
	
	// Close all the open forms.
	FrmCloseAllForms();

	// Close the application's database.
	DmCloseDatabase(gMemoDatabaseRef);

}


/***********************************************************************
 *
 * FUNCTION:    Find
 *
 * DESCRIPTION: Search the database for records containing the string passed.
 *
 *					 This routine is called in response to a system launch 
 * 				 code from PilotMain.   The application may not
 *					 be running and if so the applications global variables
 *					 are not set up.  This means the global variables may not
 *					 be read and may not be written to.
 *
 *					 Because this routine is called whenever the system find
 *					 feature is used this routine must be fast.  Slow performance
 *					 makes the whole device seem slow.
 *
 * PARAMETERS:	 findParams - text search parameter block setup by the system
 *
 * RETURNED:	 nothing
 *
 ***********************************************************************/
void Search(FindParamsPtr findParams)
{
	Word pos;
	CharPtr header;
	UInt recordNum;
	Int frameNum, numFrames;
	UInt offset;
	VoidHand recordH;
	VoidHand headerH;
	Boolean done;
	Boolean match;
	DmOpenRef dbP;
	UInt cardNo = 0;
	LocalID dbID;
	RollHeaderPackedPtr recordP;
	FrameHeaderPackedPtr	frameP;
	BuddyPreferencesType	prefs;

	// Find the application's data file.
	dbP = DmOpenDatabaseByTypeCreator(kNBuddyDBType, kNBuddyCreator, findParams->dbAccesMode);
	if (!dbP)
	{
		// Return without anything.  Also indicate that no more records are expected.
		findParams->more = false;
		return;
	}
	DmOpenDatabaseInfo(dbP, &dbID, 0, 0, &cardNo, 0);

	// Display the heading line.
	headerH = DmGetResource (strRsc, AppNameString);
	header = MemHandleLock (headerH);
	done = FindDrawHeader (findParams, header);
	MemHandleUnlock (headerH);
	if (done) goto Exit;				// There was no more room to display the heading line
	
	prefs.nextSearchRoll = kNoRecordSelected;
	prefs.nextSearchFrame = kNoRecordSelected;
	prefs.lastSearchInFrame = false;
	GetPreferences(&prefs);
	
	recordNum = findParams->recordNum;
		 	
	/*
	 *	If we didn't leave off in the middle of a frame, start at the next record
	 */
	 
	while (true)
	{
		// Get the next record.  Skip private records if neccessary.
		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);

		// Have we run out of records?
		if (! recordH)
		{
			findParams->more = false;			
			break;
		}

		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);
		
		frameNum = prefs.nextSearchFrame;
		if (prefs.lastSearchInFrame && 
			(prefs.nextSearchRoll == recordNum) && 
			(frameNum >= 0))
			goto SEARCH_FRAMES;

SEARCH_ROLL_CAPTION:

		prefs.lastSearchInFrame = false;
		frameNum = kNoRecordSelected;
		offset = GetCaptionOffset(recordP);
		match = FindStrInStr((CharPtr) recordP + offset, findParams->strToFind, &pos);
		
		if (match)
			done = SearchDoMatch(recordP, frameNum, findParams, recordNum, offset + pos, cardNo, dbID);
		
		if (done)
			prefs.lastSearchInFrame = true;
		else
			frameNum = 0;
		
SEARCH_FRAMES:

		numFrames = recordP->frameCount;
			
		while ((done == false) && (frameNum < numFrames))
		{
			offset = GetFrameOffset(recordP, frameNum);
			frameP = (FrameHeaderPackedPtr) ((CharPtr) recordP + offset);
			offset += GetFrameCaptionOffset(frameP);
			match = FindStrInStr((CharPtr) recordP + offset, findParams->strToFind, &pos);
			if (match)
				done = SearchDoMatch(recordP, frameNum, findParams, recordNum, 
											offset + pos, cardNo, dbID);
				
			if (done)
				prefs.lastSearchInFrame = true;
			else			
				frameNum++;
		}

		MemHandleUnlock (recordH);

		if (done)
			break;
	
		recordNum++;
	}
			
	prefs.nextSearchRoll = recordNum;
	prefs.nextSearchFrame = frameNum;
	SetPreferences(&prefs);
		
Exit:
	DmCloseDatabase (dbP);	
}

Boolean SearchDoMatch(RollHeaderPackedPtr recordP, Int frame, FindParamsPtr findParams, 
					UInt recordNum, Word pos, UInt cardNo, LocalID dbID)
{
	RectangleType r;
	Boolean done;
	Int		tab,width;
	Int		textLen;
	FontID	currFont;
	Boolean	fits;
	CharPtr	text;
	Char		roll[kRollDigits + 1];
	
	done = FindSaveMatch(findParams, recordNum, pos, 0, 0, cardNo, dbID);
	if (! done)
	{
		// Get the bounds of the region where we will draw the results.
		FindGetLineBounds(findParams, &r);
		
		// Display the record in the search dialog.  We move in one pixel
		// so that when the record is inverted the left edge is solid.
		r.topLeft.x++;
		tab = r.topLeft.x + 1 + 36;
		r.extent.x--;
		width = r.extent.x - 2 - 36;

		currFont = FntSetFont(stdFont);
		
		textLen = kRollDigits;
		GetNumberString(roll, recordP->rollNumber, textLen);
		WinDrawChars(roll, textLen, r.topLeft.x, r.topLeft.y);
		r.topLeft.x += FntCharsWidth(roll, textLen);
		
		if (frame >= 0)
		{
			roll[0] = '/';
			roll[1] = 0;
			WinDrawChars(roll, 1, r.topLeft.x, r.topLeft.y);
			r.topLeft.x +=  FntCharWidth('/');
			textLen = 2;
			GetNumberString(roll, frame+1, textLen);
			WinDrawChars(roll, textLen, r.topLeft.x, r.topLeft.y);
			
		}
		text = (CharPtr) recordP + pos;
		textLen = StrLen(text);
		FntCharsInWidth(text, &width, &textLen, &fits);
		WinDrawChars(text, textLen, tab, r.topLeft.y);
		FntSetFont(currFont);

		// The line number needs to be increment since a line is used for the record.
		findParams->lineNumber++;
	}
	
	return(done);
}


/***********************************************************************
 *
 * FUNCTION:    GoToItem
 *
 * DESCRIPTION: It is generally called as the result of hitting a 
 *              "Go to" button in the text search dialog.  The record
 *              identifies by the parameter block passed will be display,
 *              with the character range specified highlighted.
 *
 * PARAMETERS:	 goToParams   - parameter block that identifies the record to
 *                             display.
 *              launchingApp - true if the application is being launched.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	Word recordNum;
	UInt attr;
	ULong uniqueID;
	EventType event;

	recordNum = goToParams->recordNum;
	DmRecordInfo (gMemoDatabaseRef, recordNum, &attr, &uniqueID, NULL);

	// If the application is already running, close all open forms.  This 
	// may cause in the database record to be reordered, so we'll find the 
	// records index by its unique id.
	if (! launchingApp)
	{
		FrmCloseAllForms();
		DmFindRecordByID(gMemoDatabaseRef, uniqueID, &recordNum);
	}
		

	// Send an event to goto a form and select the matching text.
	MemSet(&event, sizeof(EventType), 0);

	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = EditRollForm;
	EvtAddEventToQueue (&event);
 
	event.eType = frmGotoEvent;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->searchStrLen;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	event.data.frmGoto.formID = EditRollForm;
	EvtAddEventToQueue (&event);
	
	gCurrentView = EditRollForm;
}


/***********************************************************************
 *
 * FUNCTION:		ApplicationHandleMenu
 *
 * DESCRIPTION:	Handles the standard menu items
 *
 * PARAMETERS:		itemID		- the menu ID
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/

Boolean ApplicationHandleMenu(Word item)
{
	Boolean	handled = false;
	FieldPtr	fld;
	
	// First clear the menu status from the display.
	MenuEraseStatus(NULL);
	fld = GetFocusObjectPtr();

	switch(item)
	{
		case CameraCameraControl:
			FrmGotoForm(CameraControlForm);
			handled = true;
			break;
		case CameraAutoFocusOptions:
		case CCameraAutoFocusOptions:
			FrmPopupForm(AFOptionsForm);
			handled = true;
			break;
		case CameraBeepOptions:
		case CCameraBeepOptions:
			FrmPopupForm(BeepOptionsForm);
			handled = true;
			break;
		case CameraCameraOptions:
		case CCameraCameraOptions:
			FrmPopupForm(CameraOptionsForm);
			handled = true;
			break;
		case CameraViewfinderOptions:
		case CCameraViewfinderOptions:
			FrmPopupForm(ViewFinderOptionsForm);
			handled = true;
			break;
		case CameraMemoHolderSettings:
		case CCameraMemoHolderSettings:
			FrmPopupForm(MemoSettingsForm);
			handled = true;
			break;
		case CameraDownloadShootingData:
		case CCameraDownloadShootingData:
			DownloadMemoHolder();
			FrmGotoForm(MainForm);
			handled = true;
			break;
		case EditAppendRoll:
			FrmGotoForm(MergeForm);
			handled = true;
			break;
		case EditDeleteRolls:
			FrmGotoForm(DeleteForm);
			handled = true;
			break;
		case EditCut:
			if (fld) FldCut(fld);
			handled = true;
			break;
		case EditCopy:
			if (fld) FldCopy(fld);	
			handled = true;
			break;
		case EditPaste:
			if (fld) FldPaste(fld);		
			handled = true;
			break;
		case EditUndo:
			if (fld) FldUndo(fld);
			handled = true;
			break;
		case EditSelectAll:
			if (fld) FldSetSelection(fld, 0, FldGetTextLength(fld));
			handled = true;
			break;
		case EditKeyboard:
			SysKeyboardDialogV10();
			handled = true;
			break;
		case EditGraffiti:
			if (gPalmOS2)
				SysGraffitiReferenceDialog (referenceDefault);
			else
				SndPlaySystemSound(sndError);
			handled = true;
			break;
			
		case OptionsAboutN90Buddy:
			FrmHelp(HelpString);
			handled = true;
			break;
		case OptionsRegister:
			handled = true;
			break;
#ifdef DEBUGBUILD
		case DebugAddSampleRolls:
			AddSampleRolls();
			FrmGotoForm(MainForm);
			handled = true;
			break;
		case DebugDeleteAllRolls:
			FrmSetFocus(FrmGetActiveForm(),noFocus);
			DeleteAllRolls();
			FrmGotoForm(MainForm);
			handled = true;
			break;
		case DebugResetMemoHolder:
			ResetMemoHolder();
			handled = true;
			break;
		case DebugPeekPoke:
		case CDebugPeekPoke:
			FrmPopupForm(debugForm);
			handled = true;
			break;
		case DebugDisableSerial:
		case CDebugDisableSerial:
			DebugToggleSerial();
			handled = true;
			break;
		case DebugDisableBaudChange:
		case CDebugDisableBaudChange:
			DebugToggleBaud();
			handled = true;
			break;
#endif
		
	}
	
	return(handled);
}


/***********************************************************************
 *
 * FUNCTION:    ApplicationHandleEvent
 *
 * DESCRIPTION: P3. This routine loads a form resource and sets the event handler for the form.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    True if the event has been handled and should not be
 *						passed to a higher level handler.
 *
 ***********************************************************************/
Boolean ApplicationHandleEvent (EventPtr event)
{
	FormPtr	frm;
	Word		formId;
	Boolean	handled = false;

	if (event->eType == frmLoadEvent)
	{
		// load the form resource specified in the event then activate the form.
		formId = event->data.frmLoad.formID;
		frm = FrmInitForm(formId);
		FrmSetActiveForm(frm);

		// set the event handler for the form.  The handler of the currently 
		// active form is called by FrmDispatchEvent each time it receives an event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frm, MainFormHandleEvent);
				break;
				
			case MemoSettingsForm:
				FrmSetEventHandler(frm, MemoFormHandleEvent);
				break;
				
			case CameraControlForm:
				FrmSetEventHandler(frm, CameraFormHandleEvent);
				break;
				
			case CameraOptionsForm:
				FrmSetEventHandler(frm, CamOptionFormHandleEvent);
				break;
				
			case ViewFinderOptionsForm:
				FrmSetEventHandler(frm, ViewOptionFormHandleEvent);
				break;
				
			case AFOptionsForm:
				FrmSetEventHandler(frm, AFOptionFormHandleEvent);
				break;
				
			case BeepOptionsForm:
				FrmSetEventHandler(frm, BeepOptionFormHandleEvent);
				break;
				
			case EditRollForm:
				FrmSetEventHandler(frm, EditRollFormHandleEvent);
				break;
				
			case timeForm:
				FrmSetEventHandler(frm, TimeFormHandleEvent);
				break;
				
			case MergeForm:
				FrmSetEventHandler(frm, MergeFormHandleEvent);
				break;
				
			case DeleteForm:
				FrmSetEventHandler(frm, DeleteFormHandleEvent);
				break;
				
			case AutoSequenceForm:
				FrmSetEventHandler(FrmGetActiveForm(), AutoSeqFormHandleEvent);
				break;
				
			case BracketingForm:
				FrmSetEventHandler(FrmGetActiveForm(), BracketFormHandleEvent);
				break;
				
			case FreezeFocusForm:
				FrmSetEventHandler(FrmGetActiveForm(), FreezeFocusFormHandleEvent);
				break;

			case MultipleExposureForm:
				FrmSetEventHandler(FrmGetActiveForm(), MExpFormHandleEvent);
				break;

#ifdef DEBUGBUILD
			case debugForm:
				FrmSetEventHandler(frm, DebugFormHandleEvent);
				break;
#endif
		}
		handled = true;
	}
	else if (event->eType == menuEvent)
		handled = ApplicationHandleMenu(event->data.menu.itemID);
#ifdef DEBUGBUILD
	else if (event->eType == winEnterEvent)
	{
		FixDebugMenu();
	}
#endif	
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:		EventLoop
 *
 * DESCRIPTION:	A simple loop that obtains events from the Event
 *						Manager and passes them on to various applications and
 *						system event handlers before passing them on to
 *						FrmDispatchEvent for default processing.
 *
 * PARAMETERS:		None.
 *
 * RETURNED:		Nothing.
 *
 ***********************************************************************/
void EventLoop(void)
{
	EventType	event;
	Word			error;
	extern UInt		serialRefNum;	
	do
	{
		// Get the next available event.
		EvtGetEvent(&event, evtWaitForever);
		
		// Give the system a chance to handle the event.
		if (! SysHandleEvent(&event))

			if (! MenuHandleEvent(NULL, &event, &error))

				if (! ApplicationHandleEvent(&event))
				{
					FrmDispatchEvent(&event);
					if (serialRefNum != 0)
					{
						PostError(kSerialOpenErr);
						EndSession();
					}
				}
	}
	while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:		PilotMain
 *
 * DESCRIPTION:	This function is the equivalent of a main() function
 *						under standard ÒCÓ.  It is called by the Emulator to begin
 *						execution of this application.
 *
 * PARAMETERS:		cmd - command specifying how to launch the application.
 *						cmdPBP - parameter block for the command.
 *						launchFlags - flags used to configure the launch.			
 *
 * RETURNED:		Any applicable error codes.
 *
 ***********************************************************************/
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
	Err	err;

	if (cmd == sysAppLaunchCmdNormalLaunch)
	{
		if ((err = StartApplication()) == 0)
		{
			FrmGotoForm(gCurrentView);
			EventLoop();
			StopApplication();
		}
	}
	else if (cmd == sysAppLaunchCmdFind)
	{
		Search((FindParamsPtr)cmdPBP);
	}
	else if (cmd == sysAppLaunchCmdGoTo)
	{
		if (launchFlags & sysAppLaunchFlagNewGlobals)
		{
			err = StartApplication();
			if (err) return(err);
			
			GoToItem((GoToParamsPtr) cmdPBP, true);
			
			EventLoop();
			StopApplication();
		}
		else
			GoToItem((GoToParamsPtr) cmdPBP, false);
	}
	else if (cmd == sysAppLaunchCmdSaveData)
	{
		FrmSaveAllForms();
	}

	return err;
}
