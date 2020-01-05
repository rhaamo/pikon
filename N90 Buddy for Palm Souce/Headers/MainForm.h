/*
	File:		MainForm.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <2+>   1/23/2002    ksh     Update for PalmOS 3.5 headers
         <2>     1/26/98    KSH     Fix formatting
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				MainForm.h
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

Boolean MainFormHandleEvent(EventPtr event);
Boolean MainHandleMenu(Word item);
Err 	MainFormInit();
void	MainDrawRollNumber(VoidPtr table, Int16 row, Int16 column, RectangleType *bounds);
void 	MainScroll(WinDirectionType direction, Boolean oneLine);
void 	MainLoadTable(Boolean fillTable);
void 	MainUpdateScrollers(Word bottomRecord, Boolean lastItemClipped);
void	MainLoadTableItem (TablePtr table, Word recordNum, Word row);
void	MainInitTableRow (TablePtr table, Word row, Word recordNum, SWord rowHeight);
void 	MainScroll(WinDirectionType direction, Boolean oneLine);
Boolean	MainSaveRollCaption (VoidPtr table, Int16 row, Int16 column);
Err	MainGetRollCaption(VoidPtr table, Int16 row, Int16 column,
	Boolean editable, MemHandle *textH, Int16 *textOffset, Int16 *textAllocSize, FieldPtr fld);
Word 	MainGetDescriptionHeight (Word recordNum, Word width);
void	MainItemSelected (EventPtr event);
void 	MainResizeDescription(EventPtr event);
void	MainClearEditState (void);
