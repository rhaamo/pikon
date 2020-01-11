#ifndef NIKON_DATALINK_H
#define NIKON_DATALINK_H

#include <libserialport.h>
#include <string.h>

#include "log.h"
#include "nikonPackets.h"
#include "nikonErrors.h"

#define kSerialBufSize	256
#define kCameraNameBufSize	16
#define kSerialTimeout 2000

#define kMaxCommandData       0xEE

#define SET_ADDRESS(d,s)        \
        { \
                unsigned char *dp; \
                dp = (unsigned char *)d;        \
                *dp++ = (unsigned char) (s>>16);        \
                *dp++ = (unsigned char) (s>>8); \
                *dp++ = (unsigned char) s;      \
        }

class NikonDatalink {
    public:
        NikonDatalink(const char *serialPort);
        ~NikonDatalink();
        int startSession();
        int endSession();
        CameraType getCameraType();
        void setLogLevel(int);
        void focus();

        const char *serialPortName;

    private:
        int serialClose();
        int writeDataSlow(const void *buf, int size);
        int writeData (const void *buf, int size);
        int readData(void *buf, int size);
        int sendCommand(int mode, unsigned long address, void *buf, int size);
        int sendCommandLoop(int mode, unsigned long address, void *buf, int size);
        void makeDataPacket(unsigned char *buf, int size);
        int readDataPacket(unsigned char *buf, int size);
        int readStatusPacket();
        int serialOpen();
        int identifyCamera();
        bool switchBaudrate();

        struct sp_port *serialPort;
        char cameraName[kCameraNameBufSize];
        CameraType cameraType = CameraType::unknown;
        unsigned char serialBuffer[kSerialBufSize];
        int logLevel = LOG_ERROR;
        int serialPortBaudrate = 1200;
        bool baudrateChange = false;
        int sessionErr;
};

#endif