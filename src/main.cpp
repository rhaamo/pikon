#include "nikonDatalink.h"

#include <stdio.h>

int main(int argc, char **argv) {
    NikonDatalink dl("/dev/ttyUSB0", 1200);

    int err = dl.serialOpen();
    if (err) {
        printf("Error opening serial port.\n");
        return err;
    }

    printf("Serial port opened: %s\n", dl.serialPortName);

    err = dl.identifyCamera();
    if (err) {
        printf("Error identifying camera\n");
        return err;
    }

    if (dl.getCameraType() == cameraN90) {
        printf("Camera is a N90\n");
    } else if (dl.getCameraType() == cameraN90s) {
        printf("Camera is a N90s\n");
    } else {
        printf("Camera is unknown\n");
    }

    // do stuff
    return 0;
}