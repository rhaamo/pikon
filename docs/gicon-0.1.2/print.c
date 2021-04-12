/*

  print.c

  the post script codes found in this file are
  Copyright (C) 1997 Liang-Wu Cai (cai@mit.edu)

  the rest is
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "file.h"
#include "nkn.h"
#include "print.h"

#define FONT_SIZE              6
#define LINE_SPACING           (FONT_SIZE + 1)
#define FRAME_NUM_FONT_SIZE    (FONT_SIZE * 2)

#define POSITION_FIRST_ROW_X(x) (x)
#define POSITION_FIRST_ROW_Y(y) (y - LINE_SPACING)

#define POSITION_SECOND_ROW_X(x) (x)
#define POSITION_SECOND_ROW_Y(y) (y - 2 * LINE_SPACING)

#define POSITION_THIRD_ROW_X(x) (x)
#define POSITION_THIRD_ROW_Y(y) (y - 3 * LINE_SPACING)

#define POSITION_FOURTH_ROW_X(x) (x)
#define POSITION_FOURTH_ROW_Y(y) (y - 4 * LINE_SPACING)

#define POSITION_FRAMENUM_X(x) (x)
#define POSITION_FRAMENUM_Y(y) (y - FRAME_NUM_FONT_SIZE)

extern tNKN_DataArray rpNKN_ProgramMode[];
extern tNKN_DataArray rpNKN_LightMetering[];
extern tNKN_DataArray rpNKN_FlashMode[];

tPaperSize rPaperSize[] = {
  {"Letter    (216 x 279 mm)",   612,  792},
  {"Legal     (216 x 356 mm)",   612,  1008},
  {"Statement (140 x 216 mm)",   396,  612},
  {"Tabloid   (279 x 432 mm)",   792,  1224},
  {"Ledger    (432 x 279 mm)",   1224, 792},
  {"Folio     (216 x 330 mm)",   612,  936},
  {"Quarto    (215 x 275 mm)",   610,  780},
  {"7x9       (178 x 229 mm)",   504,  648},
  {"9x11      (229 x 279 mm)",   648,  792},
  {"9x12      (229 x 305 mm)",   648,  864},
  {"10x13     (254 x 330 mm)",   720,  936},
  {"10x14     (254 x 356 mm)",   720,  1008},
  {"Executive (190 x 254 mm)",   540,  720},
  {"A0        (841 x 1189 mm)",  2384, 3370},
  {"A1        (594 x 841 mm)",   1684, 2384},
  {"A2        (420 x 594 mm)",   1191, 1684},
  {"A3        (297 x 420 mm)",   842,  1191},
  {"A4        (210 x 297 mm)",   595,  842},
  {"A5        (148 x 210 mm)",   420,  595},
  {"A6        (105 x 148 mm)",   297,  420},
  {"A7        (74 x 105 mm)",    210,  297},
  {"A8        (52 x 74 mm)",     148,  210},
  {"A9        (37 x 52 mm)",     105,  148},
  {"A10       (26 x 37 mm)",     73,   105},
  {"B0        (1030 x 1456 mm)", 2920, 4127},
  {"B1        (728 x 1030 mm)",  2064, 2920},
  {"B2        (515 x 728 mm)",   1460, 2064},
  {"B3        (364 x 515 mm)",   1032, 1460},
  {"B4        (257 x 364 mm)",   729,  1032},
  {"B5        (182 x 257 mm)",   516,  729},
  {"B6        (128 x 182 mm)",   363,  516},
  {"B7        (91 x 128 mm)",    258,  363},
  {"B8        (64 x 91 mm)",     181,  258},
  {"B9        (45 x 64 mm)",     127,  181},
  {"B10       (32 x 45 mm)",     91,   127},
  {"ISOB0     (1000 x 141 mm)",  2835, 4008},
  {"ISOB1     (707 x 1000 mm)",  2004, 2835},
  {"ISOB2     (500 x 707 mm)",   1417, 2004},
  {"ISOB3     (353 x 500 mm)",   1001, 1417},
  {"ISOB4     (250 x 353 mm)",   709,  1001},
  {"ISOB5     (176 x 250 mm)",   499,  709},
  {"ISOB6     (125 x 176 mm)",   354,  499},
  {"ISOB7     (88 x 125 mm)",    249,  354},
  {"ISOB8     (62 x 88 mm)",     176,  249},
  {"ISOB9     (44 x 62 mm)",     125,  176},
  {"ISOB10    (31 x 44 mm)",     88,   125},
  {"C0        (917 x 1297 mm)",  2599, 3676},
  {"C1        (648 x 917 mm)",   1837, 2599},
  {"C2        (458 x 648 mm)",   1298, 1837},
  {"C3        (324 x 457 mm)",   918,  1296},
  {"C4        (229 x 324 mm)",   649,  918},
  {"C5        (162 x 229 mm)",   459,  649},
  {"C6        (114 x 162 mm)",   323,  459},
  {"C7        (81 x 114 mm)",    230,  323},
  {NULL,                         0,    0}
};

void vProlog   (FILE *, uint8 *, uint16, uint16);
void vFrameInfo(FILE *, tRoll *, uint8*, uint8, uint8, uint8, uint16, uint16, uint16);
void vPostlog  (FILE *);

sint16 si16PrintSlide(uint8 *pui8FileName, uint8 *pui8CopyrightNote, uint8 *pui8PaperName, tRoll *prRoll, tPrintLayout *prPrintLayout) {
  FILE   *pFile;
  uint8  ui8FrameIndex;
  uint8  ui8ColumnIndex;
  uint16 ui16Data;
  uint16 ui16Index;
  uint16 ui16PaperWidth;
  uint16 ui16PaperHeight;
  uint16 ui16PosX;
  uint16 ui16PosY;

  if ((pFile = fopen(pui8FileName, "w" )) == NULL) {
    printf(" Cannot open output file %s.\n", pui8FileName);
    return FAIL;
  }

  /* get paper size */
  ui16PaperWidth = 595;  /* A4 fallback */
  ui16PaperHeight = 842;
  for (ui16Index = 0; rPaperSize[ui16Index].pui8PaperName != NULL; ui16Index++)
    if (strcmp(rPaperSize[ui16Index].pui8PaperName, pui8PaperName) == 0) {
      ui16PaperWidth  = rPaperSize[ui16Index].ui16PaperWidth;
      ui16PaperHeight = rPaperSize[ui16Index].ui16PaperHeight;
      break;
    }

  /* check orientation */
  if (prPrintLayout->ui8Orientation) {
    ui16Data = ui16PaperWidth;
    ui16PaperWidth = ui16PaperHeight;
    ui16PaperHeight = ui16Data;
  }

  vProlog(pFile, prRoll->pui8RollTitle, ui16PaperWidth,  ui16PaperHeight);

  ui8ColumnIndex = 0;
  for (ui8FrameIndex = 0; ui8FrameIndex < prRoll->ui8FrameCounter; ui8FrameIndex++) {
    /* label position */
    ui16PosX = (prPrintLayout->fLabelWidth + prPrintLayout->fColumnSpacing) * ui8ColumnIndex;
    ui16PosY = (prPrintLayout->fLabelHeight + prPrintLayout->fRowSpacing) * (ui8FrameIndex/prPrintLayout->ui8Columns);

    vFrameInfo(pFile,
               prRoll,
               pui8CopyrightNote,
               ui8FrameIndex,
               prPrintLayout->ui8PrintCaption,
               prPrintLayout->ui8PrintTitle,
               (uint16)rint((prPrintLayout->fHorizontalOffset + ui16PosX)*72/25.4),
               (uint16)rint(ui16PaperHeight - (prPrintLayout->fVerticalOffset + ui16PosY)*72/25.4),
               (uint16)rint((prPrintLayout->fLabelWidth)*72/25.4));

    ui8ColumnIndex++;
    if (ui8ColumnIndex >= prPrintLayout->ui8Columns)
      ui8ColumnIndex = 0;
  }

  vPostlog(pFile);
  fclose(pFile);
  return OK;
}

void vFrameInfo(FILE *pFile, tRoll *prRoll, uint8* pui8CopyrightNote, uint8 ui8FrameIndex,
                uint8 ui8PrintCaption, uint8 ui8PrintTitle, uint16 ui16PosX, uint16 ui16PosY, uint16 ui16Width) {
  uint16 ui16Data;
  /* frame number */
  fprintf(pFile, "%d RF %d %d M (%d)RS\n", FRAME_NUM_FONT_SIZE, POSITION_FRAMENUM_X(ui16PosX + ui16Width), POSITION_FRAMENUM_Y(ui16PosY), ui8FrameIndex + 1);
  /* copyright note, date */
  fprintf(pFile, "%d RF %d %d M (%s, %s)Co\n", FONT_SIZE, POSITION_FIRST_ROW_X(ui16PosX), POSITION_FIRST_ROW_Y(ui16PosY), pui8CopyrightNote, prRoll->pui8TimeStamp);
  /* frame data: program mode, metering mode ... */
  fprintf(pFile, "%d RF %d %d M ", FONT_SIZE, POSITION_SECOND_ROW_X(ui16PosX), POSITION_SECOND_ROW_Y(ui16PosY));

    /* program mode */
  vNKN_StringToData(rpNKN_ProgramMode, prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_MODE], &ui16Data);
  switch (ui16Data) {
  default:
  case 0x00: fprintf(pFile, "Ma "); break;
  case 0x01: fprintf(pFile, "Ap "); break;
  case 0x02: fprintf(pFile, "Sh "); break;
  case 0x03: fprintf(pFile, "Pr "); break;
  case 0x08: fprintf(pFile, "Po "); break;
  case 0x09: fprintf(pFile, "rE "); break;
  case 0x0A: fprintf(pFile, "HF "); break;
  case 0x0B: fprintf(pFile, "LA "); break;
  case 0x0C: fprintf(pFile, "SL "); break;
  case 0x0D: fprintf(pFile, "SP "); break;
  case 0x0E: fprintf(pFile, "CU "); break;
  case 0x0F: fprintf(pFile, "CU "); break;
  }

  /* metering mode */
  vNKN_StringToData(rpNKN_LightMetering, prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_METERING], &ui16Data);
  switch  (ui16Data) {
  case 0x00: fprintf(pFile, "Ce "); break;
  case 0x01: fprintf(pFile, "Sp "); break;
  default:
  case 0x02: fprintf(pFile, "Mt "); break;
  }

  /* exposure */
  fprintf(pFile, "(%s\" " , prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_EXPOSURE]);

  /* aperture */
  fprintf(pFile, "f/%s )s ", prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_APERTURE]);

  /* exposure compensation */
  if ((strcmp(prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_EXP_COMP], "0.0") != 0) &&
      (strcmp(prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_EXP_COMP], "") != 0))
    fprintf(pFile, "Ec (%s )s ", prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_EXP_COMP]);

    /* flash mode */
  vNKN_StringToData(rpNKN_FlashMode, prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_FLASH_MODE], &ui16Data);
  switch  (ui16Data) {
  default:
  case 0x00: fprintf(pFile, "Ns "); break;
  case 0x01: fprintf(pFile, "Ss "); break;
  case 0x02: fprintf(pFile, "Rs "); break;
  case 0x03:
  }

  /* flash compensation */
  if ((strcmp(prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_FLASH_COMP], "0.0") != 0) &&
      (strcmp(prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_FLASH_COMP], "") != 0))
    fprintf(pFile, "Fc (%s )s", prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_FLASH_COMP]);

  fprintf(pFile, "(ISO%s %smm)s\n", prRoll->pui8FilmSpeed, prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_FOCAL_LENGHT]);

  /* rollt title, film speed, focal lenght*/
  if (ui8PrintTitle)
    fprintf(pFile, "%d RF %d %d M (%s)s\n", FONT_SIZE, POSITION_THIRD_ROW_X(ui16PosX), POSITION_THIRD_ROW_Y(ui16PosY), prRoll->pui8RollTitle);
  else
    fprintf(pFile, "%d RF %d %d M (%s)s\n", FONT_SIZE, POSITION_THIRD_ROW_X(ui16PosX), POSITION_THIRD_ROW_Y(ui16PosY), "");
  /* caption */
  if (ui8PrintCaption)
    fprintf(pFile, "%d RF %d %d M (%s)s\n", FONT_SIZE, POSITION_FOURTH_ROW_X(ui16PosX), POSITION_FOURTH_ROW_Y(ui16PosY), prRoll->table[ui8FrameIndex].list[ENTRY_INDEX_CAPTION]);
  else
    fprintf(pFile, "%d RF %d %d M (%s)s\n", FONT_SIZE, POSITION_FOURTH_ROW_X(ui16PosX), POSITION_FOURTH_ROW_Y(ui16PosY), "");
}

void vProlog(FILE *pFile, uint8 *pui8Title, uint16 ui16Width, uint16 ui16Height) {
  fprintf(pFile ,"%c! PS-Adobe-3.0 EPSF-2.0\n%c%cCreator: Liang-Wu Cai (cai@mit.edu)\n",37,37,37);
  fprintf(pFile ,"%c%cTitle:%s\n%c%cBoundingBox: 0 0 %d %d\n", 37,37, pui8Title, 37,37, ui16Width, ui16Height);
  fprintf(pFile ,"%c%cPages: 1\n%c%cEndComments\n%c%cBeginProlog\n", 37, 37, 37, 37, 37, 37);
  fprintf(pFile ,"\n/USERdict 250 dict def USERdict begin\n");
  fprintf(pFile ,"/M{moveto}def /L{lineto}def /RM{rmoveto}def /RL{rlineto}def /l{sz\n");
  fprintf(pFile ,"mul}def /S{stroke}def /D{exch def}def /F{fill}def /N{newpath}def\n");
  fprintf(pFile ,"/A{l add}def /E{x 1.1 A y M}def /s{show} def /c{currentpoint}def\n");
  fprintf(pFile ,"/SA{gsave}def /RE{grestore}def /T{translate}def /CL{closepath}def\n");
  fprintf(pFile ,"/LW{setlinewidth}def /G{setgray}def /Fn{findfont[sz .92 mul 0 0 sz\n");
  fprintf(pFile,"0 0]makefont setfont}def /RF{/sz D/Times-Roman Fn}def /IF{/sz\n");
  fprintf(pFile,"D/Times-Italic Fn}def /SF{/Courier-Oblique findfont [sz .7 mul 0 0\n");
  fprintf(pFile ,"sz 0 0]makefont setfont} def /BF{/Courier-BoldOblique findfont [sz\n");
  fprintf(pFile ,"1.05 mul 0 0 sz 0 0]makefont setfont} def /RS{/txt D txt\n");
  fprintf(pFile ,"stringwidth pop neg 0 RM txt s}def /B{c /y D /x D N x .1 A y M .8\n");
  fprintf(pFile ,"l 0 RL x .9 A y .1 A .1 l 270 360 arc x .9 A y .7 A .1 l 0 90 arc\n");
  fprintf(pFile ,"x .1 A y .7 A .1 l 90 180 arc x .1 A y .1 A .1 l 180 270 arc\n");
  fprintf(pFile ,"CL}def /FB{c /y D /x D /x x .1 A def N x .1 A y .1 l add .1 l 180\n");
  fprintf(pFile ,"270 arc .8 l 0 RL x .9 A y .1 A .1 l 270 360 arc x .9 A y .7 A .1\n");
  fprintf(pFile ,"l 0 90 arc x .1 A y .7 A .1 l 90 180 arc 0 LW S .07 l LW x .07 A y\n");
  fprintf(pFile ,".65 A M -.1 l -.25 l RL .1 l 0 RL -.1 l -.25 l RL S sz .8 mul BF\n");
  fprintf(pFile ,"}def /Mt{B F 1 G .1 l LW x y .4 A M 1 l 0 RL x .5 A y M 0 .8 l RL\n");
  fprintf(pFile ,"S x .5 A y .4 A .2 l 0 360 arc F 0 G x .5 A y .4 A .12 l 0 360 arc\n");
  fprintf(pFile ,"F E}def /Sp{B 0 LW S x .5 A y .4 A .135 l 0 360 arc F E}def /Ce{B\n");
  fprintf(pFile ,"0 LW S x .5 A y .4 A .12 l 0 360 arc F .04 l LW x .5 A y .4 A .22\n");
  fprintf(pFile ,"l -80 80 arc S x .5 A y .4 A .22 l 100 260 arc S E}def /Pr{SA B 0\n");
  fprintf(pFile ,"LW S sz .8 mul BF x .13 A y .12 A M (P) s RE E}def /Ap{SA B 0 LW S\n");
  fprintf(pFile ,"sz .8 mul BF x .16 A y .12 A M (A) s RE E}def /Ma{SA B 0 LW S sz\n");
  fprintf(pFile ,".8 mul BF x .13 A y .12 A M (M) s RE E}def /Sh{SA B 0 LW S sz .8\n");
  fprintf(pFile ,"mul BF x .13 A y .12 A M (S) s RE E}def /rE{SA B 0 LW S sz .7 mul\n");
  fprintf(pFile ,"SF x .05 A y .13 A M (rE) s RE E}def /LA{SA B 0 LW S sz .7 mul SF\n");
  fprintf(pFile ,"x .05 A y .12 A M (LA) s RE E}def /SP{SA B 0 LW S sz .7 mul SF x\n");
  fprintf(pFile ,".05 A y .12 A M (SP) s RE E}def /Po{SA B 0 LW S sz .7 mul SF x .05\n");
  fprintf(pFile ,"A y .12 A M (Po) s RE E}def /SL{SA B 0 LW S sz .7 mul SF x .05 A y\n");
  fprintf(pFile ,".12 A M (SL) s RE E}def /CU{SA B 0 LW S sz .7 mul SF x .05 A y .12\n");
  fprintf(pFile ,"A M (CU) s RE E}def /HF{SA B 0 LW S sz .7 mul SF x .05 A y .12 A M\n");
  fprintf(pFile ,"(HF) s RE E}def /Ec{c /y D /x D N x y .05 A M .7 l .7 l RL -.7 l 0\n");
  fprintf(pFile ,"RL CL F x .1 A y M .7 l 0 RL 0 .7 l RL CL F .08 l LW 1 G x .05 A y\n");
  fprintf(pFile ,".55 A M .34 l 0 RL S x .23 A y .37 A M 0 .34 l RL S x .4 A y .2 A\n");
  fprintf(pFile ,"M .34 l 0 RL S 0 G x .9 A y M}def /Fc{SA FB /ox x def /oy y def x\n");
  fprintf(pFile ,".2 A y .08 A T .85 .85 scale 0 0 M Ec RE /x ox def /y oy def E}def\n");
  fprintf(pFile ,"/Ns{SA FB x .15 A y .12 A M (N) s RE E}def /Rs{SA FB x .15 A y .12\n");
  fprintf(pFile ,"A M (R) s RE E}def /Ss{SA FB x .15 A y .12 A M (S) s RE E}def\n");
  fprintf(pFile ,"/Co{/txt D txt stringwidth 0 RM c /y D /x D N 0 LW\n");
  fprintf(pFile ,"x .4 A y .4 A .45 l 0 360 arc S x y M .1 l .1 l RM SA .9 l RF (C)\n");
  fprintf(pFile ,"s RE x 1.1 A y M txt s}def\n%c%cEndProlog\n\n%c%cPage: 1\n",37,37,37,37);
}

void vPostlog(FILE *pFile) {
  fprintf(pFile, "showpage\n%c%cTrailer\nend\n%c%cEOF",37,37,37,37);
}
