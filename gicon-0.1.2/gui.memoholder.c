/*

  gui.memoholder.c

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

#include "common.h"
#include "nkn.h"
#include "file.h"
#include "util.h"

#include "gui.main.h"
#include "gui.notebook.h"

extern uint8  gpui8XmlPath[256];
extern uint8  gpui8Device[256];
extern uint16 gui16Baud;

extern tNKN_DataArray rpCameraIsoFilmSpeed[];
extern tNKN_DataArray rpCameraExposureEffective[];
extern tNKN_DataArray rpCameraApertureEffective[];
extern tNKN_DataArray rpCameraFocusDistance[];
extern tNKN_DataArray rpNKN_LightMetering[];
extern tNKN_DataArray rpNKN_ProgramMode[];
extern tNKN_DataArray rpNKN_FlashMode[];
extern tNKN_DataArray rpNKN_Compensation[];

extern GtkWidget *gpMainApplicationBar;
extern GtkWidget *gpMainWindow;
extern GtkWidget *gpMainWindowNoteBook;

GtkWidget *gpStatusWindow                     = NULL;
GtkWidget *gpStatusEntryTotalBytes            = NULL;
GtkWidget *gpStatusEntryUsedBytes             = NULL;
GtkWidget *gpStatusEntryFreeBytes             = NULL;
GtkWidget *gpStatusDialMemoryUsage            = NULL;

GtkWidget *gpStatusEntryStoredRollNumber      = NULL;
GtkWidget *gpStatusEntryAvailableSpaceMaximum = NULL;
GtkWidget *gpStatusEntryAvailableSpaceMedium  = NULL;
GtkWidget *gpStatusEntryAvailableSpaceMinimum = NULL;
GtkWidget *gpStatusButtonDownload             = NULL;
GtkWidget *gpStatusApplicationBar             = NULL;

uint16 gui16FileDescriptor;
void vStatusButtonDownload_Released();

void vMenuMemoStatusActivate(void) {
  GladeXML *xmlStatusWindow;

  if (gpStatusWindow != NULL)
    return;

  if(!(xmlStatusWindow = glade_xml_new(gpui8XmlPath, "windowStatus"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xmlStatusWindow);

  gpStatusWindow = glade_xml_get_widget(xmlStatusWindow, "windowStatus");

  gpStatusEntryTotalBytes            = glade_xml_get_widget(xmlStatusWindow, "entryStatusTotalBytes");
  gpStatusEntryUsedBytes             = glade_xml_get_widget(xmlStatusWindow, "entryStatusUsedBytes");
  gpStatusEntryFreeBytes             = glade_xml_get_widget(xmlStatusWindow, "entryStatusFreeBytes");
  gpStatusDialMemoryUsage            = glade_xml_get_widget(xmlStatusWindow, "dialStatusMemoryUsage");
  gpStatusEntryStoredRollNumber      = glade_xml_get_widget(xmlStatusWindow, "entryStatusStoredRollNumber");
  gpStatusEntryAvailableSpaceMaximum = glade_xml_get_widget(xmlStatusWindow, "entryStatusAvailableSpaceMaximum");
  gpStatusEntryAvailableSpaceMedium  = glade_xml_get_widget(xmlStatusWindow, "entryStatusAvailableSpaceMedium");
  gpStatusEntryAvailableSpaceMinimum = glade_xml_get_widget(xmlStatusWindow, "entryStatusAvailableSpaceMinimum");
  gpStatusButtonDownload             = glade_xml_get_widget(xmlStatusWindow, "buttonDownloadStatus");
  gpStatusApplicationBar             = glade_xml_get_widget(xmlStatusWindow, "appbarStatus");

  while (gtk_events_pending())
    gtk_main_iteration();

  vStatusButtonDownload_Released();
}

void vStatusButtonDownload_Released(void) {
  uint16 ui16ReturnValue;

  uint32 ui32MH_CurrentAddress;
  uint32 ui32MH_DataStartAddress;
  uint32 ui32MH_DataEndAddress;
  uint32 ui32MH_StartAddress;
  uint32 ui32MH_EndAddress;
  uint32 ui32MH_RollStartAddress;
  uint32 ui32MH_TotalBytes;
  uint32 ui32MH_UsedBytes;
  uint32 ui32MH_FreeBytes;

  uint32 ui32Progress;
  uint16 ui16Value;
  uint16 ui16MH_RollSize;
  uint16 ui16MH_RollNum;

  uint8 pui8Buffer[50];

  gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "Collecting data...");
  gtk_widget_set_sensitive(gpStatusButtonDownload, FALSE);

  gnome_appbar_set_progress(GNOME_APPBAR(gpStatusApplicationBar), 0);

  ui16ReturnValue =  ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue) {
    gtk_widget_set_sensitive(gpStatusButtonDownload, TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "");
    gnome_app_error(GNOME_APP(gpStatusWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue += ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_CURRENT_MEMO_POINTER, &ui16Value);
  ui32MH_DataEndAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_START_POINTER, &ui16Value);
  ui32MH_DataStartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_ROLL_START_POINTER, &ui16Value);
  ui32MH_RollStartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_BASE_MEMO_HOLDER, &ui16Value);
  ui32MH_StartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_END_MEMO_HOLDER, &ui16Value);
  ui32MH_EndAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_SETTINGS, &ui16Value);

  if (ui16ReturnValue) {
    gtk_widget_set_sensitive(gpStatusButtonDownload, TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "");
    gnome_app_error(GNOME_APP(gpStatusWindow), "Failed to download data.");
    return;
  }

  switch (ui16Value) {
  case 0x05:
    ui16MH_RollSize = 84;
    break;
  case 0x0E:
    ui16MH_RollSize = 158;
    break;
  case 0x1F:
  default:
    ui16MH_RollSize = 232;
  }

  ui32MH_TotalBytes = ui32MH_EndAddress - ui32MH_StartAddress;

  if (ui32MH_DataStartAddress < ui32MH_DataEndAddress)
    ui32MH_UsedBytes = ui32MH_DataEndAddress - ui32MH_DataStartAddress;
  else
    ui32MH_UsedBytes =
      ui32MH_EndAddress - ui32MH_DataStartAddress +
      ui32MH_DataEndAddress - ui32MH_StartAddress;
  ui32MH_FreeBytes = ui32MH_TotalBytes - ui32MH_UsedBytes;

  sprintf(pui8Buffer, "%ld", ui32MH_TotalBytes);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryTotalBytes), pui8Buffer);

  sprintf(pui8Buffer, "%ld", ui32MH_UsedBytes);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryUsedBytes), pui8Buffer);

  sprintf(pui8Buffer, "%ld", ui32MH_FreeBytes);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryFreeBytes), pui8Buffer);

  gtk_dial_set_percentage(GTK_DIAL(gpStatusDialMemoryUsage), ((gfloat)ui32MH_UsedBytes)/ui32MH_TotalBytes);

  ui32Progress = 0;
  ui16MH_RollNum = 0;
  /* dunno why, but I always get checksum error if I try to read more than 2 bytes at once */
  for (ui32MH_CurrentAddress = ui32MH_DataStartAddress;
       ui32Progress < ui32MH_UsedBytes;
       ui32MH_CurrentAddress += 2) {

    ui16ReturnValue = ui16NKN_Read16(gui16FileDescriptor, M_RD, ui32MH_CurrentAddress, 2, 0xFFFF, 0, &ui16Value);
    if (ui16ReturnValue) {
      gnome_app_error(GNOME_APP(gpStatusWindow), "There were some communication errors\n during estimation.\nEstimation aborted.");
      return;
    }

    if (ui32MH_CurrentAddress == (ui32MH_EndAddress - 2))
      ui32MH_CurrentAddress = ui32MH_StartAddress;

    if (((ui16Value & 0xFF) == 0xFF) ||
        (((ui16Value >> 8) & 0xFF) == 0xFF))
      ui16MH_RollNum++;

    gnome_appbar_set_progress(GNOME_APPBAR(gpStatusApplicationBar), (gfloat)(1/(gfloat)ui32MH_UsedBytes)*ui32Progress);
    while (gtk_events_pending())
      gtk_main_iteration();
    ui32Progress += 2;
  }
  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  if (ui32MH_RollStartAddress != ui32MH_DataEndAddress)
    sprintf(pui8Buffer, "%d+", ui16MH_RollNum);
  else
    sprintf(pui8Buffer, "%d", ui16MH_RollNum);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryStoredRollNumber), pui8Buffer);

  sprintf(pui8Buffer, "%ld", ui32MH_FreeBytes/232);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryAvailableSpaceMaximum), pui8Buffer);

  sprintf(pui8Buffer, "%ld", ui32MH_FreeBytes/158);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryAvailableSpaceMedium), pui8Buffer);

  sprintf(pui8Buffer, "%ld", ui32MH_FreeBytes/84);
  gtk_entry_set_text(GTK_ENTRY(gpStatusEntryAvailableSpaceMinimum), pui8Buffer);

  gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "Done.");
  gtk_widget_set_sensitive(gpStatusButtonDownload, TRUE);
  gnome_appbar_set_progress(GNOME_APPBAR(gpStatusApplicationBar), 1);

  return;
}


typedef enum {
  DLSM_STATE_ROLL_NUM,
  DLSM_STATE_DATA_MIN,
  DLSM_STATE_DATA_MED,
  DLSM_STATE_DATA_MAX,
  DLSM_STATE_ISO,
  DLSM_STATE_HEADER
} DLSM_STATE;


void vDownloadData(uint8 ui8Mode) {
  uint16 ui16ReturnValue;
  uint8  pui8MH_Data[2];
  uint8  *pui8String = NULL;
  uint8  ui8Data;

  uint16 ui16Value;
  uint16 ui16MH_Settings;
  uint32 ui32MH_CurrentAddress;
  uint32 ui32MH_UsedBytes;
  uint32 ui32Progress;
  uint32 ui32MH_DataStartAddress; /* start addres, to read data from, depends on the ui8Mode */
  uint32 ui32MH_DataEndAddress;   /* end address depends on the ui8Mode */
  uint32 ui32MH_StartAddress;     /* lower bound of the ring buffer */
  uint32 ui32MH_EndAddress;       /* upper bound of the ring buffer */


  uint16 ui16EntryIndex;
  uint16 ui16FrameIndex;

  GladeXML  *xml = NULL;
  GtkWidget *pVboxMain  = NULL;
  GtkWidget *pPageLabel = NULL;
  tRoll     rRoll;

  time_t tCurrentTime = time(NULL);
  DLSM_STATE eState = DLSM_STATE_ROLL_NUM;

  vMainButtonSetSensitive(FALSE);
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Downloading...");
  while (gtk_events_pending())
    gtk_main_iteration();

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue) {
    vMainButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue  = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);
/*    keep this here for test purposes    */
/*    ui16ReturnValue = ui16NKN_Write16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_START_POINTER, 0x760); */
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_SETTINGS, &ui16MH_Settings);

  switch (ui16MH_Settings) {
  case 0x05: rRoll.ui8StorageLevel = 0; break;
  case 0x0E: rRoll.ui8StorageLevel = 1; break;
  case 0x1F:
  default:   rRoll.ui8StorageLevel = 2; break;
  }

  if (ui8Mode == 0) {
    /* download mode*/
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_START_POINTER, &ui16Value);
    ui32MH_DataStartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_ROLL_START_POINTER, &ui16Value);
    ui32MH_DataEndAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  }
  else {
    /* preview mode*/
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_ROLL_START_POINTER, &ui16Value);
    ui32MH_DataStartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
    ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_CURRENT_MEMO_POINTER, &ui16Value);
    ui32MH_DataEndAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value + 6 - (2 * rRoll.ui8StorageLevel);
  }

  DEBUG(("start: %8lx end: %8lx\n", ui32MH_DataStartAddress, ui32MH_DataEndAddress));

  if (ui32MH_DataStartAddress == ui32MH_DataEndAddress) {
    gnome_app_message(GNOME_APP(gpMainWindow), "Memo holder is empty.");
    gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
    vMainButtonSetSensitive(TRUE);
    return;
  }

  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_BASE_MEMO_HOLDER, &ui16Value);
  ui32MH_StartAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_END_MEMO_HOLDER, &ui16Value);
  ui32MH_EndAddress = OFFSET_MEMO_HOLDER_ADDRESS + ui16Value;
  if (ui16ReturnValue) {
    gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors getting memo holder status.\nDownload aborted.");
    vMainButtonSetSensitive(TRUE);
    return;
  }

  if (ui32MH_DataStartAddress < ui32MH_DataEndAddress)
    ui32MH_UsedBytes = ui32MH_DataEndAddress - ui32MH_DataStartAddress;
  else
    ui32MH_UsedBytes = ui32MH_EndAddress - ui32MH_DataStartAddress + ui32MH_DataEndAddress - ui32MH_StartAddress;

  /* PRINTER START */
/*    ui32Progress = 0; */
/*    for (ui32MH_CurrentAddress = ui32MH_DataStartAddress; */
/*         ui32Progress < ui32MH_UsedBytes; */
/*         ui32MH_CurrentAddress += 2) { */
/*      ui16ReturnValue = ui16NKN_Read16(gui16FileDescriptor, M_RD, ui32MH_CurrentAddress, 2, 0xFFFF, 0, &ui16Value); */
/*      printf("%.2x %.2x ", ((ui16Value & 0xFF00)>>8)& 0xFF, ui16Value & 0xFF); */
/*      ui32Progress++; */
/*      if (ui32Progress == 10) { */
/*        printf("\n"); */
/*        ui32Progress = 0; */
/*      } */
/*    } */
/*    return; */
  /* PRINTER END */

  ui32Progress = 0;
  /* dunno why, but I always get checksum error if I try to read more than 2 bytes at once */
  for (ui32MH_CurrentAddress = ui32MH_DataStartAddress;
       ui32Progress < ui32MH_UsedBytes;
       ui32MH_CurrentAddress += 2) {

    /* read data */
    switch (eState) {
    case DLSM_STATE_ISO:
    case DLSM_STATE_HEADER:
      break;
    case DLSM_STATE_ROLL_NUM:
    case DLSM_STATE_DATA_MIN:
    default:
      ui16ReturnValue = ui16NKN_Read16(gui16FileDescriptor, M_RD, ui32MH_CurrentAddress, 2, 0xFFFF, 0, &ui16Value);
      if (ui16ReturnValue) {
        gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors during download.\nDownload aborted.");
        vMainButtonSetSensitive(TRUE);
        return;
      }
      printf("%x ", ui16Value);
      if (ui32MH_CurrentAddress == (ui32MH_EndAddress - 2))
        ui32MH_CurrentAddress = ui32MH_StartAddress;

      pui8MH_Data[0] = ui16Value & 0xFF;
      pui8MH_Data[1] = (ui16Value >> 8) & 0xFF;

      gnome_appbar_set_progress(GNOME_APPBAR(gpMainApplicationBar), (gfloat)(1/(gfloat)ui32MH_UsedBytes)*ui32Progress);
      ui32Progress += 2;
    }

    /* in preview mode force to end into this state as download finished,
       get film iso, which, normally would be available at the end of the roll  */
    if (ui8Mode && (ui32Progress >= ui32MH_UsedBytes)) {
      ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_FILM_ISO_EFFECTIVE, &ui16Value);
      if (ui16ReturnValue) {
        gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors during download.\nDownload aborted.");
        vMainButtonSetSensitive(TRUE);
        return;
      }
      vNKN_DataToString(rpCameraIsoFilmSpeed, ui16Value, &pui8String);
      if(pui8String != NULL)
        strcpy(rRoll.pui8FilmSpeed, pui8String);
      eState = DLSM_STATE_HEADER;
    }

    switch (eState) {
    case DLSM_STATE_ROLL_NUM:
      sprintf(rRoll.pui8FileName, "D%.4d.nkn", ui16Value);
      sprintf(rRoll.pui8RollTitle, "New roll %.4d", ui16Value);
      sprintf(rRoll.pui8CameraType, "F90X/N90s");

      sprintf(rRoll.pui8TimeStamp, ctime(&tCurrentTime));
      rRoll.ui32TimeStamp = tCurrentTime;
      rRoll.ui8FrameCounter = 0;
      for (ui16EntryIndex = 0; ui16EntryIndex < ENTRY_NUMBER; ui16EntryIndex++)
        for (ui16FrameIndex = 0; ui16FrameIndex < MAX_FRAME_NUMBER; ui16FrameIndex++) {
          rRoll.table[ui16FrameIndex].list[ui16EntryIndex] = malloc(256);
          strcpy(rRoll.table[ui16FrameIndex].list[ui16EntryIndex],"");
        }

      eState = DLSM_STATE_DATA_MIN;
      break;
    case DLSM_STATE_DATA_MIN:
      if (pui8MH_Data[0] == 0xff) {
        eState = DLSM_STATE_ISO;
        break;
      }
      sprintf(rRoll.table[rRoll.ui8FrameCounter].list[0], "%.2d", rRoll.ui8FrameCounter + 1);

      vNKN_DataToString(rpCameraExposureEffective, pui8MH_Data[0], &pui8String);
      if (pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[1], pui8String);
      vNKN_DataToString(rpCameraApertureEffective, pui8MH_Data[1], &pui8String);
      if (pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[2], pui8String);

      if (ui16MH_Settings > 0x05)
        eState = DLSM_STATE_DATA_MED;
      else
        rRoll.ui8FrameCounter++;
      break;
    case DLSM_STATE_DATA_MED:
      ui8Data = ((pui8MH_Data[0]) & 0xf);
      vNKN_DataToString(rpNKN_ProgramMode, ui8Data, &pui8String);
      if(pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[3], pui8String);

      vNKN_DataToString(rpNKN_LightMetering, (pui8MH_Data[0] >> 4) & 3, &pui8String);
      if (pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[4], pui8String);

      ui8Data = ((pui8MH_Data[0] >> 6) & 3);
      vNKN_DataToString(rpNKN_FlashMode, ui8Data, &pui8String);
      if(pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[5], pui8String);

      vNKN_DataToString(rpCameraFocusDistance, pui8MH_Data[1], &pui8String);
      if(pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[6], pui8String);

      if (ui16MH_Settings > 0x0E)
        eState = DLSM_STATE_DATA_MAX;
      else {
        eState = DLSM_STATE_DATA_MIN;
        rRoll.ui8FrameCounter++;
      }
      break;
    case DLSM_STATE_DATA_MAX:
      /* use flash compensation array for exposure as well, strange but works */
      vNKN_DataToString(rpNKN_Compensation, pui8MH_Data[0], &pui8String);
      if (pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[7], pui8String);

      vNKN_DataToString(rpNKN_Compensation, pui8MH_Data[1], &pui8String);
      if (pui8String != NULL)
        strcpy(rRoll.table[rRoll.ui8FrameCounter].list[8], pui8String);
      strcpy(rRoll.table[rRoll.ui8FrameCounter].list[9], "");
      eState = DLSM_STATE_DATA_MIN;
      rRoll.ui8FrameCounter++;
      printf("\n");
      break;
    case DLSM_STATE_ISO:
      vNKN_DataToString(rpCameraIsoFilmSpeed, pui8MH_Data[1], &pui8String);
      if(pui8String != NULL)
        strcpy(rRoll.pui8FilmSpeed, pui8String);
      eState = DLSM_STATE_HEADER;
      break;
    case DLSM_STATE_HEADER:
      printf("add notebook page\n");
      vMainCreateNoteBook();
      if(!(xml = glade_xml_new(gpui8XmlPath, "vboxMain"))) {
        g_warning("We could not load the interface!");
        return;
      }
      glade_xml_signal_autoconnect(xml);
      pVboxMain  = glade_xml_get_widget(xml, "vboxMain");
      pPageLabel = gtk_label_new(rRoll.pui8RollTitle);
      gtk_notebook_insert_page(GTK_NOTEBOOK(gpMainWindowNoteBook), pVboxMain, pPageLabel, 0);
      gtk_notebook_set_page(GTK_NOTEBOOK(gpMainWindowNoteBook), 0);
      vNoteBook_AddPage(xml, &rRoll, 1);
      eState = DLSM_STATE_ROLL_NUM;
      ui32MH_CurrentAddress += 2;
    }
    while (gtk_events_pending())
      gtk_main_iteration();
  }

  ui16ReturnValue = ui16NKN_SignOff(gui16FileDescriptor);
  vMainButtonSetSensitive(TRUE);
  gnome_appbar_set_progress(GNOME_APPBAR(gpMainApplicationBar), 1);
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Download complete.");

  if (ui16ReturnValue)
    gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors during download.\n");
}

void vMenuMemoDeleteWithoutQuestions(gint giReply, gpointer vNotUsedDataPointer) {
  uint16 ui16ReturnValue;
  uint16 ui16MH_RollStartAddress;

  if (giReply != 0)
    return;

  ui16ReturnValue  = ui16NKN_WakeUp(&gui16FileDescriptor, gpui8Device);
  ui16ReturnValue += ui16NKN_SignIn(gui16FileDescriptor);
  if (ui16ReturnValue) {
    vMainButtonSetSensitive(TRUE);
    gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed establish connection.");
    return;
  }

  ui16ReturnValue = ui16NKN_SetBaud(gui16FileDescriptor, gui16Baud);
  ui16ReturnValue += ui16NKN_Read16(gui16FileDescriptor, CAMERA_ROLL_START_POINTER, &ui16MH_RollStartAddress);

  DEBUG(("new start address: %4x\n", ui16MH_RollStartAddress));

  if (ui16ReturnValue == OK)
    ui16ReturnValue = ui16NKN_Write16(gui16FileDescriptor, CAMERA_MEMO_HOLDER_START_POINTER, ui16MH_RollStartAddress);

  ui16ReturnValue += ui16NKN_SignOff(gui16FileDescriptor);

  if (ui16ReturnValue)
    gnome_app_error(GNOME_APP(gpMainWindow), "There were some errors during download.\n");
}

void vMenuMemoDeleteActivate() {
  gnome_app_question(GNOME_APP(gpMainWindow),
                     "Warning!\nThis will remove the shooting data from your camera.\n",
                     vMenuMemoDeleteWithoutQuestions,
                     NULL);
}

void vStatusWindowDestroy() {
  gtk_widget_destroy(GTK_WIDGET(gpStatusWindow));
  gpStatusWindow = NULL;
}

void vMenuMemoDownloadActivate() {
  vDownloadData(0);
}

void vMenuMemoPreviewActivate() {
  vDownloadData(1);
}

void vMainButtonDownload_Released() {
  vDownloadData(0);
}

void vMainButtonPreview_Released() {
  vDownloadData(1);
}

void vMainButtonStatus_Released() {
  vMenuMemoStatusActivate();
}

void vStatusButtonHelp_Released(void) {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/status/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpStatusApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpStatusWindow), "Failed to start the help browser.");
}
