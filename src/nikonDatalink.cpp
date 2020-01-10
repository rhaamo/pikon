#include "nikonDatalink.h"

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Construct a new Nikon Datalink:: Nikon Datalink object
 * 
 * @param serialPort eg. /dev/ttyUSB0, COM2,...
 * @param baudrate  1200bps by default
 */
NikonDatalink::NikonDatalink (const char *serialPort) {
    serialPortName = serialPort;
}

/**
 * @brief Destroy the Nikon Datalink:: Nikon Datalink object
 * 
 */
NikonDatalink::~NikonDatalink () {
    // Close serial
    serialClose();
}

void NikonDatalink::setLogLevel (int log_level) {
    log_set_level(log_level);
    logLevel = log_level;
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
            log_fatal("Error opening serial device: %s", serialPortName);
            return error;
        }
    } else {
        log_fatal("Error finding serial device");
        log_fatal("%s", sp_last_error_message());
        return error;
    }

    log_info("libserialport version: %s", sp_get_package_version_string());
    log_info("Port name: %s", sp_get_port_name(serialPort));
    log_info("Description: %s", sp_get_port_description(serialPort));
    return 0;
}

/**
 * @brief Closes serial port
 * 
 * @return int 
 */
int NikonDatalink::serialClose () {
    if (serialPort) {
        sp_close(serialPort);
        sp_free_port(serialPort);
    } else {
        log_error("Serial port == NULL, cannot close");
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

    log_info("Identifying camera...");
    log_info("Sending wakeup string...");

    err = writeDataSlow(kNullString, 1);
    if (err != SP_OK) {
        log_error("SP_ERR: %s", sp_last_error_message());
        goto ERROR;
    }

RETRY_QUERY:
    done = false;

    log_info("Sending nikon inquiry string...");
    err = writeData(kQueryString, kQueryStringSize);
    if (err != SP_OK) {
        log_error("%s", sp_last_error_message());
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

    log_debug("writeDataSlow: '%s'", buf);

    sp_flush(serialPort, SP_BUF_INPUT);

    p = (char *)buf;
    for (i=0; i<size; i++) {
        // delay, whatever, 200ms
        // original PalmOS port was using a SysTaskDelay(1)
        usleep(200);
        sp_blocking_write(serialPort, p, 1, 0);
        sp_return err = sp_drain(serialPort);
        if (err != SP_OK) {
            log_error("Cannot write one byte: %s", sp_last_error_message());
            return err;
        }
        p++;
    }
    return SP_OK;
}

/**
 * @brief Write buffer to serial port
 * 
 * @param buf Buffer content
 * @param size Buffer size
 * @return int 
 */
int NikonDatalink::writeData (const void *buf, int size) {
    log_debug("writeData: '%s'", buf);

    sp_flush(serialPort, SP_BUF_INPUT);

    sp_blocking_write(serialPort, buf, size, 0);
    sp_return err = sp_drain(serialPort);
    if (err != SP_OK) {
        log_error("Cannot write buffer: %s", sp_last_error_message());
    }
    return err;
}


/**
 * @brief Read datas from serial port
 * 
 * @param buf Buffer content
 * @param size Size of buffer
 * @return int 0 if expected count, else error
 */
int NikonDatalink::readData (void *buf, int size) {
    unsigned long byteCount = 0;
    int err;

    if (size > kSerialBufSize) {
        return kPacketTooLargeErr;
    }

    int readCount = sp_blocking_read(serialPort, buf, size, kSerialTimeout);
    if (readCount < 0) {
        if ((readCount > 0) && (readCount != size)) {
            err = kPacketSizeErr;
            sp_flush(serialPort, SP_BUF_INPUT);
        } else {
            sp_flush(serialPort, SP_BUF_INPUT);
            err = readCount;
        }
        log_debug("Error re\ading datas: %s", sp_last_error_message());
    } else if (readCount == size) {
        err = 0; // it's ok
    } else {
        err = -1; // oopsie
    }

    log_debug("readData: '%s', read: %i, expected: %i, err: %i", buf, readCount, size, err);

    return err;
}

/**
 * @brief Switch to 9600 bps after camera identification has been done
 * 
 */
bool NikonDatalink::switchBaudrate() {
    if (!baudrateChange || cameraType == CameraType::unknown) {
        log_error("baudrateChange not enabled or camera type unknown. cannot switch baudrate");
        return false;
    }
    log_info("Switching baudrate...");
    if (sendCommand(kBaudChangeMode, 0, 0, 0)) {
        return true;
    }

    return false;
}

/**
 * @brief Send a command
 * 
 * @param mode 
 * @param address 
 * @param buff 
 * @param size 
 * @return int 0 if ok; <0 if err
 */
int NikonDatalink::sendCommand(int mode, unsigned long address, void *buf, int size) {
    log_debug("Sending command, mode: %i, address: %u, buffer: %s, size: %i", mode, address, buf, size);

    int partial;
    int err = 0;

    do {
        partial = size;
        if (size > kMaxCommandData) {
            partial = kMaxCommandData;
        }

        err = sendCommandLoop(mode, address, buf, partial);
        if (err) {
            goto ERROR;
        }

        size -= partial;
        // (unsigned char *) buf += partial;
        // https://clang.llvm.org/compatibility.html#lvalue-cast
        #warning "pls unbork"
        address += partial;
    } while (size > 0);

ERROR:
    return 0;
}

/**
 * @brief Send the command in loop or whatever
 * 
 * @param mode 
 * @param address 
 * @param buf 
 * @param size 
 * @return int 0 = ok, <0 = lol rip
 */
int NikonDatalink::sendCommandLoop(int mode, unsigned long address, void *buf, int size) {
    log_debug("Sending command loop, mode: %i, address: %u, buffer: %s, size: %i", mode, address, buf, size);

    CommandPacket cp;
    char retry;
    bool slow = false;
    int err = 0;
    unsigned char *cmdBuf;

    cmdBuf = (unsigned char *)buf;

    retry = false;

    cp.startMark = kCommandStartMark;
    if (cameraType == cameraN90s) {
        cp.commandFlag = kCommandN90SCommandFlag;
    } else {
        cp.commandFlag = kCommandN90CommandFlag;
    }
    cp.modeFlag = mode;
    cp.stopMark = kCommandStopMark;
    cp.length = 0;
    SET_ADDRESS(&cp.address, address);

COMMAND_RETRY:
    switch (mode) {
        case kReadDataMode:
            SET_ADDRESS(&cp.address, address);
            cp.length = size;
            err = writeData(&cp, kCommandPacketSize);
            break;
        case kWriteDataMode:
            SET_ADDRESS(&cp.address, address);
            cp.length = size;
            makeDataPacket(cmdBuf, size);
            err = writeData(&cp, kCommandPacketSize - 1); // leave off stop bit
            err = writeData(serialBuffer, kDataPacketStartSize + size + kDataPacketStopSize);
            break;
        case kShutterMode:
        case kFocusMode:
            err = writeData(&cp, kCommandPacketSize);
            break;
        case kBaudChangeMode:
            SET_ADDRESS(&cp.address, kBaud9600Address);
            err = writeDataSlow(&cp, kCommandPacketSize);
            break;
        case kMemoHolderMode:
            SET_ADDRESS(&cp.address, kMemoHolderAddress);
            err = writeData(&cp, kCommandPacketSize);
            break;
        default:
            err = kUnknownModeErr;
            break;
    }

    if (err) {
        goto ERROR;
    }

    switch (mode) {
        case kReadDataMode:
            err = readDataPacket(cmdBuf, size);
            break;
        case kWriteDataMode:
        case kBaudChangeMode:
            err = readStatusPacket();
            break;
        case kShutterMode:
        case kFocusMode:
            // ???
            break;
        case kMemoHolderMode:
            err = readDataPacket(cmdBuf, kMemoHolderResponseSize);
            break;
        default:
            err = kUnknownModeErr;
            break;
    }

    if (err && (retry == false)) {
        retry = true;
        goto COMMAND_RETRY;
    }

    if (err) {
        goto ERROR;
    }

    if (mode == kBaudChangeMode) {
        int baudErr = sp_set_baudrate(serialPort, 9600);
        if (baudErr != SP_OK) {
            log_fatal("Error setting baudrate to 9600: %s", sp_last_error_message());
            goto ERROR;
        }
        usleep(200);
        log_info("Serial port is now configured for 9600bps");
    }

ERROR:
    return err;
}

/**
 * @brief Create a data packet
 * 
 * @param buf 
 * @param size 
 */
void NikonDatalink::makeDataPacket(unsigned char *buf, int size) {
    unsigned char *p;
    unsigned char cs;
    unsigned int count;
    DataPacketStart dstart;
    DataPacketStop dstop;

    count = size;
    p = (unsigned char *)buf;
    cs = 0;
    while (count--) {
        cs += *p++;
    }

    dstart.startMark = kDataPacketStartMark;
    dstop.checkByte = cs;
    dstop.stopMark = kDataPacketStopMark;

    p = (unsigned char *)serialBuffer;
    memmove(p, &dstart, kDataPacketStartSize);
    p += kDataPacketStartSize;
    memmove(p, buf, size);
    p+= size;
    memmove(p, &dstop, kDataPacketStopSize);
}

/**
 * @brief Read a data packet
 * 
 * @param buf 
 * @param size 
 * @return int 
 */
int NikonDatalink::readDataPacket(unsigned char *buf, int size) {
    int err;
    unsigned char *p;
    unsigned char cs;
    int count;

    err = readData(serialBuffer, size + kDataPacketStartSize + kDataPacketStopSize);
    if (err) {
        goto ERROR;
    }

    p = serialBuffer;
    if (((DataPacketStart *) p)->startMark != kDataPacketStartMark) {
        goto ERROR;
    }
    p += kDataPacketStartSize;
    cs = 0;
    count = size;
    while (count--) {
        cs += *p;
        *buf++ = *p++;
    }
    if (((DataPacketStop *) p)->checkByte != cs) {
        err = kPacketCSErr;
    } else if (((DataPacketStop *) p)->stopMark != kDataPacketStopMark) {
        err = kPacketSizeErr;
    }

    if (err) {
        goto ERROR;
    }

    return err;
ERROR:
    return err;
}

/**
 * @brief Read a status packet
 * 
 * @return int 
 */
int NikonDatalink::readStatusPacket() {
    StatusPacket ep;
    int err;

    err = readData(&ep, kStatusPacketSize);
    if (err) {
        goto ERROR;
    }

    if (ep.status != kStatusOK) {
        err = kPacketResponseErr;
    }

ERROR:
    return err;
}