/*
	File:		CameraControl.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <2+>      8/6/99    ksh     Add trap focus feature
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				CameraControls.h
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

Boolean CameraHandleMenu(Word item);
Boolean	CameraFormHandleEvent(EventPtr event);
Err 	CameraFormInit();
Err 	CameraFormClose();
Err		CameraUpdateInfo();
Err		CameraFocus();
Err		CameraShutter();
Err		CameraSetSettings();
void	SetLensLabel(Word labelID, UChar lensId, UChar focalMin, UChar focalMax, UChar apMin, UChar apMax);

Boolean	AutoSeqFormHandleEvent(EventPtr event);
Err 	AutoSeqFormInit();
Err 	AutoSeqFormSave();

Boolean	BracketFormHandleEvent(EventPtr event);
Err 	BracketFormInit();
Err 	BracketFormSave();

Boolean	FreezeFocusFormHandleEvent(EventPtr event);
Err 	FreezeFocusFormInit();
Err 	FreezeFocusFormSave();

Boolean	MExpFormHandleEvent(EventPtr event);
Err 	MExpFormInit();
Err 	MExpFormSave();
