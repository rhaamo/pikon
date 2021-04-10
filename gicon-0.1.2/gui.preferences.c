/*

  gui.preferences.c

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

#include "file.h"
#include "tty.h"
#include "common.h"

extern uint8 gpui8XmlPath[256];
uint8  gpui8Device[256];
uint16 gui16Baud;

GtkWidget *gpPreferencesDialog                 = NULL;
GtkWidget *gpPreferencesEntryDeviceSpeed       = NULL;
GtkWidget *gpPreferencesEntryDeviceOther       = NULL;
GtkWidget *gpPreferencesEntryDefaultFileFormat = NULL;
GtkWidget *gpPreferencesRadiobuttonDeviceS0    = NULL;
GtkWidget *gpPreferencesRadiobuttonDeviceS1    = NULL;
GtkWidget *gpPreferencesRadiobuttonDeviceS2    = NULL;
GtkWidget *gpPreferencesRadiobuttonDeviceS3    = NULL;
GtkWidget *gpPreferencesRadiobuttonDeviceOther = NULL;

tFileFormat geDefaultFileFormat;

void vMenuEditPropertiesActivate(void) {
  GladeXML *xmlDialogPreferences;

  if (gpPreferencesDialog != NULL)
    return;

  if(!(xmlDialogPreferences = glade_xml_new(gpui8XmlPath, "dialogPreferences"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xmlDialogPreferences);

  gpPreferencesDialog = glade_xml_get_widget(xmlDialogPreferences, "dialogPreferences");

  gpPreferencesEntryDeviceSpeed       = glade_xml_get_widget(xmlDialogPreferences, "entryPreferencesDeviceSpeed");
  gpPreferencesEntryDeviceOther       = glade_xml_get_widget(xmlDialogPreferences, "entryPreferencesDeviceOther");
  gpPreferencesEntryDefaultFileFormat = glade_xml_get_widget(xmlDialogPreferences, "entryPreferencesDefaultFileFormat");
  gpPreferencesRadiobuttonDeviceS0    = glade_xml_get_widget(xmlDialogPreferences, "radiobuttonPreferencesDeviceS0");
  gpPreferencesRadiobuttonDeviceS1    = glade_xml_get_widget(xmlDialogPreferences, "radiobuttonPreferencesDeviceS1");
  gpPreferencesRadiobuttonDeviceS2    = glade_xml_get_widget(xmlDialogPreferences, "radiobuttonPreferencesDeviceS2");
  gpPreferencesRadiobuttonDeviceS3    = glade_xml_get_widget(xmlDialogPreferences, "radiobuttonPreferencesDeviceS3");
  gpPreferencesRadiobuttonDeviceOther = glade_xml_get_widget(xmlDialogPreferences, "radiobuttonPreferencesDeviceOther");

  switch (gui16Baud) {
  case TTY_BAUD_1200:
    gtk_entry_set_text(GTK_ENTRY(gpPreferencesEntryDeviceSpeed), "1200 bps");
    break;
  case TTY_BAUD_9600:
  default:
    gtk_entry_set_text(GTK_ENTRY(gpPreferencesEntryDeviceSpeed), "9600 bps");
  }

  GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS0)))->active = FALSE;
  GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS1)))->active = FALSE;
  GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS2)))->active = FALSE;
  GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS3)))->active = FALSE;
  GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceOther)))->active = FALSE;

  if (strcmp(gpui8Device, "/dev/ttyS0") == 0)
    GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS0)))->active = TRUE;
  else if (strcmp(gpui8Device, "/dev/ttyS1") == 0)
    GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS1)))->active = TRUE;
  else if (strcmp(gpui8Device, "/dev/ttyS2") == 0)
    GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS2)))->active = TRUE;
  else if (strcmp(gpui8Device, "/dev/ttyS3") == 0)
    GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS3)))->active = TRUE;
  else {
    GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceOther)))->active = TRUE;
    gtk_entry_set_text(GTK_ENTRY(gpPreferencesEntryDeviceOther), gpui8Device);
    gtk_widget_set_sensitive(GTK_WIDGET(gpPreferencesEntryDeviceOther), TRUE);
  }

  switch (geDefaultFileFormat) {
  case FILE_FORMAT_TXT:
    gtk_entry_set_text(GTK_ENTRY(gpPreferencesEntryDefaultFileFormat), "TXT");
    break;
  case FILE_FORMAT_NKN:
  case FILE_FORMAT_UNKNOWN:
  default:
    gtk_entry_set_text(GTK_ENTRY(gpPreferencesEntryDefaultFileFormat), "NKN");
  }

  gtk_object_unref(GTK_OBJECT(xmlDialogPreferences));
}

void vPreferencesGeneralButtonHandler(void) {
  if (GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS0)))->active)
    strcpy(gpui8Device, "/dev/ttyS0");
  else if (GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS1)))->active)
    strcpy(gpui8Device, "/dev/ttyS1");
  else if (GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS2)))->active)
    strcpy(gpui8Device, "/dev/ttyS2");
  else if (GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceS3)))->active)
    strcpy(gpui8Device, "/dev/ttyS3");
  else if (GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceOther)))->active) {
    strcpy(gpui8Device, gtk_entry_get_text(GTK_ENTRY(gpPreferencesEntryDeviceOther)));
  }

  if (strcmp(gtk_entry_get_text(GTK_ENTRY(gpPreferencesEntryDeviceSpeed)), "9600 bps") == 0)
    gui16Baud = TTY_BAUD_9600;
  else if (strcmp(gtk_entry_get_text(GTK_ENTRY(gpPreferencesEntryDeviceSpeed)), "1200 bps") == 0)
    gui16Baud = TTY_BAUD_1200;
  else
    gui16Baud = TTY_BAUD_1200;

  if (strcmp(gtk_entry_get_text(GTK_ENTRY(gpPreferencesEntryDefaultFileFormat)), "TXT") == 0)
    geDefaultFileFormat = FILE_FORMAT_TXT;
  else if (strcmp(gtk_entry_get_text(GTK_ENTRY(gpPreferencesEntryDefaultFileFormat)), "NKN") == 0)
    geDefaultFileFormat = FILE_FORMAT_NKN;
  else
    geDefaultFileFormat = FILE_FORMAT_UNKNOWN;
}


void vPreferencesRadioButtonDevOther_Toggled(void) {
  gtk_widget_set_sensitive(GTK_WIDGET(gpPreferencesEntryDeviceOther), GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(GTK_RADIO_BUTTON(gpPreferencesRadiobuttonDeviceOther)))->active);
}

void vPreferencesButtonSave_Released(void) {
  vPreferencesGeneralButtonHandler();
  gtk_widget_destroy(gpPreferencesDialog);
  gpPreferencesDialog = NULL;
}

void vPreferencesButtonApply_Released(void) {
  vPreferencesGeneralButtonHandler();
}

void vPreferencesButtonCancel_Released(void) {
  gtk_widget_destroy(gpPreferencesDialog);
  gpPreferencesDialog = NULL;
}

void vPreferencesLoadDefaults(void) {
  strcpy(gpui8Device, "/dev/ttyS1");
  gui16Baud = TTY_BAUD_9600;
  geDefaultFileFormat = FILE_FORMAT_NKN;
}

void vPreferencesSet(uint8 *pui8Device, uint16 ui16Baud, uint8 ui8DefaultFileFormat) {
  strcpy(gpui8Device, pui8Device);
  gui16Baud = ui16Baud;
  geDefaultFileFormat = ui8DefaultFileFormat;
}

void vPreferencesGet(uint8 **ppui8Device, uint16 *pui16Baud, uint8 *ui8DefaultFileFormat) {
  *ppui8Device = gpui8Device;
  *pui16Baud   = gui16Baud;
  *ui8DefaultFileFormat = geDefaultFileFormat;
}
