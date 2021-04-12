/*
	File:		NBuddyPDBFormat.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#pragma 

//	NOTE: These are 68K aligned!
//	Use macros below for x86 compatiblity

typedef struct {
	unsigned long	timeDate;
	unsigned short	rollNumber;
	unsigned char	iso;
	unsigned char	storageLevel;
	unsigned char	frameCount;
//	followed by CameraString
//	followed by CaptionString
} RollHeaderPackedRec, *RollHeaderPackedPtr;
#define kRollHeaderPackedSize	9

typedef struct {
	unsigned char	shutterIndex;
	unsigned char	apertureIndex;
	unsigned char	flashIndex:2;
	unsigned char	meterIndex:2;
	unsigned char	exposureIndex:4;
	unsigned char	focalIndex;
	unsigned char	expCompIndex;
	unsigned char	flashCompIndex;
} FrameHeaderPackedRec, *FrameHeaderPackedPtr;
#define kFrameHeaderPackedSize	6

#define FHGetShutterIndex(frame)	(((unsigned char *) frame)[0])
#define FHGetApertureIndex(frame)	(((unsigned char *) frame)[1])
#define FHGetFlashIndex(frame)		(((unsigned char *) frame)[2] >> 6)
#define FHGetMeterIndex(frame)		(((unsigned char *) frame)[2] >> 4 & 0x03)
#define FHGetExposureIndex(frame)	(((unsigned char *) frame)[2] & 0x0F)
#define FHGetFocalIndex(frame)		(((unsigned char *) frame)[3])
#define FHGetExpCompIndex(frame)	(((unsigned char *) frame)[4])
#define FHGetFlashCompIndex(frame)	(((unsigned char *) frame)[5])

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif
