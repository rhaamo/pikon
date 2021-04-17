/*
	File:		StringTables.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright � Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <3+>   1/24/2002    ksh     Update for Flash Compensation
         <3>     2/15/98    KSH     Add shutter string table for N90
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/***********************************************************************
 *
 *	Copyright � 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				StringTables.h
 *	  
 * REVISION HISTORY:
 * 	 5/7/97	ksh		Migrated from FormUtils.h
 *
 **********************************************************************/

#ifndef NIKON_STRING_TABLES_H
#define NIKON_STRING_TABLES_H

#define kNoFlashIndex	3

extern const char * kISOTable[];

extern const char * kShutterSpeedTable[];
#define kShutterSpeedMin	0x04
#define kShutterSpeedMax	0x4C

extern const char * kApertureTable[];
extern const char * kExposureModeTable[];
extern const char * kFlashSyncTable[];
extern const char * kFocalTable[];
extern const char * kExpCompTable[];
#define kFlashCompMax	0xFA
#define kFlashCompMin	0x12

extern const char * kMeterTable[];
extern const char * kCameraBracketTable[];
extern const char * kFlashSyncSpeedTable[];
extern const char * kCameraShutterTable[];
extern const char * kCameraN90ShutterTable[];
#define kCameraShutterMin		0x00
#define kCameraShutterMax		0x39
#define kCameraN90ShutterMax	0x14

extern const char * kCameraApertureTable[];
extern const char * kCameraExpModeTable[];
extern const char * kCameraMeterTable[];

extern const char * kCameraFlashCompTable[];
#define kCameraExpCompMin	0x00
#define kCameraExpCompMax	0x1E

extern const char * kCameraExpCompTable[];
#define kCameraFlashCompMax 0x0C

#define kVariProgramIndex			0x08
#define kVariProgramTable	kExposureModeTable

#define FLASH_COMP2REG(x) (x << 4)
#define REG2FLASH_COMP(x) (x >> 4)

short GetStringTableMax(const char ** table);
char *	GetStringTable(const char ** table, unsigned char index);
short GetStringIndex(const char ** table, char * str);

#endif