/*
	File:		nikonIF.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <6+>      8/6/99    ksh     Add trap focus feature
         <6>     6/30/98    ksh     Add prototype for get session error
         <5>     3/25/98    ksh     Update EndSession to return an error
         <4>     2/18/98    ksh     Update for 1.1b1
         <3>     2/14/98    KSH     Update for N90/F90
         <2>      2/2/98    KSH     Add define for kAutoSequenceFlag
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


#pragma once

#include "nikonPackets.h"

extern Char 		gCameraName[16];
extern CameraType	gCameraType;
extern Boolean		gSerialEnabled;
extern Boolean		gBaudChangeEnabled;

Err StartSession();
Err GetSessionError();
Err EndSession();
Err FireShutterSession();
Err SendCommand(Int	mode, ULong address, void *buf, Int size);
Err SendCommandLoop(Int	mode, ULong address, void *buf, Int size);

//	MEMO HOLDER SETTINGS
extern const UChar kMemoList[];
#define	kMemoFullDownloadFlag	0x80
#define	kMemoImprintFrameFlag	0x40
#define	kMemoNoFilm				0x00

// CAMERA CONTROL FLAGS

#define kAutoSequenceFlag		0x10
#define	kCameraBracketFlag		0x20
#define	kFlashBracketFlag		0x40
#define	kMultiExpFlag			0x80

//	CAMERA OPTION SETTINGS
extern const UChar kDualList[];
extern const UChar kFlashList[];
#define	kReverseCommandFlag		0x01
#define	kLongExpTimeFlag		0x04
#define	kDXPriorityFlag			0x08

// VIEWFINDER OPTION SETTINGS
#define	kEasyCompensationFlag	0x04
#define	kCenterMeterDeltaFlag	0x08
#define	kFrameCounterFlag		0x20

// AF OPTION SETTINGS
#define	kAFContFocusFlag		0x10
#define	kAFContAFLockFlag		0x20
#define	kAFSingleReleaseFlag	0x80

// MISC OPTION SETTINGS (FD3A)
#define kFreezeFocusFlag		0x01
#define kAFSimulAFAEFlag		0x04
#define kCreativeSetFlag		0x20

// BEEP OPTION SETTINGS
#define	kBeepExpErrorFlag		0x01
#define	kBeepFocusFlag			0x02
#define	kBeepFilmErrorFlag		0x04
#define	kBeepTimerFlag			0x08

#define kN90BeepBlurFlag		0x01
#define kN90FocusFlag			0x02

// FLASH FLAGS
#define kFlashReadyFlag			0x02
#define kFlashAttachedFlag		0x80
