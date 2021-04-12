/*
	File:		N90sErrors.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

         <2>     3/25/98    ksh     Add new error for open serial port.
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:  Nikon N90S Buddy Application
 * FILE:     N90SErrors.h
 *
 * DESCRIPTION:
 *		Defined constants of the UI resources.
 *
 **********************************************************************/

#define	kWrongCameraErr		(appErrorClass | 0)
#define	kPacketSizeErr			(appErrorClass | 1)
#define	kPacketCSErr			(appErrorClass | 2)
#define	kPacketResponseErr	(appErrorClass | 3)
#define	kErrorPacketErr		(appErrorClass | 4)
#define	kNoMemoDataErr			(appErrorClass | 5)
#define	kUnknownDataErr		(appErrorClass | 6)
#define	kUnknownModeErr		(appErrorClass | 7)
#define	kPacketTooLargeErr	(appErrorClass | 8)
#define	kConvertDataErr		(appErrorClass | 9)
#define kSerialOpenErr		(appErrorClass | 10)

