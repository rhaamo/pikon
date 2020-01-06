#include "nikonDatalink.h"

#include <stdio.h>
#include <getopt.h>

int main(int argc, char **argv) {
    const char *getoptPort = "/dev/ttyUSB0";

    for (;;) {
        switch(getopt(argc, argv, "hp:")) {
            case '?':
            case 'h':
                printf("Nikon N90/N90s toolbox.\n");
                printf("Usage: %s [OPTION]...\n", argv[0]);
                printf(" -h        This help\n");
                printf(" -p PORT   Serial port to use, default: /dev/ttyUSB0\n");
                return -1;
            case 'p':
                getoptPort = optarg;
                continue;
            default:
                break;
        }
        break;
    }

    NikonDatalink dl(getoptPort, 1200);

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