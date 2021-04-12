/*

  tty.h

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

#ifndef _TTY_H
#define _TTY_H

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

#define TTY_BAUD_0       B0
#define TTY_BAUD_50      B50
#define TTY_BAUD_75      B75
#define TTY_BAUD_110     B110
#define TTY_BAUD_134     B134
#define TTY_BAUD_150     B150
#define TTY_BAUD_200     B200
#define TTY_BAUD_300     B300
#define TTY_BAUD_600     B600
#define TTY_BAUD_1200    B1200
#define TTY_BAUD_1800    B1800
#define TTY_BAUD_2400    B2400
#define TTY_BAUD_4800    B4800
#define TTY_BAUD_9600    B9600
#define TTY_BAUD_19200   B19200
#define TTY_BAUD_38400   B38400
#define TTY_BAUD_57600   B57600
#define TTY_BAUD_115200  B115200
#define TTY_BAUD_230400  B230400
#define TTY_BAUD_DEFAULT B1200

uint16 ui16TTY_Open      (uint16 *, uint8 *, uint16);
uint16 ui16TTY_Close     (uint16);
uint16 ui16TTY_SetBaud   (uint16, uint16);
uint16 ui16TTY_Read      (uint16, uint8 *, uint16);
uint16 ui16TTY_Write     (uint16, uint8 *, uint16);
uint16 ui16TTY_Flush     (uint16);

#endif
