/*

  file.c

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

#include <string.h>
#include <stdio.h>
#include <time.h>

#include "nkn.h"
#include "print.h"
#include "file.h"
#include "util.h"

#include "gui.print.h"
#include "gui.preferences.h"

extern tNKN_DataArray rpNKN_ProgramMode[];
extern tNKN_DataArray rpNKN_LightMetering[];
extern tNKN_DataArray rpNKN_FlashMode[];
extern tNKN_DataArray rpNKN_Compensation[];

extern tNKN_DataArray rpCameraIsoFilmSpeed[];
extern tNKN_DataArray rpCameraExposureEffective[];
extern tNKN_DataArray rpCameraApertureEffective[];
extern tNKN_DataArray rpCameraFocusDistance[];
extern tNKN_DataArray rpCameraIsoFilmSpeed[];
extern tNKN_DataArray rpCameraExposureEffective[];
extern tNKN_DataArray rpCameraApertureEffective[];
extern tNKN_DataArray rpCameraFocusDistance[];

extern tFileFormat geDefaultFileFormat;

void   vReadNKNString                 (FILE *, uint8 *);
void   vWriteNKNString                (FILE *, uint8 *);
void   vGetData                       (uint8 *, uint8, uint8 *);
uint16 ui16DetectFileTypeByFileName   (uint8 *, tFileFormat *);
uint16 ui16DetectFileTypeByFileContent(uint8 *, tFileFormat *);
uint16 ui16OpenNKN_File               (uint8 *, tRoll *);
uint16 ui16OpenTXT_File               (uint8 *, tRoll *);
uint16 ui16SaveNKN_File               (uint8 *, tRoll *);
uint16 ui16SaveTXT_File               (uint8 *, tRoll *);

void vGetData(uint8 *pui8Source, uint8 ui8WordNumber, uint8 *pui8Result) {
  uint16 ui8CharIndex = 0;
  uint16 ui16FirstCharIndex = 0;
  uint8  ui8WordIndex = 0;

  /* seek for the separator */
  if (ui8WordNumber > 0) {
    for (ui8CharIndex = 0 ; pui8Source[ui8CharIndex] != '\n'; ui8CharIndex++) {
      if (pui8Source[ui8CharIndex] == '\t') {
        ui8WordIndex++;
        if (ui8WordIndex == ui8WordNumber)
          break;
      }
    }
    ui16FirstCharIndex = ++ui8CharIndex;
  }

  /* scan until the next separator */
  for (; pui8Source[ui8CharIndex] != '\n'; ui8CharIndex++) {
    if ((pui8Source[ui8CharIndex] == '\t') ||
        (pui8Source[ui8CharIndex] == '\0') ||
        (pui8Source[ui8CharIndex] == '\r')) {
      ui8WordIndex++;
      if (ui8WordIndex == (ui8WordNumber + 1))
        break;
    }
    pui8Result[ui8CharIndex - ui16FirstCharIndex] = pui8Source[ui8CharIndex];
  }
  /* add null character to the end of the string */
  pui8Result[ui8CharIndex - ui16FirstCharIndex] = '\0';
}

void vReadNKNString(FILE *pFileDescriptor, uint8 *pui8String) {
  uint8 ui8Length = fgetc(pFileDescriptor);
  fread(pui8String, ui8Length, 1, pFileDescriptor);
  pui8String[ui8Length] = '\0';
}

void vWriteNKNString(FILE *pFileDescriptor, uint8 *pui8String) {
  uint8 ui8Length = strlen(pui8String);
  fwrite(&ui8Length, 1, 1, pFileDescriptor);
  fwrite(pui8String, ui8Length, 1, pFileDescriptor);
}

uint16 ui16OpenFile(uint8 *pui8FileName, tRoll *prRoll) {
  uint16 ui16EntryIndex;
  uint16 ui16FrameIndex;
  tFileFormat eFileFormat;

  if (ui16DetectFileTypeByFileContent(pui8FileName, &eFileFormat) != OK)
    return FAIL;

  if (eFileFormat != FILE_FORMAT_UNKNOWN)
    for (ui16EntryIndex = 0; ui16EntryIndex < ENTRY_NUMBER; ui16EntryIndex++)
      for (ui16FrameIndex = 0; ui16FrameIndex < MAX_FRAME_NUMBER; ui16FrameIndex++) {
        prRoll->table[ui16FrameIndex].list[ui16EntryIndex] = malloc(256);
        strcpy(prRoll->table[ui16FrameIndex].list[ui16EntryIndex],"");
      }

  switch (eFileFormat) {
  case FILE_FORMAT_TXT:
    return ui16OpenTXT_File(pui8FileName, prRoll);
  case FILE_FORMAT_NKN:
    return ui16OpenNKN_File(pui8FileName, prRoll);
  default:
    prRoll = NULL;
  }

  return FAIL;
}

uint16 ui16SaveFile(uint8 *pui8FileName, tRoll *prRoll) {
  tFileFormat eFileFormat;

  if (ui16DetectFileTypeByFileName(pui8FileName, &eFileFormat) != OK)
    return FAIL;

  switch (eFileFormat) {
  case FILE_FORMAT_TXT:
    return ui16SaveTXT_File(pui8FileName, prRoll);
  case FILE_FORMAT_NKN:
    return ui16SaveNKN_File(pui8FileName, prRoll);
  default:
    return FAIL;
  }
  return OK;
}

uint16 ui16OpenNKN_File(uint8 *pui8FileName, tRoll *prRoll) {
  FILE *pFileDescriptor;
  uint8 pui8Header[4];
  uint8 pui8Version[2];
  uint8 pui8Dummy[3];
  uint8 ui8FrameCount;

  uint8 ui8Char;
  uint8 *pui8String = NULL;

  if ((pFileDescriptor = fopen(pui8FileName, "r" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }

  strcpy(prRoll->pui8FileName, pui8FileName);

  vReadNKNString(pFileDescriptor, pui8Header);

  fread(pui8Version,   2, 1, pFileDescriptor);
  fread(&(prRoll->ui32TimeStamp), 4, 1, pFileDescriptor);

  /* compensate elapsed time between UNIX and NKN time */
  /* 01/01/1970 00:00:00 - 12/31/1969 16:00:00 = 0x7080 seconds */
  prRoll->ui32TimeStamp -= 0x7080;
  strcpy(prRoll->pui8TimeStamp, ctime(&(prRoll->ui32TimeStamp)));

  vReadNKNString(pFileDescriptor, prRoll->pui8RollTitle);
  vReadNKNString(pFileDescriptor, prRoll->pui8CameraType);

  vNKN_DataToString(rpCameraIsoFilmSpeed, fgetc(pFileDescriptor), &pui8String);
  strcpy(prRoll->pui8FilmSpeed, pui8String);
  prRoll->ui8StorageLevel = fgetc(pFileDescriptor);
  fread(pui8Dummy, 3, 1, pFileDescriptor);
  ui8FrameCount = fgetc(pFileDescriptor);
  fread(pui8Dummy, 1, 1, pFileDescriptor);

  prRoll->ui8FrameCounter = 0;
  while (prRoll->ui8FrameCounter < ui8FrameCount) {

    vReadNKNString(pFileDescriptor, prRoll->table[prRoll->ui8FrameCounter].list[9]);

    sprintf(prRoll->table[prRoll->ui8FrameCounter].list[0], "%.2d", prRoll->ui8FrameCounter + 1);

    ui8Char = fgetc(pFileDescriptor);
    vNKN_DataToString(rpCameraExposureEffective, ui8Char, &pui8String);
    if (pui8String != NULL)
      strcpy(prRoll->table[prRoll->ui8FrameCounter].list[1], pui8String);

    ui8Char = fgetc(pFileDescriptor);
    vNKN_DataToString(rpCameraApertureEffective, ui8Char, &pui8String);
    if (pui8String != NULL)
      strcpy(prRoll->table[prRoll->ui8FrameCounter].list[2], pui8String);

    ui8Char = fgetc(pFileDescriptor);
    vNKN_DataToString(rpNKN_ProgramMode,          ui8Char, &pui8String);
    if (pui8String != NULL)
      strcpy(prRoll->table[prRoll->ui8FrameCounter].list[3], pui8String);

    ui8Char = fgetc(pFileDescriptor);
    vNKN_DataToString(rpNKN_LightMetering,        ui8Char, &pui8String);
    if (pui8String != NULL)
      strcpy(prRoll->table[prRoll->ui8FrameCounter].list[4], pui8String);

    if (prRoll->ui8StorageLevel > 0x00) {
      ui8Char = fgetc(pFileDescriptor);
      vNKN_DataToString(rpNKN_FlashMode,          ui8Char, &pui8String);
      if (pui8String != NULL)
        strcpy(prRoll->table[prRoll->ui8FrameCounter].list[5], pui8String);

      ui8Char = fgetc(pFileDescriptor);
      vNKN_DataToString(rpCameraFocusDistance,   ui8Char, &pui8String);
      if (pui8String != NULL)
        strcpy(prRoll->table[prRoll->ui8FrameCounter].list[6], pui8String);

      if (prRoll->ui8StorageLevel > 0x01) {
        ui8Char = fgetc(pFileDescriptor);
        vNKN_DataToString(rpNKN_Compensation,     ui8Char, &pui8String);
        if (pui8String != NULL)
          strcpy(prRoll->table[prRoll->ui8FrameCounter].list[7], pui8String);

        ui8Char = fgetc(pFileDescriptor);
        vNKN_DataToString(rpNKN_Compensation,     ui8Char, &pui8String);
        if (pui8String != NULL)
          strcpy(prRoll->table[prRoll->ui8FrameCounter].list[8], pui8String);
      }
    }
    prRoll->ui8FrameCounter++;
  }
  fclose(pFileDescriptor);
  return OK;
}


uint16 ui16SaveNKN_File(uint8 *pui8FileName, tRoll *prRoll) {
  uint8  pui8Version[2] = {0x01, 0x01};
  uint8  pui8Dummy1[3]  = {0x00, 0x01, 0x00};
  uint8  ui8Dummy2      = 0x00;
  FILE   *pFileDescriptor;
  uint8  ui8Data;
  uint16 ui16Data;
  uint16 ui16FrameIndex;

  if ((pFileDescriptor = fopen(pui8FileName, "w" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }

  vWriteNKNString(pFileDescriptor, "N90");
  fwrite(pui8Version, 2, 1, pFileDescriptor);

  prRoll->ui32TimeStamp = ui32InverseCtime(prRoll->pui8TimeStamp);
  fwrite(&(prRoll->ui32TimeStamp), 4, 1, pFileDescriptor);

  vWriteNKNString(pFileDescriptor, prRoll->pui8RollTitle);
  vWriteNKNString(pFileDescriptor, prRoll->pui8CameraType);

  vNKN_StringToData(rpCameraIsoFilmSpeed, prRoll->pui8FilmSpeed, &ui16Data);
  ui8Data = (uint8)ui16Data;
  fwrite(&(ui8Data), 1, 1, pFileDescriptor);
  fwrite(&(prRoll->ui8StorageLevel), 1, 1, pFileDescriptor);
  fwrite(pui8Dummy1, 3, 1, pFileDescriptor);
  fwrite(&(prRoll->ui8FrameCounter), 1, 1, pFileDescriptor);
  fwrite(&ui8Dummy2, 1, 1, pFileDescriptor);

  for (ui16FrameIndex = 0; ui16FrameIndex < prRoll->ui8FrameCounter; ui16FrameIndex++) {
    /* caption */
    vWriteNKNString(pFileDescriptor, prRoll->table[ui16FrameIndex].list[9]);
    /* exposure */
    vNKN_StringToData(rpCameraExposureEffective, prRoll->table[ui16FrameIndex].list[1], &ui16Data);
    ui8Data = (uint8)ui16Data;
    fwrite(&ui8Data, 1, 1, pFileDescriptor);
    vNKN_StringToData(rpCameraApertureEffective, prRoll->table[ui16FrameIndex].list[2], &ui16Data);
    ui8Data = (uint8)ui16Data;
    fwrite(&ui8Data, 1, 1, pFileDescriptor);
    vNKN_StringToData(rpNKN_ProgramMode, prRoll->table[ui16FrameIndex].list[3], &ui16Data);
    ui8Data = (uint8)ui16Data;
    fwrite(&ui8Data, 1, 1, pFileDescriptor);
    vNKN_StringToData(rpNKN_LightMetering, prRoll->table[ui16FrameIndex].list[4], &ui16Data);
    ui8Data = (uint8)ui16Data;
    fwrite(&ui8Data, 1, 1, pFileDescriptor);

    if (prRoll->ui8StorageLevel > 0x00) {
      vNKN_StringToData(rpNKN_FlashMode, prRoll->table[ui16FrameIndex].list[5], &ui16Data);
      ui8Data = (uint8)ui16Data;
      fwrite(&ui8Data, 1, 1, pFileDescriptor);
      vNKN_StringToData(rpCameraFocusDistance, prRoll->table[ui16FrameIndex].list[6], &ui16Data);
      ui8Data = (uint8)ui16Data;
      fwrite(&ui8Data, 1, 1, pFileDescriptor);

      if (prRoll->ui8StorageLevel > 0x01) {
        vNKN_StringToData(rpNKN_Compensation, prRoll->table[ui16FrameIndex].list[7], &ui16Data);
        ui8Data = (uint8)ui16Data;
        fwrite(&ui8Data, 1, 1, pFileDescriptor);
        vNKN_StringToData(rpNKN_Compensation, prRoll->table[ui16FrameIndex].list[8], &ui16Data);
        ui8Data = (uint8)ui16Data;
        fwrite(&ui8Data, 1, 1, pFileDescriptor);
      }
    }
  }
  fclose(pFileDescriptor);
  return OK;
}

uint16 ui16OpenTXT_File(uint8 *pui8FileName, tRoll *prRoll) {
  uint8  pui8Buffer[512];
  uint16 ui16ColumnIndex;
  FILE   *pFileDescriptor;

  if ((pFileDescriptor = fopen(pui8FileName, "r" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }

  strcpy(prRoll->pui8FileName, pui8FileName);

  fgets(pui8Buffer, 512, pFileDescriptor);
  fgets(pui8Buffer, 512, pFileDescriptor);
  vGetData(pui8Buffer, 1, prRoll->pui8RollTitle);
  fgets(pui8Buffer, 512, pFileDescriptor);
  vGetData(pui8Buffer, 1, prRoll->pui8CameraType);
  fgets(pui8Buffer, 512, pFileDescriptor);
  vGetData(pui8Buffer, 1, prRoll->pui8FilmSpeed);
  fgets(pui8Buffer, 512, pFileDescriptor);
  vGetData(pui8Buffer, 1, prRoll->pui8TimeStamp);
  fgets(pui8Buffer, 512, pFileDescriptor);

  prRoll->ui8FrameCounter = 0;
  fgets(pui8Buffer, 512, pFileDescriptor);
  while (!feof(pFileDescriptor)) {
    for (ui16ColumnIndex = 0; ui16ColumnIndex < 10; ui16ColumnIndex++) {
      vGetData(pui8Buffer, ui16ColumnIndex, prRoll->table[prRoll->ui8FrameCounter].list[ui16ColumnIndex]);
    }
    prRoll->ui8FrameCounter++;
    fgets(pui8Buffer, 512, pFileDescriptor);
  }
  fclose(pFileDescriptor);
  return OK;
}

uint16 ui16SaveTXT_File(uint8 *pui8FileName, tRoll *prRoll) {
  uint8   pui8Buffer[512];
  uint16  ui16FrameIndex;
  FILE *pFileDescriptor;

  if ((pFileDescriptor = fopen(pui8FileName, "w" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }

  sprintf(pui8Buffer, "Data File:\t%s\n", pui8FileName);
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Roll Title:\t%s\n", prRoll->pui8RollTitle);
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Camera:\t%s\n", prRoll->pui8CameraType);
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Film Speed:\t%s\n", prRoll->pui8FilmSpeed);
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Time Stamp:\t%s", prRoll->pui8TimeStamp);
  fputs(pui8Buffer, pFileDescriptor);

  sprintf(pui8Buffer, "Frame\tShutter Speed\tAperture\tExposure Mode\t");
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Metering System\tFlash-Sync Mode\tFocal Length\t");
  fputs(pui8Buffer, pFileDescriptor);
  sprintf(pui8Buffer, "Exposure\tFlash\tCaption\n");
  fputs(pui8Buffer, pFileDescriptor);

  for (ui16FrameIndex = 0; ui16FrameIndex < prRoll->ui8FrameCounter; ui16FrameIndex++) {
    sprintf(pui8Buffer,
            "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
            prRoll->table[ui16FrameIndex].list[0],
            prRoll->table[ui16FrameIndex].list[1],
            prRoll->table[ui16FrameIndex].list[2],
            prRoll->table[ui16FrameIndex].list[3],
            prRoll->table[ui16FrameIndex].list[4],
            prRoll->table[ui16FrameIndex].list[5],
            prRoll->table[ui16FrameIndex].list[6],
            prRoll->table[ui16FrameIndex].list[7],
            prRoll->table[ui16FrameIndex].list[8],
            prRoll->table[ui16FrameIndex].list[9]);
    fputs(pui8Buffer, pFileDescriptor);
  }
  fclose(pFileDescriptor);
  return OK;
}

uint16 ui16DetectFileTypeByFileName(uint8 *pui8FileName, tFileFormat *peFileFormat){
  uint16 ui16FileNameLength = strlen(pui8FileName) - 3;

  if (strcasecmp(pui8FileName + ui16FileNameLength, "txt") == 0)
    *peFileFormat = FILE_FORMAT_TXT;
  else if (strcasecmp(pui8FileName + ui16FileNameLength, "nkn") == 0)
    *peFileFormat = FILE_FORMAT_NKN;
  else
    *peFileFormat = geDefaultFileFormat;

  return OK;
}

uint16 ui16DetectFileTypeByFileContent(uint8 *pui8FileName, tFileFormat *peFileFormat) {
  FILE  *pFileDescriptor;
  uint8 pui8Header[4];
  uint8 pui8Version[2];
  uint8 pui8Buffer[256];

  /* check for the NKN format by reading the version */
  if ((pFileDescriptor = fopen(pui8FileName, "r" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }

  fread(pui8Header, 4, 1, pFileDescriptor);
  fread(pui8Version, 2, 1, pFileDescriptor);
  fclose(pFileDescriptor);

  if ((pui8Version[0] == 1) && (pui8Version[1] == 1)) {
    *peFileFormat = FILE_FORMAT_NKN;
    return OK;
  }

  /* check for the TXT format by reading the first line */
  if ((pFileDescriptor = fopen(pui8FileName, "r" )) == NULL) {
    printf("Cannot open input file %s.\n", pui8FileName);
    return FAIL;
  }
  fgets(pui8Buffer, sizeof(pui8Buffer), pFileDescriptor);
  fclose(pFileDescriptor);

  if (strncmp(pui8Buffer, "Data File:", 10) == 0) {
    *peFileFormat = FILE_FORMAT_TXT;
    return OK;
  }

  *peFileFormat = FILE_FORMAT_UNKNOWN;
  return OK;
}

uint16 ui16ReadConfigFile() {
  struct stat sBuf;
  FILE  *pFileDescriptor;
  uint8 *ui8Home = getenv("HOME");
  uint8 *pui8ConfigFileName;

  uint16 ui16Baud;
  uint8 pui8Device[256];
  uint8 pui8PrintTo[256];
  uint8 pui8PrinterCommand[256];
  uint8 pui8PaperSize[256];
  uint8 pui8CopyrightNote[256];
  tFileFormat eDefaultFileFormat;
  tPrintLayout rPrintLayout;
  uint8 pui8Buffer[256];

  pui8ConfigFileName = (char *)malloc(sizeof(char)*(strlen(ui8Home)+9));
  memset(pui8ConfigFileName, 0, sizeof(char)*(strlen(ui8Home)+9));
  sprintf(pui8ConfigFileName, "%s/.gicon", ui8Home);

  /* configuration file does not exist, load defaults */
  if (stat(pui8ConfigFileName, &sBuf) != 0) {
    vPrintLoadDefaults();
    vPreferencesLoadDefaults();
    return OK;
  }

  /* open config file and load values */
  if ((pFileDescriptor = fopen(pui8ConfigFileName, "r" )) == NULL) {
    printf("Cannot open configuration file: %s.\n", pui8ConfigFileName);
    return FAIL;
  }

  vReadNKNString(pFileDescriptor, pui8Device);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  ui16Baud = atoi(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  eDefaultFileFormat = atoi(pui8Buffer);

  vReadNKNString(pFileDescriptor, pui8PrintTo);
  vReadNKNString(pFileDescriptor, pui8PrinterCommand);
  vReadNKNString(pFileDescriptor, pui8PaperSize);
  vReadNKNString(pFileDescriptor, pui8CopyrightNote);

  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.ui8Orientation = atoi(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.ui8PrintCaption = atoi(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.ui8PrintTitle = atoi(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.ui8Columns = atoi(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fHorizontalOffset = atof(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fVerticalOffset = atof(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fLabelWidth = atof(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fLabelHeight = atof(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fColumnSpacing = atof(pui8Buffer);
  vReadNKNString(pFileDescriptor, pui8Buffer);
  rPrintLayout.fRowSpacing = atof(pui8Buffer);

  vPreferencesSet(pui8Device, ui16Baud, eDefaultFileFormat);
  vPrintSet(pui8PrintTo, pui8PrinterCommand, pui8PaperSize,
            pui8CopyrightNote, &rPrintLayout);

  fclose(pFileDescriptor);
  return OK;
}

uint16 ui16WriteConfigFile() {
  FILE  *pFileDescriptor;
  uint8 *ui8Home = getenv("HOME");
  uint8 *pui8ConfigFileName;

  uint16 ui16Baud;
  uint8 ui8DefaultFileFormat;
  uint8 *pui8Device;
  uint8 *pui8PrintTo;
  uint8 *pui8PrinterCommand;
  uint8 *pui8PaperSize;
  uint8 *pui8CopyrightNote;
  uint8 pui8Buffer[256];
  tPrintLayout prPrintLayout;

  pui8ConfigFileName = (char *)malloc(sizeof(char)*(strlen(ui8Home)+9));
  memset(pui8ConfigFileName, 0, sizeof(char)*(strlen(ui8Home)+9));
  sprintf(pui8ConfigFileName, "%s/.gicon", ui8Home);

  if ((pFileDescriptor = fopen(pui8ConfigFileName, "w" )) == NULL) {
    printf("Cannot open configuration file: %s.\n", pui8ConfigFileName);
    return FAIL;
  }

  vPreferencesGet(&pui8Device, &ui16Baud, &ui8DefaultFileFormat);
  vPrintGet(&pui8PrintTo, &pui8PrinterCommand,
            &pui8PaperSize, &pui8CopyrightNote, &prPrintLayout);

  vWriteNKNString(pFileDescriptor, pui8Device);
  sprintf(pui8Buffer, "%d", ui16Baud);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%d", ui8DefaultFileFormat);
  vWriteNKNString(pFileDescriptor, pui8Buffer);

  vWriteNKNString(pFileDescriptor, pui8PrintTo);
  vWriteNKNString(pFileDescriptor, pui8PrinterCommand);
  vWriteNKNString(pFileDescriptor, pui8PaperSize);
  vWriteNKNString(pFileDescriptor, pui8CopyrightNote);

  sprintf(pui8Buffer, "%d", prPrintLayout.ui8Orientation);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%d", prPrintLayout.ui8PrintCaption);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%d", prPrintLayout.ui8PrintTitle);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%d", prPrintLayout.ui8Columns);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fHorizontalOffset);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fVerticalOffset);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fLabelWidth);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fLabelHeight);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fColumnSpacing);
  vWriteNKNString(pFileDescriptor, pui8Buffer);
  sprintf(pui8Buffer, "%f", prPrintLayout.fRowSpacing);
  vWriteNKNString(pFileDescriptor, pui8Buffer);

  fclose(pFileDescriptor);
  return OK;
}
