/*

  print.h

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

#ifndef _PRINT_H
#define _PRINT_H

#include "common.h"
#include "file.h"

typedef struct{
  uint8 *pui8PaperName;
  uint16 ui16PaperWidth;
  uint16 ui16PaperHeight;
} tPaperSize;

typedef struct{
  uint8  ui8Orientation;
  uint8  ui8PrintCaption;
  uint8  ui8PrintTitle;
  uint8  ui8Columns;
  float  fLabelWidth;
  float  fLabelHeight;
  float  fHorizontalOffset;
  float  fVerticalOffset;
  float  fColumnSpacing;
  float  fRowSpacing;
} tPrintLayout;


sint16 si16PrintSlide(uint8 *, uint8 *, uint8 *, tRoll *, tPrintLayout *);

#endif
