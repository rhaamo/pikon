/*
	File:		BuddyDatabase.h

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
 * FILE:				BuddyDatabase.h
 *
 * DESCRIPTION:	Database Record Definitions
 *	  
 * REVISION HISTORY:
 * 	 9/8/96	ksh		Initial version
 *
 **********************************************************************/
#pragma once

#include <PalmCompatibility.h>
#include <DataMgr.h>
#include "NBuddyPDBFormat.h"

Int GetCameraOffset(RollHeaderPackedPtr roll);
Int GetCaptionOffset(RollHeaderPackedPtr roll);
Int GetFrameOffset(RollHeaderPackedPtr roll, Int frame);
Int GetFrameCaptionOffset(FrameHeaderPackedPtr frame);
Int GetFrameByOffset(RollHeaderPackedPtr recordP, Word offset);
Word GetRollSize(RollHeaderPackedPtr recordP);
Err InsertCaptionHandle(DmOpenRef dbid, UInt index, SWord frame, VoidHand text);
Err DmRemoveBytes(DmOpenRef dbP, UInt index, Word offset, Word removeBytes, Word *recordSize);
Err DmInsertBytes(DmOpenRef dbP, UInt index, Word offset, CharPtr data, Word dataBytes, Word *recordSize);
