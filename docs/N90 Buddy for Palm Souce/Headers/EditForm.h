/*
	File:		EditForm.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <1+>    7/5/2001    ksh     Update for PalmOS 3.5 headers
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				EditForm.h
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

Err 		EditRollFormInit();
Err 		EditRollFormSave();
Boolean	EditRollFormHandleEvent(EventPtr event);
void		EditRollGoto(EventPtr event);
void 		EditRollUpdateScrollers();
void 		EditRollScroll(WinDirectionType direction);
void 		EditRollScrollRecord(WinDirectionType direction);
void 		EditRollInitFrameItems();
void 		EditRollClearEditState();
