/*

  gui.notebook.h

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
#include <gnome.h>
#include <glade/glade.h>
#include <math.h>

#include "common.h"
#include "file.h"

typedef struct _list{
  uint8     ui8Changed;
  uint8     ui8FrameCounter;
  uint8     ui8StorageLevel;
  uint32    ui32TimeStamp;
  GtkWidget *pCListMain;
  GtkWidget *pEntryMainFileName;
  GtkWidget *pEntryMainRollTitle;
  GtkWidget *pEntryMainCameraType;
  GtkWidget *pEntryMainTimeStamp;
  GtkWidget *pEntryMainFilmSpeed;
  GtkWidget *pEntryMainCaption;
  struct _list *pNext;
} tMyList;

void     vNoteBook_AddPage         (GladeXML  *, tRoll *, uint8);
void     vNoteBook_RemovePage      (uint16);
void     vNoteBook_GetRoll         (uint16, tRoll *);
tMyList *pNoteBook_GetPageByIndex  (uint16);
uint8    ui8NoteBook_IsPageChanged (uint16);
uint8    ui8NoteBook_IsEmpty       ();
