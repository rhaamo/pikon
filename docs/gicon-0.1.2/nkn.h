/*

  nkn.h

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

#ifndef _NIKON_H
#define _NIKON_H

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

#define WAKEUP                                         0x00
#define SIGNIN_START_BYTE                              0X53
#define SIGNIN_SEQUENCE                                0x31303030
#define SIGNIN_SEQUENCE_LENGHT                         4
#define SIGNIN_STOP_BYTE                               0X05

#define COMMAND_START_BYTE                             0x01
#define COMMAND_STOP_BYTE                              0x03

#define COMMAND_CFLAG                                  0x20

#define COMMAND_MODE_READ_DATA                         0x80
#define COMMAND_MODE_WRITE_DATA                        0x81
#define COMMAND_MODE_SHUTTER_RELEASE                   0x85
#define COMMAND_MODE_FOCUS_CAMERA                      0x86
#define COMMAND_MODE_CHANGE_BAUD                       0x87
#define COMMAND_MODE_MEMO_HOLDER_INFO                  0x1B

#define DATA_START_BYTE                                0x02
#define DATA_STOP_BYTE                                 0x03

#define SIGNOFF_START_BYTE                             0x04
#define SIGNOFF_STOP_BYTE                              0x04

#define STATUS_START_BYTE                              0x06
#define STATUS_STOP_BYTE                               0x00

#define IDENTIFICATION_START_BYTE                      0x31
#define IDENTIFICATION_STRING_SIZE                     0x0E
#define IDENTIFICATION_STOP_BYTE                       0x06

#define OFFSET_MEMO_HOLDER_ADDRESS                     0x010000

#define M_RD                                           0x00 /* read only register */
#define M_WR                                           0x01 /* write only register */
#define M_RW                                           0x02 /* read write register */
#define M_SR                                           0x03 /* special register: shutter release */
#define M_AF                                           0x04 /* special register: auto focus */
#define M_CB                                           0x05 /* special register: change baud */
#define M_MH                                           0x06 /* special register: memo holder */

#define NKN_READ_CHECKSUM_FAIL  0
#define NKN_READ_DATA_OK        1
#define NKN_READ_DATA_FAIL      2
#define NKN_READ_STATUS_OK      3
#define NKN_READ_STATUS_FAIL    4
#define NKN_READ_SIGNOFF_OK     5
#define NKN_READ_SIGNOFF_FAIL   6
#define NKN_READ_SIGNIN_OK      7
#define NKN_READ_SIGNIN_FAIL    8
#define NKN_READ_FAIL           9

/*      macro                                          mode  address  lng mask    shitf */
#define CAMERA_COMMAND_SHUTTER_RELEASE                 M_SR, 0x000000, 0, 0x0000,  0
#define CAMERA_COMMAND_FOCUS                           M_AF, 0x000000, 0, 0x0000,  0
#define CAMERA_COMMAND_CHANGE_BAUD                     M_CB, 0x050000, 0, 0x0000,  0
#define CAMERA_MEMO_HOLDER                             M_MH, 0x920000, 0, 0x0000,  0
#define CAMERA_BASE_MEMO_HOLDER                        M_RD, 0x00FD00, 2, 0xFFFF,  0
#define CAMERA_END_MEMO_HOLDER                         M_RD, 0x00FD02, 2, 0xFFFF,  0   /* Memo holder starts at 0x010000 + location FD00 and runs through 0x010000 + location FD02 */
#define CAMERA_STAUS                                   M_RD, 0x00FD20, 1, 0x00FF,  0
#define CAMERA_FRAME_NUMBER                            M_RD, 0x00FD21, 1, 0x00FF,  0   /* 0x00 = No film, 0x02.. frame + 1 */
#define CAMERA_TOTAL_FRAMES                            M_RD, 0x00FD22, 2, 0xFFFF,  0
#define CAMERA_EXPOSURE                                M_RW, 0x00FD25, 1, 0x00FF,  0
#define CAMERA_PROGRAM_MODE                            M_RW, 0x00FD26, 1, 0x00FF,  0
#define CAMERA_PROGRAM_MODE_VARI_PROGRAM               M_RW, 0x00FD27, 1, 0x00FF,  0
#define CAMERA_LIGHT_METERING                          M_RW, 0x00FD28, 1, 0x00FF,  0
#define CAMERA_MOTOR_DRIVE                             M_RW, 0x00FD29, 1, 0x00FF,  0
#define CAMERA_FLASH_MODE                              M_RW, 0x00FD2A, 1, 0x00FF,  0
#define CAMERA_FOCUS_AREA                              M_RW, 0x00FD2B, 1, 0x00FF,  0
#define CAMERA_FILM_ISO                                M_RW, 0x00FD2C, 1, 0x00FF,  0
#define CAMERA_EXPOSURE_COMPENSATION                   M_RW, 0x00FD2D, 1, 0x00FF,  0
#define CAMERA_SELF_TIMER                              M_RW, 0x00FD2E, 1, 0x00FF,  0  /* 0x00  = 2 seconds, 0x1C  = 30 seconds */
#define CAMERA_FLASH_SYNC_SPEED                        M_RW, 0x00FD30, 1, 0x00FF,  0
#define CAMERA_COMMAND_DIAL                            M_RW, 0x00FD31, 1, 0x0001,  0
#define CAMERA_EASY_COMPENSATION_IN_APERTURE_MODE      M_RW, 0x00FD31, 1, 0x0004,  2
#define CAMERA_VIEWFINDER_MATRIX_CENTER_DELTA          M_RW, 0x00FD31, 1, 0x0008,  3
#define CAMERA_BEEP_ON_EXPOSURE_ERROR                  M_RW, 0x00FD32, 1, 0x0001,  0
#define CAMERA_BEEP_WHEN_IN_FOCUS                      M_RW, 0x00FD32, 1, 0x0002,  1
#define CAMERA_BEEP_ON_FILM_ERROR                      M_RW, 0x00FD32, 1, 0x0004,  2
#define CAMERA_BEEP_WHEN_SELF_TIMER_ON                 M_RW, 0x00FD32, 1, 0x0008,  3
#define CAMERA_LCD_LIGHT_ON_TIME                       M_RW, 0x00FD33, 1, 0x00FF,  0  /* 0x00     = Default [8 seconds] 0x04..0x40  = 4-64 second */
#define CAMERA_LONG_EXPOSURES_USES                     M_RW, 0x00FD34, 1, 0x0004,  2
#define CAMERA_FILM_ISO_PRIORITY                       M_RW, 0x00FD34, 1, 0x0008,  3
#define CAMERA_FRAME_COUNTER_IN_PS_MODE                M_RW, 0x00FD34, 1, 0x0020,  5
#define CAMERA_PRINT_ROLL_NUMBER_ON_FRAME_0            M_RW, 0x00FD34, 1, 0x0040,  6
#define CAMERA_MEMO_HOLDER_DOWNLOAD                    M_RW, 0x00FD34, 1, 0x0080,  7
#define CAMERA_DUAL_RELEASE                            M_RW, 0x00FD35, 1, 0x000F,  0
#define CAMERA_FOCUS_RELEASE_PRIORITY_IN_CAM_MODE      M_RW, 0x00FD35, 1, 0x0010,  4
#define CAMERA_FOCUS_LOCK_IN_CAF_MODE                  M_RW, 0x00FD35, 1, 0x0020,  5
#define CAMERA_FOCUS_RELEASE_PRIORITY_IN_SAM_MODE      M_RW, 0x00FD35, 1, 0x0080,  7
#define CAMERA_FOCUS_TRAP                              M_RW, 0x00FD3A, 1, 0x0001,  0
#define CAMERA_FOCUS_TRAP_SIMULTANEOUS_AF_AE_LOCK      M_RW, 0x00FD3A, 1, 0x0004,  2
#define CAMERA_BRACKETING                              M_RW, 0x00FD3C, 1, 0x00F0,  7
#define CAMERA_BRACKETING_STEP                         M_RW, 0x00FD3C, 1, 0x000F,  0
#define CAMERA_SEQUENCE_FRAME_COUNTER                  M_RW, 0x00FD3D, 1, 0x00FF,  0
#define CAMERA_ROLL_NUMBER                             M_RD, 0x00FD3E, 1, 0x00FF,  0
#define CAMERA_MEMO_HOLDER_ENABLE                      M_RW, 0x00FD40, 1, 0x0040,  6
#define CAMERA_MEMO_HOLDER_SETTINGS                    M_RW, 0x00FD40, 1, 0x001F,  0
#define CAMERA_CURRENT_MEMO_POINTER                    M_RD, 0x00FD42, 2, 0xFFFF,  0
#define CAMERA_MEMO_HOLDER_START_POINTER               M_RW, 0x00FD44, 2, 0xFFFF,  0
#define CAMERA_ROLL_START_POINTER                      M_RD, 0x00FD46, 2, 0xFFFF,  0
#define CAMERA_FLASH_INSTALL_STATUS                    M_RD, 0x00FD89, 1, 0x0080,  7
#define CAMERA_FLASH_SLOW_SYNC_STATUS                  M_RD, 0x00FD89, 1, 0x0004,  2
#define CAMERA_FLASH_READY                             M_RD, 0x00FD89, 1, 0x0002,  1
#define CAMERA_FLASH_COMPENSATION_EFFECTIVE            M_RW, 0x00FD8F, 1, 0x00FF,  0  /*  rpCameraFlashCompensation[((FD8F - 0x5A) - (FD90 * 2))] */
#define CAMERA_FILM_ISO_EFFECTIVE                      M_RW, 0x00FD90, 1, 0x00FF,  0
#define CAMERA_LENS_FOCUS_MODE                         M_RD, 0x00FE20, 1, 0x00F0,  4
#define CAMERA_EXPOSURE_METERED                        M_RD, 0x00FE24, 1, 0x00FF,  0
#define CAMERA_APERTURE_CURRENT                        M_RD, 0x00FE25, 1, 0x00FF,  0
#define CAMERA_LIGHT_METER_READING                     M_RD, 0x00FE27, 1, 0x00FF,  0
#define CAMERA_APERTURE_ZOOMED                         M_RD, 0x00FE29, 1, 0x00FF,  0
#define CAMERA_FOCUS_STATUS                            M_RD, 0x00FE2B, 1, 0x00FF,  0 /* in focus if > 0xfb */
#define CAMERA_FOCUS_DISTANCE                          M_RD, 0x00FE2D, 1, 0x00FF,  0  /* Focus distance cm = 10^(x/40) */
#define CAMERA_ZOOM_CURRENT                            M_RD, 0x00FE2E, 1, 0x00FF,  0
#define CAMERA_FOCAL_LENGTH_LOWER                      M_RD, 0x00FE2F, 1, 0x00FF,  0
#define CAMERA_FOCAL_LENGTH_UPPER                      M_RD, 0x00FE30, 1, 0x00FF,  0
#define CAMERA_APERTURE_RANGE_LOWER                    M_RD, 0x00FE31, 1, 0x00FF,  0
#define CAMERA_APERTURE_RANGE_UPPER                    M_RD, 0x00FE32, 1, 0x00FF,  0
#define CAMERA_LENS_IDENTIFIER                         M_RD, 0x00FE33, 1, 0x00FF,  0
#define CAMERA_FLASH_COMPENSATION                      M_RW, 0x00FE39, 1, 0x00FF,  0
#define CAMERA_LCD_LIGHT                               M_RW, 0x00FE46, 1, 0x0008,  3
#define CAMERA_FLASH_COMPENSATION_2                    M_RW, 0x00FE4F, 1, 0x00FF,  0  /* Flash Compensation (duplicate of 0x00FE39 ?) */
#define CAMERA_EXPOSURE_EFFECTIVE                      M_RW, 0x00FE50, 1, 0x00FF,  0
#define CAMERA_APERTURE_EFFECTIVE                      M_RW, 0x00FE51, 1, 0x00FF,  0

typedef struct{
  uint8 ui8Value;
  uint8 *pui8Description;
} tNKN_DataArray;

uint16 ui16NKN_WakeUp        (uint16 *, uint8 *);
uint16 ui16NKN_SignIn        (uint16);
uint16 ui16NKN_SignOff       (uint16);

uint16 ui16NKN_Read16        (uint16, uint8, uint32, uint16, uint16, uint8, uint16 *);
uint16 ui16NKN_Write16       (uint16, uint8, uint32, uint16, uint16, uint8, uint16);
uint16 ui16NKN_SetBaud       (uint16, uint16);
uint16 ui16NKN_Focus         (uint16);
uint16 ui16NKN_Release       (uint16);
uint16 ui16NKN_Download      (uint16, uint16 *, uint16 *);

void   vNKN_DataToString     (tNKN_DataArray *, uint16 , uint8 **);
void   vNKN_StringToData     (tNKN_DataArray *, uint8 *, uint16 *);

#endif
