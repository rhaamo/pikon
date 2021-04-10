/*

  gui.main.c

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
#include <string.h>
#include <gnome.h>
#include <glade/glade.h>

#include "common.h"
#include "file.h"
#include "print.h"
#include "nkn.h"
#include "tty.h"
#include "util.h"

#include "gui.preferences.h"
#include "gui.print.h"
#include "gui.notebook.h"

#include "pixmaps/gicon.logo.xpm"

#define VERSION "0.1.1"

GtkWidget *gpMainWindow         = NULL;
GtkWidget *gpMainApplicationBar = NULL;
GtkWidget *gpMainWindowViewPort = NULL;
GtkWidget *gpMainWindowNoteBook = NULL;
GtkWidget *gpMainWindowLogo     = NULL;
GdkPixmap *gpMainWindowPixmap   = NULL;
GdkBitmap *gpMainWindowBitmap   = NULL;

GtkWidget *gpMainButtonDownload = NULL;

GtkWidget *gpMenuItemSave       = NULL;
GtkWidget *gpMenuItemSaveAs     = NULL;
GtkWidget *gpMenuItemSaveAll    = NULL;
GtkWidget *gpMenuItemClose      = NULL;
GtkWidget *gpMenuItemCloseAll   = NULL;
GtkWidget *gpMenuItemPrint      = NULL;
GtkWidget *gpMenuItemCopy       = NULL;
GtkWidget *gpMenuItemSelectAll  = NULL;

GtkWidget *gpDialogFileOpen     = NULL;
GtkWidget *gpDialogFileSaveAs   = NULL;

uint16 gui16FileDescriptor;
uint8  gpui8XmlPath[256];
uint8  gpui8FilePath[256] = "";

uint8 gui8Synchronize = 0;
uint8 gui8PageRemoved = 0;
uint8 gui8SelectedRow = 0;

void vMainMenuEnable(uint8);

void vMainCreateNoteBook() {
  if (gpMainWindowNoteBook == NULL) {
    gpMainWindowNoteBook = gtk_notebook_new();
    gtk_widget_show(gpMainWindowNoteBook);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(gpMainWindowNoteBook), TRUE);
    gtk_container_remove(GTK_CONTAINER(gpMainWindowViewPort), gpMainWindowLogo);
    gtk_container_add(GTK_CONTAINER(gpMainWindowViewPort), gpMainWindowNoteBook);
    vMainMenuEnable(1);
  }
}

void vMainDestroyNoteBook() {
  if (gpMainWindowNoteBook != NULL) {
    gpMainWindowLogo = gtk_pixmap_new(gpMainWindowPixmap, gpMainWindowBitmap);
    gtk_container_remove(GTK_CONTAINER(gpMainWindowViewPort), gpMainWindowNoteBook);
    gtk_container_add(GTK_CONTAINER(gpMainWindowViewPort), gpMainWindowLogo);
    gtk_widget_show(GTK_WIDGET(gpMainWindowLogo));
    gpMainWindowNoteBook = NULL;
    vMainMenuEnable(0);
  }
}

void vInitMainWidgets(void) {
  GtkWidget *pVboxMainTmp;
  GladeXML  *xml;

  if(!(xml = glade_xml_new(gpui8XmlPath, "windowMain"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xml);

  gpMainWindow = glade_xml_get_widget(xml, "windowMain");

  gpMainApplicationBar   = glade_xml_get_widget(xml, "appbarMain");
  pVboxMainTmp        = glade_xml_get_widget(xml, "vboxMain");
  gpMainWindowViewPort = glade_xml_get_widget(xml, "viewPortMain");
  gpMenuItemSave       = glade_xml_get_widget(xml, "menuItemSave");
  gpMenuItemSaveAs     = glade_xml_get_widget(xml, "menuItemSaveAs");
  gpMenuItemSaveAll    = glade_xml_get_widget(xml, "menuItemSaveAll");
  gpMenuItemClose      = glade_xml_get_widget(xml, "menuItemClose");
  gpMenuItemCloseAll   = glade_xml_get_widget(xml, "menuItemCloseAll");
  gpMenuItemPrint      = glade_xml_get_widget(xml, "menuItemPrint");
  gpMenuItemCopy       = glade_xml_get_widget(xml, "menuItemCopy");
  gpMenuItemSelectAll  = glade_xml_get_widget(xml, "menuItemSelectAll");
  gpMainButtonDownload = glade_xml_get_widget(xml, "buttonDownloadMain");

  gtk_widget_show(gpMainWindowViewPort);

  gpMainWindowPixmap = gdk_pixmap_create_from_xpm_d(gpMainWindow->window,
                                                  &gpMainWindowBitmap,
                                                  &gtk_widget_get_style(gpMainWindow)->bg[GTK_STATE_NORMAL],
                                                  (gchar **)gicon_logo_xpm);
  gpMainWindowLogo = gtk_pixmap_new(gpMainWindowPixmap, gpMainWindowBitmap);
  gtk_widget_show(gpMainWindowLogo);
  gtk_container_remove(GTK_CONTAINER(gpMainWindowViewPort), pVboxMainTmp);
  gtk_container_add(GTK_CONTAINER(gpMainWindowViewPort), gpMainWindowLogo);
  gtk_object_unref(GTK_OBJECT(xml));

  vMainMenuEnable(0);
  if (ui16ReadConfigFile())
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to read config file.");

}

void vCListMain_SelectRow(GtkWidget *unusedCList,
                          gint giRow,
                          gint unusedColumn,
                          GdkEventButton *unusedEvent,
                          gpointer unusedData) {

  gchar *pgcCellString;
  uint16 ui16NoteBookPageIndex;
  tMyList *pNoteBookPage;

  if (gpMainWindowNoteBook == NULL)
    return;

  gui8SelectedRow = giRow;
  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  pNoteBookPage = pNoteBook_GetPageByIndex(ui16NoteBookPageIndex);
  gtk_clist_get_text(GTK_CLIST(pNoteBookPage->pCListMain), giRow, 9, &pgcCellString);
  gtk_entry_set_text(GTK_ENTRY(pNoteBookPage->pEntryMainCaption), pgcCellString);
  gtk_widget_set_sensitive(GTK_WIDGET(pNoteBookPage->pEntryMainCaption), 1);
}

void vEntryMainCaption_Changed() {
  gchar *pgcCellString;
  uint16 ui16NoteBookPageIndex;
  tMyList *pNoteBookPage;

  if (gpMainWindowNoteBook == NULL)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  pNoteBookPage = pNoteBook_GetPageByIndex(ui16NoteBookPageIndex);
  pNoteBookPage->ui8Changed = 1;
  pgcCellString = gtk_entry_get_text(GTK_ENTRY(pNoteBookPage->pEntryMainCaption));
  gtk_clist_set_text(GTK_CLIST(pNoteBookPage->pCListMain), gui8SelectedRow, 9, pgcCellString);
}

void vEntryMainRollTitle_Changed() {
  uint16 ui16NoteBookPageIndex;
  tMyList *pNoteBookPage;

  if (gpMainWindowNoteBook == NULL)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  pNoteBookPage = pNoteBook_GetPageByIndex(ui16NoteBookPageIndex);
  pNoteBookPage->ui8Changed = 1;
}

void vMenuFileOpenActivate() {
  GladeXML *xml;

  if(!(xml = glade_xml_new(gpui8XmlPath, "dialogFileOpen"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xml);
  gpDialogFileOpen = glade_xml_get_widget(xml, "dialogFileOpen");
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(gpDialogFileOpen), gpui8FilePath);
  gtk_object_unref(GTK_OBJECT(xml));
}

void vMenuFileSaveActivate() {
  GladeXML *xml;
  tRoll    rRoll;
  uint16   ui16NoteBookPageIndex;
  uint8    *pui8FileName = NULL;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  vNoteBook_GetRoll(ui16NoteBookPageIndex, &rRoll);
  pui8FileName = gtk_entry_get_text(GTK_ENTRY(pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->pEntryMainFileName));

  if (pui8FileName == NULL) {
    if(!(xml = glade_xml_new(gpui8XmlPath, "dialogFileSaveAs"))) {
      g_warning("We could not load the interface!");
      return;
    }
    glade_xml_signal_autoconnect(xml);
    gpDialogFileSaveAs = glade_xml_get_widget(xml, "dialogFileSaveAs");
    gtk_object_unref(GTK_OBJECT(xml));
  }
  else {
    if ((ui16SaveFile(pui8FileName, &rRoll) == FAIL)) {
      gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors while saving");
      return;
    }
    pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->ui8Changed = 0;
  }
}


void vMenuFileSaveAsActivate() {
  GladeXML *xml;
  uint16   ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));

  if(!(xml = glade_xml_new(gpui8XmlPath, "dialogFileSaveAs"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xml);
  gpDialogFileSaveAs = glade_xml_get_widget(xml, "dialogFileSaveAs");
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(gpDialogFileSaveAs),
                                  gtk_entry_get_text(GTK_ENTRY(pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->pEntryMainFileName)));
  gtk_object_unref(GTK_OBJECT(xml));
}

void vMenuFileSaveAllActivate() {
  GladeXML *xml;
  uint16   ui16NoteBookPageIndex = 0;
  uint8    *pui8FileName;
  tRoll    rRoll;

  if (gpMainWindowNoteBook == NULL)
    return;

  gtk_notebook_set_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex);
  while (gtk_notebook_get_nth_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex) != NULL) {
    vNoteBook_GetRoll(ui16NoteBookPageIndex, &rRoll);
    pui8FileName = gtk_entry_get_text(GTK_ENTRY(pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->pEntryMainFileName));

    if (pui8FileName == NULL) {
      if(!(xml = glade_xml_new(gpui8XmlPath, "dialogFileSaveAs"))) {
        g_warning("We could not load the interface!");
        return;
      }
      glade_xml_signal_autoconnect(xml);
      gpDialogFileSaveAs = glade_xml_get_widget(xml, "dialogFileSaveAs");
      gtk_object_unref(GTK_OBJECT(xml));
    }
    else {
      pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->ui8Changed = 0;
      if ((ui16SaveFile(pui8FileName, &rRoll) == FAIL)) {
        gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors while saving");
        pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->ui8Changed = 1;
      }
    }
    ui16NoteBookPageIndex++;
  }
}

void vMenuFileCloseWithoutQuestions(gint giReply, gpointer vNotUsedDataPointer) {
  uint16 ui16NoteBookPageIndex;
  if (giReply != 0)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  vNoteBook_RemovePage(ui16NoteBookPageIndex);
  gtk_notebook_remove_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex);
  gui8PageRemoved = 1;
  if (ui8NoteBook_IsEmpty())
    vMainDestroyNoteBook();
  gui8Synchronize = 0;
}

void vMenuFileCloseActivate() {
  uint16 ui16NoteBookPageIndex;

  if (gpMainWindowNoteBook == NULL)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  if (ui8NoteBook_IsPageChanged(ui16NoteBookPageIndex))
    gnome_app_question(GNOME_APP(gpMainWindow),
                       "This roll is unsaved.\nStill want to close it?",
                       vMenuFileCloseWithoutQuestions,
                       NULL);
  else
    vMenuFileCloseWithoutQuestions(0, NULL);
}

void vMenuFileCloseAllActivate() {
  uint16 ui16NoteBookPageIndex = 0;

  if (gpMainWindowNoteBook == NULL)
    return;

  gtk_notebook_set_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex);
  while (gpMainWindowNoteBook != NULL) {
    if (gtk_notebook_get_nth_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex) == NULL)
      break;

    if (ui8NoteBook_IsPageChanged(ui16NoteBookPageIndex)) {
      gui8Synchronize = 1;
      gui8PageRemoved = 0;
      gnome_app_question(GNOME_APP(gpMainWindow),
                         "This roll is unsaved.\nStill want to close it?",
                         vMenuFileCloseWithoutQuestions,
                         NULL);
      /* wait here until the callback function returns */
      while (gui8Synchronize)
        if (gtk_events_pending())
          gtk_main_iteration();
      /* not removed so step forward to the next page */
      if (!gui8PageRemoved) {
        ui16NoteBookPageIndex++;
        gtk_notebook_set_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex);
      }
    }
    else
      vMenuFileCloseWithoutQuestions(0, NULL);
  }
}

void vMenuFileExitWithoutQuestions(gint giReply, gpointer vNotUsedDataPointer) {
  if (giReply != 0)
    return;

  if (ui16WriteConfigFile())
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to write config file.");
  gtk_main_quit();
}

void vMenuFileExitActivate() {
  uint16 ui16NoteBookPageIndex = 0;
  uint8  ui8SomeDataChanged = 0;

  if (gpMainWindowNoteBook == NULL) {
    vMenuFileExitWithoutQuestions(0, NULL);
    return;
  }

  while (gtk_notebook_get_nth_page(GTK_NOTEBOOK(gpMainWindowNoteBook), ui16NoteBookPageIndex) != NULL) {
    if (ui8NoteBook_IsPageChanged(ui16NoteBookPageIndex))
      ui8SomeDataChanged = 1;
    ui16NoteBookPageIndex++;
  }

   if (ui8SomeDataChanged)
     gnome_app_question(GNOME_APP(gpMainWindow),
                        "Some data is unsaved.\nQuit gIcon?",
                        vMenuFileExitWithoutQuestions,
                        NULL);
   else
     vMenuFileExitWithoutQuestions(0, NULL);
}

void vMenuHelpManualActivate() {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to start the help browser.");
}

void vMenuHelpLicenseActivate(GtkMenuItem *menuitem, gpointer aboutWindow) {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/license.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to start the help browser.");
}

void vMainButtonHelp_Released() {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/main/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to start the help browser.");
}

void vMenuHelpAboutActivate(GtkMenuItem *menuitem, gpointer aboutWindow) {
  GladeXML *xml;

  if(!(xml = glade_xml_new(gpui8XmlPath, "windowAbout"))) {
    g_warning("We could not load the interface!");
    return;
  }

  gtk_object_unref(GTK_OBJECT(xml));
}

void vMainMenuEnable(uint8 ui8Enable) {
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemSave ),     ui8Enable);
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemSaveAs),    ui8Enable);
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemSaveAll),   ui8Enable);
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemClose),     ui8Enable);
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemCloseAll),  ui8Enable);
  gtk_widget_set_sensitive(GTK_WIDGET(gpMenuItemPrint),     ui8Enable);
}

void vButtonFileSelectionOkButtonClicked() {
  gnome_app_message(GNOME_APP(gpMainWindow), "OK Button Clicked");
}

void vButtonFileSelectionCancelButtonClicked () {
  gnome_app_message(GNOME_APP(gpMainWindow), "Cancel Button Clicked");
}


void vWindowMainDestroy (GtkObject *object, gpointer data) {
  gtk_widget_destroy(GTK_WIDGET(gpMainApplicationBar));
  gtk_main_quit();
}

void vFileOpenButtonOk_Released(void) {
  GladeXML  *xml = NULL;
  GtkWidget *pVboxMain  = NULL;
  GtkWidget *pPageLabel = NULL;
  tRoll     rRoll;

  strcpy(gpui8FilePath, gtk_file_selection_get_filename(GTK_FILE_SELECTION(gpDialogFileOpen)));

  if (ui16OpenFile(gtk_file_selection_get_filename(GTK_FILE_SELECTION(gpDialogFileOpen)), &rRoll)) {
    gtk_widget_destroy(GTK_WIDGET(gpDialogFileOpen));
    gnome_app_error(GNOME_APP(gpMainWindow), "Unknown file format");
    return;
  }

  vMainCreateNoteBook();

  /* get the notebook template from the xml model */
  if(!(xml = glade_xml_new(gpui8XmlPath, "vboxMain"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xml);

  pVboxMain  = glade_xml_get_widget(xml, "vboxMain");
  pPageLabel = gtk_label_new(rRoll.pui8RollTitle);
  /* create a new page and append it to the notebook */
  gtk_notebook_insert_page(GTK_NOTEBOOK(gpMainWindowNoteBook), pVboxMain, pPageLabel, 0);
  gtk_notebook_set_page(GTK_NOTEBOOK(gpMainWindowNoteBook), 0);

  vNoteBook_AddPage(xml, &rRoll, 0);
  gtk_widget_destroy(GTK_WIDGET(gpDialogFileOpen));
}

void vFileOpenButtonCancel_Released(void) {
  gtk_widget_destroy(GTK_WIDGET(gpDialogFileOpen));
}

void vSaveFileWithoutQuestions(gint giReply, gpointer vNotUsedDataPointer) {
  uint8  *pui8FileName;
  tRoll  rRoll;
  uint16 ui16NoteBookPageIndex;

  if (giReply != 0)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  vNoteBook_GetRoll(ui16NoteBookPageIndex, &rRoll);
  pui8FileName = gtk_entry_get_text(GTK_ENTRY(pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->pEntryMainFileName));

  if ((ui16SaveFile(pui8FileName, &rRoll) == FAIL)) {
    gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors while saving");
    return;
  }
  pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->ui8Changed = 0;
}

void vFileSaveAsButtonOk_Released(void) {
  struct stat buf;
  uint16 ui16NoteBookPageIndex;
  uint8 *pui8FileName;

  if (gpMainWindowNoteBook == NULL)
    return;

  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  pui8FileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(gpDialogFileSaveAs));
  gtk_entry_set_text(GTK_ENTRY(pNoteBook_GetPageByIndex(ui16NoteBookPageIndex)->pEntryMainFileName), pui8FileName);

  if (stat(pui8FileName, &buf) == 0)
    gnome_app_question(GNOME_APP(gpMainWindow),
                       "This file already exist.\nOverwrite it?",
                       vSaveFileWithoutQuestions,
                       NULL);
  else
    vSaveFileWithoutQuestions(0, NULL);
  gtk_widget_destroy(GTK_WIDGET(gpDialogFileSaveAs));
}

void vFileSaveAsButtonCancel_Released(void) {
  gtk_widget_destroy(GTK_WIDGET(gpDialogFileSaveAs));
}

void vMainButtonSetSensitive(uint8 ui8Sensitive) {
  gtk_widget_set_sensitive(gpMainButtonDownload, ui8Sensitive);
}

int main(int argc, char *argv[]) {
  gnome_init("gIcon", VERSION, argc, argv);
  sprintf(gpui8XmlPath, "%s/gui.xml",  getenv("GICON_HOME"));
  glade_gnome_init();
  vPrintLoadDefaults();
  vInitMainWidgets();
  gtk_main();
  return 0;
}
