/*

  util.c

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "common.h"
#include "util.h"

uint8 ui8DaysPerMonths[2][12] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

uint8 pui8MonthName[12][3] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

uint32 ui32InverseCtime(uint8 *pui8String) {
  uint16 ui16Year;
  uint8  ui8Month;
  uint8  ui8Days;
  uint8  ui8Hours;
  uint8  ui8Minutes;
  uint8  ui8Seconds;
  uint8  ui8Index;

  uint8 pui8Buffer[3];
  uint8 pui8Buffer4[5];

  uint32 ui32TotalDays;
  uint32 ui32TotalSeconds;
  uint8  ui8LongYear;

  pui8String += 4; /* skip the day */

  for (ui8Index = 0; ui8Index < 12; ui8Index++)
    if (strncmp(pui8String, pui8MonthName[ui8Index], 3) == 0) {
      ui8Month = ui8Index;
      break;
    }

  pui8String += 4;
  strncpy(pui8Buffer, pui8String, 2);
  ui8Days = atoi(pui8Buffer);
  pui8String += 3;
  strncpy(pui8Buffer, pui8String, 2);
  ui8Hours = atoi(pui8Buffer);
  pui8String += 3;
  strncpy(pui8Buffer, pui8String, 2);
  ui8Minutes = atoi(pui8Buffer);
  pui8String += 3;
  strncpy(pui8Buffer, pui8String, 2);
  ui8Seconds = atoi(pui8Buffer);
  pui8String += 3;
  strncpy(pui8Buffer4, pui8String, 4);
  ui16Year = atol(pui8Buffer4);

  ui8LongYear = (((ui16Year - 1970 + 2) % 4) == 0);

  ui32TotalDays = 365 * (ui16Year - 1970);/* days past before current year, current year does not count */
  ui32TotalDays += (ui16Year - 1970 + 2) / 4; /* extra days past whitin this period */

  if (ui8LongYear)
    ui32TotalDays --;

  for (ui8Index = 0; ui8Index < ui8Month; ui8Index++)
    ui32TotalDays += ui8DaysPerMonths[ui8LongYear][ui8Index];

  ui32TotalDays += ui8Days - 1; /* -1 because the last day does not count */
  ui32TotalSeconds = ui8Seconds;
  ui32TotalSeconds += ui8Minutes * 60;
  ui32TotalSeconds += ui8Hours * 3600;
  ui32TotalSeconds += ui32TotalDays * 86400;

  return ui32TotalSeconds;
}

sint16 si16UtilLaunchHelpBrowser(uint8 *pui8HelpPath) {
  pid_t pid;
  uint8 pui8FullHelpPath[500];

  sprintf(pui8FullHelpPath, "%s/%s",  getenv("GICON_HOME"), pui8HelpPath);
  pid = fork();

  if (pid < 0) {
    DEBUG(("Fork failed.\n"));
    return FAIL;
  }

  if (pid == 0) {
    char *args[3];
    args[0]=getenv("GICON_HELP_BROWSER");
    args[1]=pui8FullHelpPath;
    args[2]=NULL;
    execvp(args[0], args);
  }
  return OK;
}
