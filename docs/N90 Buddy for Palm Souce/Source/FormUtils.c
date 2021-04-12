/***********************************************************************
 *
 *	Copyright © 1996 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			Nikon N90S Buddy Application
 *
 * FILE:				FormUtils.c
 *
 * DESCRIPTION:	Utility routines for acting on the current form
 *
 * RELEASED AS OPEN SOURCE 13 May 2004
 *	  
 * REVISION HISTORY:
 * 	 3/20/97	ksh		Initial version
 *
 **********************************************************************/

#include <PalmOS.h>				// all the system toolbox headers
#include "FormUtils.h"

const UInt8 kNumericFilter[] = 
	{	0x00, 0xF0, 0x00, 0x3F, 0x00, 0x00, 0xFF, 0xF0,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

void * GetObjectPtr(UInt16 objectID:__D0)
{
	FormPtr frm;
	
	frm = FrmGetActiveForm();
	return (FrmGetObjectPtr(frm, GetObjectIndex(objectID)));

}

UInt16 GetObjectIndex(UInt16 objectID:__D0)
{
	FormPtr	frm;
	
	frm = FrmGetActiveForm();
	return(FrmGetObjectIndex(frm, objectID));
}

void HideObject(UInt16 objectID:__D0)
{
	FormPtr	frm;
	
	frm = FrmGetActiveForm();
	FrmHideObject(frm, GetObjectIndex(objectID));
}

void ShowObject(UInt16 objectID:__D0)
{
	FormPtr	frm;
	
	frm = FrmGetActiveForm();
	FrmShowObject(frm, GetObjectIndex(objectID));
}

void CopyLabel(UInt16 objectID:__D0, Char  * label:__A0)
{
	FormPtr	frm;
//	FormLabelType	*lptr;
//	FontID			fnt;
	
	frm = FrmGetActiveForm();
	
	FrmHideObject(frm, GetObjectIndex(objectID));

#if 0
	if (FrmVisible(frm))
	{
		lptr = GetObjectPtr(objectID);
		fnt = FntGetFont();
		FntSetFont(lptr->fontID);
		WinEraseChars(lptr->text, StrLen(lptr->text), lptr->pos.x, lptr->pos.y);
		FntSetFont(fnt);
	}
#endif
	
	FrmCopyLabel(frm, objectID, label);
	
	FrmShowObject(frm, GetObjectIndex(objectID));
}

UInt16 GetControlGroupSelection(UInt16 group:__D0)
{
	FormPtr	frm;
	UInt16		item;
	
	frm = FrmGetActiveForm();
	item = FrmGetControlGroupSelection(frm, group);
	if (item)
		item = FrmGetObjectId(frm, item);
		
	return(item);
}

void SetControlGroupSelection(UInt16 group:__D0, UInt16 item:__D1)
{
	FormPtr	frm;
	
	frm = FrmGetActiveForm();
	FrmSetControlGroupSelection(frm, group, item);
}

void GetObjectRect(UInt16 objectID:__D0, RectanglePtr r:__A0)
{
	UInt16	index;
	
	index = GetObjectIndex(objectID);
	FrmGetObjectBounds(FrmGetActiveForm(), index, r);
	
	// OS 1.x doesn't handle bitmaps correctly
	if ((r->extent.x == 0) && (FrmGetObjectType(FrmGetActiveForm(), index) == frmBitmapObj))
	{
		FormBitmapType	*fbmp;
		BitmapType		*bitmap;
		MemHandle		bitmaph;
		
		fbmp = (FormBitmapType *) FrmGetObjectPtr(FrmGetActiveForm(), index);
		r->topLeft.x = fbmp->pos.x;
		r->topLeft.y = fbmp->pos.y;
		if ((bitmaph = DmGetResource(bitmapRsc, fbmp->rscID)) != NULL)
		{
			bitmap = (BitmapType *) MemHandleLock(bitmaph);
			r->extent.x = bitmap->width;
			r->extent.y = bitmap->height;
			MemHandleUnlock(bitmaph);
			DmReleaseResource(bitmaph);
		}
	}
}

void ClearLabels(UInt16 fromId:__D0, UInt16 toId:__D1)
{
	FormPtr	frm;
	UInt16		items, i, objectId;
	
	frm = FrmGetActiveForm();
	items = FrmGetNumberOfObjects(frm);
	
	for (i = 0; i < items; i++)
	{
		if (FrmGetObjectType(frm, i) == frmLabelObj)
		{
			objectId = FrmGetObjectId(frm, i);
			
			if ((fromId <= objectId) && (objectId <= toId))
				CopyLabel(objectId, "");
		}
	}
	
}

FieldPtr GetFocusObjectPtr(void)
{
	FormPtr frm;
	UInt16 focus;
	FormObjectKind objType;
	
	frm = FrmGetActiveForm ();
	focus = FrmGetFocus (frm);
	if (focus == noFocus)
		return (NULL);
	
	objType = FrmGetObjectType (frm, focus);

	if (objType == frmFieldObj)
		return (FrmGetObjectPtr(frm, focus));
	else if (objType == frmTableObj)
		return(TblGetCurrentField(FrmGetObjectPtr(frm, focus)));
		
	return (NULL);
}


Char  * GetNumberString(Char  * str:__A0, UInt16 num:__D0, UInt16 digits:__D1)
{
	UInt16	len;
	
	StrIToA(str,num);
	len = StrLen(str);
	
	if (digits == 0)
		digits = len;
		
	MemMove(str + (digits - len), str, len + 1);
	MemSet(str, (digits - len), '0');
	
	return(str);
}

void SetCtlLabel(UInt16 itemID:__D0, Char  * label:__A0)
{
	ControlPtr	ctl;
	Char  *		oldLabel;
	
	ctl = GetObjectPtr(itemID);
	oldLabel = (Char *) CtlGetLabel(ctl);
	StrCopy(oldLabel, label);
	CtlSetLabel(ctl, oldLabel);
}

UInt16 GetCtlLabelNumber(UInt16 itemID:__D0)
{
	ControlPtr	ctl;
	Char  *		label;
	
	ctl = GetObjectPtr(itemID);
	label = (Char *) CtlGetLabel(ctl);
	return (StrAToI(label));
}
void	SetCtlLabelNumber(UInt16 itemID:__D0, UInt16 number:__D1)
{
	ControlPtr	ctl;
	Char  *		label;
	
	ctl = GetObjectPtr(itemID);
	label = (Char *) CtlGetLabel(ctl);
	StrIToA(label, number);
	CtlSetLabel(ctl,label);
}

void SetPopupList(UInt16 listID:__D0, UInt16 triggerID:__D1, Int16 value:__D2)
{
	ListPtr		lst;
	Char  * 		label;
	ControlPtr 	ctl;

	if (value < 0)
		value = 0;
		
	lst = GetObjectPtr(listID);		
	LstSetSelection(lst, value);
	label = LstGetSelectionText(lst, value);
	ctl = GetObjectPtr(triggerID);
	CtlSetLabel(ctl, label);
}

void	SetFieldText(UInt16 fieldID:__D0, Char  * newtext:__A0)
{
	FieldPtr	fld;
	MemHandle	texth;
	Char  *	textp;

	fld = GetObjectPtr(fieldID);
	texth = FldGetTextHandle(fld);
	FldFreeMemory(fld);
	texth = MemHandleNew(StrLen(newtext)+1);
	if (texth)
	{
		textp = MemHandleLock(texth);
		StrCopy(textp, newtext);
		FldSetTextHandle(fld, texth);
		MemHandleUnlock(texth);
		if (FrmVisible(FrmGetActiveForm())) FldDrawField(fld);
	}
}

Char  * GetFieldText(UInt16 fieldID:__D0)
{
	return (FldGetTextPtr(GetObjectPtr(fieldID)));
}

UInt16 GetFieldTextNumber(UInt16 fieldID:__D0)
{
	Char  *	fieldText;
	
	fieldText = GetFieldText(fieldID);
	if (fieldText && *fieldText)
		return(StrAToI(fieldText));
	else
		return(0);
}

Boolean	FilterKey(UInt8 key:__D0, UInt8 *filterMap:__A0)
{
	return (!((filterMap[key >> 3]) & (0xF0 >> (key % 8))));
}

Err GetTemplateString(Char  * dest:__A0, UInt16 rsrcID:__D0)
{
	Err		err = 0;
	MemHandle	h;
	Char  *	templateP;
	
	h = DmGetResource(strRsc, rsrcID);
	if (h == NULL) err = dmErrResourceNotFound;
	if (err) goto ERROR;
	
	templateP = MemHandleLock(h);
	StrCopy(dest, templateP);
	
	MemHandleUnlock(h);
	
ERROR:
	return err;
	
}

void MungeStr(Char flag:__D0, Char  * templateStr:__A0, Char  * insertStr:__A1)
{
	Char  *	insertp;
	UInt16		insertLen;
	UInt16		remLen;
	
	if ((insertp = StrChr(templateStr,flag)) != 0)
	{
		insertLen = StrLen(insertStr);
		remLen = StrLen(insertp) - 1;
		MemMove(insertp + insertLen, insertp + 1, remLen + 1);
		MemMove(insertp, insertStr, insertLen);
	}

}

void EraseObject(UInt16 id:__D0)
{
	RectangleType	r;
	
	GetObjectRect(id, &r);
	WinEraseRectangle(&r, 0);
}

void RectangleInset(RectangleType *r:__A0, Int16 inset:__D0)
{
	r->topLeft.x += inset;
	r->topLeft.y += inset;
	inset *= 2;
	r->extent.x -= inset;
	r->extent.y -= inset;
}

void DrawGroupObject(UInt16 gadget:__D0, UInt16 label:__D1)
{
	RectangleType	r;
	
	GetObjectRect(gadget, &r);
	RectangleInset(&r, 1);
	WinDrawGrayRectangleFrame(simpleFrame, &r);

	GetObjectRect(label, &r);
	r.topLeft.x -= 2;
	r.extent.x += 4;
	WinEraseRectangle(&r,0);	
	FrmShowObject(FrmGetActiveForm(), GetObjectIndex(label));
}


void DoModalDialog(UInt16 formID, FormEventHandlerPtr handler, FormInitProcPtr initproc, UInt16 titleTemplate)
{
	FormPtr		oldForm;
	FormPtr		frm;
	EventType	event;

	oldForm = FrmGetActiveForm();
	frm = FrmInitForm(formID);
	FrmSetActiveForm(frm);
	if (titleTemplate) GetTemplateString((Char *) FrmGetTitle(frm), titleTemplate);
	FrmDrawForm(frm);
	initproc();
	FrmSetEventHandler(frm, handler);
	do
	{
		EvtGetEvent(&event, evtWaitForever);
	
		if (! SysHandleEvent(&event))
			FrmDispatchEvent(&event);
	
	} while (event.eType != DialogCompleteEvent);

	FrmEraseForm(frm);	
	FrmDeleteForm(frm);
	FrmSetActiveForm(oldForm);
	FrmUpdateForm(FrmGetActiveFormID(),frmRedrawUpdateCode);

}

void DrawBitmapInGadget(UInt16 bitmapId, UInt16 gadgetId)
{
	RectangleType	r;
	MemHandle		resourceH;
	BitmapPtr		bitmapP;
	
	GetObjectRect(gadgetId, &r);
	WinEraseRectangle(&r, 0);
	
	resourceH = DmGetResource(bitmapRsc, bitmapId);
	if (resourceH != NULL)
	{
		bitmapP = MemHandleLock(resourceH);
		if (bitmapP)
		{
			WinDrawBitmap (bitmapP, r.topLeft.x, r.topLeft.y);
			MemHandleUnlock(resourceH);
		}
		DmReleaseResource(resourceH);
	}
}

void DrawGadgetString(UInt16 id, Char  * string, Int16 xoffset, 
							Int16 yoffset, FontID font, Align align)
{
	RectangleType	r;
	Char  *			next;
	UInt16				lineheight;
	Int16				xcoord, ycoord;
	Int16				width,length;
	
	GetObjectRect(id, &r);
	FntSetFont(font);
	lineheight = FntLineHeight();
	ycoord = r.topLeft.y + yoffset;
	
	do
	{
		width = r.extent.x;
		length = StrLen(string);
		WrapString(string, &next, &width, &length);

		switch (align)
		{
			case alignLeft:
				xcoord = r.topLeft.x + xoffset;
				break;
			case alignCenter:
				xcoord = r.topLeft.x + ((r.extent.x - width) / 2);
				break;
			case alignRight:
				xcoord = r.topLeft.x + r.extent.x - width - xoffset;
				break;
		}
		WinDrawChars(string, length, xcoord, ycoord);
		
		string = next;
		ycoord += lineheight;

	} while (next != NULL);
}

Err FillTemplateString(Char  * dest, const Char  * values[])
{
	UInt16		i;
	
	i = 0;
	while (StrChr(dest, '#'))
		MungeStr('#', dest, (Char *) values[i++]);
		
	return(0);
}

Err SetTemplateString(Char  * dest, UInt16 rsrcID, const Char  * value, ...)
{
	Err		err = 0;
	
	err = GetTemplateString(dest, rsrcID);
	if (err) goto ERROR;
	
	FillTemplateString(dest, &value);

ERROR:
	return err;
}

Err SetTemplateLabel(UInt16 labelID, UInt16 templateID, const Char  * value, ...)
{
	FormPtr			frm;
	Err				err = 0;
	Char  *			labelStr;
//	FontID			fnt;
	FormLabelType	*lptr;

	frm = FrmGetActiveForm();
	lptr = GetObjectPtr(labelID);
	labelStr = lptr->text;
	
	// Erase the old label
	FrmHideObject(frm, GetObjectIndex(labelID));

#if 0	
	if (FrmVisible(frm))
	{
		fnt = FntGetFont();
		FntSetFont(lptr->fontID);
		WinEraseChars(lptr->text, StrLen(lptr->text), lptr->pos.x, lptr->pos.y);
		FntSetFont(fnt);
	}
#endif

	err = GetTemplateString(labelStr, templateID);
	if (err) goto ERROR;
		
	FillTemplateString(labelStr, &value);
	FrmCopyLabel(frm, labelID, labelStr);

	// show the new label
	FrmShowObject(frm, GetObjectIndex(labelID));

ERROR:
	return err;
}

void WrapString(Char  * string, Char  * *next, Int16 *width, Int16 *length)
{
	Boolean trunc;
	Char  *n, *e;
	char		c;
	
	FntCharsInWidth(string, width, length, &trunc);

	n = string + *length;
	e = n;
	*next = n;
	if (*n == 0)
		goto NONEXT;
		
	while (((c = *n) == ' ') || (c == '\n'))
		n++;
		
	if (*n == 0)
		goto NONEXT;
		
	*next = n;
	
	if (((c = *e) != ' ') && (c != '\n'))
	{
		while (*e != ' ')
			e--;
			
		n = e;
		while (((c = *n) == ' ') || (c == '\n'))
			n++;
		*next = n;
		
		*length = (e - string);
		*width = FntCharsWidth(string, *length);
	}
	
	return;
	
NONEXT:
	*next = NULL;
	
}
