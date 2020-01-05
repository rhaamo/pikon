/*
	File:		BuddyDatabase.c

	Contains:	Access/utility routines for the memo database

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <1+>    7/5/2001    ksh     Update for PalmOS 3.5 headers
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/



#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <StringMgr.h>
#include "BuddyDatabase.h"

Int GetCameraOffset(RollHeaderPackedPtr roll)
{
	#pragma unused(roll)
	return(kRollHeaderPackedSize);
}

Int GetCaptionOffset(RollHeaderPackedPtr roll)
{
	unsigned char	*p;
	
	p = (unsigned char *) roll + GetCameraOffset(roll);
	p += StrLen((const char *) p) + 1;
	
	return(p - (unsigned char *) roll);
}

Int GetFrameOffset(RollHeaderPackedPtr roll, Int frame)
{
	unsigned char *p;
	
	p = (unsigned char *) roll + GetCaptionOffset(roll);
	p += StrLen((const char *) p) + 1;
	
	while (frame--)
	{
		p += kFrameHeaderPackedSize;
		p += StrLen((const char *) p) + 1;
	}
	return(p - (unsigned char *) roll);
}

Int GetFrameCaptionOffset(FrameHeaderPackedPtr frame)
{
	#pragma unused(frame)
	return(kFrameHeaderPackedSize);
}

Int GetFrameByOffset(RollHeaderPackedPtr recordP, Word offset)
{
	Int			frame = 0;
		
	while (offset > GetFrameOffset(recordP, frame))
		frame++;
	frame--;
		
	return(frame);
}

Word GetRollSize(RollHeaderPackedPtr recordP)
{
	return(GetFrameOffset(recordP, recordP->frameCount));
}

Err InsertCaptionHandle(DmOpenRef dbid, UInt index, SWord frame, VoidHand text)
{
	CharPtr	caption;
	SWord		captionSize;
	Word		captionOffset;
	Word		recordSize;
	VoidHand	recordH;
	RollHeaderPackedPtr	recordP;
	Err		err = 0;
	
	recordH = DmGetRecord(dbid, index);
	if (recordH == NULL)
		err = dmErrCantFind;
	if (err) goto ERROR;
	
	recordP = MemHandleLock(recordH);
	
	recordSize = GetRollSize(recordP);
	
	if (frame >= 0)
	{
		captionOffset = GetFrameOffset(recordP, frame);
		captionOffset += GetFrameCaptionOffset((FrameHeaderPackedPtr) ((CharPtr) recordP + captionOffset));
	}
	else
		captionOffset = GetCaptionOffset(recordP);

	caption = (CharPtr) recordP + captionOffset;
	captionSize = StrLen(caption);

	MemHandleUnlock(recordH);
	DmReleaseRecord(dbid, index, false);
	
	// Remove the old caption
	err = DmRemoveBytes(dbid, index, captionOffset, captionSize, &recordSize);
	if (err) goto ERROR;
	
	// insert the new caption
	caption = MemHandleLock(text);
	captionSize = StrLen(caption);
	err = DmInsertBytes(dbid, index, captionOffset, caption, captionSize, &recordSize);
	MemHandleUnlock(text);

ERROR:
		
	return(err);
}

Err DmRemoveBytes(DmOpenRef dbP, UInt index, Word offset, Word removeBytes, Word *recordSize)
{
	VoidHand recordH;
	VoidPtr	recordP;
	Err		err = 0;

	recordH = DmGetRecord(dbP, index);
	if (recordH == NULL)
		err = dmErrCantFind;
	if (err) goto NO_RECORD;
	
	recordP = MemHandleLock(recordH);
	err = DmWrite(recordP, offset, (CharPtr) recordP + offset + removeBytes, *recordSize - offset - removeBytes);
	MemHandleUnlock(recordH);
	if (err) goto ERROR;
	
	*recordSize -= removeBytes;
	DmResizeRecord(dbP, index, *recordSize);
	
ERROR:
	DmReleaseRecord(dbP, index, true);
	
NO_RECORD:
	return(err);

}

Err DmInsertBytes(DmOpenRef dbP, UInt index, Word offset, CharPtr data, Word dataBytes, Word *recordSize)
{
	VoidHand recordH;
	VoidPtr	recordP;
	Err		err = 0;
	
	recordH = DmResizeRecord(dbP, index, *recordSize + dataBytes);
	if (recordH == NULL)
		err = dmErrCantFind;
	if (err) goto NO_RECORD;
	
	recordP = MemHandleLock(recordH);
	
	// shift the old data up to make room for the new
	err = DmWrite(recordP, offset + dataBytes, (CharPtr) recordP + offset, *recordSize - offset);
	if (err) goto ERROR;
	
	// write in the new data
	err = DmWrite(recordP, offset, data, dataBytes);
	if (err) goto ERROR;
	*recordSize += dataBytes;

	
ERROR:
	MemHandleUnlock(recordH);
	DmReleaseRecord(dbP, index, true);
	
NO_RECORD:
	return(err);
	
}
