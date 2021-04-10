/*

  nkn.c

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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>
#include <termio.h>
#include <sgtty.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "common.h"
#include "tty.h"
#include "nkn.h"

uint16 ui16NKN_SendData    (uint16, uint8, uint32, uint16, uint8 *);
uint16 ui16NKN_RequestData (uint16, uint8, uint32, uint16);
uint16 ui16NKN_ReceiveData (uint16, uint8, uint16, uint8 *);

uint16 ui16Str2Data(uint8 *, uint16);
uint8  *pui8Data2Str(uint32, uint8 *, uint16);

uint8 *pui8Data2Str(uint32 ui32Value, uint8 *pui8Buffer, uint16 ui16Lenght) {
  uint16 ui16Index;

  pui8Buffer += ui16Lenght;

  for (ui16Index = 0; ui16Index < ui16Lenght; ui16Index++) {
    pui8Buffer --;
    *pui8Buffer = (uint8)ui32Value;
    ui32Value >>= 8;
  }

  return pui8Buffer;
}

uint16 ui16Str2Data(uint8 *pui8Buffer, uint16 ui16Lenght) {
  uint16 ui16Value;

  switch (ui16Lenght) {
  case 1:
    ui16Value = pui8Buffer[0];
  case 2:
    ui16Value = pui8Buffer[1];
    ui16Value <<= 8;
    ui16Value |= pui8Buffer[0];
    break;
  default:
    ui16Value = 0;
  }
  return ui16Value;
}

uint8 ui8CountChecksum(uint8 *pui8Buffer, uint16 ui16Length) {
  uint16 ui16Index = sizeof(pui8Buffer);
  uint8 ui8Checksum = 0;
  for (ui16Index = 0; ui16Index < ui16Length; ui16Index++)
    ui8Checksum += pui8Buffer[ui16Index];
  return ui8Checksum;
}

uint16 ui16NKN_ReceiveData(uint16 ui16FileDescriptor,
                            uint8  ui8StartByte,
                            uint16 ui16Length,
                            uint8 *pui8Buffer) {
  uint8 ui8Byte;
  uint8 ui8RetryCounter = 0;
  struct timeval rTime;

  rTime.tv_sec = 0;
  rTime.tv_usec = 5000;  /*300000*/

  do {
    select(0 , NULL, NULL, NULL, &rTime);
    ui16TTY_Read(ui16FileDescriptor, &ui8Byte, 1);
    ui8RetryCounter++;
  } while ((ui8Byte != ui8StartByte) && (ui8RetryCounter < 10));

  if (ui8RetryCounter >= 10) {
    DEBUG(("Failed to read startbyte %d\n", ui8StartByte));
    return FAIL;
  }

  switch (ui8Byte) {
  case DATA_START_BYTE:
    if (ui16TTY_Read(ui16FileDescriptor, pui8Buffer, ui16Length)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }

    if (ui16TTY_Read(ui16FileDescriptor, &ui8Byte, 1)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }

    if (ui8Byte != ui8CountChecksum(pui8Buffer, ui16Length)) {
      ui16TTY_Flush(ui16FileDescriptor);
      return NKN_READ_CHECKSUM_FAIL;
    }

    if (ui16TTY_Read(ui16FileDescriptor, &ui8Byte, 1)) {
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }
    if (ui8Byte == DATA_STOP_BYTE)
      return NKN_READ_DATA_OK;
    return NKN_READ_DATA_FAIL;
  case STATUS_START_BYTE:
    if (ui16TTY_Read(ui16FileDescriptor, &ui8Byte, ui16Length - 1)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }
    if (ui8Byte == STATUS_STOP_BYTE)
      return NKN_READ_STATUS_OK;
    return NKN_READ_STATUS_FAIL;
  case SIGNOFF_START_BYTE:
    if (ui16TTY_Read(ui16FileDescriptor, &ui8Byte, ui16Length - 1)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }
    if (ui8Byte == SIGNOFF_STOP_BYTE)
      return NKN_READ_SIGNOFF_OK;
    return NKN_READ_SIGNOFF_FAIL;
  case IDENTIFICATION_START_BYTE:
    if (ui16TTY_Read(ui16FileDescriptor, pui8Buffer, ui16Length)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }

    if (ui16TTY_Read(ui16FileDescriptor, &ui8Byte, 1)) {
      ui16TTY_Flush(ui16FileDescriptor);
      DEBUG(("Cannot read %d\n", ui16FileDescriptor));
      return FAIL;
    }
    if (ui8Byte == IDENTIFICATION_STOP_BYTE)
      return NKN_READ_SIGNIN_OK;
    return NKN_READ_SIGNIN_FAIL;
  default:
      DEBUG(("Unexpected start byte: 0x%02x\n", ui8Byte));
  }
  return NKN_READ_FAIL;
}

uint16 ui16NKN_WakeUp(uint16 *ui16FileDescriptor, uint8 *pui8Path) {
  uint8  pui8Tmp[2];
  uint16 ui16ReturnValue;
  struct timeval rTime;

  DEBUG(("WAKEUP %s\n", pui8Path));

  if (ui16TTY_Open(ui16FileDescriptor, pui8Path, TTY_BAUD_1200)) {
    DEBUG(("Could not open device %s.\n", pui8Path));
    return FAIL;
  }

  ui16ReturnValue = ui16TTY_Write(*ui16FileDescriptor, pui8Data2Str(WAKEUP, pui8Tmp, 1), 1);

  rTime.tv_sec = 0;
  rTime.tv_usec = 300000;
  select(0 , NULL, NULL, NULL, &rTime);

  if (ui16TTY_Flush(*ui16FileDescriptor)) {
    DEBUG(("Could not flush device\n"));
    return FAIL;
  }

  return ui16ReturnValue;
}

uint16 ui16NKN_SignIn(uint16 ui16FileDescriptor) {
  uint8  pui8Tmp[IDENTIFICATION_STRING_SIZE];
  uint16 ui16ReturnValue = 0;
  struct timeval rTime;

  DEBUG(("SIGNIN\n"));

  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(SIGNIN_START_BYTE, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(SIGNIN_SEQUENCE, pui8Tmp, 4), 4);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(SIGNIN_STOP_BYTE, pui8Tmp, 1), 1);

  if (ui16ReturnValue) {
    DEBUG(("tty write failed %d times.\n", ui16ReturnValue));
    return FAIL;
  }

  rTime.tv_sec = 0;
  rTime.tv_usec = 200000;
  select(0 , NULL, NULL, NULL, &rTime);

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, IDENTIFICATION_START_BYTE, IDENTIFICATION_STRING_SIZE, pui8Tmp);
  if (ui16ReturnValue != NKN_READ_SIGNIN_OK) {
    DEBUG(("Failed to receive unit identification string: %d\n", ui16ReturnValue));
    return FAIL;
  }

  DEBUG(("UNIT IDENTIFICATION: %s\n", pui8Tmp));

  if (strncmp(pui8Tmp, "020F90X/N90S", 12) != 0)
    return FAIL;

  return OK;
}

uint16 ui16NKN_SignOff(uint16 ui16FileDescriptor) {
  uint8  pui8Tmp[2];
  uint16 ui16ReturnValue = 0;

  DEBUG(("SIGNOFF\n"));

  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(SIGNOFF_START_BYTE, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(SIGNOFF_STOP_BYTE, pui8Tmp, 1), 1);

  if (ui16ReturnValue) {
    DEBUG(("tty write failed %d times.\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, SIGNOFF_START_BYTE, 2, pui8Tmp);
  if (ui16ReturnValue != NKN_READ_SIGNOFF_OK) {
    DEBUG(("Failed to receive data: %d\n", ui16ReturnValue));
    return FAIL;
  }

  if ((ui16ReturnValue = ui16TTY_Close(ui16FileDescriptor))) {
    DEBUG(("Failed to close file: %d\n", ui16ReturnValue));
    return FAIL;
  }

  return OK;
}

uint16 ui16NKN_SetBaud(uint16 ui16FileDescriptor, uint16 ui16Baud) {
  uint8  pui8Tmp[2];
  uint16 ui16ReturnValue = 0;
  struct timeval rTime;

  if (ui16Baud != TTY_BAUD_9600)
    return OK;

  DEBUG(("CHANGE BAUD TO 9600bps\n"));

  ui16ReturnValue = ui16NKN_SendData(ui16FileDescriptor, M_CB, 0x050000, 0, NULL);
  if (ui16ReturnValue) {
    DEBUG(("Failed to change baud rate: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, STATUS_START_BYTE, 2, pui8Tmp);
  if (ui16ReturnValue != NKN_READ_STATUS_OK) {
    DEBUG(("Failed to receive acknowledge: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16TTY_SetBaud(ui16FileDescriptor, TTY_BAUD_9600);
  if (ui16ReturnValue) {
    DEBUG(("Failed to change tty baud rate: %d\n", ui16ReturnValue));
    return FAIL;
  }

  rTime.tv_sec = 0;
  rTime.tv_usec = 200000;
  select(0 , NULL, NULL, NULL, &rTime);

  return OK;
}

uint16 ui16NKN_Focus(uint16 ui16FileDescriptor) {
  uint8  pui8Buffer[2];
  uint16 ui16ReturnValue;

  ui16ReturnValue = ui16NKN_SendData(ui16FileDescriptor, M_AF, 0x000000, 0, NULL);
  if (ui16ReturnValue) {
    DEBUG(("Failed to send FOCUS command: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, STATUS_START_BYTE, 2, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_STATUS_OK) {
    DEBUG(("Failed to receive acknowledge: %d\n", ui16ReturnValue));
    return FAIL;
  }
  return OK;
}

uint16 ui16NKN_Release(uint16 ui16FileDescriptor) {
  uint8  pui8Buffer[2];
  uint16 ui16ReturnValue;

  ui16ReturnValue = ui16NKN_SendData(ui16FileDescriptor, M_SR, 0x000000, 0, NULL);
  if (ui16ReturnValue) {
    DEBUG(("Failed to send FOCUS command: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, STATUS_START_BYTE, 2, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_STATUS_OK) {
    DEBUG(("Failed to receive acknowledge: %d\n", ui16ReturnValue));
    return FAIL;
  }
  return OK;
}

uint16 ui16NKN_Download(uint16 ui16FileDescriptor, uint16 *pui16RollNumber, uint16 *pui16BytesInRoll) {
  uint8  pui8Buffer[4];
  uint16 ui16ReturnValue;

  ui16ReturnValue = ui16NKN_SendData(ui16FileDescriptor, M_MH, 0x000000, 0, NULL);
  if (ui16ReturnValue) {
    DEBUG(("Failed to send download command: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, DATA_START_BYTE, 4, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_DATA_OK) {
    DEBUG(("Failed to receive acknowledge: %d\n", ui16ReturnValue));
    return FAIL;
  }

  *pui16RollNumber = pui8Buffer[0];
  *pui16RollNumber <<= 8;
  *pui16RollNumber += pui8Buffer[1];

  *pui16BytesInRoll = pui8Buffer[2];
  *pui16BytesInRoll <<= 8;
  *pui16BytesInRoll += pui8Buffer[3];

  return OK;
}


uint16 ui16NKN_SendData(uint16 ui16FileDescriptor, uint8 ui8Mode, uint32 ui32Address, uint16 ui16Length, uint8 *pui8Buffer) {
  uint8  pui8Tmp[4];
  uint16 ui16ReturnValue = 0;

  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_START_BYTE, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_CFLAG, pui8Tmp, 1), 1);

  switch (ui8Mode) {
  case M_RD:
    DEBUG(("Read only register\n"));
    return FAIL;
  case M_WR:
  case M_RW:
    *pui8Tmp = COMMAND_MODE_WRITE_DATA;
    break;
  case M_SR:
    *pui8Tmp = COMMAND_MODE_SHUTTER_RELEASE;
    break;
  case M_AF:
    *pui8Tmp = COMMAND_MODE_FOCUS_CAMERA;
    break;
  case M_CB:
    *pui8Tmp = COMMAND_MODE_CHANGE_BAUD;
    break;
  case M_MH:
    *pui8Tmp = COMMAND_MODE_MEMO_HOLDER_INFO;
    break;
  default:
    DEBUG(("Invalid mode\n"));
    return FAIL;
  }
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Tmp, 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(ui32Address, pui8Tmp, 3), 3);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(ui16Length, pui8Tmp, 2), 2);

  switch (ui8Mode) {
  case M_SR:
  case M_AF:
  case M_CB:
  case M_MH:
    ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_STOP_BYTE, pui8Tmp, 1), 1);
    return OK;
  default:
    ;
  }

  /* data */
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(DATA_START_BYTE, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Buffer, ui16Length);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(ui8CountChecksum(pui8Buffer, ui16Length), pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(DATA_STOP_BYTE, pui8Tmp, 1), 1);

  if (ui16ReturnValue) {
    DEBUG(("tty write failed %d times.\n", ui16ReturnValue));
    return FAIL;
  }
  return OK;
}

uint16 ui16NKN_RequestData(uint16 ui16FileDescriptor, uint8 ui8Mode, uint32 ui32Address, uint16 ui16Length) {
  uint8  pui8Tmp[4];
  uint16 ui16ReturnValue = 0;

  switch (ui8Mode) {
  case M_RD:
  case M_RW:
    break;
  case M_WR:
  case M_SR:
  case M_AF:
  case M_CB:
  case M_MH:
    DEBUG(("Read only register\n"));
    return FAIL;
  default:
    DEBUG(("Invalid mode\n"));
    return FAIL;
  }

  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_START_BYTE, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_CFLAG, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_MODE_READ_DATA, pui8Tmp, 1), 1);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(ui32Address, pui8Tmp, 3), 3);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(ui16Length, pui8Tmp, 2), 2);
  ui16ReturnValue += ui16TTY_Write(ui16FileDescriptor, pui8Data2Str(COMMAND_STOP_BYTE, pui8Tmp, 1), 1);

  if (ui16ReturnValue) {
    DEBUG(("tty write failed %d times.\n", ui16ReturnValue));
    return FAIL;
  }
  return OK;
}


uint16 ui16NKN_Read16(uint16 ui16FileDescriptor, uint8 ui8Mode, uint32 ui32Address, uint16 ui16Length, uint16 ui16Mask, uint8 ui8Shift, uint16 *pui16Data) {
  uint8  pui8Buffer[4];
  uint16 ui16ReturnValue;

  DEBUG(("READ address:%lx mode:%d\n", ui32Address, ui8Mode));

  if ((ui16ReturnValue = ui16NKN_RequestData(ui16FileDescriptor, ui8Mode, ui32Address, ui16Length))) {
    DEBUG(("Failed to request data: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, DATA_START_BYTE, ui16Length, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_DATA_OK) {
    DEBUG(("Failed to receive data: %d\n", ui16ReturnValue));
    return FAIL;
  }

  *pui16Data = ui16Str2Data(pui8Buffer, ui16Length);
  *pui16Data &= ui16Mask;
  *pui16Data = *pui16Data >> ui8Shift;
  return OK;
}

uint16 ui16NKN_Write16(uint16 ui16FileDescriptor, uint8 ui8Mode, uint32 ui32Address, uint16 ui16Length, uint16 ui16Mask, uint8 ui8Shift, uint16 ui16Data) {
  uint8  pui8Buffer[4];
  uint16 ui16ReturnValue;
  uint16 ui16Tmp;
  uint8  ui8ReverseByte;


  DEBUG(("WRITE address:%lx mode:%d data:%x\n", ui32Address, ui8Mode, ui16Data));

  if ((ui16ReturnValue = ui16NKN_RequestData(ui16FileDescriptor, ui8Mode, ui32Address, ui16Length))) {
    DEBUG(("Failed to request register current value: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, DATA_START_BYTE, ui16Length, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_DATA_OK) {
    DEBUG(("Failed to receive register current value: %d\n", ui16ReturnValue));
    return FAIL;
  }

  DEBUG(("data to be written 0x%x\n", ui16Data));

  if (ui16Length > 1) {
    ui8ReverseByte = (ui16Data >> 8) & 0xff;
    ui16Data = (ui16Data & 0xff) << 8;
    ui16Data += ui8ReverseByte;
  }

  ui16Data = ui16Data << ui8Shift;
  ui16Data &= ui16Mask;

  ui16Tmp = ui16Str2Data(pui8Buffer, ui16Length);

  DEBUG(("before 0x%x ", ui16Tmp));

  ui16Tmp &= ~ui16Mask;

  ui16Data |= ui16Tmp;

  DEBUG(("after 0x%x\n", ui16Data));

  if ((ui16ReturnValue = ui16NKN_SendData(ui16FileDescriptor, ui8Mode, ui32Address, ui16Length, pui8Data2Str(ui16Data, pui8Buffer, ui16Length)))) {
    DEBUG(("Failed to write register new value: %d\n", ui16ReturnValue));
    return FAIL;
  }

  ui16ReturnValue = ui16NKN_ReceiveData(ui16FileDescriptor, STATUS_START_BYTE, 2, pui8Buffer);
  if (ui16ReturnValue != NKN_READ_STATUS_OK) {
    DEBUG(("Failed to receive acknowledge: %d\n", ui16ReturnValue));
    return FAIL;
  }
  return OK;
}

void vNKN_DataToString(tNKN_DataArray *rpData, uint16 ui16Value, uint8 **ppui8String) {
  uint8 ui8Index = 0;

  if (rpData == NULL) {
    DEBUG(("Cannot convert %d into string.\n", ui16Value));
    return;
  }

  while (!((rpData[ui8Index].pui8Description == NULL) ||
           (rpData[ui8Index].ui8Value == 0xFF))) {
    if (rpData[ui8Index].ui8Value == ui16Value) {
      *ppui8String = rpData[ui8Index].pui8Description;
      return;
    }
    ui8Index++;
  }
  *ppui8String = NULL;
}

void vNKN_StringToData(tNKN_DataArray *rpData, uint8 *pui8String, uint16 *ui16Value) {
  uint8 ui8Index = 0;

  while (!((rpData[ui8Index].pui8Description == NULL) ||
           (rpData[ui8Index].ui8Value == 0xFF))) {
    if (strcmp(rpData[ui8Index].pui8Description, pui8String) == 0) {
      *ui16Value = rpData[ui8Index].ui8Value;
      return;
    }
    ui8Index++;
  }
  *ui16Value = 0xFFFF;
}


tNKN_DataArray rpCameraExposure[] = {
  {0x00, "BULB"},   {0x01, "NA"},     {0x02, "NA"},
  {0x03, "30"},     {0x04, "25"},     {0x05, "20"},
  {0x06, "15"},     {0x07, "13"},     {0x08, "10"},
  {0x09, "8"},      {0x0A, "6"},      {0x0B, "5"},
  {0x0C, "4"},      {0x0D, "3"},      {0x0E, "2.5"},
  {0x0F, "2"},      {0x10, "1.6"},    {0x11, "1.3"},
  {0x12, "1"},      {0x13, "1/1.3"},  {0x14, "1/1.6"},
  {0x15, "1/2"},    {0x16, "1/2.5"},  {0x17, "1/3"},
  {0x18, "1/4"},    {0x19, "1/5"},    {0x1A, "1/6"},
  {0x1B, "1/8"},    {0x1C, "1/10"},   {0x1D, "1/13"},
  {0x1E, "1/15"},   {0x1F, "1/20"},   {0x20, "1/25"},
  {0x21, "1/30"},   {0x22, "1/40"},   {0x23, "1/50"},
  {0x24, "1/60"},   {0x25, "1/80"},   {0x26, "1/100"},
  {0x27, "1/125"},  {0x28, "1/160"},  {0x29, "1/200"},
  {0x2A, "1/250"},  {0x2B, "1/320"},  {0x2C, "1/400"},
  {0x2D, "1/500"},  {0x2E, "1/640"},  {0x2F, "1/800"},
  {0x30, "1/1000"}, {0x31, "1/1250"}, {0x32, "1/1600"},
  {0x33, "1/2000"}, {0x34, "1/2500"}, {0x35, "1/3200"},
  {0x36, "1/4000"}, {0x37, "1/5000"}, {0x38, "1/6400"},
  {0x39, "1/8000"}, {0xFF, NULL}
};

tNKN_DataArray rpExposureMetered[] = {
  {0x9B, "125"},  {0xA5, "200"},  {0xA8, "250"},
  {0xAC, "320"},  {0xAF, "400"},  {0xBF, "1000"},
  {0xE3, "8000"}, {0xFF, NULL}
};

tNKN_DataArray rpCameraIsoFilmSpeed[] = {
  {0x00, "6"},    {0x01, "8"},    {0x02, "10"},   {0x03, "12"},
  {0x04, "16"},   {0x05, "20"},   {0x06, "25"},   {0x07, "32"},
  {0x08, "40"},   {0x09, "50"},   {0x0A, "64"},   {0x0B, "80"},
  {0x0C, "100"},  {0x0D, "125"},  {0x0E, "160"},  {0x0F, "200"},
  {0x10, "250"},  {0x11, "320"},  {0x12, "400"},  {0x13, "500"},
  {0x14, "640"},  {0x15, "800"},  {0x16, "1000"}, {0x17, "1250"},
  {0x18, "1600"}, {0x19, "2000"}, {0x1A, "2500"}, {0x1B, "3200"},
  {0x1C, "4000"}, {0x1D, "5000"}, {0x1E, "6400"}, {0x1F, "DX"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraExposureCompensation[] = {
  {0x00, "+5.0"},  {0x01, "+4.7"},  {0x02, "+4.3"},
  {0x03, "+4.0"},  {0x04, "+3.7"},  {0x05, "+3.3"},
  {0x06, "+3.0"},  {0x07, "+2.7"},  {0x08, "+2.3"},
  {0x09, "+2.0"},  {0x0A, "+1.7"},  {0x0B, "+1.3"},
  {0x0C, "+1.0"},  {0x0D, "+0.7"},  {0x0E, "+0.3"},
  {0x0F, "0.0"},   {0x10, "-0.3"},  {0x11, "-0.7"},
  {0x12, "-1.0"},  {0x13, "-1.3"},  {0x14, "-1.7"},
  {0x15, "-2.0"},  {0x16, "-2.3"},  {0x17, "-2.7"},
  {0x18, "-3.0"},  {0x19, "-3.3"},  {0x1A, "-3.7"},
  {0x1B, "-4.0"},  {0x1C, "-4.3"},  {0x1D, "-4.7"},
  {0x1E, "-5.0"},  {0xFF, NULL}
};

tNKN_DataArray rpCameraBracketingStep[] = {
  {0x00, "0.0"},  {0x01, "0.25"}, {0x02, "0.3"},
  {0x03, "0.5"},  {0x04, "0.7"},  {0x05, "0.75"},
  {0x06, "1.0"},  {0x07, "1.25"}, {0x08, "1.3"},
  {0x09, "1.5"},  {0x0A, "1.7"},  {0x0B, "1.75"},
  {0x0C, "2.0"},  {0xFF, NULL}
};

tNKN_DataArray rpCameraFlashSyncSpeed[] = {
  {0x00, "60"},     {0x04, "NA"},     {0x08, "NA"},
  {0x0C, "30"},     {0x10, "25"},     {0x14, "20"},
  {0x18, "15"},     {0x1C, "13"},     {0x20, "10"},
  {0x24, "8"},      {0x28, "6"},      {0x2C, "5"},
  {0x30, "4"},      {0x34, "3"},      {0x38, "2.5"},
  {0x3C, "2"},      {0x40, "1.6"},    {0x44, "1.3"},
  {0x48, "1"},      {0x4C, "1/1.3"},  {0x50, "1/1.6"},
  {0x54, "1/2"},    {0x58, "1/2.5"},  {0x5C, "1/3"},
  {0x60, "1/4"},    {0x64, "1/5"},    {0x68, "1/6"},
  {0x6C, "1/8"},    {0x70, "1/10"},   {0x74, "1/13"},
  {0x78, "1/15"},   {0x7C, "1/20"},   {0x80, "1/25"},
  {0x84, "1/30"},   {0x88, "1/40"},   {0x8C, "1/50"},
  {0x90, "1/60"},   {0x94, "1/80"},   {0x98, "1/100"},
  {0x9C, "1/125"},  {0xA0, "1/160"},  {0xA4, "1/200"},
  {0xA8, "1/250"},  {0xAC, "1/320"},  {0xB0, "1/400"},
  {0xB4, "1/500"},  {0xB8, "1/640"},  {0xBC, "1/800"},
  {0xC0, "1/1000"}, {0xC4, "1/1250"}, {0xC8, "1/1600"},
  {0xCC, "1/2000"}, {0xD0, "1/2500"}, {0xD4, "1/3200"},
  {0xD8, "1/4000"}, {0xDC, "1/5000"}, {0xE0, "1/6400"},
  {0xE4, "1/8000"}, {0xFF, NULL}
};

tNKN_DataArray rpCameraFocusDistance[] = {
  {0x00, "-"},    {0x01, "5"},    {0x02, "5.5"},
  {0x03, "5.5"},  {0x04, "5.5"},  {0x05, "5.5"},
  {0x06, "6"},    {0x07, "6"},    {0x08, "6.5"},
  {0x09, "6.5"},  {0x0A, "6.5"},  {0x0B, "7"},
  {0x0C, "7"},    {0x0D, "7"},    {0x0E, "7.5"},
  {0x0F, "7.5"},  {0x10, "8"},    {0x11, "8"},
  {0x12, "8.5"},  {0x13, "8.5"},  {0x14, "9"},
  {0x15, "9"},    {0x16, "9.5"},  {0x17, "9.5"},
  {0x18, "10"},   {0x19, "10"},   {0x1A, "10.5"},
  {0x1B, "11"},   {0x1C, "11"},   {0x1D, "11.5"},
  {0x1E, "12"},   {0x1F, "12"},   {0x20, "13"},
  {0x21, "13"},   {0x22, "13"},   {0x23, "14"},
  {0x24, "14"},   {0x25, "14"},   {0x26, "15"},
  {0x27, "15"},   {0x28, "16"},   {0x29, "16"},
  {0x2A, "17"},   {0x2B, "17"},   {0x2C, "18"},
  {0x2D, "18"},   {0x2E, "19"},   {0x2F, "19"},
  {0x30, "20"},   {0x31, "20"},   {0x32, "21"},
  {0x33, "22"},   {0x34, "22"},   {0x35, "23"},
  {0x36, "24"},   {0x37, "24"},   {0x38, "25"},
  {0x39, "26"},   {0x3A, "26"},   {0x3B, "27"},
  {0x3C, "28"},   {0x3D, "28"},   {0x3E, "30"},
  {0x3F, "31"},   {0x40, "32"},   {0x41, "32"},
  {0x42, "34"},   {0x43, "34"},   {0x44, "35"},
  {0x45, "36"},   {0x46, "38"},   {0x47, "38"},
  {0x48, "40"},   {0x49, "40"},   {0x4A, "42"},
  {0x4B, "44"},   {0x4C, "45"},   {0x4D, "46"},
  {0x4E, "48"},   {0x4F, "48"},   {0x50, "50"},
  {0x51, "52"},   {0x52, "52"},   {0x53, "55"},
  {0x54, "56"},   {0x55, "58"},   {0x56, "60"},
  {0x57, "62"},   {0x58, "62"},   {0x59, "65"},
  {0x5A, "66"},   {0x5B, "68"},   {0x5C, "70"},
  {0x5D, "72"},   {0x5E, "75"},   {0x5F, "78"},
  {0x60, "80"},   {0x61, "82"},   {0x62, "85"},
  {0x63, "86"},   {0x64, "90"},   {0x65, "92"},
  {0x66, "95"},   {0x67, "98"},   {0x68, "100"},
  {0x69, "102"},  {0x6A, "105"},  {0x6B, "110"},
  {0x6C, "112"},  {0x6D, "116"},  {0x6E, "120"},
  {0x6F, "122"},  {0x70, "125"},  {0x71, "130"},
  {0x72, "135"},  {0x73, "135"},  {0x74, "140"},
  {0x75, "145"},  {0x76, "150"},  {0x77, "155"},
  {0x78, "160"},  {0x79, "165"},  {0x7A, "171"},
  {0x7B, "175"},  {0x7C, "180"},  {0x7D, "185"},
  {0x7E, "190"},  {0x7F, "195"},  {0x80, "200"},
  {0x81, "210"},  {0x82, "210"},  {0x83, "220"},
  {0x84, "220"},  {0x85, "230"},  {0x86, "240"},
  {0x87, "240"},  {0x88, "250"},  {0x89, "260"},
  {0x8A, "270"},  {0x8B, "270"},  {0x8C, "280"},
  {0x8D, "290"},  {0x8E, "300"},  {0x8F, "310"},
  {0x90, "320"},  {0x91, "330"},  {0x92, "340"},
  {0x93, "350"},  {0x94, "360"},  {0x95, "370"},
  {0x96, "380"},  {0x97, "390"},  {0x98, "400"},
  {0x99, "410"},  {0x9A, "420"},  {0x9B, "440"},
  {0x9C, "450"},  {0x9D, "460"},  {0x9E, "480"},
  {0x9F, "490"},  {0xA0, "500"},  {0xA1, "500"},
  {0xA2, "550"},  {0xA3, "550"},  {0xA4, "550"},
  {0xA5, "600"},  {0xA6, "600"},  {0xA7, "600"},
  {0xA8, "650"},  {0xA9, "650"},  {0xAA, "650"},
  {0xAB, "700"},  {0xAC, "700"},  {0xAD, "750"},
  {0xAE, "750"},  {0xAF, "800"},  {0xB0, "800"},
  {0xB1, "800"},  {0xB2, "850"},  {0xB3, "850"},
  {0xB4, "900"},  {0xB5, "900"},  {0xB6, "950"},
  {0xB7, "1000"}, {0xB8, "1000"}, {0xB9, "1050"},
  {0xBA, "1050"}, {0xBB, "1100"}, {0xBC, "1150"},
  {0xBD, "1150"}, {0xBE, "1200"}, {0xBF, "1250"},
  {0xC0, "1300"}, {0xC1, "1300"}, {0xC2, "1300"},
  {0xC3, "1400"}, {0xC4, "1400"}, {0xC5, "1500"},
  {0xC6, "1500"}, {0xC7, "1600"}, {0xC8, "1600"},
  {0xC9, "1600"}, {0xCA, "1700"}, {0xCB, "1700"},
  {0xCC, "1800"}, {0xCD, "1800"}, {0xCE, "1900"},
  {0xCF, "2000"}, {0xD0, "2000"}, {0xD1, "2100"},
  {0xD2, "2100"}, {0xD3, "2200"}, {0xD4, "2300"},
  {0xD5, "2300"}, {0xD6, "2400"}, {0xD7, "2500"},
  {0xD8, "2500"}, {0xD9, "2600"}, {0xDA, "2700"},
  {0xDB, "2800"}, {0xDC, "2900"}, {0xDD, "2900"},
  {0xDE, "3000"}, {0xDF, "3100"}, {0xE0, "3200"},
  {0xE1, "3300"}, {0xE2, "3400"}, {0xE3, "3500"},
  {0xE4, "3600"}, {0xE5, "3700"}, {0xE6, "3800"},
  {0xE7, "3900"}, {0xE8, "4000"}, {0xE9, "4100"},
  {0xEA, "4300"}, {0xEB, "4400"}, {0xEC, "4500"},
  {0xED, "4700"}, {0xEE, "4800"}, {0xEF, "4900"},
  {0xF0, "5100"}, {0xF1, "5200"}, {0xF2, "5400"},
  {0xF3, "5500"}, {0xF4, "5700"}, {0xF5, "5900"},
  {0xF6, "6000"}, {0xF7, "6200"}, {0xF8, "6400"},
  {0xF9, "6600"}, {0xFA, "6800"}, {0xFB, "7000"},
  {0xFC, "7200"}, {0xFD, "7400"}, {0xFE, "7600"},
  {0xFF, "INFINITE"}
};

tNKN_DataArray rpCameraApertureTable[] = {
  {0x00, "1"},   {0x01, "1"},   {0x02, "1.1"},
  {0x03, "1.1"}, {0x04, "1.1"}, {0x05, "1.2"},
  {0x06, "1.2"}, {0x07, "1.2"}, {0x08, "1.3"},
  {0x09, "1.3"}, {0x0A, "1.3"}, {0x0B, "1.4"},
  {0x0C, "1.4"}, {0x0D, "1.5"}, {0x0E, "1.5"},
  {0x0F, "1.5"}, {0x10, "1.6"}, {0x11, "1.6"},
  {0x12, "1.7"}, {0x13, "1.7"}, {0x14, "1.8"},
  {0x15, "1.8"}, {0x16, "1.9"}, {0x17, "1.9"},
  {0x18, "2"},   {0x19, "2.1"}, {0x1A, "2.1"},
  {0x1B, "2.2"}, {0x1C, "2.2"}, {0x1D, "2.3"},
  {0x1E, "2.4"}, {0x1F, "2.4"}, {0x20, "2.5"},
  {0x21, "2.6"}, {0x22, "2.7"}, {0x23, "2.7"},
  {0x24, "2.8"}, {0x25, "2.9"}, {0x26, "3"},
  {0x27, "3.1"}, {0x28, "3.2"}, {0x29, "3.3"},
  {0x2A, "3.4"}, {0x2B, "3.5"}, {0x2C, "3.6"},
  {0x2D, "3.7"}, {0x2E, "3.8"}, {0x2F, "3.9"},
  {0x30, "4"},   {0x31, "4.1"}, {0x32, "4.2"},
  {0x33, "4.4"}, {0x34, "4.5"}, {0x35, "4.6"},
  {0x36, "4.8"}, {0x37, "4.9"}, {0x38, "5"},
  {0x39, "5.2"}, {0x3A, "5.3"}, {0x3B, "5.5"},
  {0x3C, "5.6"}, {0x3D, "5.8"}, {0x3E, "6"},
  {0x3F, "6.2"}, {0x40, "6.3"}, {0x41, "6.5"},
  {0x42, "6.7"}, {0x43, "6.9"}, {0x44, "7.1"},
  {0x45, "7.3"}, {0x46, "7.6"}, {0x47, "8"},
  {0x48, "8"},   {0x49, "8.2"}, {0x4A, "8.5"},
  {0x4B, "8.7"}, {0x4C, "9"},   {0x4D, "9.2"},
  {0x4E, "9.5"}, {0x4F, "9.8"}, {0x50, "10"},
  {0x51, "10"},  {0x52, "10"},  {0x53, "11"},
  {0x54, "11"},  {0x55, "11"},  {0x56, "12"},
  {0x57, "12"},  {0x58, "12"},  {0x59, "13"},
  {0x5A, "13"},  {0x5B, "13"},  {0x5C, "14"},
  {0x5D, "14"},  {0x5E, "15"},  {0x5F, "15"},
  {0x60, "16"},  {0x61, "16"},  {0x62, "17"},
  {0x63, "17"},  {0x64, "18"},  {0x65, "18"},
  {0x66, "19"},  {0x67, "19"},  {0x68, "20"},
  {0x69, "20"},  {0x6A, "21"},  {0x6B, "22"},
  {0x6C, "22"},  {0x6D, "23"},  {0x6E, "24"},
  {0x6F, "24"},  {0x70, "25"},  {0x71, "26"},
  {0x72, "26"},  {0x73, "27"},  {0x74, "28"},
  {0x75, "29"},  {0x76, "30"},  {0x77, "31"},
  {0x78, "32"},  {0x79, "32"},  {0x7A, "33"},
  {0x7B, "34"},  {0x7C, "35"},  {0x7D, "37"},
  {0x7E, "38"},  {0x7F, "39"},  {0x80, "40"},
  {0x81, "41"},  {0x82, "42"},  {0x83, "44"},
  {0x84, "44"},  {0x85, "46"},  {0x86, "47"},
  {0x87, "49"},  {0x88, "50"},  {0x89, "52"},
  {0x8A, "53"},  {0x8B, "55"},  {0x8C, "57"},
  {0x8D, "58"},  {0x8E, "60"},  {0x8F, "62"},
  {0x90, "64"},  {0x91, "65"},  {0x92, "67"},
  {0x93, "69"},  {0x94, "71"},  {0x95, "73"},
  {0x96, "76"},  {0x97, "78"},  {0x98, "80"},
  {0x99, "83"},  {0x9A, "85"},  {0x9B, "87"},
  {0x9C, "90"},  {0x9D, "93"},  {0x9E, "95"},
  {0x9F, "98"},  {0xA0, "101"}, {0xA1, "104"},
  {0xA2, "107"}, {0xA3, "110"}, {0xA4, "114"},
  {0xA5, "117"}, {0xA6, "120"}, {0xA7, "124"},
  {0xA8, "128"}, {0xFF, NULL}
};

tNKN_DataArray rpCameraApertureEffective[] = {
  {0x00, "1"},   {0x01, "1.1"}, {0x02, "1.1"},
  {0x03, "1.2"}, {0x04, "1.2"}, {0x05, "1.3"},
  {0x06, "1.4"}, {0x07, "1.5"}, {0x08, "1.6"},
  {0x09, "1.7"}, {0x0A, "1.8"}, {0x0B, "1.9"},
  {0x0C, "2"},   {0x0D, "2.1"}, {0x0E, "2.2"},
  {0x0F, "2.4"}, {0x10, "2.5"}, {0x11, "2.7"},
  {0x12, "2.8"}, {0x13, "3"},   {0x14, "3.2"},
  {0x15, "3.3"}, {0x16, "3.5"}, {0x17, "3.8"},
  {0x18, "4"},   {0x19, "4.2"}, {0x1A, "4.5"},
  {0x1B, "4.8"}, {0x1C, "5"},   {0x1D, "5.3"},
  {0x1E, "5.6"}, {0x1F, "6"},   {0x20, "6.3"},
  {0x21, "6.7"}, {0x22, "7.1"}, {0x23, "7.6"},
  {0x24, "8"},   {0x25, "8.5"}, {0x26, "9"},
  {0x27, "9.5"}, {0x28, "10"},  {0x29, "11"},
  {0x2A, "11"},  {0x2B, "12"},  {0x2C, "13"},
  {0x2D, "13"},  {0x2E, "14"},  {0x2F, "15"},
  {0x30, "16"},  {0x31, "17"},  {0x32, "18"},
  {0x33, "19"},  {0x34, "20"},  {0x35, "21"},
  {0x36, "22"},  {0x37, "24"},  {0x38, "25"},
  {0x39, "27"},  {0x3A, "29"},  {0x3B, "30"},
  {0x3C, "32"},  {0x3D, "34"},  {0x3E, "36"},
  {0x3F, "38"},  {0x40, "40"},  {0x41, "43"},
  {0x42, "45"},  {0x43, "48"},  {0x44, "51"},
  {0x45, "54"},  {0x46, "57"},  {0x47, "60"},
  {0x48, "64"},  {0x49, "68"},  {0x4A, "72"},
  {0x4B, "76"},  {0x4C, "81"},  {0x4D, "85"},
  {0x4E, "90"},  {0x53, "EE"},  {0x54, "-"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraFlashCompensation[] = {
  {0x00, "0.0"},   {0x01, "-0.2"}, {0x02,  "-0.3"},
  {0x03, "-0.5"},  {0x04, "-0.7"}, {0x05,  "-0.8"},
  {0x06, "-1.0"},  {0x07, "-1.2"}, {0x08,  "-1.3"},
  {0x09, "-1.5"},  {0x0A, "-1.7"}, {0x0B,  "-1.8"},
  {0x0C, "-2.0"},  {0x0D, "-2.2"}, {0x0E,  "-2.3"},
  {0x0F, "-2.5"},  {0x10, "-2.7"}, {0x11,  "-2.8"},
  {0x12, "-3.0"},  {0x13, "-3.2"}, {0x14,  "-3.3"},
  {0x15, "-3.5"},  {0x16, "-3.7"}, {0x17,  "-3.8"},
  {0x18, "-4.0"},  {0x19, "-4.2"}, {0x1A,  "-4.3"},
  {0x1B, "-4.5"},  {0x1C, "-4.7"}, {0x1D,  "-4.8"},
  {0x1E, "-5.0"},  {0x1F, "-5.2"}, {0x20,  "-5.3"},
  {0x21, "-5.5"},  {0x22, "-5.7"}, {0x23,  "-5.8"},
  {0x24, "-6.0"},  {0x25, "-6.2"}, {0x26,  "-6.3"},
  {0x27, "-6.5"},  {0x28, "-6.7"}, {0x29,  "-6.8"},
  {0x2A, "-7.0"},  {0x2B, "-7.2"}, {0x2C,  "-7.3"},
  {0x2D, "-7.5"},  {0x2E, "-7.7"}, {0x2F,  "-7.8"},
  {0x30, "-8.0"},  {0x31, "-8.2"}, {0x32,  "-8.3"},
  {0x33, "-8.5"},  {0x34, "-8.7"}, {0x35,  "-8.8"},
  {0x36, "-9.0"},  {0x37, "-9.2"}, {0x38,  "-9.3"},
  {0x39, "-9.5"},  {0x3A, "-9.7"}, {0x3B,  "-9.8"},
  {0x3C, "-10.0"}, {0x3D, "-10.2"},{0x3E, "-10.3"},
  {0x3F, "-10.5"}, {0x40, "-10.7"},{0x41, "-10.8"},
  {0x42, "-11.0"}, {0x43, "-11.2"},{0x44, "-11.3"},
  {0x45, "-11.5"}, {0x46, "-11.7"},{0x47, "-11.8"},
  {0x48, "-12.0"}, {0x49, "-12.2"},{0x4A, "-12.3"},
  {0x4B, "-12.5"}, {0x4C, "-12.7"},{0x4D, "-12.8"},
  {0x4E, "-13.0"}, {0x4F, "-13.2"},{0x50, "-13.3"},
  {0x51, "-13.5"}, {0x52, "-13.7"},{0x53, "-13.8"},
  {0x54, "-14.0"}, {0x55, "-14.2"},{0x56, "-14.3"},
  {0x57, "-14.5"}, {0x58, "-14.7"},{0x59, "-14.8"},
  {0x5A, "-15.0"}, {0x5B, "-15.2"},{0x5C, "-15.3"},
  {0x5D, "-15.5"}, {0x5E, "-15.7"},{0x5F, "-15.8"},
  {0x60, "-16.0"}, {0x61, "-16.2"},{0x62, "-16.3"},
  {0x63, "-16.5"}, {0x64, "-16.7"},{0x65, "-16.8"},
  {0x66, "-17.0"}, {0x67, "-17.2"},{0x68, "-17.3"},
  {0x69, "-17.5"}, {0x6A, "-17.7"},{0x6B, "-17.8"},
  {0x6C, "-18.0"}, {0x6D, "-18.2"},{0x6E, "-18.3"},
  {0x6F, "-18.5"}, {0x70, "-18.7"},{0x71, "-18.8"},
  {0x72, "-19.0"}, {0x73, "-19.2"},{0x74, "-19.3"},
  {0x75, "-19.5"}, {0x76, "-19.7"},{0x77, "-19.8"},
  {0x78, "-20.0"}, {0x79, "-20.0"},{0x7A, "-20.0"},
  {0x7B, "-20.0"}, {0x7C, "-20.0"},{0x7D, "-20.0"},
  {0x7E, "-20.0"}, {0x7F, "-20.0"},{0x80, "+20.0"},
  {0x81, "+20.0"}, {0x82, "+20.0"},{0x83, "+20.0"},
  {0x84, "+20.0"}, {0x85, "+20.0"},{0x86, "+20.0"},
  {0x87, "+20.0"}, {0x88, "+20.0"},{0x89, "+19.8"},
  {0x8A, "+19.7"}, {0x8B, "+19.5"},{0x8C, "+19.3"},
  {0x8D, "+19.2"}, {0x8E, "+19.0"},{0x8F, "+18.8"},
  {0x90, "+18.7"}, {0x91, "+18.5"},{0x92, "+18.3"},
  {0x93, "+18.2"}, {0x94, "+18.0"},{0x95, "+17.8"},
  {0x96, "+17.7"}, {0x97, "+17.5"},{0x98, "+17.3"},
  {0x99, "+17.2"}, {0x9A, "+17.0"},{0x9B, "+16.8"},
  {0x9C, "+16.7"}, {0x9D, "+16.5"},{0x9E, "+16.3"},
  {0x9F, "+16.2"}, {0xA0, "+16.0"},{0xA1, "+15.8"},
  {0xA2, "+15.7"}, {0xA3, "+15.5"},{0xA4, "+15.3"},
  {0xA5, "+15.2"}, {0xA6, "+15.0"},{0xA7, "+14.8"},
  {0xA8, "+14.7"}, {0xA9, "+14.5"},{0xAA, "+14.3"},
  {0xAB, "+14.2"}, {0xAC, "+14.0"},{0xAD, "+13.8"},
  {0xAE, "+13.7"}, {0xAF, "+13.5"},{0xB0, "+13.3"},
  {0xB1, "+13.2"}, {0xB2, "+13.0"},{0xB3, "+12.8"},
  {0xB4, "+12.7"}, {0xB5, "+12.5"},{0xB6, "+12.3"},
  {0xB7, "+12.2"}, {0xB8, "+12.0"},{0xB9, "+11.8"},
  {0xBA, "+11.7"}, {0xBB, "+11.5"},{0xBC, "+11.3"},
  {0xBD, "+11.2"}, {0xBE, "+11.0"},{0xBF, "+10.8"},
  {0xC0, "+10.7"}, {0xC1, "+10.5"},{0xC2, "+10.3"},
  {0xC3, "+10.2"}, {0xC4, "+10.0"},{0xC5, "+9.8"},
  {0xC6, "+9.7"},  {0xC7, "+9.5"}, {0xC8,  "+9.3"},
  {0xC9, "+9.2"},  {0xCA, "+9.0"}, {0xCB,  "+8.8"},
  {0xCC, "+8.7"},  {0xCD, "+8.5"}, {0xCE,  "+8.3"},
  {0xCF, "+8.2"},  {0xD0, "+8.0"}, {0xD1,  "+7.8"},
  {0xD2, "+7.7"},  {0xD3, "+7.5"}, {0xD4,  "+7.3"},
  {0xD5, "+7.2"},  {0xD6, "+7.0"}, {0xD7,  "+6.8"},
  {0xD8, "+6.7"},  {0xD9, "+6.5"}, {0xDA,  "+6.3"},
  {0xDB, "+6.2"},  {0xDC, "+6.0"}, {0xDD,  "+5.8"},
  {0xDE, "+5.7"},  {0xDF, "+5.5"}, {0xE0,  "+5.3"},
  {0xE1, "+5.2"},  {0xE2, "+5.0"}, {0xE3,  "+4.8"},
  {0xE4, "+4.7"},  {0xE5, "+4.5"}, {0xE6,  "+4.3"},
  {0xE7, "+4.2"},  {0xE8, "+4.0"}, {0xE9,  "+3.8"},
  {0xEA, "+3.7"},  {0xEB, "+3.5"}, {0xEC,  "+3.3"},
  {0xED, "+3.2"},  {0xEE, "+3.0"}, {0xEF,  "+2.8"},
  {0xF0, "+2.7"},  {0xF1, "+2.5"}, {0xF2,  "+2.3"},
  {0xF3, "+2.2"},  {0xF4, "+2.0"}, {0xF5,  "+1.8"},
  {0xF6, "+1.7"},  {0xF7, "+1.5"}, {0xF8,  "+1.3"},
  {0xF9, "+1.2"},  {0xFA, "+1.0"}, {0xFB,  "+0.8"},
  {0xFC, "+0.7"},  {0xFD, "+0.5"}, {0xFE,  "+0.3"},
  {0xFF, "+0.2"}
};


tNKN_DataArray rpCameraExposureEffective[] = {
  {0x00, "ERR"},    {0x01, "BULB"},   {0x02, "NA"},
  {0x03, "NA"},     {0x04, "30"},     {0x05, "25"},
  {0x06, "20"},	    {0x07, "20"},     {0x08, "15"},
  {0x09, "13"},	    {0x0A, "10"},     {0x0B, "10"},
  {0x0C, "8"},      {0x0D, "6"},      {0x0E, "6"},
  {0x0F, "5"},      {0x10, "4"},      {0x11, "3"},
  {0x12, "3"},      {0x13, "2.5"},    {0x14, "2"},
  {0x15, "1.6"},    {0x16, "1.45"},   {0x17, "1.3"},
  {0x18, "1"},      {0x19, "1/1.3"},  {0x1A, "1/1.5"},
  {0x1B, "1/1.6"},  {0x1C, "1/2"},    {0x1D, "1/2.5"},
  {0x1E, "1/3"},    {0x1F, "1/3"},    {0x20, "1/4"},
  {0x21, "1/5"},    {0x22, "1/6"},    {0x23, "1/6"},
  {0x24, "1/8"},    {0x25, "1/10"},   {0x26, "1/10"},
  {0x27, "1/13"},   {0x28, "1/15"},   {0x29, "1/20"},
  {0x2A, "1/20"},   {0x2B, "1/25"},   {0x2C, "1/30"},
  {0x2D, "1/40"},   {0x2E, "1/45"},   {0x2F, "1/50"},
  {0x30, "1/60"},   {0x31, "1/80"},   {0x32, "1/90"},
  {0x33, "1/100"},  {0x34, "1/125"},  {0x35, "1/160"},
  {0x36, "1/180"},  {0x37, "1/200"},  {0x38, "1/250"},
  {0x39, "1/320"},  {0x3A, "1/350"},  {0x3B, "1/400"},
  {0x3C, "1/500"},  {0x3D, "1/640"},  {0x3E, "1/750"},
  {0x3F, "1/800"},  {0x40, "1/1000"}, {0x41, "1/1250"},
  {0x42, "1/1500"}, {0x43, "1/1600"}, {0x44, "1/2000"},
  {0x45, "1/2500"}, {0x46, "1/3000"}, {0x47, "1/3200"},
  {0x48, "1/4000"}, {0x49, "1/5000"}, {0x4A, "1/6000"},
  {0x4B, "1/6400"}, {0x4C, "1/8000"}, {0x4D, "NA"},
  {0x4E, "NA"},     {0x4F, "NA"},     {0x50, "NA"},
  {0x51, "NA"},     {0x52, "NA"},     {0x53, "NA"},
  {0x54, "BULB"},   {0x55, "HI"},     {0x56, "LOW"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraProgramMode[] = {
  {0x00, "Multi Program"},
  {0x01, "Shutter Priority"},
  {0x02, "Aperture Priority"},
  {0x03, "Manual"},
  {0x08, "Vari Program"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraProgramModeVariProgram[] = {
  {0x00, "Portrait"},
  {0x01, "Red Eye Reduction"},
  {0x02, "Hyper Focal"},
  {0x03, "Landscape"},
  {0x04, "Silhouette"},
  {0x05, "Sports"},
  {0x06, "Close Up"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraLightMetering[] = {
  {0x00, "Matrix"},
  {0x01, "Center Weighted"},
  {0x02, "Spot"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraMotorDrive[] = {
  {0x00, "Single"},
  {0x01, "Low Speed"},
  {0x02, "High Speed"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraFlashMode[] = {
  {0x00, "Normal Sync"},
  {0x01, "Slow Sync"},
  {0x02, "Rear Curtain Sync"},
  {0x03, "Red Eye Reduction"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraFocusArea[] = {
  {0x00, "Spot"},
  {0x01, "Wide"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraFocusMode[] = {
  {0x00, "Manual"},    /* continous overrided */
  {0x02, "Continous"},
  {0x04, "Manual"},
  {0x06, "Single"},
  {0xF6, NULL}
};

tNKN_DataArray rpCameraLensIdentifier[] = {
  {0x00, "No Lens Info Available"},
  {0x22, "AF Nikkor 70-210/4.5-5.6 D"},
  {0x25, "AF Micro Nikkor 60/2.8 D"},
  {0x36, "AF Nikkor 20/2.8 D"},
  {0x3A, "AF Nikkor 35-70/2.8 D"},
  {0x4F, "AF Nikkor 24-120/3.5-5.6 D"},
  {0x60, "AF Nikkor 80-200/2.8 D"},
  {0xFF, NULL}
};


tNKN_DataArray rpCameraCommandDial[] = {
  {0x00, "Normal"},
  {0x01, "Reverse"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraLongExposuresUses[] = {
  {0x00, "Bulb"},
  {0x01, "Time"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraFilmISOPriority[] = {
  {0x00, "Manual"},
  {0x01, "DX"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraMemoHolderDownload[] = {
  {0x00, "Stop Storing Memo Holder Data When Full"},
  {0x01, "Force Download Memo Holder Data When Full"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraDualRelease[] = {
  {0x00, "Independent"},
  {0x04, "Simultenous"},
  {0x08, "Alternative"},
  {0x0F, "Disabled"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraBracketing[] = {
  {0x0, "Disable"},
  {0x1, "Auto Sequence"},
  {0x2, "Bracket Exposure"},
  {0x4, "Bracket Flash"},
  {0x8, "Multiple Exposure"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraMemoHolderSettings[] = {
  {0x05, "Store Minimum Data"},
  {0x0E, "Store Medium Data"},
  {0x1F, "Store Maximum Data"},
  {0xFF, NULL}
};

tNKN_DataArray rpFlashInstallStatus[] = {
  {0x00, "Flash Not Installed"},
  {0x01, "Flash Installed"},
  {0xFF, NULL}
};

tNKN_DataArray rpFlashReady[] = {
  {0x00, "Flash Not Ready"},
  {0x01, "Flash Ready"},
  {0xFF, NULL}
};

tNKN_DataArray rpFocusStatus[] = {
  {0x00, "In Focus"},
  {0x01, "Out of Focus"},
  {0xFF, NULL}
};

tNKN_DataArray rpCameraGeneralStatus[] = {
  {0x00, "Off"},
  {0x01, "On"},
  {0xFF, NULL}
};

tNKN_DataArray rpNKN_ProgramMode[] = {
  {0x00, "M - Manual"},
  {0x01, "A - Aperature Priority"},
  {0x02, "S - Shutter Priority"},
  {0x03, "P - Multi Program"},
  {0x08, "Ps/Po - Portrait"},
  {0x09, "Ps/Re - Red-Eye Reduction"},
  {0x0A, "Ps/Hf - Hyperfocal"},
  {0x0B, "Ps/La - Landscape"},
  {0x0C, "Ps/Sl - Silhouette"},
  {0x0D, "Ps/Sp - Sports"},
  {0x0E, "Ps/Cu - Close-Up"},
  {0x0F, "P/CP - Custom Program"},
  {0xFF, NULL}
};

tNKN_DataArray rpNKN_LightMetering[] = {
  {0x00, "Center Weighted"},
  {0x01, "Spot"},
  {0x02, "Matrix"},
  {0xFF, NULL}
};

tNKN_DataArray rpNKN_FlashMode[] = {
  {0x00, "Normal Sync"},
  {0x01, "Slow Sync"},
  {0x02, "Rear Curtain Sync"},
  {0x03, "-"},
  {0xFF, NULL}
};

tNKN_DataArray rpNKN_Compensation[] = {
  {0x00, "0.0"},    {0x01, "-0.2"},   {0x02, "-0.3"},   {0x03, "-0.5"},
  {0x04, "-0.7"},   {0x05, "-0.8"},   {0x06, "-1.0"},   {0x07, "-1.2"},
  {0x08, "-1.3"},   {0x09, "-1.5"},   {0x0A, "-1.7"},   {0x0B, "-1.8"},
  {0x0C, "-2.0"},   {0x0D, "-2.2"},   {0x0E, "-2.3"},   {0x0F, "-2.5"},
  {0x10, "-2.7"},   {0x11, "-2.8"},   {0x12, "-3.0"},   {0x13, "-3.2"},
  {0x14, "-3.3"},   {0x15, "-3.5"},   {0x16, "-3.7"},   {0x17, "-3.8"},
  {0x18, "-4.0"},   {0x19, "-4.2"},   {0x1A, "-4.3"},   {0x1B, "-4.5"},
  {0x1C, "-4.7"},   {0x1D, "-4.8"},   {0x1E, "-5.0"},   {0x1F, "-5.2"},
  {0x20, "-5.3"},   {0x21, "-5.5"},   {0x22, "-5.7"},   {0x23, "-5.8"},
  {0x24, "-6.0"},   {0x25, "-6.2"},   {0x26, "-6.3"},   {0x27, "-6.5"},
  {0x28, "-6.7"},   {0x29, "-6.8"},   {0x2A, "-7.0"},   {0x2B, "-7.2"},
  {0x2C, "-7.3"},   {0x2D, "-7.5"},   {0x2E, "-7.7"},   {0x2F, "-7.8"},
  {0x30, "-8.0"},   {0x31, "-8.2"},   {0x32, "-8.3"},   {0x33, "-8.5"},
  {0x34, "-8.7"},   {0x35, "-8.8"},   {0x36, "-9.0"},   {0x37, "-9.2"},
  {0x38, "-9.3"},   {0x39, "-9.5"},   {0x3A, "-9.7"},   {0x3B, "-9.8"},
  {0x3C, "-10.0"},  {0x3D, "-10.2"},  {0x3E, "-10.3"},  {0x3F, "-10.5"},
  {0x40, "-10.7"},  {0x41, "-10.8"},  {0x42, "-11.0"},  {0x43, "-11.2"},
  {0x44, "-11.3"},  {0x45, "-11.5"},  {0x46, "-11.7"},  {0x47, "-11.8"},
  {0x48, "-12.0"},  {0x49, "-12.2"},  {0x4A, "-12.3"},  {0x4B, "-12.5"},
  {0x4C, "-12.7"},  {0x4D, "-12.8"},  {0x4E, "-13.0"},  {0x4F, "-13.2"},
  {0x50, "-13.3"},  {0x51, "-13.5"},  {0x52, "-13.7"},  {0x53, "-13.8"},
  {0x54, "-14.0"},  {0x55, "-14.2"},  {0x56, "-14.3"},  {0x57, "-14.5"},
  {0x58, "-14.7"},  {0x59, "-14.8"},  {0x5A, "-15.0"},  {0x5B, "-15.2"},
  {0x5C, "-15.3"},  {0x5D, "-15.5"},  {0x5E, "-15.7"},  {0x5F, "-15.8"},
  {0x60, "-16.0"},  {0x61, "-16.2"},  {0x62, "-16.3"},  {0x63, "-16.5"},
  {0x64, "-16.7"},  {0x65, "-16.8"},  {0x66, "-17.0"},  {0x67, "-17.2"},
  {0x68, "-17.3"},  {0x69, "-17.5"},  {0x6A, "-17.7"},  {0x6B, "-17.8"},
  {0x6C, "-18.0"},  {0x6D, "-18.2"},  {0x6E, "-18.3"},  {0x6F, "-18.5"},
  {0x70, "-18.7"},  {0x71, "-18.8"},  {0x72, "-19.0"},  {0x73, "-19.2"},
  {0x74, "-19.3"},  {0x75, "-19.5"},  {0x76, "-19.7"},  {0x77, "-19.8"},
  {0x78, "-20.0"},  {0x79, "-20.0"},  {0x7A, "-20.0"},  {0x7B, "-20.0"},
  {0x7C, "-20.0"},  {0x7D, "-20.0"},  {0x7E, "-20.0"},  {0x7F, "-20.0"},
  {0x80, "+20.0"},  {0x81, "+20.0"},  {0x82, "+20.0"},  {0x83, "+20.0"},
  {0x84, "+20.0"},  {0x85, "+20.0"},  {0x86, "+20.0"},  {0x87, "+20.0"},
  {0x88, "+20.0"},  {0x89, "+19.8"},  {0x8A, "+19.7"},  {0x8B, "+19.5"},
  {0x8C, "+19.3"},  {0x8D, "+19.2"},  {0x8E, "+19.0"},  {0x8F, "+18.8"},
  {0x90, "+18.7"},  {0x91, "+18.5"},  {0x92, "+18.3"},  {0x93, "+18.2"},
  {0x94, "+18.0"},  {0x95, "+17.8"},  {0x96, "+17.7"},  {0x97, "+17.5"},
  {0x98, "+17.3"},  {0x99, "+17.2"},  {0x9A, "+17.0"},  {0x9B, "+16.8"},
  {0x9C, "+16.7"},  {0x9D, "+16.5"},  {0x9E, "+16.3"},  {0x9F, "+16.2"},
  {0xA0, "+16.0"},  {0xA1, "+15.8"},  {0xA2, "+15.7"},  {0xA3, "+15.5"},
  {0xA4, "+15.3"},  {0xA5, "+15.2"},  {0xA6, "+15.0"},  {0xA7, "+14.8"},
  {0xA8, "+14.7"},  {0xA9, "+14.5"},  {0xAA, "+14.3"},  {0xAB, "+14.2"},
  {0xAC, "+14.0"},  {0xAD, "+13.8"},  {0xAE, "+13.7"},  {0xAF, "+13.5"},
  {0xB0, "+13.3"},  {0xB1, "+13.2"},  {0xB2, "+13.0"},  {0xB3, "+12.8"},
  {0xB4, "+12.7"},  {0xB5, "+12.5"},  {0xB6, "+12.3"},  {0xB7, "+12.2"},
  {0xB8, "+12.0"},  {0xB9, "+11.8"},  {0xBA, "+11.7"},  {0xBB, "+11.5"},
  {0xBC, "+11.3"},  {0xBD, "+11.2"},  {0xBE, "+11.0"},  {0xBF, "+10.8"},
  {0xC0, "+10.7"},  {0xC1, "+10.5"},  {0xC2, "+10.3"},  {0xC3, "+10.2"},
  {0xC4, "+10.0"},  {0xC5, "+9.8"},   {0xC6, "+9.7"},   {0xC7, "+9.5"},
  {0xC8, "+9.3"},   {0xC9, "+9.2"},   {0xCA, "+9.0"},   {0xCB, "+8.8"},
  {0xCC, "+8.7"},   {0xCD, "+8.5"},   {0xCE, "+8.3"},   {0xCF, "+8.2"},
  {0xD0, "+8.0"},   {0xD1, "+7.8"},   {0xD2, "+7.7"},   {0xD3, "+7.5"},
  {0xD4, "+7.3"},   {0xD5, "+7.2"},   {0xD6, "+7.0"},   {0xD7, "+6.8"},
  {0xD8, "+6.7"},   {0xD9, "+6.5"},   {0xDA, "+6.3"},   {0xDB, "+6.2"},
  {0xDC, "+6.0"},   {0xDD, "+5.8"},   {0xDE, "+5.7"},   {0xDF, "+5.5"},
  {0xE0, "+5.3"},   {0xE1, "+5.2"},   {0xE2, "+5.0"},   {0xE3, "+4.8"},
  {0xE4, "+4.7"},   {0xE5, "+4.5"},   {0xE6, "+4.3"},   {0xE7, "+4.2"},
  {0xE8, "+4.0"},   {0xE9, "+3.8"},   {0xEA, "+3.7"},   {0xEB, "+3.5"},
  {0xEC, "+3.3"},   {0xED, "+3.2"},   {0xEE, "+3.0"},   {0xEF, "+2.8"},
  {0xF0, "+2.7"},   {0xF1, "+2.5"},   {0xF2, "+2.3"},   {0xF3, "+2.2"},
  {0xF4, "+2.0"},   {0xF5, "+1.8"},   {0xF6, "+1.7"},   {0xF7, "+1.5"},
  {0xF8, "+1.3"},   {0xF9, "+1.2"},   {0xFA, "+1.0"},   {0xFB, "+0.8"},
  {0xFC, "+0.7"},   {0xFD, "+0.5"},   {0xFE, "+0.3"},   {0xFF, "+0.2"}
};
