/***********************************************************************
 *
 *	Copyright © 1997 Ken Hancock -- All Rights Reserved
 *
 * PROJECT:			DateTime Library
 *
 * FILE:				DateTime.c
 *
 * DESCRIPTION:		Routines for handilng Date/Time UI controls
 *
 * RELEASED AS OPEN SOURCE 13 May 2004
 *	  
 * REVISION HISTORY:
 * 	6/12/97	ksh		Initial version
 *
 **********************************************************************/

#include <PalmOS.h>

#include "DateTimeLib.h"
#include "FormUtils.h"

UInt32				*sDateTime;
UInt32				sMax;
Char				sDigit;
UInt16				sTipsCount;
DecimalEntryType	sAllowDecimal;

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditTime
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in time
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditTime(UInt32 *seconds)
{
	
	sDateTime = seconds;
	sDigit = 0;
	
	DoModalDialog(TimeForm, TimeFormHandleEvent, TimeFormInit, 0);
}

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditDuration
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in time
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditDuration(UInt32 *seconds)
{
	
	sDateTime = seconds;
	sDigit = 0;
	
	DoModalDialog(TimeForm, TimeFormHandleEvent, TimeFormDurationInit, 0);
}

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditDate
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in date
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditDate(UInt32 *seconds)
{
	DateTimeType	dateTime;
	
	sDateTime = seconds;
	TimSecondsToDateTime(*sDateTime, &dateTime);

	if (SelectDayV10(&(dateTime.month), &(dateTime.day), &(dateTime.year), ""))
		*sDateTime = TimDateTimeToSeconds(&dateTime);
	
	FrmUpdateForm(FrmGetActiveFormID(),frmRedrawUpdateCode);

}


/***********************************************************************
 *
 * FUNCTION:    DateTimeEditShortNumber
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in number
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditShortNumber(UInt16 *number, UInt16 max, UInt16 templateStr)
{
	UInt32	longNumber = *number;
	
	iDateTimeEditDecimalNumber(&longNumber, max, templateStr, kDecimalShortFlag, kDecimalNotAllowed);
	
	*number = longNumber;
}

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditShortDecimalNumber
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in number
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditShortDecimalNumber(UInt16 *number, UInt16 max, UInt16 templateStr, DecimalEntryType entryType)
{
	UInt32 longNumber = *number;
	
	iDateTimeEditDecimalNumber(&longNumber, max, templateStr, kDecimalShortFlag, entryType);
	
	*number = longNumber;
}

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditDecimalNumber
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in number
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditDecimalNumber(UInt32 *number, UInt32 max, UInt16 templateStr, DecimalEntryType entryType)
{
	iDateTimeEditDecimalNumber(number, max, templateStr, kDecimalLongFlag, entryType);
}

/***********************************************************************
 *
 * FUNCTION:    DateTimeEditNumber
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in number
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void DateTimeEditNumber(UInt32 *number, UInt32 max, UInt16 templateStr)
{
	iDateTimeEditDecimalNumber(number, max, templateStr, kDecimalLongFlag, kDecimalNotAllowed);
}


/***********************************************************************
 *
 * FUNCTION:    iDateTimeEditDecimalNumber
 *
 * DESCRIPTION: Brings up a dialog for editing the passed in number
 *
 * PARAMETERS:  pointer to seconds
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void iDateTimeEditDecimalNumber(UInt32 *number, UInt32 max, UInt16 templateStr, UInt32 mask, DecimalEntryType entryType)
{	
	UInt32 longNumber = *number;
	
	sAllowDecimal = kDecimalNotAllowed;
	
	switch (entryType)
	{
		case kDecimalNotAllowed:				// if it's a decimal, convert it
			if (longNumber & mask)
				longNumber = (longNumber & ~mask) / 10;
			break;
		case kDecimalAllowed:
		case kDecimalForce:
			if (!(longNumber & mask))				// if it's not decimal convert it
				longNumber = longNumber * 10;
			else
				longNumber = longNumber & ~mask;	// clear the decimal flag
			sAllowDecimal = kDecimalComplete;
			break;
	}
	
	sDigit = 0;
	sMax = max;
	sDateTime = &longNumber;
	DoModalDialog(NumberForm, NumberFormHandleEvent, NumberFormInit, templateStr);
	longNumber = *sDateTime;

	if (sAllowDecimal == kDecimalComplete)
		longNumber |= mask;
	
	// if we're supposed to return a decimal number, make sure we do
	if ((entryType == kDecimalForce) && (!(longNumber & mask)))
		longNumber = (longNumber * 10) | mask;
		
	*number = longNumber;
}

/***********************************************************************
 *
 * FUNCTION:		DateTimeSetDateCtl
 *
 * DESCRIPTION:		Sets the control label to the specified date
 *
 * PARAMETERS:		UInt16 whichControl
 *					UInt32 dateTime
 *
 * RETURNED:		
 *
 ***********************************************************************/

void DateTimeSetDateCtl(UInt16 whichControl, UInt32 seconds)
{
	DateTimeType			dateTime;
	ControlPtr				ctl;
	Char  *					label;
	SystemPreferencesType	sysPrefs;

	PrefGetPreferences(&sysPrefs);
	
	EraseObject(whichControl);
	ctl = GetObjectPtr(whichControl);
	label = (Char *) CtlGetLabel(ctl);

	if (seconds != 0)
	{
		TimSecondsToDateTime(seconds, &dateTime);
		DateToAscii(dateTime.month, dateTime.day, dateTime.year, sysPrefs.dateFormat, label);
	}
	else
		GetTemplateString(label, NoDateString);
		
	CtlSetLabel(ctl, label);
}

/***********************************************************************
 *
 * FUNCTION:		DateTimeSetTimeCtl
 *
 * DESCRIPTION:		Sets the control label to the specified time
 *
 * PARAMETERS:		UInt16 whichControl
 *					UInt32 dateTime
 *
 * RETURNED:		
 *
 ***********************************************************************/

void DateTimeSetTimeCtl(UInt16 whichControl, UInt32 seconds)
{
	DateTimeType			dateTime;
	ControlPtr				ctl;
	Char  *					label;
	SystemPreferencesType	sysPrefs;

	PrefGetPreferences(&sysPrefs);
	
	EraseObject(whichControl);
	ctl = GetObjectPtr(whichControl);
	label = (Char *) CtlGetLabel(ctl);

	if (seconds != 0)
	{
		TimSecondsToDateTime(seconds, &dateTime);
		TimeToAscii(dateTime.hour, dateTime.minute, sysPrefs.timeFormat, label);
	}
	else
		GetTemplateString(label, NoTimeString);

	CtlSetLabel(ctl, label);
}

/***********************************************************************
 *
 * FUNCTION:		DateTimeSetDurationCtl
 *
 * DESCRIPTION:		Sets the control label to the specified time
 *
 * PARAMETERS:		UInt16 whichControl
 *					UInt32 dateTime
 *
 * RETURNED:		
 *
 ***********************************************************************/

void DateTimeSetDurationCtl(UInt16 whichControl, UInt32 seconds)
{
	DateTimeType			dateTime;
	ControlPtr				ctl;
	Char  *					label;
	EraseObject(whichControl);
	ctl = GetObjectPtr(whichControl);
	label = (Char *) CtlGetLabel(ctl);

	if (seconds != 0)
	{
		TimSecondsToDateTime(seconds, &dateTime);
		TimeToAscii(dateTime.hour, dateTime.minute, tfColon24h, label);
	}
	else
		GetTemplateString(label, NoTimeString);

	CtlSetLabel(ctl, label);
}

/***********************************************************************
 *
 * FUNCTION:    TimeFormHandleEvent
 *
 * DESCRIPTION: Handles events for the Time editing form
 *
 * PARAMETERS:  EventPtr
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

Boolean TimeFormHandleEvent(EventPtr event)
{
	Boolean handled = false;
	UInt8	key = event->data.keyDown.chr;
	
	switch (event->eType)
	{			
		case keyDownEvent:
			TimeFormHandleKey(event);
			handled = true;
			break;
			
		case ctlSelectEvent:
			sDigit = 0;
			if (event->data.ctlSelect.controlID == TimeOKButton)
			{
				TimeFormSave();
				event->eType = DialogCompleteEvent;
				handled = true;
			}
			else if (event->data.ctlSelect.controlID == TimeCancelButton)
			{
				event->eType = DialogCompleteEvent;
				handled = true;
			}
			break;
				
		case ctlRepeatEvent:
			if (event->data.ctlRepeat.controlID == TimeScrollUpRepeating)
				TimeFormScroll(winUp);
			else if (event->data.ctlRepeat.controlID == TimeScrollDownRepeating)
				TimeFormScroll(winDown);
			break;

	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:    TimeFormInit
 *
 * DESCRIPTION: This routine initializes the Time Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
void TimeFormInit (void)
{	
	DateTimeType	dateTime;
	SystemPreferencesType	sysPrefs;

	PrefGetPreferences(&sysPrefs);
	
	TimSecondsToDateTime(*sDateTime, &dateTime);
	
	if ((sysPrefs.timeFormat != tfColonAMPM) && (sysPrefs.timeFormat != tfDotAMPM))
	{
		HideObject(TimeAMPushButton);
		HideObject(TimePMPushButton);
		HideObject(Time24PushButton);
		SetControlGroupSelection(TimeAMPMGroup, Time24PushButton);
	}
	else
	{
		HideObject(Time24PushButton);
		if (dateTime.hour >= 12)
		{
			dateTime.hour -= 12;
			SetControlGroupSelection(TimeAMPMGroup, TimePMPushButton);
		}
		else
		{
			SetControlGroupSelection(TimeAMPMGroup, TimeAMPushButton);
		}
		if (dateTime.hour == 0) dateTime.hour = 12;
	}
	
	if ((sysPrefs.timeFormat >= tfDot) && (sysPrefs.timeFormat <= tfDot24h))
		CopyLabel(TimeColonLabel, ".");
	
	SetCtlLabelNumber(TimeHoursPushButton, dateTime.hour);
	SetCtlLabelNumber(TimeTensPushButton, dateTime.minute / 10);		
	SetCtlLabelNumber(TimeOnesPushButton, dateTime.minute % 10);
	
	SetControlGroupSelection(TimeTimeGroup, TimeHoursPushButton);
}

/***********************************************************************
 *
 * FUNCTION:    TimeFormDurationInit
 *
 * DESCRIPTION: This routine initializes the Time Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
void TimeFormDurationInit (void)
{	
	DateTimeType	dateTime;
	SystemPreferencesType	sysPrefs;

	PrefGetPreferences(&sysPrefs);
	
	TimSecondsToDateTime(*sDateTime, &dateTime);

	HideObject(TimeAMPushButton);
	HideObject(TimePMPushButton);
	HideObject(Time24PushButton);
	SetControlGroupSelection(TimeAMPMGroup, Time24PushButton);
	
	if ((sysPrefs.timeFormat >= tfDot) && (sysPrefs.timeFormat <= tfDot24h))
		CopyLabel(TimeColonLabel, ".");
	
	SetCtlLabelNumber(TimeHoursPushButton, dateTime.hour);
	SetCtlLabelNumber(TimeTensPushButton, dateTime.minute / 10);		
	SetCtlLabelNumber(TimeOnesPushButton, dateTime.minute % 10);
	
	SetControlGroupSelection(TimeTimeGroup, TimeHoursPushButton);
}

/***********************************************************************
 *
 * FUNCTION:    TimeFormSave
 *
 * DESCRIPTION: This routine save the current value into gRollTime
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void TimeFormSave(void)
{	
	DateTimeType	dateTime;
	Boolean			hour24;
	
	TimSecondsToDateTime(*sDateTime, &dateTime);
	
	dateTime.hour 	=	GetCtlLabelNumber(TimeHoursPushButton);
	dateTime.minute	=	GetCtlLabelNumber(TimeTensPushButton) * 10;
	dateTime.minute	+=	GetCtlLabelNumber(TimeOnesPushButton);

	hour24 = (GetControlGroupSelection(TimeAMPMGroup) == Time24PushButton);
	
	if (!hour24)
	{
		if (dateTime.hour == 12)
			dateTime.hour = 0;
			
		if (GetControlGroupSelection(TimeAMPMGroup) == TimePMPushButton)
			dateTime.hour += 12;
	}
		
	*sDateTime = TimDateTimeToSeconds(&dateTime);
}


/***********************************************************************
 *
 * FUNCTION:    TimeFormHandleKey
 *
 * DESCRIPTION: Handles keydown events
 *
 * PARAMETERS:  EventPtr
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void TimeFormHandleKey(EventPtr event)
{
	Boolean	hour24;
	UInt16	timegroup;
	UInt16	amgroup;
	UInt16	maxhour;
	UInt16	hours;
	UInt16	digit;
	UInt8	key = event->data.keyDown.chr;
	
	amgroup = GetControlGroupSelection(TimeAMPMGroup);
	timegroup = GetControlGroupSelection(TimeTimeGroup);
	
	hour24 = (amgroup == Time24PushButton);
	
	if (((key == 'a') || (key == 'A')) && (!hour24))
		SetControlGroupSelection(TimeAMPMGroup, TimeAMPushButton);
	else if (((key == 'p') || (key == 'P')) && (!hour24))
		SetControlGroupSelection(TimeAMPMGroup, TimePMPushButton);
	else if (key == tabChr)
	{
		if (timegroup == TimeOnesPushButton)
			timegroup = TimeHoursPushButton;
		else
			timegroup++;
		SetControlGroupSelection(TimeTimeGroup, timegroup);
	}
	else if ((key >= '0') && (key <= '9'))
	{
		digit = key - '0';
		if (timegroup == TimeHoursPushButton)
		{
			maxhour = (hour24) ? 23 : 12;
			
			if (sDigit == 0)
			{
				SetCtlLabelNumber(TimeHoursPushButton,digit);
				if (digit * 10 > maxhour)
					SetControlGroupSelection(TimeTimeGroup, TimeTensPushButton);
				else
					sDigit = 1;
			}
			else
			{
				hours = GetCtlLabelNumber(TimeHoursPushButton) * 10 + digit;
				
				if (hours > maxhour)
				{
					SetCtlLabelNumber(TimeTensPushButton, digit);
					SetControlGroupSelection(TimeTimeGroup, TimeOnesPushButton);
				}
				else
				{
					if ((hour24 == 0) && (hours == 0))
						hours = 12;
						
					SetCtlLabelNumber(TimeHoursPushButton, hours);
					SetControlGroupSelection(TimeTimeGroup, TimeTensPushButton);
					sDigit = 0;
				}
			}
		}
		else if ((timegroup == TimeTensPushButton) && (digit < 6))
		{	
			SetCtlLabelNumber(TimeTensPushButton,digit);
			SetControlGroupSelection(TimeTimeGroup, TimeOnesPushButton);
		}
		else if (timegroup == TimeOnesPushButton)
		{
			SetCtlLabelNumber(TimeOnesPushButton,digit);
			SetControlGroupSelection(TimeTimeGroup, TimeHoursPushButton);
		}
	}
	else if (key == linefeedChr)
	{
		TimeFormSave();
		event->eType = DialogCompleteEvent;
	}
}

/***********************************************************************
 *
 * FUNCTION:    TimeFormScroll
 *
 * DESCRIPTION: This routine scrolls the list of frames
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void TimeFormScroll(WinDirectionType direction)
{
	Int16		item;
	UInt16	value;

	item = GetControlGroupSelection(TimeTimeGroup);
	value = GetCtlLabelNumber(item);
	value += (direction == winUp) ? 1 : -1;
	
	if (item == TimeHoursPushButton)
	{
		value = (value + 24) % 24;
		
		if (GetControlGroupSelection(TimeAMPMGroup) != Time24PushButton)
		{
			value %= 12;
			if (value == 0) value = 12;
		}
	}
	else if (item == TimeTensPushButton)
		value = (value + 6) % 6;
	else if (item == TimeOnesPushButton)
		value = (value + 10) % 10;
		
	SetCtlLabelNumber(item, value);
}


/***********************************************************************
 *
 * FUNCTION:    NumberFormHandleEvent
 *
 * DESCRIPTION: Handles events for the Time editing form
 *
 * PARAMETERS:  EventPtr
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

Boolean NumberFormHandleEvent(EventPtr event)
{
	Boolean handled = false;
	
	switch (event->eType)
	{			
		case keyDownEvent:
			NumberFormHandleKey(event);
			handled = true;
			break;
			
		case ctlSelectEvent:
			sDigit = 0;
			if (event->data.ctlSelect.controlID == NumberOKButton)
			{
				NumberFormSave();
				event->eType = DialogCompleteEvent;
				handled = true;
			}
			else if (event->data.ctlSelect.controlID == NumberCancelButton)
			{
				event->eType = DialogCompleteEvent;
				handled = true;
			}
			break;
				
		case ctlRepeatEvent:
			if (event->data.ctlRepeat.controlID == NumberScrollUpRepeating)
				NumberFormScroll(winUp);
			else if (event->data.ctlRepeat.controlID == NumberScrollDownRepeating)
				NumberFormScroll(winDown);
			break;

	}
	return(handled);
}

/***********************************************************************
 *
 * FUNCTION:    NumberFormInit
 *
 * DESCRIPTION: This routine initializes the Time Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
void NumberFormInit (void)
{	
	sTipsCount = 1;
	SetControlGroupSelection(0, NumberNumberPushButton);
	iDateTimeSetDecimalEditCtl(NumberNumberPushButton, *sDateTime, sAllowDecimal);
}

/***********************************************************************
 *
 * FUNCTION:    NumberFormSave
 *
 * DESCRIPTION: This routine save the current value into gRollTime
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void NumberFormSave(void)
{			
	*sDateTime = DateTimeGetDecimalCtl(NumberNumberPushButton);
}


/***********************************************************************
 *
 * FUNCTION:    NumberFormHandleKey
 *
 * DESCRIPTION: Handles keydown events
 *
 * PARAMETERS:  EventPtr
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void NumberFormHandleKey(EventPtr event)
{
	UInt16	digit;
	UInt16	newval;
	UInt8	key = event->data.keyDown.chr;
	
	if ((key >= '0') && (key <= '9'))
	{
		sTipsCount = 0;						// if they've entered a digit, they don't need tips
		
		digit = key - '0';

		newval = DateTimeGetDecimalCtl(NumberNumberPushButton) * 10 + digit;

		if ((sAllowDecimal == kDecimalComplete) || 
			((sDigit == 0) && (sAllowDecimal != kDecimalEntered)) || 
			(newval > sMax))
		{
			if (sAllowDecimal != kDecimalNotAllowed)
				sAllowDecimal = kDecimalAllowed;
				
			iDateTimeSetDecimalEditCtl(NumberNumberPushButton,digit,sAllowDecimal);
			sDigit = 1;
		}
		else
		{
			sDigit++;
			if (sAllowDecimal == kDecimalEntered)
				sAllowDecimal = kDecimalComplete;
			iDateTimeSetDecimalEditCtl(NumberNumberPushButton, newval, sAllowDecimal);
		}
	}
	else if (key == backspaceChr)
	{
		if (sAllowDecimal == kDecimalComplete)
		{
			iDateTimeSetDecimalEditCtl(NumberNumberPushButton,0,sAllowDecimal);
			sDigit = 1;
		}
		else if (sAllowDecimal != kDecimalEntered)
		{
			iDateTimeSetDecimalEditCtl(NumberNumberPushButton, DateTimeGetDecimalCtl(NumberNumberPushButton) / 10, sAllowDecimal);
			if (sDigit) sDigit--;
		}
		else
		{
			sAllowDecimal = kDecimalAllowed;
			iDateTimeSetDecimalEditCtl(NumberNumberPushButton, DateTimeGetDecimalCtl(NumberNumberPushButton), sAllowDecimal);
		}
	}
	else if ((sAllowDecimal == kDecimalAllowed) && (key == '.'))
	{
		sAllowDecimal = kDecimalEntered;
		iDateTimeSetDecimalEditCtl(NumberNumberPushButton, DateTimeGetDecimalCtl(NumberNumberPushButton), sAllowDecimal);
	}
	else if (key == linefeedChr)
	{
		NumberFormSave();
		event->eType = DialogCompleteEvent;
	}
}

/***********************************************************************
 *
 * FUNCTION:    NumberFormScroll
 *
 * DESCRIPTION: This routine scrolls the list of frames
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void NumberFormScroll(WinDirectionType direction)
{
	UInt32	value;

	value = iDateTimeGetDecimalCtl(NumberNumberPushButton, 0);
	value += (direction == winUp) ? 1 : -1;
	
	if (value <= sMax)
		iDateTimeSetDecimalEditCtl(NumberNumberPushButton, value, sAllowDecimal);
		
	if (sTipsCount)
		sTipsCount++;
		
	if (sTipsCount > kTipsCountMax)
	{
		sTipsCount = 0;
		FrmHelp(NumberHelpString);
	}
}

UInt16 DateTimeGetDecimalShortCtl(UInt16 itemID)
{
	return (iDateTimeGetDecimalCtl(itemID, kDecimalShortFlag));
}

UInt32 DateTimeGetDecimalCtl(UInt16 itemID)
{
	return (iDateTimeGetDecimalCtl(itemID, kDecimalLongFlag));
}

UInt32 iDateTimeGetDecimalCtl(UInt16 itemID, UInt32 mask)
{
	ControlPtr	ctl;
	Char  *		label;
	Char  *		p;
	UInt32		result;
	
	ctl = GetObjectPtr(itemID);
	label = (Char *) CtlGetLabel(ctl);
	p = StrChr(label, '.');
	if (p)
	{
		result = StrAToI(label);
		if (StrLen(++p))
			result = result * 10 + StrAToI(p);
		result |= mask;
	}
	else
	{
		result = StrAToI(label);
	}
	return (result);
}

void	DateTimeSetDecimalShortCtl(UInt16 itemID, UInt16 number)
{
	iDateTimeSetDecimalCtl(itemID, number, kDecimalShortFlag);
}

void	DateTimeSetDecimalCtl(UInt16 itemID, UInt32 number)
{
	iDateTimeSetDecimalCtl(itemID, number, kDecimalLongFlag);
}

void 	iDateTimeSetDecimalCtl(UInt16 itemID, UInt32 number, UInt32 mask)
{
	ControlPtr	ctl;
	Char  *		label;
	Char  *		p;
	
	ctl = GetObjectPtr(itemID);
	label = (Char *) CtlGetLabel(ctl);
	if (number & mask)
	{
		number = number & ~mask;

		p = StrIToA(label, number / 10);
		p += StrLen(p);
		StrIToA(p + 1, number % 10);
		*p = '.';
	}
	else
	{
		StrIToA(label, number);
	}
	CtlSetLabel(ctl,label);
}

void iDateTimeSetDecimalEditCtl(UInt16 itemID, UInt32 number, UInt16 decimal)
{
	ControlPtr	ctl;
	Char  *		label;
	Char  *		p;
	
	ctl = GetObjectPtr(itemID);
	label = (Char *) CtlGetLabel(ctl);
	number = (number & ~kDecimalLongFlag);
	if (decimal == kDecimalComplete)
	{
		p = StrIToA(label, number / 10);
		p += StrLen(p);
		StrIToA(p + 1, number % 10);
		*p = '.';
	}
	else if (decimal == kDecimalEntered)
	{
		p = StrIToA(label, number);
		p += StrLen(p);
		*p++ = '.';
		*p = 0;
	}
	else
	{
		StrIToA(label, number);
	}
	CtlSetLabel(ctl,label);
}


#pragma mark --- OLD

#if 0

/***********************************************************************
 *
 * FUNCTION:    NumberFormInit
 *
 * DESCRIPTION: This routine initializes the Time Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
void NumberFormInit (void)
{	
	sTipsCount = 1;
	SetControlGroupSelection(0, NumberNumberPushButton);
	SetCtlLabelNumber(NumberNumberPushButton, *sDateTime);
}

/***********************************************************************
 *
 * FUNCTION:    NumberFormSave
 *
 * DESCRIPTION: This routine save the current value into gRollTime
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void NumberFormSave(void)
{			
	*sDateTime = GetCtlLabelNumber(NumberNumberPushButton);
}


/***********************************************************************
 *
 * FUNCTION:    NumberFormHandleKey
 *
 * DESCRIPTION: Handles keydown events
 *
 * PARAMETERS:  EventPtr
 *
 * REVISION HISTORY:
 *
 ***********************************************************************/

void NumberFormHandleKey(UInt8 key)
{
	UInt16	digit;
	UInt16	newval;
	
	if ((key >= '0') && (key <= '9'))
	{
		sTipsCount = 0;						// if they've entered a digit, they don't need tips
		
		digit = key - '0';

		if (sDigit == 0)
		{
			SetCtlLabelNumber(NumberNumberPushButton,digit);
			sDigit++;
		}
		else
		{
			newval = GetCtlLabelNumber(NumberNumberPushButton) * 10 + digit;
			if (newval > sMax)
			{
				SetCtlLabelNumber(NumberNumberPushButton, digit);
				sDigit = 1;
			}
			else
			{
				sDigit++;
				SetCtlLabelNumber(NumberNumberPushButton, newval);
			}
		}
	}
	else if (key == backspaceChr)
	{
		SetCtlLabelNumber(NumberNumberPushButton, GetCtlLabelNumber(NumberNumberPushButton) / 10);
		if (sDigit) sDigit--;
	}
	else if ((sAllowDecimal == kDecimalAllowed) && (key == '.'))
	{
		sAllowDecimal = kDecimalEntered;
	}
}

/***********************************************************************
 *
 * FUNCTION:    NumberFormScroll
 *
 * DESCRIPTION: This routine scrolls the list of frames
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *
 ***********************************************************************/
void NumberFormScroll(WinDirectionType direction)
{
	UInt32	value;

	value = GetCtlLabelNumber(NumberNumberPushButton);
	value += (direction == up) ? 1 : -1;
	
	if (value <= sMax)
		SetCtlLabelNumber(NumberNumberPushButton, value);
		
	if (sTipsCount)
		sTipsCount++;
		
	if (sTipsCount > kTipsCountMax)
	{
		sTipsCount = 0;
		FrmHelp(NumberHelpString);
	}
}

#endif