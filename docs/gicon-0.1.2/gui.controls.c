/*

  gui.controls.c

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
#include <string.h>

#include "common.h"
#include "nkn.h"
#include "util.h"

extern uint8  gpui8XmlPath[256];
extern uint8  gpui8Device[256];
extern uint16 gui16Baud;

extern tNKN_DataArray rpCameraIsoFilmSpeed[];
extern tNKN_DataArray rpCameraExposure[];
extern tNKN_DataArray rpCameraExposureEffective[];
extern tNKN_DataArray rpCameraExposureCompensation[];
extern tNKN_DataArray rpCameraProgramMode[];
extern tNKN_DataArray rpCameraProgramModeVariProgram[];
extern tNKN_DataArray rpCameraLightMetering[];
extern tNKN_DataArray rpCameraMotorDrive[];
extern tNKN_DataArray rpCameraFocusArea[];
extern tNKN_DataArray rpCameraBracketing[];
extern tNKN_DataArray rpCameraBracketingStep[];
extern tNKN_DataArray rpCameraFlashMode[];
extern tNKN_DataArray rpFlashInstallStatus[];
extern tNKN_DataArray rpFlashReady[];
extern tNKN_DataArray rpCameraFlashSyncSpeed[];
extern tNKN_DataArray rpCameraFlashCompensation[];
extern tNKN_DataArray rpCameraLensIdentifier[];
extern tNKN_DataArray rpCameraApertureTable[];
extern tNKN_DataArray rpCameraApertureEffective[];
extern tNKN_DataArray rpCameraFocusMode[];
extern tNKN_DataArray rpCameraFocusDistance[];
extern tNKN_DataArray rpFocusStatus[];
extern tNKN_DataArray rpCameraGeneralStatus[];
extern tNKN_DataArray rpCameraMemoHolderDownload[];



void   vControlsButtonDownload_Released(void);
void   vControlsButtonUpload_Released(void);
void   vControlsButtonDefault_Released(void);
void   vControlsButtonFocus_Released(void);
void   vControlsButtonRelease_Released(void);
void   vControlsButtonHelp_Released(void);
void   vControlsWindow_Destroy(void);
void   vControlBracketingMode_Changed(void);
void   vControlsButtonSetSensitive(uint8);
void   vControlsWindowDestroy(void);
uint16 ui16ControlsConvertFocusDistance(uint8);

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
  {NULL, "entryControlProgramMode",                     1, 0, 0, 0, CAMERA_PROGRAM_MODE,                 rpCameraProgramMode},
  {NULL, "entryControlLightMeteringMode",               1, 0, 0, 0, CAMERA_LIGHT_METERING,               rpCameraLightMetering},
  {NULL, "entryControlMotorDrive",                      1, 0, 0, 0, CAMERA_MOTOR_DRIVE,                  rpCameraMotorDrive},
  {NULL, "entryControlAutoFocusArea",                   1, 0, 0, 0, CAMERA_FOCUS_AREA,                   rpCameraFocusArea},
  {NULL, "entryControlISOFilmSpeed",                    1, 0, 0, 0, CAMERA_FILM_ISO,                     rpCameraIsoFilmSpeed},

  {NULL, "entryControlExposure",                        1, 0, 0, 0, CAMERA_EXPOSURE,                     rpCameraExposure},
  {NULL, "entryControlExposureCompensation",            1, 0, 0, 0, CAMERA_EXPOSURE_COMPENSATION,        rpCameraExposureCompensation},
  {NULL, "entryControlFlashMode",                       1, 0, 0, 0, CAMERA_FLASH_MODE,                   rpCameraFlashMode},
  {NULL, "entryControlFlashExposure",                   1, 0, 0, 0, CAMERA_FLASH_SYNC_SPEED,             rpCameraFlashSyncSpeed},
  {NULL, "entryControlFlashExposureCompensation",       1, 0, 0, 0, CAMERA_FLASH_COMPENSATION,           rpCameraFlashCompensation},

  {NULL, "spinbuttonControlMiscellaneousSelfTimer",     1, 0, 0, 0, CAMERA_SELF_TIMER,                   NULL},
  {NULL, "spinbuttonControlMiscellaneousLCDLightTimer", 1, 0, 0, 0, CAMERA_LCD_LIGHT_ON_TIME,            NULL},
  {NULL, "entryControlMiscellaneousLCDLights",          1, 0, 0, 0, CAMERA_LCD_LIGHT,                    rpCameraGeneralStatus},
  {NULL, "entryControlBracketingMode",                  1, 0, 0, 0, CAMERA_BRACKETING,                   rpCameraBracketing},
  {NULL, "entryControlFramesPerSequence",               0, 0, 0, 0, CAMERA_SEQUENCE_FRAME_COUNTER,       NULL},

  {NULL, "entryControlStepsPerFrame",                   0, 0, 0, 0, CAMERA_BRACKETING_STEP,              rpCameraBracketingStep},
  /* these widgets are for display only, do not uplodad */
  {NULL, "entryControlAperture",                        1, 0, 0, 0, CAMERA_APERTURE_EFFECTIVE,           rpCameraApertureEffective},
  {NULL, "entryControlRollNumber",                      1, 0, 0, 0, CAMERA_ROLL_NUMBER,                  NULL},
  {NULL, "entryControlTotalFrames",                     1, 0, 0, 0, CAMERA_TOTAL_FRAMES,                 NULL},
  {NULL, "entryControlCurrentFrame",                    1, 0, 0, 0, CAMERA_FRAME_NUMBER,                 NULL},

  {NULL, "entryControlFlashInstallStatus",              1, 0, 0, 0, CAMERA_FLASH_INSTALL_STATUS,         rpFlashInstallStatus},
  {NULL, "entryControlFlashOperationalStatus",          1, 0, 0, 0, CAMERA_FLASH_READY,                  rpFlashReady},
  {NULL, "entryControlFlashSlowSyncStatus",             1, 0, 0, 0, CAMERA_FLASH_SLOW_SYNC_STATUS,       rpCameraGeneralStatus},
  {NULL, "entryControlLensType",                        1, 0, 0, 0, CAMERA_LENS_IDENTIFIER,              rpCameraLensIdentifier},
  {NULL, "entryControlApertureRange",                   1, 0, 0, 0, CAMERA_APERTURE_RANGE_LOWER,         rpCameraApertureTable},

  {NULL, "entryControlZoomedAperture",                  1, 0, 0, 0, CAMERA_APERTURE_ZOOMED,              rpCameraApertureTable},
  {NULL, "entryControlZoomCurrent",                     1, 0, 0, 0, CAMERA_ZOOM_CURRENT,                 rpCameraFocusDistance},
  {NULL, "entryControlFocalLength",                     1, 0, 0, 0, CAMERA_FOCAL_LENGTH_LOWER,           rpCameraFocusDistance},
  {NULL, "entryControlFocusDistance",                   1, 0, 0, 0, CAMERA_FOCUS_DISTANCE,               NULL},
  {NULL, "entryControlFocusMode",                       1, 0, 0, 0, CAMERA_LENS_FOCUS_MODE,              rpCameraFocusMode},

  {NULL, "entryControlFocusStatus",                     1, 0, 0, 0, CAMERA_FOCUS_STATUS,                 rpFocusStatus},
};

#define WIDGET_TABLE_SIZE                 31
#define WIDGET_TABLE_UPLOAD_SIZE          16

#define WIDGET_INDEX_PROGRAM_MODE          0
#define WIDGET_INDEX_EXPOSURE              5
#define WIDGET_INDEX_FLASH_EXPOSURE        8
#define WIDGET_INDEX_SELF_TIMER           10
#define WIDGET_INDEX_LCD_TIMER            11
#define WIDGET_INDEX_BRACKETING_MODE      13
#define WIDGET_INDEX_FRAMES_PER_SEQUENCE  14
#define WIDGET_INDEX_STEPS_PER_FRAME      15
#define WIDGET_INDEX_CURRENT_FRAME        19
#define WIDGET_INDEX_FLASH_INSTALL_STATUS 20
#define WIDGET_INDEX_APERTURE_RANGE_LOWER 24
#define WIDGET_INDEX_FOCAL_LENGTH_LOWER   27
#define WIDGET_INDEX_FOCUS_DISTANCE       28
#define WIDGET_INDEX_FOCUS_STATUS         30

static struct {
  uint16    ui16WidgetValue;
  uint16    ui16DefaultValue;
  uint16    ui16RegisterValue;
  uint8     ui8RegisterMode;
  uint32    ui32RegisterAddress;
  uint8     ui8RegisterWidth;
  uint16    ui16RegisterMask;
  uint8     ui8RegisterShift;
  tNKN_DataArray *prData;
} grAuxWidgetTable[] = {
  {0, 0, 0, CAMERA_PROGRAM_MODE_VARI_PROGRAM, rpCameraProgramModeVariProgram},
  {0, 0, 0, CAMERA_FILM_ISO_EFFECTIVE,        rpCameraIsoFilmSpeed},
  {0, 0, 0, CAMERA_APERTURE_RANGE_UPPER,      rpCameraApertureEffective},
  {0, 0, 0, CAMERA_FOCAL_LENGTH_UPPER,        rpCameraFocusDistance},
  {0, 0, 0, CAMERA_EXPOSURE_EFFECTIVE,        rpCameraExposureEffective},
};

#define WIDGET_AUX_TABLE_SIZE                 5
#define WIDGET_AUX_TABLE_UPLOAD_SIZE          1
#define WIDGET_AUX_INDEX_MODE_VARI_PROGRAM    0
#define WIDGET_AUX_INDEX_FILM_ISO             1
#define WIDGET_AUX_INDEX_APERTURE_RANGE_UPPER 2
#define WIDGET_AUX_INDEX_FOCAL_LENGTH_UPPER   3
#define WIDGET_AUX_INDEX_EXPOSURE_EFFECTIVE   4

GtkWidget *gpControlsWindow = NULL;

GtkWidget *gpControlsApplicationBar         = NULL;
GtkWidget *gpControlsComboFramesPerSequence = NULL;
GtkWidget *gpControlsComboStepsPerFrame     = NULL;
GtkWidget *gpControlsComboExposure          = NULL;
GtkWidget *gpControlsComboFlashExposure     = NULL;

GtkWidget *gpControlsButtonDownload         = NULL;
GtkWidget *gpControlsButtonUpload           = NULL;
GtkWidget *gpControlsButtonDefault          = NULL;
GtkWidget *gpControlsButtonFocus            = NULL;
GtkWidget *gpControlsButtonRelease          = NULL;

uint8  gui8DefaultControlsLoaded = FALSE;
uint16 gui16FileDescriptor;

uint16 ui16ControlsConvertFocusDistance(uint8 ui8RawValue) {
  double ui64RetVal = pow(10, ui8RawValue/1.25);
  ui64RetVal = sqrt(ui64RetVal);
  ui64RetVal = sqrt(ui64RetVal);
  ui64RetVal = sqrt(ui64RetVal);
  ui64RetVal = sqrt(ui64RetVal);
  ui64RetVal = sqrt(ui64RetVal);
  return (uint16)ui64RetVal;
}

void vMenuViewShowControlsActivate(void) {
  uint16 ui16Index;
  GladeXML *xmlControlsWindow;

  if (gpControlsWindow != NULL)
    return;

  if(!(xmlControlsWindow = glade_xml_new(gpui8XmlPath, "windowControls"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xmlControlsWindow);

  gpControlsWindow = glade_xml_get_widget(xmlControlsWindow, "windowControls");

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++)
    grWidgetTable[ui16Index].pWidget = glade_xml_get_widget(xmlControlsWindow, grWidgetTable[ui16Index].pui8WidgetString);

  gpControlsButtonDownload         = glade_xml_get_widget(xmlControlsWindow, "buttonDownloadControl");
  gpControlsButtonUpload           = glade_xml_get_widget(xmlControlsWindow, "buttonUploadControl");
  gpControlsButtonDefault          = glade_xml_get_widget(xmlControlsWindow, "buttonDefaultControl");
  gpControlsButtonFocus            = glade_xml_get_widget(xmlControlsWindow, "buttonFocusControl");
  gpControlsButtonRelease          = glade_xml_get_widget(xmlControlsWindow, "buttonReleaseControl");
  gpControlsComboFramesPerSequence = glade_xml_get_widget(xmlControlsWindow, "comboControlFramesPerSequence");
  gpControlsComboStepsPerFrame     = glade_xml_get_widget(xmlControlsWindow, "comboControlStepsPerFrame");
  gpControlsComboExposure          = glade_xml_get_widget(xmlControlsWindow, "comboControlExposure");
  gpControlsComboFlashExposure     = glade_xml_get_widget(xmlControlsWindow, "comboControlFlashExposure");

  gpControlsApplicationBar = glade_xml_get_widget(xmlControlsWindow, "appbarControls");

  vControlsButtonDownload_Released();
  gtk_object_unref(GTK_OBJECT(xmlControlsWindow));
}

void vControlsButtonDownload_Released(void) {
  uint8 *pui8String  = NULL;
  uint8 *pui8String2 = NULL;
  uint8  ui8Buffer[256];
  uint16 ui16Value;
  uint16 ui16ReturnValue;
  uint16 ui16Index;
  float  fFocusDistance;

  vControlsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Downloading...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue =  ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vControlsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  for (ui16Index = 0; ui16Index < WIDGET_AUX_TABLE_SIZE; ui16Index++) {
    /* get data from register */
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor,
                                       grAuxWidgetTable[ui16Index].ui8RegisterMode,
                                       grAuxWidgetTable[ui16Index].ui32RegisterAddress,
                                       grAuxWidgetTable[ui16Index].ui8RegisterWidth,
                                       grAuxWidgetTable[ui16Index].ui16RegisterMask,
                                       grAuxWidgetTable[ui16Index].ui8RegisterShift,
                                       &ui16Value);

    grAuxWidgetTable[ui16Index].ui16RegisterValue = ui16Value;
    grAuxWidgetTable[ui16Index].ui16WidgetValue = ui16Value;
    if (!gui8DefaultControlsLoaded)
      grAuxWidgetTable[ui16Index].ui16DefaultValue = ui16Value;
    gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_SIZE + WIDGET_AUX_TABLE_SIZE))*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();
  }

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
    if (!gui8DefaultControlsLoaded)
      grWidgetTable[ui16Index].ui16DefaultValue = ui16Value;

    switch(ui16Index) {
    case WIDGET_INDEX_PROGRAM_MODE:
      if (grWidgetTable[ui16Index].ui16RegisterValue >= 0x08) {
        vNKN_DataToString(grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].prData,
                           grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].ui16RegisterValue,
                           &pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }
      else {
        vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }

      grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable = ((grWidgetTable[ui16Index].ui16RegisterValue == 0x01) || /* shutter */
                                            (grWidgetTable[ui16Index].ui16RegisterValue == 0x03)); /* manual */
      gtk_widget_set_sensitive(gpControlsComboExposure, grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable);
      break;
    case WIDGET_INDEX_EXPOSURE:
      /* if not enabled, get effective exposure, so we show always up to date value */
      if (!grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable) {
        vNKN_DataToString(grAuxWidgetTable[WIDGET_AUX_INDEX_EXPOSURE_EFFECTIVE].prData,
                          grAuxWidgetTable[WIDGET_AUX_INDEX_EXPOSURE_EFFECTIVE].ui16RegisterValue,
                          &pui8String);
      }
      else {
        vNKN_DataToString(grWidgetTable[WIDGET_INDEX_EXPOSURE].prData,
                          grWidgetTable[WIDGET_INDEX_EXPOSURE].ui16RegisterValue,
                          &pui8String);
      }
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      break;
    case WIDGET_INDEX_CURRENT_FRAME:
      if (grWidgetTable[ui16Index].ui16RegisterValue == 0)
        sprintf(ui8Buffer, "NO FILM");
      else
        sprintf(ui8Buffer, "%d", grWidgetTable[ui16Index].ui16RegisterValue - 1);
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      break;
    case WIDGET_INDEX_FLASH_INSTALL_STATUS:
      vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      /* enable/disable controls flash controls */
      grWidgetTable[WIDGET_INDEX_FLASH_EXPOSURE].ui8Enable = (grWidgetTable[ui16Index].ui16RegisterValue == 0x01);
      gtk_widget_set_sensitive(gpControlsComboFlashExposure, grWidgetTable[WIDGET_INDEX_FLASH_EXPOSURE].ui8Enable);
      break;
    case WIDGET_INDEX_APERTURE_RANGE_LOWER:
      vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
      if (grWidgetTable[ui16Index].ui16RegisterValue == grAuxWidgetTable[WIDGET_AUX_INDEX_APERTURE_RANGE_UPPER].ui16RegisterValue)
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      else {
        vNKN_DataToString(grAuxWidgetTable[WIDGET_AUX_INDEX_APERTURE_RANGE_UPPER].prData,
                           grAuxWidgetTable[WIDGET_AUX_INDEX_APERTURE_RANGE_UPPER].ui16RegisterValue,
                           &pui8String2);
        sprintf(ui8Buffer, "%s - %s", pui8String2, pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      }
      break;
    case WIDGET_INDEX_FOCAL_LENGTH_LOWER:
      vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
      if (grWidgetTable[ui16Index].ui16RegisterValue == grAuxWidgetTable[WIDGET_AUX_INDEX_FOCAL_LENGTH_UPPER].ui16RegisterValue)
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      else {
        vNKN_DataToString(grAuxWidgetTable[WIDGET_AUX_INDEX_FOCAL_LENGTH_UPPER].prData,
                          grAuxWidgetTable[WIDGET_AUX_INDEX_FOCAL_LENGTH_UPPER].ui16RegisterValue,
                          &pui8String2);
        sprintf(ui8Buffer, "%s - %s", pui8String, pui8String2);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      }
      break;
    case WIDGET_INDEX_FOCUS_DISTANCE:
      fFocusDistance = ui16ControlsConvertFocusDistance(grWidgetTable[ui16Index].ui16RegisterValue);
      sprintf(ui8Buffer, "%.2f meter", fFocusDistance/100);
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      break;
    case WIDGET_INDEX_SELF_TIMER:
      sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16RegisterValue + 2);
      gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(grWidgetTable[ui16Index].pWidget)), ui8Buffer);
      break;
    case WIDGET_INDEX_LCD_TIMER:
      sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16RegisterValue ? grWidgetTable[ui16Index].ui16RegisterValue : 8);
      gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(grWidgetTable[ui16Index].pWidget)), ui8Buffer);
      break;
    case WIDGET_INDEX_FOCUS_STATUS:
/*        sprintf(ui8Buffer, "%x", grWidgetTable[ui16Index].ui16RegisterValue); */
      vNKN_DataToString(grWidgetTable[ui16Index].prData, (grWidgetTable[ui16Index].ui16RegisterValue < 0xFB), &pui8String);
      gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      break;
    default:
      if (grWidgetTable[ui16Index].prData == NULL) {
        sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16RegisterValue);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      }
      else {
        vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16RegisterValue, &pui8String);
        if (pui8String == NULL) {
          sprintf(ui8Buffer, "0x%x:UNKNOWN",  grWidgetTable[ui16Index].ui16RegisterValue);
          gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
        }
        else
          gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }
    }

    gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_SIZE + WIDGET_AUX_TABLE_SIZE))*(ui16Index + WIDGET_AUX_TABLE_SIZE));
    while (gtk_events_pending())
      gtk_main_iteration();
  }
  gui8DefaultControlsLoaded = TRUE;

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Download complete.");

  vControlsButtonSetSensitive(TRUE);
}


void vControlsButtonUpload_Released(void) {
  uint8 *pui8String;
  uint16 ui16Index;
  uint16 ui16ReturnValue;

  vControlsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Uploading...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue =  ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vControlsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_UPLOAD_SIZE; ui16Index++) {
    if (!grWidgetTable[ui16Index].ui8Enable)
      continue;

    switch(ui16Index) {
    case WIDGET_INDEX_PROGRAM_MODE:
      pui8String = gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget));
      vNKN_StringToData(grWidgetTable[ui16Index].prData, pui8String, &grWidgetTable[ui16Index].ui16WidgetValue);
      if (grWidgetTable[ui16Index].ui16WidgetValue == 0xFFFF) {
        grWidgetTable[ui16Index].ui16WidgetValue = 0x08;
        vNKN_StringToData(grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].prData,
                           pui8String,
                           &grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].ui16WidgetValue);
        break;
      }
      break;
    case WIDGET_INDEX_SELF_TIMER:
      pui8String = gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget));
      grWidgetTable[ui16Index].ui16WidgetValue = atoi(pui8String) - 2;
      break;
    case WIDGET_INDEX_LCD_TIMER:
      pui8String = gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget));
      grWidgetTable[ui16Index].ui16WidgetValue = atoi(pui8String);
      if (grWidgetTable[ui16Index].ui16WidgetValue == 8)
        grWidgetTable[ui16Index].ui16WidgetValue = 0;
      break;
    default:
      if (grWidgetTable[ui16Index].prData == NULL) {
        grWidgetTable[ui16Index].ui16WidgetValue = atoi(gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget)));
      }
      else {
        vNKN_StringToData(grWidgetTable[ui16Index].prData, gtk_entry_get_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget)), &grWidgetTable[ui16Index].ui16WidgetValue);
      }
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

    gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_UPLOAD_SIZE + 1))*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();
  }

  ui16Index = WIDGET_AUX_INDEX_MODE_VARI_PROGRAM;
  if (grAuxWidgetTable[ui16Index].ui16WidgetValue != grAuxWidgetTable[ui16Index].ui16RegisterValue) {
      grAuxWidgetTable[ui16Index].ui16RegisterValue = grAuxWidgetTable[ui16Index].ui16WidgetValue;
      ui16ReturnValue += ui16NKN_Write16(gui16FileDescriptor,
                                          grAuxWidgetTable[ui16Index].ui8RegisterMode,
                                          grAuxWidgetTable[ui16Index].ui32RegisterAddress,
                                          grAuxWidgetTable[ui16Index].ui8RegisterWidth,
                                          grAuxWidgetTable[ui16Index].ui16RegisterMask,
                                          grAuxWidgetTable[ui16Index].ui8RegisterShift,
                                          grAuxWidgetTable[ui16Index].ui16RegisterValue);

    }
  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_UPLOAD_SIZE + 1))*ui16Index);
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Upload complete.");

  vControlsButtonSetSensitive(TRUE);
}

void vControlsButtonDefault_Released(void) {
  uint8 *pui8String = NULL;
  uint8  ui8Buffer[256];
  uint16 ui16Index;
  uint16 ui16ReturnValue;


  if (!gui8DefaultControlsLoaded) {
    gnome_app_message(GNOME_APP(gpControlsWindow), "No default values loaded.\nDefault values will be loaded\nat the first download.");
    return;
  }

  vControlsButtonSetSensitive(FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Uploading defaults...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue =  ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vControlsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_UPLOAD_SIZE; ui16Index++) {
    if (!grWidgetTable[ui16Index].ui8Enable)
      continue;

    switch(ui16Index) {
    case WIDGET_INDEX_PROGRAM_MODE:
      if (grWidgetTable[ui16Index].ui16DefaultValue == 0x08) {
        vNKN_DataToString(grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].prData,
                           grAuxWidgetTable[WIDGET_AUX_INDEX_MODE_VARI_PROGRAM].ui16DefaultValue,
                           &pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }
      else {
        vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16DefaultValue, &pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }
      grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable = ((grWidgetTable[ui16Index].ui16DefaultValue == 0x01) || /* shutter */
                                                        (grWidgetTable[ui16Index].ui16DefaultValue == 0x03)); /* manual */
      gtk_widget_set_sensitive(gpControlsComboExposure, grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable);
      break;
    case WIDGET_INDEX_SELF_TIMER:
      sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16DefaultValue + 2);
      gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(grWidgetTable[ui16Index].pWidget)), ui8Buffer);
      break;
    case WIDGET_INDEX_LCD_TIMER:
      sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16DefaultValue ? grWidgetTable[ui16Index].ui16DefaultValue : 8);
      gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(grWidgetTable[ui16Index].pWidget)), ui8Buffer);
      break;
    default:
      if (grWidgetTable[ui16Index].prData == NULL) {
        sprintf(ui8Buffer, "%d",  grWidgetTable[ui16Index].ui16DefaultValue);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), ui8Buffer);
      }
      else {
        vNKN_DataToString(grWidgetTable[ui16Index].prData, grWidgetTable[ui16Index].ui16DefaultValue, &pui8String);
        gtk_entry_set_text(GTK_ENTRY(grWidgetTable[ui16Index].pWidget), pui8String);
      }
    }

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

    gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_UPLOAD_SIZE + 1))*ui16Index);
    while (gtk_events_pending())
      gtk_main_iteration();
  }

  ui16Index = WIDGET_AUX_INDEX_MODE_VARI_PROGRAM;
  if (grAuxWidgetTable[ui16Index].ui16WidgetValue != grAuxWidgetTable[ui16Index].ui16RegisterValue) {
    grAuxWidgetTable[ui16Index].ui16RegisterValue = grAuxWidgetTable[ui16Index].ui16WidgetValue;
    ui16ReturnValue += ui16NKN_Write16(gui16FileDescriptor,
                                        grAuxWidgetTable[ui16Index].ui8RegisterMode,
                                        grAuxWidgetTable[ui16Index].ui32RegisterAddress,
                                        grAuxWidgetTable[ui16Index].ui8RegisterWidth,
                                        grAuxWidgetTable[ui16Index].ui16RegisterMask,
                                        grAuxWidgetTable[ui16Index].ui8RegisterShift,
                                        grAuxWidgetTable[ui16Index].ui16DefaultValue);
  }
  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), (gfloat)(1/(gfloat)(WIDGET_TABLE_UPLOAD_SIZE + 1))*ui16Index);
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Upload complete.");

  vControlsButtonSetSensitive(TRUE);
}

void vControlsButtonFocus_Released(void) {
  uint16 ui16ReturnValue;

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Focusing...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vControlsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed establish connection.");
    return;
  }
  ui16ReturnValue  = ui16NKN_Focus(gui16FileDescriptor);;
  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Focusing complete.");
}

void vControlsButtonRelease_Released(void) {
  uint16 ui16ReturnValue;

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 0);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Taking picture...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue > 0) {
    vControlsButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue  = ui16NKN_Release(gui16FileDescriptor);;
  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  gnome_appbar_set_progress(GNOME_APPBAR(gpControlsApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Picture taken.");
}

void vControlsButtonHelp_Released(void) {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/controls/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpControlsApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpControlsWindow), "Failed to start the help browser.");
}

void vControlsBracketingMode_Changed(void) {
  uint8 *pui8String = gtk_entry_get_text(GTK_ENTRY(grWidgetTable[WIDGET_INDEX_BRACKETING_MODE].pWidget));
  uint16 ui16BracketingMode = 0;
  vNKN_StringToData(grWidgetTable[WIDGET_INDEX_BRACKETING_MODE].prData, pui8String, &ui16BracketingMode);

  switch (ui16BracketingMode) {
  default:
    gnome_app_error (GNOME_APP(gpControlsWindow), "Invalid value ");
  case 0x0:
    gtk_widget_set_sensitive(gpControlsComboFramesPerSequence, 0);
    gtk_widget_set_sensitive(gpControlsComboStepsPerFrame, 0);
    grWidgetTable[WIDGET_INDEX_FRAMES_PER_SEQUENCE].ui8Enable = 0;
    grWidgetTable[WIDGET_INDEX_STEPS_PER_FRAME].ui8Enable = 0;
    break;
  case 0x1:
  case 0x8:
    gtk_widget_set_sensitive(gpControlsComboFramesPerSequence, 1);
    gtk_widget_set_sensitive(gpControlsComboStepsPerFrame, 0);
    grWidgetTable[WIDGET_INDEX_FRAMES_PER_SEQUENCE].ui8Enable = 1;
    grWidgetTable[WIDGET_INDEX_STEPS_PER_FRAME].ui8Enable = 0;
    break;
  case 0x2:
  case 0x4:
    gtk_widget_set_sensitive(gpControlsComboFramesPerSequence, 1);
    gtk_widget_set_sensitive(gpControlsComboStepsPerFrame, 1);
    grWidgetTable[WIDGET_INDEX_FRAMES_PER_SEQUENCE].ui8Enable = 1;
    grWidgetTable[WIDGET_INDEX_STEPS_PER_FRAME].ui8Enable = 1;
  }
}

void vControlsProgramMode_Changed(void) {
  uint8 *pui8String = gtk_entry_get_text(GTK_ENTRY(grWidgetTable[WIDGET_INDEX_PROGRAM_MODE].pWidget));
  uint16 ui16ProgramMode = 0;
  vNKN_StringToData(grWidgetTable[WIDGET_INDEX_PROGRAM_MODE].prData, pui8String, &ui16ProgramMode);

  grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable = ((ui16ProgramMode == 0x01) || /* shutter */
                                                    (ui16ProgramMode == 0x03));  /* manual */
  gtk_widget_set_sensitive(gpControlsComboExposure, grWidgetTable[WIDGET_INDEX_EXPOSURE].ui8Enable);
}

void vControlsButtonSetSensitive(uint8 ui8Sensitive) {
  gtk_widget_set_sensitive(gpControlsButtonDownload, ui8Sensitive);
  gtk_widget_set_sensitive(gpControlsButtonUpload,   ui8Sensitive);
  gtk_widget_set_sensitive(gpControlsButtonDefault,  ui8Sensitive);
  gtk_widget_set_sensitive(gpControlsButtonFocus,    ui8Sensitive);
  gtk_widget_set_sensitive(gpControlsButtonRelease,  ui8Sensitive);
}

void vControlsWindowDestroy(void) {
  uint16 ui16Index;

  for (ui16Index = 0; ui16Index < WIDGET_TABLE_SIZE; ui16Index++) {
    gtk_widget_destroy(GTK_WIDGET(grWidgetTable[ui16Index].pWidget));
    grWidgetTable[ui16Index].pWidget = NULL;
  }

  gtk_widget_destroy(GTK_WIDGET(gpControlsWindow));
  gtk_widget_destroy(GTK_WIDGET(gpControlsComboFramesPerSequence));
  gtk_widget_destroy(GTK_WIDGET(gpControlsComboStepsPerFrame));
  gtk_widget_destroy(GTK_WIDGET(gpControlsApplicationBar));

  gpControlsWindow = NULL;
  gpControlsComboFramesPerSequence = NULL;
  gpControlsComboStepsPerFrame = NULL;
  gpControlsApplicationBar = NULL;

}

void vMainButtonControls_Released() {
  vMenuViewShowControlsActivate();
}
