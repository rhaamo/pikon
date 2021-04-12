/*

  file.h

  Copyright (C) 2002 Balint Kis (balint.kis@mail.com)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _FILE_H
#define _FILE_H

#include <string.h>
#include <stdio.h>

#include "common.h"

#define MAX_ROLL_NUMBER          100
#define MAX_FRAME_NUMBER         40
#define ENTRY_STRING_LENGTH      256

#define ENTRY_INDEX_FRAME        0
#define ENTRY_INDEX_EXPOSURE     1
#define ENTRY_INDEX_APERTURE     2
#define ENTRY_INDEX_MODE         3
#define ENTRY_INDEX_METERING     4
#define ENTRY_INDEX_FLASH_MODE   5
#define ENTRY_INDEX_FOCAL_LENGHT 6
#define ENTRY_INDEX_EXP_COMP     7
#define ENTRY_INDEX_FLASH_COMP   8
#define ENTRY_INDEX_CAPTION      9
#define ENTRY_NUMBER             10

typedef enum {
  FILE_FORMAT_TXT,
  FILE_FORMAT_NKN,
  FILE_FORMAT_UNKNOWN,
} tFileFormat;

typedef struct {
  char *list[10];
} tTableElement;

typedef struct {
  uint8  pui8FileName[512];
  uint8  pui8RollTitle[256];
  uint8  pui8CameraType[20];
  uint8  pui8FilmSpeed[6];
  uint8  pui8TimeStamp[30];
  uint8  ui8FrameCounter;
  uint8  ui8StorageLevel;
  uint32 ui32TimeStamp;
  tTableElement table[40];
} tRoll;

uint16 ui16OpenFile(uint8 *, tRoll *);
uint16 ui16SaveFile(uint8 *, tRoll *);

uint16 ui16ReadConfigFile();
uint16 ui16WriteConfigFile();

#endif
