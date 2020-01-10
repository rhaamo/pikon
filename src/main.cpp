#include "nikonDatalink.h"

#include <stdio.h>
#include <getopt.h>

int main(int argc, char **argv) {
    const char *getoptPort = "/dev/tty.usbserial-00000000";

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

    NikonDatalink dl(getoptPort);

    dl.setLogLevel(LOG_DEBUG);

    dl.startSession();

    // Camera focus
    dl.focus();

    dl.endSession();
    return 0;
}