/*

  gui.notebook.c

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
#include "gui.notebook.h"

extern GtkWidget *pViewPortMain;
extern GtkWidget *pPixmapWindowMain;

uint16 ui16ColumMinWidth[10] = {35,50,50,50,55,55,40,80,80,180};
tMyList *pPageList = NULL;

void vNoteBook_AddPage(GladeXML  *xml, tRoll *prRoll, uint8 ui8New) {
  uint16  ui16FrameIndex;
  uint16  ui16ColumnIndex;
  tMyList *pNewPage = NULL;

  pNewPage = malloc(sizeof(tMyList));
  pNewPage->pNext = pPageList;
  pPageList = pNewPage;

  /* save widgets on the new notebook page for further reference */
  pNewPage->pCListMain           = glade_xml_get_widget(xml, "clistMain");
  pNewPage->pEntryMainFileName   = glade_xml_get_widget(xml, "entryMainFileName");
  pNewPage->pEntryMainRollTitle  = glade_xml_get_widget(xml, "entryMainRollTitle");
  pNewPage->pEntryMainCameraType = glade_xml_get_widget(xml, "entryMainCameraType");
  pNewPage->pEntryMainFilmSpeed  = glade_xml_get_widget(xml, "entryMainFilmSpeed");
  pNewPage->pEntryMainTimeStamp  = glade_xml_get_widget(xml, "entryMainTimeStamp");
  pNewPage->pEntryMainCaption    = glade_xml_get_widget(xml, "entryMainCaption");

  if (prRoll == NULL)
    return;

  /* load the widgets */
  gtk_entry_set_text(GTK_ENTRY(pNewPage->pEntryMainFileName),  prRoll->pui8FileName);
  gtk_entry_set_text(GTK_ENTRY(pNewPage->pEntryMainRollTitle), prRoll->pui8RollTitle);
  gtk_entry_set_text(GTK_ENTRY(pNewPage->pEntryMainCameraType),prRoll->pui8CameraType);
  gtk_entry_set_text(GTK_ENTRY(pNewPage->pEntryMainFilmSpeed), prRoll->pui8FilmSpeed);
  gtk_entry_set_text(GTK_ENTRY(pNewPage->pEntryMainTimeStamp), prRoll->pui8TimeStamp);
  pNewPage->ui8Changed = 0 || ui8New;
  pNewPage->ui8FrameCounter = prRoll->ui8FrameCounter;
  pNewPage->ui8StorageLevel = prRoll->ui8StorageLevel;
  pNewPage->ui32TimeStamp   = prRoll->ui32TimeStamp;

  gtk_clist_freeze (GTK_CLIST(pNewPage->pCListMain));
  /* load the list with the shooting data */
  for (ui16FrameIndex = 0; ui16FrameIndex < pNewPage->ui8FrameCounter; ui16FrameIndex++)
    gtk_clist_append(GTK_CLIST(pNewPage->pCListMain),
                     prRoll->table[ui16FrameIndex].list);
  /* set minimum column widths */
  for (ui16ColumnIndex = 0; ui16ColumnIndex < 10; ui16ColumnIndex++)
    gtk_clist_set_column_min_width(GTK_CLIST(pNewPage->pCListMain),
                                   ui16ColumnIndex,
                                   ui16ColumMinWidth[ui16ColumnIndex]);
  /* enable column autosize */
  gtk_clist_columns_autosize (GTK_CLIST(pNewPage->pCListMain));
  gtk_clist_thaw   (GTK_CLIST(pNewPage->pCListMain));
  gtk_object_unref(GTK_OBJECT(xml));
}

void vNoteBook_RemovePage(uint16 ui16Index) {
  tMyList *pCurrentPage  = pPageList;
  tMyList *pPreviousPage = pPageList;
  uint16 ui16CurrentIndex = 0;
  while(pCurrentPage != NULL) {
    if (ui16CurrentIndex == ui16Index) {
      if (pCurrentPage == pPageList) {
        pPageList = pCurrentPage->pNext;
      }
      else {
        pPreviousPage->pNext = pCurrentPage->pNext;
      }
      gtk_widget_destroy(pCurrentPage->pEntryMainCaption);
      gtk_widget_destroy(pCurrentPage->pCListMain);
      gtk_widget_destroy(pCurrentPage->pEntryMainFileName);
      gtk_widget_destroy(pCurrentPage->pEntryMainRollTitle);
      gtk_widget_destroy(pCurrentPage->pEntryMainCameraType);
      gtk_widget_destroy(pCurrentPage->pEntryMainFilmSpeed);
      gtk_widget_destroy(pCurrentPage->pEntryMainTimeStamp);

      free(pCurrentPage);
      break;
    }
    pPreviousPage = pCurrentPage;
    pCurrentPage = pCurrentPage->pNext;
    ui16CurrentIndex++;
  }
}

tMyList *pNoteBook_GetPageByIndex(uint16 ui16Index) {
  tMyList *pCurrentPage = pPageList;
  uint16 ui16CurrentIndex = 0;
  while(pCurrentPage != NULL) {
    if (ui16CurrentIndex == ui16Index)
      return pCurrentPage;
    pCurrentPage = pCurrentPage->pNext;
    ui16CurrentIndex++;
  }
  return NULL;
}

void vNoteBook_GetRoll(uint16 ui16Index, tRoll *prRoll) {
  tMyList *pCurrentPage = pNoteBook_GetPageByIndex(ui16Index);
  uint16  ui16FrameIndex;
  uint16  ui16ColumnIndex;

  strcpy(prRoll->pui8FileName,   gtk_entry_get_text(GTK_ENTRY(pCurrentPage->pEntryMainFileName)));
  strcpy(prRoll->pui8RollTitle,  gtk_entry_get_text(GTK_ENTRY(pCurrentPage->pEntryMainRollTitle)));
  strcpy(prRoll->pui8CameraType, gtk_entry_get_text(GTK_ENTRY(pCurrentPage->pEntryMainCameraType)));
  strcpy(prRoll->pui8FilmSpeed,  gtk_entry_get_text(GTK_ENTRY(pCurrentPage->pEntryMainFilmSpeed)));
  strcpy(prRoll->pui8TimeStamp,  gtk_entry_get_text(GTK_ENTRY(pCurrentPage->pEntryMainTimeStamp)));

  prRoll->ui8FrameCounter = pCurrentPage->ui8FrameCounter;
  prRoll->ui8StorageLevel = pCurrentPage->ui8StorageLevel;
  prRoll->ui32TimeStamp   = pCurrentPage->ui32TimeStamp;

  gtk_clist_freeze(GTK_CLIST(pCurrentPage->pCListMain));
  /* load the list with the shooting data */
  for (ui16FrameIndex = 0; ui16FrameIndex < pCurrentPage->ui8FrameCounter; ui16FrameIndex++)
    for (ui16ColumnIndex = 0; ui16ColumnIndex < 10; ui16ColumnIndex++) {
      gtk_clist_get_text(GTK_CLIST(pCurrentPage->pCListMain),
                         ui16FrameIndex,
                         ui16ColumnIndex,
                         &prRoll->table[ui16FrameIndex].list[ui16ColumnIndex]);
    }
  gtk_clist_thaw(GTK_CLIST(pCurrentPage->pCListMain));
}

uint8 ui8NoteBook_IsPageChanged(uint16 ui16Index) {
  tMyList *pCurrentPage = pPageList;
  uint16 ui16CurrentIndex = 0;
  while(pCurrentPage != NULL) {
    if (ui16CurrentIndex == ui16Index)
      return pCurrentPage->ui8Changed;
    pCurrentPage = pCurrentPage->pNext;
    ui16CurrentIndex++;
  }
  return 0;
}

uint8 ui8NoteBook_IsEmpty() {
  return (pPageList == NULL);
}
