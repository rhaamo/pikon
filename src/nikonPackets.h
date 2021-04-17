/*
	File:		nikonPackets.h

	Contains:	

	Written by:	Ken Hancock

	Copyright:	Copyright © Ken Hancock, All Rights Reserved
 
 	RELEASED AS OPEN SOURCE 13 May 2004

	Change History (most recent first):

        <3+>    7/5/2001    ksh     Update for OS 3.5 headers
         <3>     2/15/98    KSH     Update to handle N90 protocol
         <2>     2/14/98    KSH     Update for N90/F90
         <1>     1/15/98    ksh     Initial Projector check-in from N90s Buddy 1.0
*/

/*

NIKON Unit Inquiry/1200 baud

QUERY:		$53 31 30 30 30 05
RESPONSE:	$31 30 32 30 46 39 30 58		1020F90X
			$2F 4E 39 30 53 00 03 06		/N90S...

*/

#define kNullString				"\0"
#define kQueryString			"S1000\x05"
#define kQueryStringSize		6
#define kN90sResponseString		"1020F90X/N90S"
#define kN90ResponseString		"1010F90/N90"

// Different names between USA and everywhere else
// N90 is also F90
// N90s is also F90x
typedef enum {
	unknown, cameraN90, cameraN90s
} CameraType;

/*
N90S Command Packet - 9 bytes

  02   |  20   |  80   |  02   |  02   |  02   |  FD   |  00   |  03   |
 start | cflag | mode  |        address        |    length     | stop  |
1 byte |1 byte |1 byte |        3 bytes        |    2 bytes    |1 byte |
*/

/*
N90 Command Packet - 9 bytes

  02   |  10   |  80   |  02   |  02   |  02   |  FD   |  00   |  03   |
 start | cflag | mode  |        address        |    length     | stop  |
1 byte |1 byte |1 byte |        3 bytes        |    2 bytes    |1 byte |
*/

typedef struct {
	unsigned char	startMark;
	unsigned char	commandFlag;
	unsigned char	modeFlag;
	unsigned char	address[3];
	unsigned int	length;
	unsigned char	stopMark;
} CommandPacket;

#define kCommandPacketSize			9
#define kCommandStartMark			0x01
#define kCommandN90SCommandFlag		0x20
#define kCommandN90CommandFlag		0x10
#define kCommandStopMark			0x03

#define kReadDataMode		0x80
#define kWriteDataMode		0x81
#define kShutterMode		0x85
#define kFocusMode			0x86
#define kBaudChangeMode		0x87
#define kMemoHolderMode		0x1B

#define kBaud9600Address	0x050000
#define kMemoHolderAddress	0x920000

/*
N90S Data Packet - 3+length bytes

  02   |     data      |   00  |   03  |			chk = SUM(data)
 start |               |  chk  |  stop |
1 byte |    n bytes    |1 byte |1 byte |
*/

#define kDataPacketStartMark	0x02
#define kDataPacketStopMark		0x03

typedef struct {
	unsigned char	startMark;
	
} DataPacketStart;
#define kDataPacketStartSize	1

typedef struct {
	unsigned char	checkByte;
	unsigned char	stopMark;
} DataPacketStop;
#define kDataPacketStopSize		2
/*
N90S Status Packet

  06   |   00  |
     status    |
     2 bytes   |
*/

typedef struct {
	unsigned int	status;
} StatusPacket;
#define kStatusPacketSize	2

#define kStatusOK	0x0600

/*
N90S Signoff Packet - resets the camera to default (1200) baud

  04   |   04  |
    signoff    |
    2 bytes    |
*/

#define kSignoffPacketWord	0x0404

typedef struct {
	unsigned int	signoffWord;
} SignoffPacket;
#define kSignoffPacketSize	2

typedef struct {
        unsigned int    rollNumber;
        unsigned int    byteCount;
} MemoHolderResponse;
#define kMemoHolderResponseSize 4

typedef struct CameraControlGlobals {
	unsigned char	valid;
	unsigned char	locationFD21;
	
	unsigned char	locationFD25;
	unsigned char	locationFD26;
	unsigned char	locationFD27;
	unsigned char	locationFD28;
	unsigned char	locationFD29;
	unsigned char	locationFD2A;
	unsigned char	locationFD2B;
	unsigned char	locationFD2C;
	unsigned char	locationFD2D;
	
	unsigned char	locationFD3A;
	unsigned char	locationFD3B;
	unsigned char	locationFD3C;
	unsigned char	locationFD3D;
	
	unsigned char	locationFD89;
	
	unsigned char	locationFD8E;
	unsigned char	locationFD8F;
	unsigned int	locationFD90;
	
	unsigned int	locationFD9D;
	
	unsigned char	locationFE20;
	unsigned char	locationFE21;
	unsigned char	locationFE22;
	unsigned char	locationFE23;
	unsigned char	locationFE24;
	unsigned char	locationFE25;
	unsigned char	locationFE26;
	unsigned char	locationFE27;
	unsigned char	locationFE28;
	unsigned char	locationFE29;
	unsigned char	locationFE2A;
	unsigned char	locationFE2B;
	unsigned char	locationFE2C;
	unsigned char	locationFE2D;
	unsigned char	locationFE2E;
	unsigned char	locationFE2F;
	unsigned char	locationFE30;
	unsigned char	locationFE31;
	unsigned char	locationFE32;
	unsigned char	locationFE33;
	unsigned char	locationFE34;
	
	unsigned char	locationFE3A;
	
	unsigned char	locationFE4F;
	unsigned char	locationFE50;
	unsigned char	locationFE51;
} CameraControlGlobals;
