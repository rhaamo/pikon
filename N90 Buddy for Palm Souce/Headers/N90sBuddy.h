/*
	File:		N90sBuddy.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <3+>    12/29/00    ksh     Add SET/CLEAR flags macro
         <3>     2/18/98    ksh     Update for 1.1b1
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				N90sBuddy.h
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

#include <PalmOS.h>				// all the system toolbox headers
#include <FeatureMgr.h>
#include "nikonIF.h"
#include "N90SBuddyRsc.h"		// application resource defines
#include "BuddyDatabase.h"
#include "N90SErrors.h"

#include "MainForm.h"
#include "MiscForms.h"
#include "EditForm.h"
#include "CameraControl.h"
#include "DateTimeLib.h"
#include "FormUtils.h"
#include "Debug.h"
#include "StringTables.h"

#define	kRollDigits	4

#define	kNBuddyCreator	'NBdy'
#define	kNBuddyDBType	'NBmh'
#define	kNBuddyDBName	"NBuddyMemoDB"

#define	kNBuddyOldDBName	"N90sMemoDB"

#define	kN90sBuddyVersion	1
#define	kNoRecordSelected	-1

#define offsetof(T, member)	((unsigned long)&(((T *)0)->member))

#define CLEAR_FLAGS(x,flags)	x &= ~(flags)
#define SET_FLAGS(x, flags)		x |= (flags)

typedef struct {
	UInt				nextSearchRoll;
	Int					nextSearchFrame;
	Boolean				lastSearchInFrame;
	Word				lastDownloadMark;
	SWord				lastSyncedRec;
	Word				version;
} BuddyPreferencesType;

extern Int				gCurrentView;
extern DmOpenRef		gMemoDatabaseRef;
extern UInt				gCurrentRoll;
extern Word				gCurrentFrame;
extern Word				gTopVisibleRecord;
extern Boolean			gRecordDirty;				// true if a record has been modified
extern ULong			gRollTime;
extern Boolean			gPalmOS2;

extern TimeFormatType	gTimeFormat;					// system preference
extern DateFormatType	gShortDateFormat;			// system preference

extern const UChar kMemoList[];
extern const UChar kMemoDisableList[];
extern const UChar kDualList[];
extern const UChar kFlashList[];

extern Char gCameraName[16];

void	SwapShort(Word *x);
Word	MemoSettings2Bytes(UChar setting);
Word	BCD2Short(Word bcd);
Word	Short2BCD(Word value);
void	Index2Value(const UChar *list, UChar index, UChar *value);
void	Value2Index(const UChar *list, UChar value, SWord *index);
void	SetFrameLabel(Word labelID, Word rsrcID, Word value1, Word value2);
Boolean SeekRecord(WordPtr indexP, Word offset, Word direction);
void	DirtyRecord (Word index);
void	PostError(Err err);

Int 	CompareRecords(RollHeaderPackedPtr r1, RollHeaderPackedPtr r2, Int other);
void	DrawRollNumber(Word rollNumber, SWord x, SWord y, SWord width);

Boolean	GetPreferences(BuddyPreferencesType *prefs);
void	SetPreferences(BuddyPreferencesType *prefs);

Err	 	StartApplication(void);
void	StopApplication(void);
void 	Search(FindParamsPtr findParams);
Boolean	SearchDoMatch(RollHeaderPackedPtr recordP, Int frameNum, 
						FindParamsPtr findParams, UInt recordNum, Word pos,
						UInt cardNo, LocalID dbID);
void 	GoToItem (GoToParamsPtr goToParams, Boolean launchingApp);

Boolean ApplicationHandleMenu(Word item);
Boolean ApplicationHandleEvent(EventPtr event);
void 	EventLoop(void);
