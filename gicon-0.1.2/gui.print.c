/*

  gui.print.c

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
#include <string.h>
#include <gnome.h>
#include <glade/glade.h>
#include <math.h>


#include "common.h"
#include "file.h"
#include "print.h"
#include "util.h"

#include "gui.notebook.h"
#include "gui.print.h"

extern uint8      gpui8XmlPath[256];
extern GtkWidget *gpMainWindow;
extern GtkWidget *gpMainApplicationBar;
extern GtkWidget *gpMainWindowNoteBook;

GtkWidget *gpPrintDialog                              = NULL;
GtkWidget *gpPrintDialogEntryPrintTo                  = NULL;
GtkWidget *gpPrintDialogEntryPrinterCommand           = NULL;
GtkWidget *gpPrintDialogEntryFile                     = NULL;
GtkWidget *gpPrintDialogEntryOrientation              = NULL;
GtkWidget *gpPrintDialogEntryPaperSize                = NULL;
GtkWidget *gpPrintDialogEntryFileBrowse               = NULL;
GtkWidget *gpPrintDialogCheckButtonCaption            = NULL;
GtkWidget *gpPrintDialogCheckButtonTitle              = NULL;
GtkWidget *gpPrintDialogCopyrightNote                 = NULL;
GtkWidget *gpPrintDialogSpinButtonColumns             = NULL;
GtkWidget *gpPrintDialogSpinButtonHorizontalOffset    = NULL;
GtkWidget *gpPrintDialogSpinButtonLabelWidth          = NULL;
GtkWidget *gpPrintDialogSpinButtonColumnSpacing       = NULL;
GtkWidget *gpPrintDialogSpinButtonVerticalOffset      = NULL;
GtkWidget *gpPrintDialogSpinButtonLabelHeight         = NULL;
GtkWidget *gpPrintDialogSpinButtonRowSpacing          = NULL;

uint8 gpui8PrintTo[256]        = "File";
uint8 gpui8PrinterCommand[256] = "lpr";
uint8 gui8Orientation[256]     = "Portrait"; /* 0-portrait 1-landscape */
uint8 gui8PaperSize[256]       = "A4        (210 x 297 mm)";
uint8 gpui8CopyrightNote[256];

uint8 gpui8PaperSize[256];
uint8 gpui8FileName[256];
tRoll grRoll;

tPrintLayout grPrintLayout;

void vEntryPrintTo_Changed();

void vPrintSlideWithoutQuestions(gint giReply, gpointer vNotUsedDataPointer) {
  sint16 si16ReturnValue;
  if (giReply == 0) { /* OK */
    si16ReturnValue = si16PrintSlide(gpui8FileName,
                                     gpui8CopyrightNote,
                                     gpui8PaperSize,
                                     &grRoll,
                                     &grPrintLayout);
    if (si16ReturnValue < 0)
      gnome_app_error(GNOME_APP(gpMainWindow), "Failed to print slide.");
  }
}

void vMenuFilePrintActivate() {
  GladeXML *xml;
  uint8 pui8Buffer[100];
  if (gpPrintDialog != NULL)
    return;

  if(!(xml = glade_xml_new(gpui8XmlPath, "dialogPrint"))) {
    g_warning("We could not load the interface!");
    return;
  }
  glade_xml_signal_autoconnect(xml);
  gpPrintDialog = glade_xml_get_widget(xml, "dialogPrint");

  gpPrintDialogEntryPrintTo        = glade_xml_get_widget(xml, "entryPrintTo");
  gpPrintDialogEntryPrinterCommand = glade_xml_get_widget(xml, "entryPrintCommand");
  gpPrintDialogEntryFile           = glade_xml_get_widget(xml, "entryPrintFile");
  gpPrintDialogEntryOrientation    = glade_xml_get_widget(xml, "entryPrintOrientation");
  gpPrintDialogEntryPaperSize      = glade_xml_get_widget(xml, "entryPrintPaperSize");
  gpPrintDialogEntryFileBrowse     = glade_xml_get_widget(xml, "entryPrintFileBrowse");
  gpPrintDialogCheckButtonCaption  = glade_xml_get_widget(xml, "checkButtonPrintCaption");
  gpPrintDialogCheckButtonTitle    = glade_xml_get_widget(xml, "checkButtonPrintTitle");
  gpPrintDialogCopyrightNote       = glade_xml_get_widget(xml, "entryPrintCopyrightNote");
  gpPrintDialogSpinButtonColumns   = glade_xml_get_widget(xml, "spinButtonPrintColumns");
  gpPrintDialogSpinButtonHorizontalOffset    = glade_xml_get_widget(xml, "spinButtonPrintHorizontalOffset");
  gpPrintDialogSpinButtonLabelWidth          = glade_xml_get_widget(xml, "spinButtonPrintLabelWitdth");
  gpPrintDialogSpinButtonColumnSpacing       = glade_xml_get_widget(xml, "spinButtonPrintColumnSpacing");
  gpPrintDialogSpinButtonVerticalOffset      = glade_xml_get_widget(xml, "spinButtonPrintVerticalOffset");
  gpPrintDialogSpinButtonLabelHeight         = glade_xml_get_widget(xml, "spinButtonPrintLabelHeight");
  gpPrintDialogSpinButtonRowSpacing          = glade_xml_get_widget(xml, "spinButtonPrintRowSpacing");

  gtk_entry_set_text(GTK_ENTRY(gpPrintDialogEntryPrintTo), gpui8PrintTo);
  if (strcmp(gpui8PrintTo, "Printer") == 0) {
    gtk_widget_set_sensitive(gpPrintDialogEntryFileBrowse, 0);
    gtk_widget_set_sensitive(gpPrintDialogEntryPrinterCommand, 1);
  }
  else if (strcmp(gpui8PrintTo, "File") == 0) {
    gtk_widget_set_sensitive(gpPrintDialogEntryFileBrowse, 1);
    gtk_widget_set_sensitive(gpPrintDialogEntryPrinterCommand, 0);
  }

  gtk_entry_set_text(GTK_ENTRY(gpPrintDialogEntryPrinterCommand), gpui8PrinterCommand);
  gtk_entry_set_text(GTK_ENTRY(gpPrintDialogEntryOrientation), grPrintLayout.ui8Orientation ? "Landscape" : "Portrait");
  gtk_entry_set_text(GTK_ENTRY(gpPrintDialogEntryPaperSize), gui8PaperSize);
  gtk_entry_set_text(GTK_ENTRY(gpPrintDialogCopyrightNote), gpui8CopyrightNote);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(gpPrintDialogCheckButtonTitle)), grPrintLayout.ui8PrintTitle);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(gpPrintDialogCheckButtonCaption)), grPrintLayout.ui8PrintCaption);

  sprintf(pui8Buffer, "%d", grPrintLayout.ui8Columns);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonColumns)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fHorizontalOffset);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonHorizontalOffset)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fVerticalOffset);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonVerticalOffset)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fLabelWidth);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonLabelWidth)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fLabelHeight);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonLabelHeight)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fColumnSpacing);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonColumnSpacing)), pui8Buffer);
  sprintf(pui8Buffer, "%f", grPrintLayout.fRowSpacing);
  gtk_entry_set_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonRowSpacing)), pui8Buffer);

  gtk_object_unref(GTK_OBJECT(xml));
}

void vPrintButtonPrint_Released(void) {
  struct stat  sBuf;
  uint16       ui16NoteBookPageIndex;
  sint16       si16ReturnValue;
  pid_t        pid;


  grPrintLayout.ui8Orientation    = strcmp(gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryOrientation)), "Portrait");
  grPrintLayout.ui8Columns        = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonColumns))));
  grPrintLayout.ui8PrintTitle     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(gpPrintDialogCheckButtonTitle)));
  grPrintLayout.ui8PrintCaption   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(gpPrintDialogCheckButtonCaption)));

  grPrintLayout.ui8Columns        = atoi(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonColumns))));
  grPrintLayout.fHorizontalOffset = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonHorizontalOffset))));
  grPrintLayout.fVerticalOffset   = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonVerticalOffset))));
  grPrintLayout.fLabelWidth       = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonLabelWidth))));
  grPrintLayout.fLabelHeight      = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonLabelHeight))));
  grPrintLayout.fColumnSpacing    = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonColumnSpacing))));
  grPrintLayout.fRowSpacing       = atof(gtk_entry_get_text(GTK_ENTRY(GTK_SPIN_BUTTON(gpPrintDialogSpinButtonRowSpacing))));

  strcpy(gpui8CopyrightNote, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogCopyrightNote)));
  strcpy(gpui8PrintTo, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryPrintTo)));
  strcpy(gpui8PrinterCommand, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryPrinterCommand)));
  strcpy(gui8PaperSize, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryPaperSize)));
  ui16NoteBookPageIndex = gtk_notebook_get_current_page(GTK_NOTEBOOK(gpMainWindowNoteBook));
  vNoteBook_GetRoll(ui16NoteBookPageIndex, &grRoll);

  if (strcmp(gpui8PrintTo, "Printer") == 0) {
    si16ReturnValue = si16PrintSlide("/tmp/top.secret.ps",
                                     gpui8CopyrightNote,
                                     gui8PaperSize,
                                     &grRoll,
                                     &grPrintLayout);
    if (si16ReturnValue < 0) {
      gnome_app_error(GNOME_APP(gpMainWindow), "Failed to print slide.");
      return;
    }

    pid = fork();

    if (pid < 0) {
      gnome_app_error(GNOME_APP(gpMainWindow), "Failed to launch printer command.");
      return;
    }

    if (pid == 0) {
      char *args[3];
      args[0]=gpui8PrinterCommand;
      args[1]="/tmp/top.secret.ps";
      args[2]=NULL;
      execvp(args[0], args);
      exit(0);
    }
  }

  if (strcmp(gpui8PrintTo, "File") == 0) {
    /* check if the file exist */
    strcpy(gpui8PaperSize, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryPaperSize)));
    strcpy(gpui8FileName, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryFile)));
    if (stat(gpui8FileName, &sBuf) == 0) {
      gnome_app_question(GNOME_APP(gpMainWindow),
                         "This file already exist.\nOverwrite it?",
                         vPrintSlideWithoutQuestions,
                         NULL);
    }
    else
      vPrintSlideWithoutQuestions(0, NULL);
  }
  gtk_widget_destroy(GTK_WIDGET(gpPrintDialog));
  gpPrintDialog = NULL;
}


void vPrintButtonCancel_Released(void) {
  gtk_widget_destroy(GTK_WIDGET(gpPrintDialog));
  gpPrintDialog = NULL;
}

void vPrintEntryPrintTo_Changed(){
  strcpy(gpui8PrintTo, gtk_entry_get_text(GTK_ENTRY(gpPrintDialogEntryPrintTo)));

  if (strcmp(gpui8PrintTo, "Printer") == 0) {
    gtk_widget_set_sensitive(gpPrintDialogEntryFileBrowse, 0);
    gtk_widget_set_sensitive(gpPrintDialogEntryPrinterCommand, 1);
  }
  else if (strcmp(gpui8PrintTo, "File") == 0) {
    gtk_widget_set_sensitive(gpPrintDialogEntryFileBrowse, 1);
    gtk_widget_set_sensitive(gpPrintDialogEntryPrinterCommand, 0);
  }
}

void vPrintLoadDefaults(void) {
  strcpy(gpui8PrintTo,        "Printer");
  strcpy(gpui8PrinterCommand, "lpr");
  strcpy(gui8PaperSize,       "A4        (210 x 297 mm)");
  strcpy(gpui8CopyrightNote,  "type your name here");

  grPrintLayout.ui8Orientation    = 0;
  grPrintLayout.ui8Columns        = 4;
  grPrintLayout.fHorizontalOffset = 20;
  grPrintLayout.fVerticalOffset   = 13.5;
  grPrintLayout.fLabelWidth       = 40;
  grPrintLayout.fLabelHeight      = 8.60;
  grPrintLayout.fColumnSpacing    = 5.72;
  grPrintLayout.fRowSpacing       = 0;
}

void vPrintSet(uint8 *pui8PrintTo,
               uint8 *pui8PrinterCommand,
               uint8 *pui8PaperSize,
               uint8 *pui8CopyrightNote,
               tPrintLayout *prPrintLayout) {
  strcpy(gpui8PrintTo,        pui8PrintTo);
  strcpy(gpui8PrinterCommand, pui8PrinterCommand);
  strcpy(gui8PaperSize,       pui8PaperSize);
  strcpy(gpui8CopyrightNote,  pui8CopyrightNote);

  grPrintLayout.ui8Orientation    = prPrintLayout->ui8Orientation;
  grPrintLayout.ui8PrintCaption   = prPrintLayout->ui8PrintCaption;
  grPrintLayout.ui8PrintTitle     = prPrintLayout->ui8PrintTitle;
  grPrintLayout.ui8Columns        = prPrintLayout->ui8Columns;
  grPrintLayout.fHorizontalOffset = prPrintLayout->fHorizontalOffset;
  grPrintLayout.fVerticalOffset   = prPrintLayout->fVerticalOffset;
  grPrintLayout.fLabelWidth       = prPrintLayout->fLabelWidth;
  grPrintLayout.fLabelHeight      = prPrintLayout->fLabelHeight;
  grPrintLayout.fColumnSpacing    = prPrintLayout->fColumnSpacing;
  grPrintLayout.fRowSpacing       = prPrintLayout->fRowSpacing;
}

void vPrintGet(uint8 **ppui8PrintTo,
               uint8 **ppui8PrinterCommand,
               uint8 **ppui8PaperSize,
               uint8 **ppui8CopyrightNote,
               tPrintLayout *prPrintLayout) {

  *ppui8PrintTo        = gpui8PrintTo;
  *ppui8PrinterCommand = gpui8PrinterCommand;
  *ppui8PaperSize      = gui8PaperSize;
  *ppui8CopyrightNote  = gpui8CopyrightNote;

  prPrintLayout->ui8Orientation    = grPrintLayout.ui8Orientation;
  prPrintLayout->ui8PrintCaption   = grPrintLayout.ui8PrintCaption;
  prPrintLayout->ui8PrintTitle     = grPrintLayout.ui8PrintTitle;
  prPrintLayout->ui8Columns        = grPrintLayout.ui8Columns;
  prPrintLayout->fHorizontalOffset = grPrintLayout.fHorizontalOffset;
  prPrintLayout->fVerticalOffset   = grPrintLayout.fVerticalOffset;
  prPrintLayout->fLabelWidth       = grPrintLayout.fLabelWidth;
  prPrintLayout->fLabelHeight      = grPrintLayout.fLabelHeight;
  prPrintLayout->fColumnSpacing    = grPrintLayout.fColumnSpacing;
  prPrintLayout->fRowSpacing       = grPrintLayout.fRowSpacing;
}

void vPrintButtonHelp_Released() {
  sint16 si16ReturnValue;
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "Starting Browser...");
  si16ReturnValue = si16UtilLaunchHelpBrowser("help/print/index.html");
  gnome_appbar_set_status(GNOME_APPBAR(gpMainApplicationBar), "");
  if (si16ReturnValue < 0)
    gnome_app_error(GNOME_APP(gpMainWindow), "Failed to start the help browser.");
}
