/*

  tty.c

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

#include "common.h"
#include "tty.h"

uint16 ui16TTY_Open(uint16 *ui16FileDescriptor, uint8 *pui8Path, uint16 ui16Baud) {
  DEBUG(("opening path %s\n", pui8Path));

  if ((*ui16FileDescriptor = open(pui8Path, O_RDWR | O_NDELAY )) < 0) {
    DEBUG(("Cannot open %s\n", pui8Path));
    return FAIL;
  }

  if (ui16TTY_SetBaud(*ui16FileDescriptor, ui16Baud) < 0) {
    DEBUG(("Cannot set speed to: %d\n", ui16Baud));
    close(*ui16FileDescriptor);
    return FAIL;
  }

  return OK;
}


uint16 ui16TTY_Close(uint16 ui16FileDescriptor) {
  return close(ui16FileDescriptor);
}

uint16 ui16TTY_SetBaud(uint16 ui16FileDescriptor, uint16 ui16Baud) {
  struct termios tio;

  if (tcgetattr(ui16FileDescriptor, &tio) < 0) {
    DEBUG(("Cannot get tty attribute\n"));
    return FAIL;
  }

  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_cflag = CS8 | CREAD | CLOCAL  ; /* 8bit non parity stop 1 */
  tio.c_lflag = 0;
  tio.c_cc[VMIN] = 1;
  tio.c_cc[VTIME] = 5 ;
  cfsetispeed(&tio, ui16Baud);
  cfsetospeed(&tio, ui16Baud);

  if (tcsetattr(ui16FileDescriptor, TCSANOW, &tio) < 0) {
    perror("Cannot set tty attribute\n");
    return FAIL;
  }

  if (ui16TTY_Flush(ui16FileDescriptor) < 0) {
    perror("Cannot flush tty\n");
    return FAIL;
  }

  return OK;
}

uint16 ui16TTY_Flush(uint16 ui16FileDescriptor) {
  uint8 ui8Buffer;
  fd_set rFileDescriptorSet;
  struct timeval rTime;

  FD_ZERO(&rFileDescriptorSet);
  FD_SET(ui16FileDescriptor, &rFileDescriptorSet);
  rTime.tv_sec = 0;
  rTime.tv_usec = 0;

  while (TRUE) {
    switch(select(ui16FileDescriptor +1 , &rFileDescriptorSet, NULL, NULL, &rTime)) {
    case -1:
      DEBUG(("tty read fail.\n"));
      return FAIL;
    case 0:
      // already flushed
      return OK;
    default:
      // flush garbage
      if (FD_ISSET(ui16FileDescriptor, &rFileDescriptorSet)) {
        if (read(ui16FileDescriptor, &ui8Buffer, 1) < 0) {
          DEBUG(("tty read fail.\n"));
          return FAIL;
        }
      }
    }
  }
}

uint16 ui16TTY_Read(uint16 ui16FileDescriptor, uint8 *pui8Buffer, uint16 ui16Length) {
  fd_set rFileDescriptorSet;
  struct timeval rTime;

  FD_ZERO(&rFileDescriptorSet);
  FD_SET(ui16FileDescriptor, &rFileDescriptorSet);
  rTime.tv_sec = 0;
  rTime.tv_usec = 100000;

  switch(select(ui16FileDescriptor +1 , &rFileDescriptorSet, NULL, NULL, &rTime)) {
  case -1:
    DEBUG(("tty read fail.\n"));
    return FAIL;
  case 0:
    return OK;
  default:
    // read data
    if (FD_ISSET(ui16FileDescriptor, &rFileDescriptorSet)) {
      if (read(ui16FileDescriptor, pui8Buffer, ui16Length) < 0) {
        DEBUG(("tty read fail.\n"));
        return FAIL;
      }
    }
  }
  return OK;
}


uint16 ui16TTY_Write(uint16 ui16FileDescriptor, uint8 *pui8Buffer, uint16 ui16Length) {
  uint16 ui16BytesWritten;

  ui16BytesWritten = write(ui16FileDescriptor, pui8Buffer, ui16Length);
  return ui16BytesWritten == ui16Length ? OK : FAIL;
}
