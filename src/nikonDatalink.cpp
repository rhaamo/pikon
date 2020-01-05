#include "nikonDatalink.h"

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Construct a new Nikon Datalink:: Nikon Datalink object
 * 
 * @param serialPort eg. /dev/ttyUSB0, COM2,...
 * @param baudrate  1200bps by default
 */
NikonDatalink::NikonDatalink (const char *serialPort, int baudrate) {
    serialPortName = serialPort;
    serialPortBaudrate = baudrate;
}

/**
 * @brief Destroy the Nikon Datalink:: Nikon Datalink object
 * 
 */
NikonDatalink::~NikonDatalink () {
    // Close serial
    serialClose();
}

/**
 * @brief Open serial port with wanted name and baudrate
 * 
 * @return int 
 */
int NikonDatalink::serialOpen () {
    // Open serial port, 8N1 1200bps by default
    enum sp_return error = sp_get_port_by_name(serialPortName, &serialPort);
    if (error == SP_OK) {
        error = sp_open(serialPort, SP_MODE_READ_WRITE);
        if (error == SP_OK) {
            sp_set_baudrate(serialPort, serialPortBaudrate);
        } else {
            printf("Error opening serial device: %s\n", serialPortName);
            return error;
        }
    } else {
        printf("Error finding serial device\n");
        return error;
    }
    return 0;
}

/**
 * @brief Closes serial port
 * 
 * @return int 
 */
int NikonDatalink::serialClose () {
    if (serialPort) {
        return sp_close(serialPort);
    } else {
        printf("Serial port == NULL, cannot close\n");
    }
    return 0;
}

/**
 * @brief Tries to identify the camera
 * 
 * @return int 
 */
int NikonDatalink::identifyCamera () {
    bool retry = false;
    bool done;
    unsigned char *p;
    int err;

    err = writeDataSlow(&kNullString, 1);
    if (err != SP_OK) {
        goto ERROR;
    }

RETRY_QUERY:
    done = false;
    err = writeDataSlow(kQueryString, kQueryStringSize);
    if (err != SP_OK) {
        goto ERROR;
    }

    p = serialBuffer;
    while ((done == false) && (err = readData(p, 1)) == 0) {
        if (*p++ == 0) {
            err = readData(p, 2);
            done = true;
        }
    }

    if (err) {
        if (retry == false) {
            retry = true;
            goto RETRY_QUERY;
        }
        if (err == kPacketSizeErr) {
            err = kWrongCameraErr;
        }
        goto ERROR;
    }

    if (strlen((char *) &(serialBuffer[4])) < kCameraNameBufSize) {
        strcpy(cameraName, (char *) &(serialBuffer[4]));
        if (memcmp(serialBuffer, kN90sResponseString, sizeof(kN90sResponseString)) == 0) {
            cameraType = CameraType::cameraN90s;
        } else if (memcmp(serialBuffer, kN90ResponseString, sizeof(kN90ResponseString)) == 0) {
			cameraType = CameraType::cameraN90;
        } else {
			err = kWrongCameraErr;
        }
    } else {
        err = kWrongCameraErr;
    }

ERROR:
    return err;
}

/**
 * @brief Get detected Camera Type
 * 
 * @return CameraType 
 */
CameraType NikonDatalink::getCameraType () {
    if (cameraType) {
        return cameraType;
    }
    return CameraType::unknown;
}

/**
 * @brief Write buffer to serial port with a slight delay
 * 
 * @param buf Buffer content
 * @param size Size of buffer
 * @return int 
 */
int NikonDatalink::writeDataSlow (const void *buf, int size) {
    unsigned int i;
    char *p;

    sp_flush(serialPort, SP_BUF_INPUT);

    p = (char *)buf;
    for (i=0; i<size; i++) {
        // delay, whatever, 200ms
        // original PalmOS port was using a SysTaskDelay(1)
        usleep(200);
        sp_return err = sp_blocking_write(serialPort, p, 1, 0);
        if (err != SP_OK) {
            return err;
        }
        p++;
    }
    return SP_OK;
}

/**
 * @brief Read datas from serial port
 * 
 * @param buf Buffer content
 * @param size Size of buffer
 * @return int 
 */
int NikonDatalink::readData (void *buf, int size) {
    unsigned long byteCount = 0;
    int err;

    if (size > kSerialBufSize) {
        return kPacketTooLargeErr;
    }

    int readCount = sp_blocking_read(serialPort, buf, size, kSerialTimeout);
    if (readCount > 0 && (readCount != size)) {
        err = kPacketSizeErr;
        sp_flush(serialPort, SP_BUF_INPUT);
    } else if (readCount < 0) {
        sp_flush(serialPort, SP_BUF_INPUT);
        err = -1;
    }

    return err;
}