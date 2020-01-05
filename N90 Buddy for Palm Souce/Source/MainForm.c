/*
	File:		MainForm.c

	Contains:	Routines for handling Main form

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <2+>   1/23/2002    ksh     Update for PalmOS 3.5 headers
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial check-in from N90s Buddy 1.0
*/



#include "N90sBuddy.h"

/***********************************************************************
 *
 * FUNCTION:		MainFormHandleEvent
 *
 * DESCRIPTION:	Handles processing of events for the ÒmainÓ form.
 *
 * PARAMETERS:		event		- the most recent event.
 *
 * RETURNED:		True if the event is handled, false otherwise.
 *
 ***********************************************************************/
Boolean MainFormHandleEvent(EventPtr event)
{
	Boolean		handled = false;
	Err			err;
	
	switch (event->eType)
		{  	
  		case frmOpenEvent:
  			// It has already been loaded and activated so just draw it.
  			if ((err = MainFormInit()) == 0)
				FrmDrawForm(FrmGetActiveForm());
			else
				PostError(err);
			handled = true;
			break;
	
		case penDownEvent:
			handled = FrmHandleEvent(FrmGetActiveForm(), event);
			if (! handled) MainClearEditState();
			break;
			
		case keyDownEvent:
			if (event->data.keyDown.chr == pageUpChr)
			{
				MainScroll(winUp, false);
				handled = true;
			}
			else if (event->data.keyDown.chr == pageDownChr)
			{
				MainScroll(winDown, false);
				handled = true;
			}
			break;

		case ctlEnterEvent:
			if ((event->data.ctlEnter.controlID == MainScrollUpRepeating) ||
				(event->data.ctlEnter.controlID == MainScrollDownRepeating))
					MainClearEditState();
			break;

		case ctlRepeatEvent:
			if (event->data.ctlRepeat.controlID == MainScrollUpRepeating)
				MainScroll(winUp, true);
			else if (event->data.ctlRepeat.controlID == MainScrollDownRepeating)
				MainScroll(winDown,true);
			break;
			
		case tblSelectEvent:
			MainItemSelected(event);
			handled = true;
			break;
			
		case tblExitEvent:
			MainClearEditState();
			handled = true;
			break;

		case fldHeightChangedEvent:
			MainResizeDescription(event);
			handled = true;
			break;
		
		case frmSaveEvent:
			MainClearEditState();
			break;
		
	}

	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:		MainFormInit
 *
 * DESCRIPTION:	Fills in the table of all the rolls stored in the memo holder
 *
 * PARAMETERS:		none
 *
 * RETURNED:		Err
 *
 ***********************************************************************/
Err MainFormInit()
{
	Err			err = 0;
	Word 			row;
	Word 			rowsInTable;
	TablePtr 	table;

	table = GetObjectPtr(MainRollsTable);

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
	{		
		TblSetItemStyle(table, row, MainRollColumn, customTableItem);
		TblSetItemStyle(table, row, MainCaptionColumn, textTableItem);
		TblSetRowUsable(table, row, false);
	}

	TblSetColumnUsable(table, MainRollColumn, true);	
	TblSetColumnUsable(table, MainCaptionColumn, true);	

	// Set the callback routines that will load and save the 
	// description field.
	TblSetLoadDataProcedure(table, MainCaptionColumn, MainGetRollCaption);
	TblSetSaveDataProcedure(table, MainCaptionColumn, MainSaveRollCaption);

	// Set the callback routine that draws the due date field.
	TblSetCustomDrawProcedure(table, MainRollColumn, MainDrawRollNumber);

	MainLoadTable(true);

ERROR:
	if (err) PostError(err);	
	return(err);
}


/***********************************************************************
 *
 * FUNCTION:    MainDrawRollNumber
 *
 * DESCRIPTION: This routine the draw the roll number and the caption
 *						for each roll in the database
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw 
 *              bounds - bound to the draw region
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/
void MainDrawRollNumber(VoidPtr table, Int16 row, Int16 column, RectangleType *bounds)
{
	#pragma unused(column)
	Word 	recordNum;
	Word	x, y;
	RollHeaderPackedPtr		 recordP;
	Handle						recordH;
	
	recordNum = TblGetRowID(table, row);
	
	recordH = DmQueryRecord(gMemoDatabaseRef, recordNum);
	if (recordH)
	{
		recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);

		FntSetFont (stdFont);
		
		x = bounds->topLeft.x + 1;
		y = bounds->topLeft.y;
		
		DrawRollNumber(recordP->rollNumber, x, y, bounds->extent.x - x);

		MemHandleUnlock(recordH);
	}	
}


/***********************************************************************
 *
 * FUNCTION:    MainUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the list view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frm          -  pointer to the to do list form
 *              bottomRecord -  record index of the last visible record
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void MainUpdateScrollers(Word bottomRecord, Boolean lastItemClipped)
{
	Word upIndex;
	Word downIndex;
	Word recordNum;
	Boolean scrollableUp;
	Boolean scrollableDown;
		
	// If the first record displayed is not the fist record in the category,
	// enable the up scroller.
	recordNum = gTopVisibleRecord;
	scrollableUp = SeekRecord(&recordNum, 1, dmSeekBackward);


	// If the last record displayed is not the last record in the category,
	// enable the down scroller.
	recordNum = bottomRecord;
	scrollableDown = lastItemClipped || SeekRecord(&recordNum, 1, dmSeekForward); 


	// Update the scroll button.
	upIndex = GetObjectIndex(MainScrollUpRepeating);
	downIndex = GetObjectIndex(MainScrollDownRepeating);
	FrmUpdateScrollers (FrmGetActiveForm(), upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    MainLoadTable
 *
 * DESCRIPTION: This routine loads memo holder database records into
 *              the main form.
 *
 * PARAMETERS:  recordNum index of the first record to display.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void MainLoadTableItem (TablePtr table, Word recordNum, Word row)
{
	TblSetRowID(table, row, recordNum);
	TblSetItemStyle(table, row, MainRollColumn, customTableItem);
	TblSetItemStyle(table, row, MainCaptionColumn, textTableItem);
	TblSetRowUsable(table, row, true);
}


/***********************************************************************
 *
 * FUNCTION:    MainInitTableRow
 *
 * DESCRIPTION: This routine initialize a row in the to do list.
 *
 * PARAMETERS:  table      - pointer to the table of to do items
 *              row        - row number (first row is zero)
 *              recordNum  - the index of the record display in the row
 *              rowHeight  - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
void MainInitTableRow (TablePtr table, Word row, Word recordNum, SWord rowHeight)
{

	// Make the row usable.
	TblSetRowUsable (table, row, true);
	
	// Set the height of the row to the height of the description.
	TblSetRowHeight(table, row, rowHeight);
	
	// Store the record number as the row id.
	TblSetRowID(table, row, recordNum);
	
	// Mark the row invalid so that it will drawn when we call the 
	// draw routine.
	TblMarkRowInvalid (table, row);
}

/***********************************************************************
 *
 * FUNCTION:    MainLoadTable
 *
 * DESCRIPTION: This routine loads memo database records into
 *              the list view form.
 *
 * PARAMETERS:  recordNum index of the first record to display.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/16/95		Initial Revision
 *
 ***********************************************************************/
void MainLoadTable(Boolean fillTable)
{
	Word row;
	Word height;
	Word numRows;
	Word recordNum;
	Word lastRecordNum;
	SWord dataHeight;
	SWord tableHeight;
	Word columnWidth;
	TablePtr table;
	Boolean rowsInserted = false;
	Boolean lastItemClipped;
	RectangleType r;

	table = GetObjectPtr(MainRollsTable);

	// Make sure the global variable that holds the index of the 
	// first visible reocord has a valid value.
	if (! SeekRecord(&gTopVisibleRecord, 0, dmSeekForward))
		if (! SeekRecord(&gTopVisibleRecord, 0, dmSeekBackward))
			gTopVisibleRecord = 0;
	
	// If we have a currently selected record, make sure that it is not
	// above the first visible record.
	if (gCurrentRoll != kNoRecordSelected)
		if (gCurrentRoll < gTopVisibleRecord)
			gCurrentRoll = gTopVisibleRecord;

	// Get the height of the table and the width of the description
	// column.
	table = GetObjectPtr(MainRollsTable);
	TblGetBounds(table,&r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth(table, MainCaptionColumn);
	
	row = 0;
	dataHeight = 0;
	recordNum = gTopVisibleRecord;
	lastRecordNum = recordNum;
	
	while (true)
	{
		if (! SeekRecord(&recordNum, 0, dmSeekForward))
			break;
			
		height = MainGetDescriptionHeight(recordNum, columnWidth);
		dataHeight += height;
		
		// If the record is not already being displayed in the current 
		// row load the record into the table.
		if ((TblGetRowID(table, row) != recordNum) || (! TblRowUsable (table, row)))
		{
			MainInitTableRow (table, row, recordNum, height);
		}
		else if (TblGetRowHeight (table, row) != height)
		{
			TblSetRowHeight (table, row, height);
			TblMarkRowInvalid (table, row);
		}

		lastRecordNum = recordNum;
		row++;
		recordNum++;

		// Is the table full?
		if (dataHeight >= tableHeight)		
		{
			// If we have a currently selected record, make sure that it is
			// not below  the last visible record.
			if ((gCurrentRoll == kNoRecordSelected) || (gCurrentRoll <= lastRecordNum))
				break;

			gTopVisibleRecord = recordNum;
			row = 0;
			dataHeight = 0;
		}
	}


	// Hide the items that don't have any data.
	numRows = TblGetNumberOfRows (table);
	while (row < numRows)
	{		
		TblSetRowUsable (table, row, false);
		row++;
	}
		
	// If the table is not full and the first visible record is 
	// not the first record	in the database, displays enough records
	// to fill out the table.
	while (dataHeight < tableHeight)
	{
		if (! fillTable) 
			break;
			
		recordNum = gTopVisibleRecord;
		if ( ! SeekRecord (&recordNum, 1, dmSeekBackward))
			break;

		// Compute the height of the to do item's description.
		height = MainGetDescriptionHeight (recordNum, columnWidth);
			
		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;
		
		// Insert a row before the first row.
		TblInsertRow (table, 0);

		MainInitTableRow (table, 0, recordNum, height);
		
		gTopVisibleRecord = recordNum;
		
		rowsInserted = true;

		dataHeight += height;
	}
		
	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		TblMarkTableInvalid (table);

	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clipped and the 
	// table is scrollable.
	lastItemClipped = (dataHeight > tableHeight);

	// Update the scroll arrows.
	MainUpdateScrollers(lastRecordNum, lastItemClipped);
}



/***********************************************************************
 *
 * FUNCTION:    MainScroll
 *
 * DESCRIPTION: This routine scrolls the list of rolls
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
void MainScroll(WinDirectionType direction, Boolean oneLine)
{
	Word				row;
	Word				index;
	Word				height;
	Word 				recordNum;
	Word 				columnWidth;
	Word 				tableHeight;
	TablePtr 		table;
	RectangleType	r;

	MainClearEditState();

	table = GetObjectPtr(MainRollsTable);

	TblGetBounds(table, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth(table, MainCaptionColumn);
	

	// Scroll the table down.
	if (direction == winDown)
	{
		if (oneLine)
		{
			recordNum = gTopVisibleRecord;
			SeekRecord (&recordNum, 1, dmSeekForward);
		}
		else
		{
			// Get the record index of the last visible record.  A row 
			// number of minus one indicates that there are no visible rows.
			row = TblGetLastUsableRow (table);
			if (row == -1) return;
			
			recordNum = TblGetRowID (table, row);				
	
			// If there is only one record visible, this is the case 
			// when a record occupies the whole screeen, move to the 
			// next record.
			if (row == 0)
				SeekRecord (&recordNum, 1, dmSeekForward);
		}
	}

	// Scroll the table up.
	else
	{
		// Scan the records before the first visible record to determine 
		// how many record we need to scroll.  Since the heights of the 
		// records vary,  we sum the heights of the records until we get
		// a screen full.
		recordNum = TblGetRowID(table, 0);
		height = TblGetRowHeight(table, 0);
		if (height >= tableHeight)
			height = 0;

		while (height < tableHeight)
		{
			index = recordNum;
			if ( ! SeekRecord (&index, 1, dmSeekBackward) ) break;
			height += MainGetDescriptionHeight (index, columnWidth);
			if ((height <= tableHeight) || (recordNum == TblGetRowID (table, 0)))
				recordNum = index;
			if (oneLine) break;
		}
	}

	TblMarkTableInvalid (table);
	gTopVisibleRecord = recordNum;
	MainLoadTable (!oneLine);	

	TblUnhighlightSelection (table);
	TblRedrawTable (table);
}


/***********************************************************************
 *
 * FUNCTION:   MainGetRollCaption
 *
 * DESCRIPTION: This routine returns a pointer to the roll caption field.
 *              This routine is called by the table  object as a callback
 *              routine when it wants to display or edit a caption
 *
 * PARAMETERS:  table  - pointer to the to do list table (TablePtr)
 *              row    - row of the table
 *              column - column of the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
Err MainGetRollCaption(VoidPtr table, Int16 row, Int16 column,
	Boolean editable, MemHandle *textH, Int16 *textOffset, Int16 *textAllocSize, FieldPtr fld)
{
	#pragma unused(column, editable)
	Word recordNum;
	VoidHand recordH;
	FieldAttrType attr;
	RollHeaderPackedPtr rollP;
	
	// Get the record number that corresponds to the table item.
	// The record number is stored as the row id.
	recordNum = TblGetRowID(table, row);
	recordH = DmQueryRecord(gMemoDatabaseRef, recordNum);
	ErrFatalDisplayIf ((! recordH), "Record not found");
	
	rollP = (RollHeaderPackedPtr) MemHandleLock(recordH);

	*textOffset = GetCaptionOffset(rollP);
	*textAllocSize = StrLen((CharPtr) rollP + *textOffset) + 1;
	*textH = recordH;
	
	MemHandleUnlock(recordH);

	if (gPalmOS2 && fld)
	{
		FldGetAttributes (fld, &attr);
		attr.autoShift = true;
		FldSetAttributes (fld, &attr);
	}

	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    MainSaveRollCaption
 *
 * DESCRIPTION: This routine is called by the table object, as a callback  
 *              routine, when it wants to save a roll caption.
 *              The description is edited in place (directly in the database 
 *              record),  so we don't need to save it here,  we do however
 *              want to capture the current edit state.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw 
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
Boolean MainSaveRollCaption (VoidPtr table, Int16 row, Int16 column)
{
	#pragma unused(column)
	Word recordNum;
	Boolean dirty;
	FieldPtr fld;
	
	// Get the record number that corresponds to the table item to save.
	recordNum = TblGetRowID(table, row);

	// If the description has been modified mark the record dirty, any 
	// change make to the to do's description were written directly
	// to the to do record.
	fld = TblGetCurrentField(table);
	dirty = FldDirty(fld);
	FldCompactText(fld);
	if (dirty)
		DirtyRecord(recordNum);

	// Save the dirty state, we're need it if we auto-delete an empty record.
	gRecordDirty = dirty;

	// Check if the top of the description is scroll off the top of the 
	// field, if it is then redraw the field.
	if (FldGetScrollPosition (fld))
	{
		FldSetSelection(fld, 0, 0);
		FldSetScrollPosition(fld, 0);
	}

	return (false);
}



/***********************************************************************
 *
 * FUNCTION:    MainGetDescriptionHeight
 *
 * DESCRIPTION: This routine returns the height, in pixels, of a to do 
 *              description.
 *
 * PARAMETERS:  recordNum - record index
 *              width     - width of description
 *
 * RETURNED:    height in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
Word MainGetDescriptionHeight (Word recordNum, Word width)
{
	Word 			height;
	FontID 		curFont;
	VoidHand 	recordH;
	RollHeaderPackedPtr recordP;
	
	// Get a pointer to the to do record.
	recordH = DmQueryRecord(gMemoDatabaseRef, recordNum);
	ErrFatalDisplayIf ((! recordH), "Record not found");
	
	recordP = (RollHeaderPackedPtr) MemHandleLock(recordH);

	// Compute the height of the to do item's description.
	curFont = FntSetFont(stdFont);

	height = FldCalcFieldHeight((CharPtr) recordP + GetCaptionOffset(recordP), width);
	height = MIN(height, maxFieldLines);
	height *= FntLineHeight();
	
	FntSetFont(curFont);

	MemHandleUnlock(recordH);

	return (height);
}


/***********************************************************************
 *
 * FUNCTION:    MainItemSelected
 *
 * DESCRIPTION: This routine is called when an item in the roll list
 *              is selected.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void MainItemSelected(EventPtr event)
{
	Word row;
	Word column;
	TablePtr table;
	
	table = event->data.tblSelect.pTable;
	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;

	gCurrentRoll = TblGetRowID(table,row);

	if (column == MainRollColumn)
		FrmGotoForm(EditRollForm);
	else if (column == MainCaptionColumn)
		gCurrentRoll = TblGetRowID(table, row);
	
}


/***********************************************************************
 *
 * FUNCTION:    MainResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of an item's
 *              description is changed as a result of user input.
 *              If the new hieght of the field is shorter,  more items
 *              may need to be added to the bottom of the list.
 *              
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/18/95		Initial Revision
 *
 ***********************************************************************/
void MainResizeDescription(EventPtr event)
{
	Word lastRow;
	Word lastRecord;
	Word topRecord;
	FieldPtr fld;
	TablePtr table;
	Boolean lastItemClipped;
	RectangleType itemR;
	RectangleType tableR;
	RectangleType fieldR;
	
	// Get the current height of the field;
	fld = event->data.fldHeightChanged.pField;
	FldGetBounds(fld, &fieldR);

	// Have the table object resize the field and move the items below
	// the field up or down.
	table = GetObjectPtr(MainRollsTable);
	TblHandleEvent(table, event);
	
	
	// If the field's height has expanded , and there are no items scrolled
	// off the top of the table, just update the scrollers.
	if (event->data.fldHeightChanged.newHeight >= fieldR.extent.y)
	{
		topRecord = TblGetRowID (table, 0);
		if (topRecord != gTopVisibleRecord)
			gTopVisibleRecord = topRecord;
		else
		{
			// Update the scroll arrows.
			lastRow = TblGetLastUsableRow (table);
			TblGetBounds (table, &tableR);
			TblGetItemBounds (table, lastRow, MainCaptionColumn, &itemR);
			lastItemClipped = (itemR.topLeft.y + itemR.extent.y > tableR.topLeft.y + tableR.extent.y);
			lastRecord = TblGetRowID (table, lastRow);
			MainUpdateScrollers(lastRecord, lastItemClipped);

			return;
		}
	}
		
	// Add items to the table to fill in the space made available by the 
	// shortening the field.
	MainLoadTable(false);
	TblRedrawTable (table);
}


/***********************************************************************
 *
 * FUNCTION:    MainClearEditState
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
void MainClearEditState (void)
{
	gCurrentRoll = kNoRecordSelected;
	TblReleaseFocus(GetObjectPtr(MainRollsTable));
}
