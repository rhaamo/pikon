#ifndef NIKON_DATALINK_H
#define NIKON_DATALINK_H

#include <libserialport.h>
#include <string.h>

#include "nikonPackets.h"
#include "nikonErrors.h"

#define kSerialBufSize	256
#define kCameraNameBufSize	16
#define kSerialTimeout 2000

class NikonDatalink {
    public:
        NikonDatalink(const char *serialPort, int baudrate);
        ~NikonDatalink();
        int serialOpen();
        int identifyCamera();
        CameraType getCameraType();

        const char *serialPortName;
        int serialPortBaudrate;

    private:
        int serialClose();
        int writeDataSlow(const void *buf, int size);
        int readData(void *buf, int size);

        struct sp_port *serialPort;
        char cameraName[kCameraNameBufSize];
        CameraType cameraType = CameraType::unknown;
        unsigned char serialBuffer[kSerialBufSize];
};

#endif