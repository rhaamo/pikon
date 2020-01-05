/*
	File:		nikonIF.c

	Contains:	serial-level routines for camera communication

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

       <13+>    7/5/2001    ksh     Update for PalmOS 3.5 headers
        <13>     2/16/99    ksh     Add support for logging to memo pad
        <12>     2/16/99    ksh     Only call SysTicksPerSecond if PalmOS >= 2
        <11>     6/30/98    ksh     Add call for returning session error
        <10>     3/25/98    ksh     Make sure serialRefNum is set to zero on EndSession
         <9>     3/25/98    ksh     Fix expected return in EndSession when gSerialEnabled == false
         <8>     3/25/98    ksh     Update EndSession to return an error
         <7>     3/25/98    ksh     Terminate SendCommand if a session error exists
         <6>     3/25/98    ksh     Have EndSession return session error
         <5>     2/18/98    ksh     Update sysTicksPerSecond to SysTicksPerSecond
         <4>     2/15/98    KSH     Update to handle N90 protocol
         <3>     2/14/98    KSH     Update for N90/F90
         <2>     1/31/98    KSH     Remove call to SysTaskDelay when running on simulator
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/



//#define SERIALOFF

#include		<PalmOS.h>
#include		<SerialMgrOld.h>

#include "N90SErrors.h"
#include "N90SBuddyRsc.h"
#include "nikonPackets.h"
#include "nikonIF.h"

#if DEBUGBUILD
	#include "Debug.h"
#endif

extern Boolean gPalmOS2;

UInt		serialRefNum = 0;
Err			sessionErr;
Boolean		gSerialEnabled = true;
Boolean		gBaudChangeEnabled = true;

#define kCameraNameBufSize	16
#define kSerialBufSize	256
CameraType	gCameraType;
Char		gCameraName[kCameraNameBufSize];
UChar		serialBuffer[kSerialBufSize];

#define MicroSecondsToTicks(microSeconds)	\
		(((ULong)((ULong)(gPalmOS2 ? SysTicksPerSecond() : 100) * (microSeconds)) / 1000000L) + 1)

#define MilliSecondsToTicks(milliSeconds)	\
		(((ULong)((ULong)(gPalmOS2 ? SysTicksPerSecond() : 100) * (milliSeconds)) / 1000L) + 1)
		
#define Wait(x)	SysTaskDelay(MilliSecondsToTicks(x))

#define SET_ADDRESS(d,s)	\
	{ \
		UChar *dp; \
		dp = (UChar *)d;	\
		*dp++ = (UChar) (s>>16);	\
		*dp++ = (UChar) (s>>8);	\
		*dp++ = (UChar) s;	\
	}
	
#define kSerialTimeout			(MilliSecondsToTicks(2000))
#define kSerialFlushTimeout	(0)

#define prvFakeSerRefNum	1			// for Emulator

#define kMaxCommandData	0xEE

Err IdentifyCamera();
void MakeDataPacket(UChar * buf, Int size);
Err SlowWriteData(void *buf, Int size);
Err WriteData(void *buf, Int size);
Err ReadData(void * buf, Int	size);
Err ReadDataPacket(UChar * buf, Int size);
Err ReadStatusPacket();
Int MemCompare(void *s1, void *s2, ULong size);

Err StartSession()
{
	Err					err;
	SerSettingsType	settingsP;
	
	if (!gSerialEnabled) return  0;
	
	sessionErr = 0;
	serialRefNum = 0;

#if EMULATION_LEVEL == EMULATION_NONE
	err = SysLibFind("Serial Library",&serialRefNum);
	if (err) goto ERROR;
#else
	serialRefNum = prvFakeSerRefNum;
#endif

	err = SerOpen(serialRefNum, 0, 1200);
	if (err) goto ERROR;
	
	settingsP.baudRate = 1200;
	settingsP.flags = serSettingsFlagStopBits1 + serSettingsFlagBitsPerChar8;
	settingsP.ctsTimeout = 0;
	err = SerSetSettings(serialRefNum, &settingsP);
	if (err) goto ERROR;

	SerReceiveFlush(serialRefNum, kSerialFlushTimeout);
	
	err = IdentifyCamera();
	if (err) goto ERROR;
	
	if (gBaudChangeEnabled)
		SendCommand(kBaudChangeMode, 0, 0, 0);
	
ERROR:
	sessionErr = err;
	
	if (sessionErr)
		EndSession();
		
	return(err);
}


Err FireShutterSession()
{
	Err					err;
	SerSettingsType	settingsP;
	
	if (!gSerialEnabled) return  0;
	
	sessionErr = 0;
	serialRefNum = 0;

#if EMULATION_LEVEL == EMULATION_NONE
	err = SysLibFind("Serial Library",&serialRefNum);
	if (err) goto ERROR;
#else
	serialRefNum = prvFakeSerRefNum;
#endif

	err = SerOpen(serialRefNum, 0, 1200);
	if (err) goto ERROR;
	
	settingsP.baudRate = 1200;
	settingsP.flags = serSettingsFlagStopBits1 + serSettingsFlagBitsPerChar8;
	settingsP.ctsTimeout = 0;
	err = SerSetSettings(serialRefNum, &settingsP);
	if (err) goto ERROR;

#if 0	
	err = IdentifyCamera();
	if (err) goto ERROR;
	
	if (gBaudChangeEnabled)
		SendCommand(kBaudChangeMode, 0, 0, 0);
#endif

	// Send at 1200 baud so we don't waste time switching
	SendCommand(kShutterMode, 0, 0, 0);	
ERROR:
	sessionErr = err;
	
	if (sessionErr)
		EndSession();
		
	return(err);
}

Err GetSessionError()
{
	return(sessionErr);
}

Err EndSession()
{
	SignoffPacket	sp;
	Err				err = 0;
	Boolean			retry;
	
	if (!gSerialEnabled) return 0;
	
	if (serialRefNum)
	{

RETRY_SIGNOFF:
		sp.signoffWord = 0x0404;

		err = WriteData(&sp, kSignoffPacketSize);
		err = ReadData(&sp, kSignoffPacketSize);

		if ((err || (sp.signoffWord != 0x0404)) && (retry == false))
		{
			retry = true;
			goto RETRY_SIGNOFF;
		}
		
		SerReceiveFlush(serialRefNum,kSerialFlushTimeout);
		SerClose(serialRefNum);
	}
	
	serialRefNum = 0;
	Wait(200);

	return (sessionErr);
}

Err IdentifyCamera()
{
	Err		err;
	Boolean	retry = false;
	Boolean done;
	UChar	*p;
	
	if (sessionErr)
		return sessionErr;
		
	if (SlowWriteData(&kNullString, 1))
		goto ERROR;
	
	Wait(200);

	
RETRY_QUERY:

	done = false;
	if (SlowWriteData(kQueryString,kQueryStringSize))
		goto ERROR;
	
	p = serialBuffer;
	while ((done == false) &&
			(err = ReadData(p, 1)) == 0)
	{
		if (*p++ == 0)
		{
			err = ReadData(p, 2);
			done = true;
		}
	}
			
	if (err)
	{		
		if (retry == false)
		{
			retry = true;
			goto RETRY_QUERY;
		}
		
		if (err == kPacketSizeErr)
			err = kWrongCameraErr;
			
		goto ERROR;
	}
	
#if 0	
	if ((err = ReadData(serialBuffer, sizeof(kN90sResponseString))) != 0)
	{
		if (retry == false)
		{
			retry = true;
			goto RETRY_QUERY;
		}
		if (err == kPacketSizeErr)
			err = kWrongCameraErr;
			
		goto ERROR;
	}
#endif

	if (StrLen((Char *) &(serialBuffer[4])) < kCameraNameBufSize)
	{
		StrCopy(gCameraName, (Char *) &(serialBuffer[4]));
		if (MemCompare(serialBuffer, kN90sResponseString, sizeof(kN90sResponseString)) == 0)
			gCameraType = cameraN90s;
		else if (MemCompare(serialBuffer, kN90ResponseString, sizeof(kN90ResponseString)) == 0)
			gCameraType = cameraN90;
		else
			err = kWrongCameraErr;
	}
	else
		err = kWrongCameraErr;

		
ERROR:

	sessionErr = err;
	return(err);
}

Err SendCommand(Int mode, ULong address, void *buf, Int size)
{
	Int	partial;
	Err	err = 0;
	
	if (!gSerialEnabled) return  0;
	
	if (sessionErr) return sessionErr;
	
	do
	{
		partial = size;
		if (size > kMaxCommandData)
			partial = kMaxCommandData;
			
		err = SendCommandLoop(mode, address, buf, partial);
		
		if (err)
			goto ERROR;
			
		size -= partial;
		(UChar *) buf += partial;
		address += partial;
		
	} while (size > 0);
	
ERROR:
	sessionErr = err;
	return(err);
}

Err SendCommandLoop(Int	mode, ULong address, void* buf, Int size)
{
	CommandPacket	cp;
	Char			retry;
	Boolean		slow = false;
	Err			err = 0;
	UChar	*cmdBuf;
	
	if (sessionErr)
		return sessionErr;
	
	cmdBuf = (UChar *) buf;
	
	retry = false;
	
	cp.startMark = kCommandStartMark;
	if (gCameraType == cameraN90s)
		cp.commandFlag = kCommandN90SCommandFlag;
	else
		cp.commandFlag = kCommandN90CommandFlag;
	cp.modeFlag = mode;
	cp.stopMark = kCommandStopMark;
	cp.length = 0;
	SET_ADDRESS(&cp.address,address);


COMMAND_RETRY:

	switch(mode)
	{
		case kReadDataMode:
			SET_ADDRESS(&cp.address, address);
			cp.length = size;
			err = WriteData(&cp,kCommandPacketSize);
			break;
			
		case kWriteDataMode:
			SET_ADDRESS(&cp.address, address);
			cp.length = size;
			MakeDataPacket(cmdBuf,size);
			err = WriteData(&cp,kCommandPacketSize - 1);		// leave off stop bit
			err = WriteData(serialBuffer,kDataPacketStartSize + 
									size + kDataPacketStopSize);
			break;
		
		case kShutterMode:
		case kFocusMode:
			err = WriteData(&cp,kCommandPacketSize);
			break;
			
		case kBaudChangeMode:
			SET_ADDRESS(&cp.address, kBaud9600Address);
			err = SlowWriteData(&cp, kCommandPacketSize);
			break;
		
		case kMemoHolderMode:
			SET_ADDRESS(&cp.address, kMemoHolderAddress);
			err = WriteData(&cp, kCommandPacketSize);
			break;
			
		default:
			err = kUnknownModeErr;
			break;
	}
		
	if (err)
		goto ERROR;
			
	switch(mode)
	{
		case kReadDataMode:
			err = ReadDataPacket(cmdBuf, size);
			break;
			
		case kWriteDataMode:
		case kBaudChangeMode:
			err = ReadStatusPacket();
			break;
		
		case kShutterMode:
		case kFocusMode:
			//BOZO;
			break;
		
		case kMemoHolderMode:
			err = ReadDataPacket(cmdBuf, kMemoHolderResponseSize);
			break;
			
		default:
			err = kUnknownModeErr;
			break;
	}
	
	if (err && (retry == false))
	{
		retry = true;
		goto COMMAND_RETRY;
	}
	
	if (err)
		goto ERROR;
		
	if (mode == kBaudChangeMode)
	{
		SerSettingsType	settingsP;
		
		settingsP.baudRate = 9600;
		settingsP.flags = serSettingsFlagStopBits1 + serSettingsFlagBitsPerChar8;
		settingsP.ctsTimeout = 0;
		err = SerSetSettings(serialRefNum, &settingsP);
		if (err)
			goto ERROR;
		
		Wait(200);
	}
	
ERROR:
	sessionErr = err;
	return(err);
}

void MakeDataPacket(UChar * buf, Int size)
{
	UChar	*p;
	UChar	cs;
	UInt				count;
	DataPacketStart	dstart;
	DataPacketStop	dstop;
	
	count = size;
	p = (UChar *) buf;
	cs = 0;
	while (count--)
		cs += *p++;
		
	dstart.startMark = kDataPacketStartMark;
	dstop.checkByte = cs;
	dstop.stopMark = kDataPacketStopMark;
	
	p = (UChar *) serialBuffer;
	MemMove(p, &dstart, kDataPacketStartSize);	p+= kDataPacketStartSize;
	MemMove(p, buf, size);								p+= size;
	MemMove(p, &dstop, kDataPacketStopSize);
}

Err WriteData(void *buf, Int size)
{
	Err	err;
	
	if (sessionErr)
		return sessionErr;
	
	SerReceiveFlush(serialRefNum, 0);
	err = SerSend10(serialRefNum, buf, size);

#if DEBUGBUILD
	{
		Char	output[800];
		
		if (gMemoDebug.memoRecP)
		{
			if (size < 220)
			{
				DataToHex(buf, size, output, NULL);
				DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, output);
				gMemoDebug.memoOffset += StrLen(output);
				DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, "\n");
				gMemoDebug.memoOffset += StrLen("\n");
			}
		}
	}
#endif

ERROR:
	if (err)
		sessionErr = err;

	return(err);
}

Err SlowWriteData(void *buf, Int size)
{
	Err	err;
	UInt	i;
	Char	*p;

	if (sessionErr)
		return sessionErr;
	
	SerReceiveFlush(serialRefNum, 0);
	
	p = (Char *) buf;
	for (i = 0; i < size; i++)
	{
	
#if EMULATION_LEVEL != EMULATION_NONE
		SysTaskDelay(1);
#endif

		err = SerSend10(serialRefNum, p, 1);
		if (err)
			goto ERROR;
		p++;
	}

ERROR:
	if (err)
		sessionErr = err;

	return(err);
}

Err ReadData(void *buf, Int size)
{
	ULong			byteCount = 0;
	Err			err;
	
	if (sessionErr)
		return sessionErr;

	if (size > kSerialBufSize)
		return kPacketTooLargeErr;

	err = SerReceive10(serialRefNum, buf, size, kSerialTimeout);

	if (err)
	{
		SerReceiveCheck(serialRefNum,&byteCount);
		
		if ((byteCount > 0) && (byteCount != size))
			err = kPacketSizeErr;
		SerReceiveFlush(serialRefNum,kSerialFlushTimeout);
	}


#if DEBUGBUILD
	if (err == 0)
	{
		Char	output[800];
		
		if (gMemoDebug.memoRecP)
		{
			if (size < 220)
			{
				DataToHex(buf, size, output, "     ");
				DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, output);
				gMemoDebug.memoOffset += StrLen(output);
				DmStrCopy(gMemoDebug.memoRecP, gMemoDebug.memoOffset, "\n");
				gMemoDebug.memoOffset += StrLen("\n");
			}
		}
	}
#endif
	
	return err;
}

Err ReadDataPacket(UChar * buf, Int size)
{
	Err			err;
	UChar *	p;
	UChar	cs;
	Int			count;
			
	err = ReadData(serialBuffer, size + kDataPacketStartSize + kDataPacketStopSize);
	
	if (err)
		goto ERROR;
		
	p = serialBuffer;
	if (((DataPacketStart *) p)->startMark != kDataPacketStartMark)
		goto ERROR;
	p += kDataPacketStartSize;
	cs = 0;
	count = size;
	while (count--)
	{
		cs += *p;
		*buf++ = *p++;
	}
	if (((DataPacketStop *) p)->checkByte != cs)
		err = kPacketCSErr;
	else if (((DataPacketStop *) p)->stopMark != kDataPacketStopMark)
		err = kPacketSizeErr;
	
	if (err)
		goto ERROR;

	return(err);

ERROR:
	return err;
}

Err ReadStatusPacket()
{
	StatusPacket		ep;
	Err				err;
	
	err = ReadData(&ep, kStatusPacketSize);
	
	if (err)
		goto ERROR;
	
	if (ep.status != kStatusOK)
		err = kPacketResponseErr;

ERROR:
		
	return(err);
}


Int	MemCompare(void *s1, void *s2, ULong size)
{
	const UChar *p1;
	const UChar *p2;
	
	for (p1 = (const UChar *) s1, p2 = (const UChar *) s2, size++; --size ;)
		if (*p1++ != *p2++)
			return((*--p1 < *--p2) ? -1 : +1);
	return (0);
}

#if EMULATION_LEVEL == EMULATION_MAC
Boolean YieldTime();
Boolean YieldTime()
{
	/*
	 *	Our serial packets are always short enough that we
	 *	won't quit out from under a serial transaction
	 */
	 
	return(false);
}
#endif
