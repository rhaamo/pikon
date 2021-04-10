/*

  common.h

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


#ifndef _COMMON_H
#define _COMMON_H

#define OK    0
#define FAIL  1

#define FALSE 0
#define TRUE  1

#define DISABLE 0
#define ENABLE  1

typedef signed char   sint8;
typedef unsigned char uint8;
typedef signed int    sint16;
typedef unsigned int  uint16;
typedef signed long   sint32;
typedef unsigned long uint32;

#if 0
# define DEBUG(a) { printf a; }
#else
# define DEBUG(a)
#endif

#endif
