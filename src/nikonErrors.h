#ifndef NIKON_ERRORS_H
#define NIKON_ERRORS_H

#define appErrorClass 1008

#define	kWrongCameraErr		(appErrorClass | 0)
#define	kPacketSizeErr			(appErrorClass | 1)
#define	kPacketCSErr			(appErrorClass | 2)
#define	kPacketResponseErr	(appErrorClass | 3)
#define	kErrorPacketErr		(appErrorClass | 4)
#define	kNoMemoDataErr			(appErrorClass | 5)
#define	kUnknownDataErr		(appErrorClass | 6)
#define	kUnknownModeErr		(appErrorClass | 7)
#define	kPacketTooLargeErr	(appErrorClass | 8)
#define	kConvertDataErr		(appErrorClass | 9)
#define kSerialOpenErr		(appErrorClass | 10)

#endif