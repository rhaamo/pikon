/*
	File:		StringTables.c

	Contains:	String constants used for the memo holder

	Written by:	Ken Hancock

	Copyright:	Copyright � Ken Hancock, All Rights Reserved

	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <3+>   1/24/2002    ksh     Remove hack for adding current A5 for PalmOS 1.x as it looks
                                    like its fixed by the compiler
         <3>     2/15/98    KSH     Add shutter string table for N90
         <2>     2/14/98    KSH     Checkin before modifications start for N90/F90 support
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/


/*
*/

//#define BUILDPALMOS

#include <string.h>

#include "nikonStringTables.h"

#define kUnknownValueString "???"


short GetStringTableMax(const char **table)
{
	long			offset;
	unsigned char	maxIndex;
		
	offset = 0;
	
	maxIndex = *(table[0] + offset);

	return maxIndex;	
}

char * GetStringTable(const char ** table, unsigned char index)
{
	char *	result;
	long	offset;
	unsigned char	maxIndex;
		
	offset = 0;
	
	maxIndex = *(table[0] + offset);
	
	if (index <= maxIndex)
	{
		result = ((char *) table[index+1] + offset);
		
	}
	else
		result = (char *) kUnknownValueString;

	return result;	
}

short GetStringIndex(const char ** table, char * str)
{
	long		offset;
	unsigned char	maxIndex;
	const char *			indstr;
	unsigned char	i;
	
	offset = 0;
	
	maxIndex = *(table[0] + offset);
	
	for (i = 0; i < maxIndex; i++)
	{
		indstr = GetStringTable(table, i);
		if (strcmp(indstr, str) == 0)
			return(i);
	}		
	return (-1);
}

const char * kISOTable[] = { 
	"\x1F",		// max index in table
	"6",	"8",	"10",	"12",	"16",	"20",	"25",	"32",	
	"40",	"50",	"64",	"80",	"100",	"125",	"160",	"200",	
	"250",	"320",	"400",	"500",	"640",	"800",	"1000",	"1250",	
	"1600",	"2000",	"2500",	"3200",	"4000",	"5000",	"6400",	"DX" };


const char * kShutterSpeedTable[] = { 
	"\x56",		// max index in table
	"ERR",		"BULB",		"??",		"??",		"30",		"25",		"20",		"20",		
	"15",		"13",		"10",		"10",		"8",		"6",		"6",		"5",		
	"4",		"3",		"3",		"2.5",		"2",		"1.6",		"1.45",		"1.3",		
	"1",		"1/1.3",	"1/1.5",	"1/1.6",	"1/2",		"1/2.5",	"1/3",		"1/3",		
	"1/4",		"1/5",		"1/6",		"1/6",		"1/8",		"1/10",		"1/10",		"1/13",		
	"1/15",		"1/20",		"1/20",		"1/25",		"1/30",		"1/40",		"1/45",		"1/50",		
	"1/60",		"1/80",		"1/90",		"1/100",	"1/125",	"1/160",	"1/180",	"1/200",	
	"1/250",	"1/320",	"1/350",	"1/400",	"1/500",	"1/640",	"1/750",	"1/800",	
	"1/1000",	"1/1250",	"1/1500",	"1/1600",	"1/2000",	"1/2500",	"1/3000",	"1/3200",	
	"1/4000",	"1/5000",	"1/6000",	"1/6400",	"1/8000",	"??",		"??",		"??",		
	"??",		"??",		"??",		"??",		"BULB",		"HI",		"LOW"		};

const char * kApertureTable[] = { 
	"\x4E",		// max index in table
	"1",  	"1.1",	"1.1",	"1.2",	"1.2",	"1.3",	"1.4",	"1.5",
	"1.6",	"1.7",	"1.8",	"1.9",	"2",  	"2.1",	"2.2",	"2.4",
	"2.5",	"2.7",	"2.8",	"3",  	"3.2",	"3.3",	"3.5",	"3.8",
	"4",  	"4.2",	"4.5",	"4.8",	"5",  	"5.3",	"5.6",	"6",
	"6.3",	"6.7",	"7.1",	"7.6",	"8",	"8.5",	"9",	"9.5",
	"10",	"11",	"11",	"12",	"13",	"13",	"14",	"15",		
	"16",	"17",	"18",	"19",	"20",	"21",	"22",	"24",		
	"25",	"27",	"29",	"30",	"32",	"34",	"36",	"38",		
	"40",	"43",	"45",	"48",	"51",	"54",	"57",	"60",		
	"64",	"68",	"72",	"76",	"81",	"85",	"90" };

	
const char * kExposureModeTable[] = { 
	"\x16",		// max index in table
	"M - Manual",			"A - Aperature-Priority",	
	"S - Shutter-Priority",	"P - Multi-Program",		
	"",						"",							
	"",						"",							
	"Ps/Po - Portrait",		"Ps/Re - Red-Eye Reduction",
	"Ps/Hf - Hyperfocal",	"Ps/La - Landscape"	,		
	"Ps/Sl - Silhouette",	"Ps/Sp - Sports",			
	"Ps/Cu - Close-Up",		"P/CP - Custom Program"	};

const char * kFlashSyncTable[] = { 
	"\x03",		// max index in table
	"Normal Sync",	"Slow Sync",	"Rear-Curtain Sync",	"Red-Eye Reduction"	};


const char * kFocalTable[] = { 
	"\xFF",		// max index in table
	"-",	"5",	"5.5",	"5.5",	"5.5",	"5.5",	"6",	"6",	
	"6.5",	"6.5",	"6.5",	"7",	"7",	"7",	"7.5",	"7.5",	
	"8",	"8",	"8.5",	"8.5",	"9",	"9",	"9.5",	"9.5",	
	"10",	"10",	"10.5",	"11",	"11",	"11.5",	"12",	"12",	
	"13",	"13",	"13",	"14",	"14",	"14",	"15",	"15",
	"16",	"16",	"17",	"17",	"18",	"18",	"19",	"19",	
	"20",	"20",	"21",	"22",	"22",	"23",	"24",	"24",	
	"25",	"26",	"26",	"27",	"28",	"28",	"30",	"31",	
	"32",	"32",	"34",	"34",	"35",	"36",	"38",	"38",	
	"40",	"40",	"42",	"44",	"45",	"46",	"48",	"48",	
	"50",	"52",	"52",	"55",	"56",	"58",	"60",	"62",	
	"62",	"65",	"66",	"68",	"70",	"72",	"75",	"78",	
	"80",	"82",	"85",	"86",	"90",	"92",	"95",	"98",	
	"100",	"102",	"105",	"110",	"112",	"116",	"120",	"122",	
	"125",	"130",	"135",	"135",	"140",	"145",	"150",	"155",	
	"160",	"165",	"171",	"175",	"180",	"185",	"190",	"195",	
	"200",	"210",	"210",	"220",	"220",	"230",	"240",	"240",	
	"250",	"260",	"270",	"270",	"280",	"290",	"300",	"310",	
	"320",	"330",	"340",	"350",	"360",	"370",	"380",	"390",	
	"400",	"410",	"420",	"440",	"450",	"460",	"480",	"490",	
	"500",	"500",	"550",	"550",	"550",	"600",	"600",	"600",	
	"650",	"650",	"650",	"700",	"700",	"750",	"750",	"800",	
	"800",	"800",	"850",	"850",	"900",	"900",	"950",	"1000",	
	"1000",	"1050",	"1050",	"1100",	"1150",	"1150",	"1200",	"1250",	
	"1300",	"1300",	"1300",	"1400",	"1400",	"1500",	"1500",	"1600",	
	"1600",	"1600",	"1700",	"1700",	"1800",	"1800",	"1900",	"2000",	
	"2000",	"2100",	"2100",	"2200",	"2300",	"2300",	"2400",	"2500",	
	"2500",	"2600",	"2700",	"2800",	"2900",	"2900",	"3000",	"3100",	
	"3200",	"3300",	"3400",	"3500",	"3600",	"3700",	"3800",	"3900",	
	"4000",	"4100",	"4300",	"4400",	"4500",	"4700",	"4800",	"4900",	
	"5100",	"5200",	"5400",	"5500",	"5700",	"5900",	"6000",	"6200",	
	"6400",	"6600",	"6800",	"7000",	"7200",	"7400",	"7600",	"-" };


const char * kExpCompTable[] = { 
	"\xFF",		// max index in table
	" 0.0",	"-0.2",	"-0.3",	"-0.5",	"-0.7",	"-0.8",	"-1.0",	"-1.2",		
	"-1.3",	"-1.5",	"-1.7",	"-1.8",	"-2.0",	"-2.2",	"-2.3",	"-2.5",		
	"-2.7",	"-2.8",	"-3.0",	"-3.2",	"-3.3",	"-3.5",	"-3.7",	"-3.8",		
	"-4.0",	"-4.2",	"-4.3",	"-4.5",	"-4.7",	"-4.8",	"-5.0",	"-5.2",		
	"-5.3",	"-5.5",	"-5.7",	"-5.8",	"-6.0",	"-6.2",	"-6.3",	"-6.5",		
	"-6.7",	"-6.8",	"-7.0",	"-7.2",	"-7.3",	"-7.5",	"-7.7",	"-7.8",		
	"-8.0",	"-8.2",	"-8.3",	"-8.5",	"-8.7",	"-8.8",	"-9.0",	"-9.2",		
	"-9.3",	"-9.5",	"-9.7",	"-9.8",	"-10.0","-10.2","-10.3","-10.5",	
	"-10.7","-10.8","-11.0","-11.2","-11.3","-11.5","-11.7","-11.8",	
	"-12.0","-12.2","-12.3","-12.5","-12.7","-12.8","-13.0","-13.2",	
	"-13.3","-13.5","-13.7","-13.8","-14.0","-14.2","-14.3","-14.5",	
	"-14.7","-14.8","-15.0","-15.2","-15.3","-15.5","-15.7","-15.8",	
	"-16.0","-16.2","-16.3","-16.5","-16.7","-16.8","-17.0","-17.2",	
	"-17.3","-17.5","-17.7","-17.8","-18.0","-18.2","-18.3","-18.5",	
	"-18.7","-18.8","-19.0","-19.2","-19.3","-19.5","-19.7","-19.8",	
	"-20.0","-20.0","-20.0","-20.0","-20.0","-20.0","-20.0","-20.0",	
	"+20.0","+20.0","+20.0","+20.0","+20.0","+20.0","+20.0","+20.0",	
	"+20.0","+19.8","+19.7","+19.5","+19.3","+19.2","+19.0","+18.8",	
	"+18.7","+18.5","+18.3","+18.2","+18.0","+17.8","+17.7","+17.5",	
	"+17.3","+17.2","+17.0","+16.8","+16.7","+16.5","+16.3","+16.2",	
	"+16.0","+15.8","+15.7","+15.5","+15.3","+15.2","+15.0","+14.8",	
	"+14.7","+14.5","+14.3","+14.2","+14.0","+13.8","+13.7","+13.5",	
	"+13.3","+13.2","+13.0","+12.8","+12.7","+12.5","+12.3","+12.2",	
	"+12.0","+11.8","+11.7","+11.5","+11.3","+11.2","+11.0","+10.8",	
	"+10.7","+10.5","+10.3","+10.2","+10.0","+9.8",	"+9.7",	"+9.5",		
	"+9.3",	"+9.2",	"+9.0",	"+8.8",	"+8.7",	"+8.5",	"+8.3",	"+8.2",		
	"+8.0",	"+7.8",	"+7.7",	"+7.5",	"+7.3",	"+7.2",	"+7.0",	"+6.8",		
	"+6.7",	"+6.5",	"+6.3",	"+6.2",	"+6.0",	"+5.8",	"+5.7",	"+5.5",		
	"+5.3",	"+5.2",	"+5.0",	"+4.8",	"+4.7",	"+4.5",	"+4.3",	"+4.2",		
	"+4.0",	"+3.8",	"+3.7",	"+3.5",	"+3.3",	"+3.2",	"+3.0",	"+2.8",		
	"+2.7",	"+2.5",	"+2.3",	"+2.2",	"+2.0",	"+1.8",	"+1.7",	"+1.5",		
	"+1.3",	"+1.2",	"+1.0",	"+0.8",	"+0.7",	"+0.5",	"+0.3",	"+0.2" };

const char * kMeterTable[] = { 
	"\x03",		// max index in table
	"Center",	"Spot",	"Matrix",	""	};

const char * kCameraBracketTable[] = {
	"\x0C",		// max index in table
	"0.0",	".25",	".3",	".5",	".7",	".75",	"1.0",	"1.25",
	"1.3",	"1.5",	"1.7",	"1.75",	"2.0"	};

const char * kFlashSyncSpeedTable[] = {
	"\x39",
	"60",		"??",		"??",		"30",		"25",		"20",		"15",		"13",	
	"10",		"8",		"6",		"5",		"4",		"3",		"2.5",		"2",	
	"1.6",		"1.3",		"1",		"1/1.3",	"1/1.6",	"1/2",		"1/2.5",	"1/3",	
	"1/4",		"1/5",		"1/6",		"1/8",		"1/10",		"1/13",		"1/15",		"1/20",	
	"1/25",		"1/30",		"1/40",		"1/50",		"1/60",		"1/80",		"1/100",	"1/125",	
	"1/160",	"1/200",	"1/250",	"1/320",	"1/400",	"1/500",	"1/640",	"1/800",	
	"1/1000",	"1/1250",	"1/1600",	"1/2000",	"1/2500",	"1/3200",	"1/4000",	"1/5000",	
	"1/6400",	"1/8000"	};

const char * kCameraShutterTable[] = {
	"\x39",
	"Bulb",		"??",		"??",		"30",		"25",		"20",		"15",		"13",
	"10",		"8",		"6",		"5",		"4",		"3",		"2.5",		"2",
	"1.6",		"1.3",		"1",		"1/1.3",	"1/1.6",	"1/2",		"1/2.5",	"1/3",
	"1/4",		"1/5",		"1/6",		"1/8",		"1/10",		"1/13",		"1/15",		"1/20",
	"1/25",		"1/30",		"1/40",		"1/50",		"1/60",		"1/80",		"1/100",	"1/125",
	"1/160",	"1/200",	"1/250",	"1/320",	"1/400",	"1/500",	"1/640",	"1/800",
	"1/1000",	"1/1250",	"1/1600",	"1/2000",	"1/2500",	"1/3200",	"1/4000",	"1/5000",
	"1/6400",	"1/8000"
};

const char * kCameraN90ShutterTable[] = {
	"\x13",
	"Bulb",		"30",		"15",		"8",		"4",		"2",		"1",		"1/2",
	"1/4",		"1/8",		"1/15",		"1/30",		"1/60",		"1/125",	"1/250",	"1/500",
	"1/1000",	"1/2000",	"1/4000",	"1/8000"
};
const char * kCameraApertureTable[] = {
	"\xA8",		// max index in table
	"1",	"1",	"1.1",	"1.1",	"1.1",	"1.2",	"1.2",	"1.2",	
	"1.3",	"1.3",	"1.3",	"1.4",	"1.4",	"1.5",	"1.5",	"1.5",
	"1.6",	"1.6",	"1.7",	"1.7",	"1.8",	"1.8",	"1.9",	"1.9",	
	"2",	"2.1",	"2.1",	"2.2",	"2.2",	"2.3",	"2.4",	"2.4",	
	"2.5",	"2.6",	"2.7",	"2.7",	"2.8",	"2.9",	"3",	"3.1",	
	"3.2",	"3.3",	"3.4",	"3.5",	"3.6",	"3.7",	"3.8",	"3.9",	
	"4",	"4.1",	"4.2",	"4.4",	"4.5",	"4.6",	"4.8",	"4.9",	
	"5",	"5.2",	"5.3",	"5.5",	"5.6",	"5.8",	"6",	"6.2",	
	"6.3",	"6.5",	"6.7",	"6.9",	"7.1",	"7.3",	"7.6",	"8",	
	"8",	"8.2",	"8.5",	"8.7",	"9",	"9.2",	"9.5",	"9.8",	
	"10",	"10",	"10",	"11",	"11",	"11",	"12",	"12",	
	"12",	"13",	"13",	"13",	"14",	"14",	"15",	"15",	
	"16",	"16",	"17",	"17",	"18",	"18",	"19",	"19",	
	"20",	"20",	"21",	"22",	"22",	"23",	"24",	"24",	
	"25",	"26",	"26",	"27",	"28",	"29",	"30",	"31",	
	"32",	"32",	"33",	"34",	"35",	"37",	"38",	"39",	
	"40",	"41",	"42",	"44",	"44",	"46",	"47",	"49",	
	"50",	"52",	"53",	"55",	"57",	"58",	"60",	"62",	
	"64",	"65",	"67",	"69",	"71",	"73",	"76",	"78",	
	"80",	"83",	"85",	"87",	"90",	"93",	"95",	"98",	
	"101",	"104",	"107",	"110",	"114",	"117",	"120",	"124",	
	"128" };
	
/*	Calculated values with whole f-stops tweaked at some points
	
	formula = SQRT(2)^(x/12)
	
	"1.0",	"1.0",	"1.1",	"1.1",	"1.1",	"1.2",	"1.2",	"1.2",	"1.3",
	"1.3",	"1.3",	"1.4",	"1.4",	"1.5",	"1.5",	"1.5",	"1.6",	"1.6",
	"1.7",	"1.7",	"1.8",	"1.8",	"1.9",	"1.9",	"2.0",	"2.1",	"2.1",
	"2.2",	"2.2",	"2.3",	"2.4",	"2.4",	"2.5",	"2.6",	"2.7",	"2.7",
	"2.8",	"2.9",	"3.0",	"3.1",	"3.2",	"3.3",	"3.4",	"3.5",	"3.6",
	"3.7",	"3.8",	"3.9",	"4.0",	"4.1",	"4.2",	"4.4",	"4.5",	"4.6",
	"4.8",	"4.9",	"5.0",	"5.2",	"5.3",	"5.5",	"5.6",	"5.8",	"6.0",
	"6.2",	"6.3",	"6.5",	"6.7",	"6.9",	"7.1",	"7.3",	"7.6",	"8.0",
	"8.0",	"8.2",	"8.5",	"8.7",	"9.0",	"9.2",	"9.5",	"9.8",	"10.1",
	"10.4",	"10.7",	"11.0",	"11.0",	"11.6",	"12.0",	"12.3",	"12.7",	"13.1",
	"13.5",	"13.8",	"14.3",	"14.7",	"15.1",	"15.5",	"16.0",	"16.5",	"17.0",
	"17.4",	"18.0",	"18.5",	"19.0",	"19.6",	"20.2",	"20.7",	"21.4",	"22.0",
	"22.0",	"23.3",	"24.0",	"24.7",	"25.4",	"26.1",	"26.9",	"27.7",	"28.5",
	"29.3",	"30.2",	"31.1",	"32.0",	"32.9",	"33.9",	"34.9",	"35.9",	"37.0",
	"38.1",	"39.2",	"40.3",	"41.5",	"42.7",	"44.0",	"44.0",	"46.6",	"47.9",
	"49.4",	"50.8",	"52.3",	"53.8",	"55.4",	"57.0",	"58.7",	"60.4",	"62.2",
	"64.0",	"65.9",	"67.8",	"69.8",	"71.8",	"73.9",	"76.1",	"78.3",	"80.6",
	"83.0",	"85.4",	"87.9",	"90.0",	"93.2",	"95.9",	"98.7",	"101.6","104.6",
	"107.6","110.8","114.0","117.4","120.8","124.4","128.0"
*/

const char * kCameraExpModeTable[] = { 
	"\x03",		// max index in table
	"P - Program",	"S - Shutter Priority",	"A - Aperture Priority",	"M - Manual"};

const char * kCameraMeterTable[] = {
	"\x02",		// max index in table
	"Matrix", "Center", "Spot" };
	

const char * kCameraExpCompTable[] = {
	"\x1E",
	"+5.0","+4.7","+4.3","+4.0","+3.7","+3.3","+3.0","+2.7",
	"+2.3","+2.0","+1.7","+1.3","+1.0","+0.7","+0.3","0.0",
	"-0.3","-0.7","-1.0","-1.3","-1.7","-2.0","-2.3","-2.7",
	"-3.0","-3.3","-3.7","-4.0","-4.3","-4.7","-5.0" };

const char * kCameraFlashCompTable[] = {
	"\x0C",
	"+1.0","+0.7","+0.3", "0.0","-0.3","-0.7","-1.0","-1.3",
	"-1.7","-2.0","-2.3","-2.7","-3.0" };
	