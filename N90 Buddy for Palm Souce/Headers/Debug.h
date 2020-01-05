/*
	File:		Debug.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

         <2>     2/16/99    ksh     Add support for logging to memo pad
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				Debug.h
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

#if ERROR_CHECK_LEVEL != ERROR_CHECK_NONE

typedef struct MemoDebug
{
	DmOpenRef	memoRef;
	UInt			memoIndex;
	UInt			memoOffset;
	VoidHand		memoRecH;
	VoidPtr		memoRecP;
} MemoDebug;

extern MemoDebug	gMemoDebug;

void		DebugStartLog(void);
void		DebugStopLog(void);
Boolean	DebugFormHandleEvent(EventPtr event);
Err 		AddSampleRolls();
void 		DeleteAllRolls();
Err 		ResetMemoHolder();
Boolean	DebugFormHandleEvent(EventPtr event);
Err		Peek();
Err		Poke();
void		HexToData(CharPtr input, UChar *output, Word *byteCount);
void		DataToHex(UChar *input, Word count, CharPtr output, CharPtr prefix);
Long		TextToNum(CharPtr text);
void		DebugToggleSerial(void);
void 		DebugToggleBaud(void);
void		FixDebugMenu(void);

#endif
