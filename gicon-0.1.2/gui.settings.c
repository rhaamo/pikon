/*

  gui.settings.c

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

#include "nkn.h"
#include "common.h"
#include "util.h"

extern uint8  gpui8XmlPath[256];
extern uint8  gpui8Device[256];
extern uint16 gui16Baud;

extern tNKN_DataArray rpCameraCommandDial[];
extern tNKN_DataArray rpCameraLongExposuresUses[];
extern tNKN_DataArray rpCameraMemoHolderDownload[];
extern tNKN_DataArray rpCameraMemoHolderSettings[];
extern tNKN_DataArray rpCameraDualRelease[];
extern tNKN_DataArray rpCameraFilmISOPriority[];

void vSettingsButtonDownload_Released(void);
void vSettingsButtonUpload_Released(void);
void vSettingsButtonDefault_Released(void);
void vSettingsButtonHelp_Released(void);
void vSettingsButtonSetSensitive(uint8);
void vSettingsWindowDestroy(void);

static struct {
  GtkWidget *pWidget;
  uint8     *pui8WidgetString;
  uint8     ui8Enable;
  uint16    ui16WidgetValue;
  uint16    ui16DefaultValue;
  uint16    ui16RegisterValue;
  uint8     ui8RegisterMode;
  uint32    ui32RegisterAddress;
  uint8     ui8RegisterWidth;
  uint16    ui16RegisterMask;
  uint8     ui8RegisterShift;
  tNKN_DataArray *prData;
} grWidgetTable[] = {
  {NULL, "entrySettingsCommandDial",                         1, 0, 0, 0, CAMERA_COMMAND_DIAL,                       rpCameraCommandDial},
  {NULL, "entrySettingsLongExposure",                        1, 0, 0, 0, CAMERA_LONG_EXPOSURES_USES,                rpCameraLongExposuresUses},
  {NULL, "entrySettingsISOPriority",                         1, 0, 0, 0, CAMERA_FILM_ISO_PRIORITY,                  rpCameraFilmISOPriority},
  {NULL, "entrySettingsDualRelease",                         1, 0, 0, 0, CAMERA_DUAL_RELEASE,                       rpCameraDualRelease},
  {NULL, "checkButtonSettingsViewfinderFrameCounterPsModes", 1, 0, 0, 0, CAMERA_FRAME_COUNTER_IN_PS_MODE,           NULL},

  {NULL, "checkButtonSettingsViewfinderMatrixCenterDelta",   1, 0, 0, 0, CAMERA_VIEWFINDER_MATRIX_CENTER_DELTA,     NULL},
  {NULL, "checkButtonSettingsFocusReleasePriorityInCSM",     1, 0, 0, 0, CAMERA_FOCUS_RELEASE_PRIORITY_IN_CAM_MODE, NULL},
  {NULL, "checkButtonSettingsFocusBetweenFramesCSM",         1, 0, 0, 0, CAMERA_FOCUS_LOCK_IN_CAF_MODE,             NULL},
  {NULL, "checkButtonSettingsReleaseFocusPriorityInSM",      1, 0, 0, 0, CAMERA_FOCUS_RELEASE_PRIORITY_IN_SAM_MODE, NULL},
  {NULL, "checkButtonSettingsAEAFLock",                      1, 0, 0, 0, CAMERA_FOCUS_TRAP_SIMULTANEOUS_AF_AE_LOCK, NULL},

  {NULL, "checkButtonSettingsTrapFocusing",                  1, 0, 0, 0, CAMERA_FOCUS_TRAP,                         NULL},
  {NULL, "checkButtonSettingsBeeperOnFilmError",             1, 0, 0, 0, CAMERA_BEEP_ON_FILM_ERROR,                 NULL},
  {NULL, "checkButtonSettingsBeeperOnExposureError",         1, 0, 0, 0, CAMERA_BEEP_ON_EXPOSURE_ERROR,             NULL},
  {NULL, "checkButtonSettingsBeeperWhenInFocus",             1, 0, 0, 0, CAMERA_BEEP_WHEN_IN_FOCUS,                 NULL},
  {NULL, "checkButtonSettingsBeeperDuringSelfTimer",         1, 0, 0, 0, CAMERA_BEEP_WHEN_SELF_TIMER_ON,            NULL},

  {NULL, "checkButtonSettingsPrintRollNumerOnFilm",          1, 0, 0, 0, CAMERA_PRINT_ROLL_NUMBER_ON_FRAME_0,       NULL},
  {NULL, "checkButtonSettingsEasyCompensationInAM",          1, 0, 0, 0, CAMERA_EASY_COMPENSATION_IN_APERTURE_MODE, NULL},
  {NULL, "entrySettingsMemoHolderSettings",                  0, 0, 0, 0, CAMERA_MEMO_HOLDER_SETTINGS,               rpCameraMemoHolderSettings},
  {NULL, "entrySettingsMemoHolderDataDownload",              0, 0, 0, 0, CAMERA_MEMO_HOLDER_DOWNLOAD,               rpCameraMemoHolderDownload},
  {NULL, "checkButtonSettingsMemoHolderEnable",              0, 0, 0, 0, CAMERA_MEMO_HOLDER_ENABLE,                 NULL},
};

#define WIDGET_TABLE_SIZE          20
#define WIDGET_INDEX_MEMO_ENABLE   19
#define WIDGET_INDEX_MEMO_DOWNLOAD 18
#define WIDGET_INDEX_MEMO_SETTING  17

GtkWidget *gpSettingsWindow = NULL;

GtkWidget *gpSettingsComboMemoHolderData         = NULL;
GtkWidget *gpSettingsComboMemoHolderDataDownload = NULL;
GtkWidget *gpSettingsButtonDownload              = NULL;
GtkWidget *gpSettingsButtonUpload                = NULL;
GtkWidget *gpSettingsButtonDefault               = NULL;
GtkWidget *gpSettingsApplicationBar              = NULL;

uint8  gui8DefaultSettingsLoaded = FALSE;
uint16 gui16FileDescriptor;



void vMenuViewShowSettingsActivate(void) {
  uint16 ui16Index;
  GladeXML *xmlSettingsWindow;

  if (gpSettingsWindow != NULL)
    return;

  if(!(xmlSettingsWindow = glade_xml_new(gpui8XmlPath, "windowSettings"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xmlSettingsWindow);

  gpSettingsWindow = glade_xml_get_widget(xmlSettingsWindow, "windowSettings");

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++)
    grWidgetTable[ui16Index].pWidget = glade_xml_get_widget(xmlSettingsWindow, grWidgetTable[ui16Index].pui8WidgetString);

  gpSettingsComboMemoHolderData = glade_xml_get_widget(xmlSettingsWindow, "comboSettingsMemoHolderData");
  gpSettingsComboMemoHolderDataDownload = glade_xml_get_widget(xmlSettingsWindow, "comboSettingsMemoHolderDataDownload");
  gpSettingsApplicationBar = glade_xml_get_widget(xmlSettingsWindow, "appbarSettings");

  gpSettingsButtonDownload = glade_xml_get_widget(xmlSettingsWindow, "buttonSettingsDownload");
  gpSettingsButtonUpload   = glade_xml_get_widget(xmlSettingsWindow, "buttonSettingsUpload");
  gpSettingsButtonDefault  = glade_xml_get_widget(xmlSettingsWindow, "buttonSettingsDefault");

  vSettingsButtonDownload_Released();
  gtk_object_unref(GTK_OBJECT(xmlSettingsWindow));
}

void vSettingsButtonDownload_Released(void) {
  uint8 *pui8String = NULL;
  uint16 ui16Value;
  uint16 ui16ReturnValue;
  uint16 ui16Index;
  uint16 ui16CurrentFrameNumber;
  uint16 ui16DataStartAddress;
  uint16 ui16DataEndAddress;

  vSettingsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Downloading...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vSettingsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpSettingsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue  = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  /* enable memo holder settings only if there is no film in the camera*/
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_FRAME_NUMBER, &ui16CurrentFrameNumber);
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_CURRENT_MEMO_POINTER, &ui16DataEndAddress);
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_START_POINTER, &ui16DataStartAddress);

  gtk_widget_set_sensitive(grWidgetTable[WIDGET_INDEX_MEMO_ENABLE].pWidget, (ui16CurrentFrameNumber == 0));
  gtk_widget_set_sensitive(gpSettingsComboMemoHolderDataDownload, (ui16CurrentFrameNumber == 0));
  /* allow format changes only if no data is stored in the memo holder */
  if (ui16DataStartAddress == ui16DataEndAddress)
    gtk_widget_set_sensitive(gpSettingsComboMemoHolderData, (ui16CurrentFrameNumber == 0));

  grWidgetTable[WIDGET_INDEX_MEMO_ENABLE].ui8Enable   = (ui16CurrentFrameNumber == 0);
  grWidgetTable[WIDGET_INDEX_MEMO_DOWNLOAD].ui8Enable = (ui16CurrentFrameNumber == 0);
  grWidgetTable[WIDGET_INDEX_MEMO_SETTING].ui8Enable  = (ui16CurrentFrameNumber == 0);

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++) {
    /* get data from register */
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor,
                                      grWidgetTable[ui16Index].ui8RegisterMode,
                                      grWidgetTable[ui16Index].ui32RegisterAddress,
                                      grWidgetTable[ui16Index].ui8RegisterWidth,
                                      grWidgetTable[ui16Index].ui16RegisterMask,
                                      grWidgetTable[ui16Index].ui8RegisterShift,
                                      &ui16Value);

    grWidgetTable[ui16Index].ui16RegisterValue = ui16Value;
    grWidgetTable[ui16Index].ui16WidgetValue = ui16Value;
    if (!gui8DefaultSettingsLoaded)
      grWidgetTable[ui16Index].ui16DefaultValue = ui16Value;

    if (grWidgetTable[ui16Index].prData == NULL) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(grWidgetTable[ui16Index].pWidget)), grWidgetTable[ui16Index].ui16RegisterValue);
    }
    else {
      vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
      if (pui8String == NULL) {
        uint8 ui8Buffer[256];
        sprintf(ui8Buffer, "0x%x:UNKNOWN",  grWidgetTable[ui16Index].ui16RegisterValue);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      }
      else
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
    }

    gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), (gfloat)(1/(gfloat)WIDGET_TABLE_SIZE)*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();
  }
  gui8DefaultSettingsLoaded = TRUE;

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Download complete.");

  vSettingsButtonSetSensitive(TRUE);
}

void vSettingsButtonUpload_Released(void) {
  uint16 ui16Index;
  uint16 ui16ReturnValue;

  vSettingsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Uploading...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vSettingsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpSettingsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue  = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++) {
    if (!grWidgetTable[ui16Index].ui8Enable)
      continue;

    /* get data from widget */
    if (grWidgetTable[ui16Index].prData == NULL) {
      grWidgetTable[ui16Index].ui16WidgetValue = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(grWidgetTable[ui16Index].pWidget)));
    }
    else {
      vNKN_StringToData(grWidgetTable[ui16Index].prData, gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget)), &grWidgetTable[ui16Index].ui16WidgetValue);
    }

    if (grWidgetTable[ui16Index].ui16WidgetValue != grWidgetTable[ui16Index].ui16RegisterValue) {
      grWidgetTable[ui16Index].ui16RegisterValue = grWidgetTable[ui16Index].ui16WidgetValue;
      ui16ReturnValue += ui16NKN_Write16(gui16FileDescriptor,
                                          grWidgetTable[ui16Index].ui8RegisterMode,
                                          grWidgetTable[ui16Index].ui32RegisterAddress,
                                          grWidgetTable[ui16Index].ui8RegisterWidth,
                                          grWidgetTable[ui16Index].ui16RegisterMask,
                                          grWidgetTable[ui16Index].ui8RegisterShift,
                                          grWidgetTable[ui16Index].ui16RegisterValue);

    }

    gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), (gfloat)(1/(gfloat)WIDGET_TABLE_SIZE)*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();
  }

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Upload complete.");

  vSettingsButtonSetSensitive(TRUE);
}

void vSettingsButtonDefault_Released(void) {
  uint8 *pui8String = NULL;
  uint16 ui16Index;
  uint16 ui16ReturnValue;

  if (!gui8DefaultSettingsLoaded) {
    gnome_app_message(GNOME_APP(gpSettingsWindow), "No default values loaded.\nDefault values will be loaded\nat the first download.");
    return;
  }

  vSettingsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Uploading defaults...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vSettingsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpSettingsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue  = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++) {
    if (!grWidgetTable[ui16Index].ui8Enable)
      continue;

    if (grWidgetTable[ui16Index].prData == NULL) {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(grWidgetTable[ui16Index].pWidget)), grWidgetTable[ui16Index].ui16DefaultValue);
    }
    else {
      vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16DefaultValue, &pui8String);
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
    }

    /* update register value */
    if (grWidgetTable[ui16Index].ui16RegisterValue != grWidgetTable[ui16Index].ui16DefaultValue) {
      grWidgetTable[ui16Index].ui16RegisterValue = grWidgetTable[ui16Index].ui16DefaultValue;
      ui16ReturnValue += ui16NKN_Write16(gui16FileDescriptor,
                                          grWidgetTable[ui16Index].ui8RegisterMode,
                                          grWidgetTable[ui16Index].ui32RegisterAddress,
                                          grWidgetTable[ui16Index].ui8RegisterWidth,
                                          grWidgetTable[ui16Index].ui16RegisterMask,
                                          grWidgetTable[ui16Index].ui8RegisterShift,
                                          grWidgetTable[ui16Index].ui16DefaultValue);

    }
    grWidgetTable[ui16Index].ui16WidgetValue = grWidgetTable[ui16Index].ui16DefaultValue;

    gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), (gfloat)(1/(gfloat)WIDGET_TABLE_SIZE)*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();

  }

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpSettingsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Upload complete.");

  vSettingsButtonSetSensitive(TRUE);
}

void vSettingsButtonHelp_Released(void) {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/settings/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpSettingsApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpSettingsWindow), "Failed to start the help browser.");
}

void vSettingsButtonSetSensitive(uint8 ui8Sensitive) {
  gtk_widget_set_sensitive(gpSettingsButtonDownload, ui8Sensitive);
  gtk_widget_set_sensitive(gpSettingsButtonUpload, ui8Sensitive);
  gtk_widget_set_sensitive(gpSettingsButtonDefault, ui8Sensitive);
}

void vSettingsWindowDestroy(void) {
  uint16 ui16Index;
  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++) {
    gtk_widget_destroy(GTK_WIDGET(grWidgetTable[ui16Index].pWidget));
    grWidgetTable[ui16Index].pWidget = NULL;
  }

  gtk_widget_destroy(GTK_WIDGET(gpSettingsWindow));

  gpSettingsWindow = NULL;
  gpSettingsComboMemoHolderData = NULL;
  gpSettingsComboMemoHolderDataDownload = NULL;
  gpSettingsApplicationBar = NULL;
}

void vMainButtonSettings_Released() {
  vMenuViewShowSettingsActivate();
}
