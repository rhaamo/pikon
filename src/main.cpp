#include "nikonDatalink.h"

#include <stdio.h>
#include <unistd.h>
#include <CLI/CLI.hpp>

struct Options {
    std::string serialPort;
    bool debug;
};

void identify_camera(Options const &opt);
void focus(Options const &opt);
void fireShutter(Options const &opt);
void getCameraSettings(Options const &opt);

int main(int argc, char **argv) {
    CLI::App app("Pikon DataLink");
    auto opts = std::make_shared<Options>();

    app.add_flag("-d,--debug", opts->debug, "Verbose logging");
    auto optSerialPort = app.add_option("-p,--port", opts->serialPort, "Serial port to use, like /dev/ttyUSB0");
    // for when https://github.com/CLIUtils/CLI11/pull/497 lands in a release
    // optSerialPort->option_text("PORT");
    optSerialPort->default_val("/dev/ttyUSB0");

    CLI::App *cmdIdentify = app.add_subcommand("identify", "Identify camera");
    cmdIdentify->callback([opts]() { identify_camera(*opts); });

    CLI::App *cmdFocus = app.add_subcommand("focus", "trigger focus");
    cmdFocus->callback([opts]() { focus(*opts); });

    CLI::App *cmdFireShutter = app.add_subcommand("fire_shutter", "fire shutter");
    cmdFireShutter->callback([opts]() { fireShutter(*opts); });

    CLI::App *cmdGetSettings = app.add_subcommand("get_settings", "get settings");
    cmdGetSettings->callback([opts]() { getCameraSettings(*opts); });


    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);

}

void identify_camera(Options const &opt) {
    NikonDatalink dl(opt.serialPort);

    if (opt.debug) {
        dl.setLogLevel(LOG_DEBUG);
    } else {
        dl.setLogLevel(LOG_INFO);
    }

    dl.startSession();

    dl.endSession();
}

void focus(Options const &opt) {
    NikonDatalink dl(opt.serialPort);

    if (opt.debug) {
        dl.setLogLevel(LOG_DEBUG);
    } else {
        dl.setLogLevel(LOG_INFO);
    }

    dl.startSession();

    //dl.switchBaudrate();
    dl.focus();

    dl.endSession();
} 

void fireShutter(Options const &opt) {
    NikonDatalink dl(opt.serialPort);

    if (opt.debug) {
        dl.setLogLevel(LOG_DEBUG);
    } else {
        dl.setLogLevel(LOG_INFO);
    }

    dl.startSession();

    //dl.switchBaudrate();
    dl.fireShutter();

    dl.endSession();
} 

void lensInfos(unsigned char lensId, unsigned char focalMin, unsigned char focalMax, unsigned char apMin, unsigned char apMax) {
	char *focalMinStr, *focalMaxStr, *apMinStr, *apMaxStr;

    std::string lensStr = std::to_string(lensId);
    focalMinStr = GetStringTable(kFocalTable, focalMin);
	focalMaxStr = GetStringTable(kFocalTable, focalMax);
	apMinStr = GetStringTable(kCameraApertureTable, apMin);
	apMaxStr = GetStringTable(kCameraApertureTable, apMax);

    if (focalMin == focalMax) {
        printf("Lens: id: %s - focal min: %s - ap min: %s\r\n", lensStr.c_str(), focalMinStr, apMinStr);
    } else if (apMin == apMax) {
        printf("Lens: id: %s - focal min: %s - focal max: %s - ap min: %s\r\n", lensStr.c_str(), focalMinStr, focalMaxStr, apMinStr);
    } else {
        printf("Lens: id: %s - focal min: %s - focal max: %s - ap min: %s - ap max: %s\r\n", lensStr.c_str(), focalMinStr, focalMaxStr, apMinStr, apMaxStr);
    }
}

void getCameraSettings(Options const &opt) {
    NikonDatalink dl(opt.serialPort);

    if (opt.debug) {
        dl.setLogLevel(LOG_DEBUG);
    } else {
        dl.setLogLevel(LOG_INFO);
    }

    dl.startSession();

    dl.switchBaudrate();

    if (dl.getCameraType() == CameraType::cameraN90s) {
        printf("Camera type: F90x/N90s\r\n");
    } else {
        printf("Camera type: F90/N90\r\n");
    }

    CameraControlGlobals *cameraControls = NULL;
    cameraControls = dl.getCameraSettings();

    if (cameraControls->locationFD21 == 0) {
        printf("No film present\r\n");
    } else {
        printf("Film present, frame %i\r\n",  cameraControls->locationFD21 - 1);
    }

    if (dl.getCameraType() == CameraType::cameraN90s) {
        char *iso = GetStringTable(kISOTable, cameraControls->locationFD90);
        printf("ISO: %s\r\n", iso);
        lensInfos(cameraControls->locationFE32, cameraControls->locationFE2C,
					cameraControls->locationFE2D, cameraControls->locationFE2E, cameraControls->locationFE2F);
        char *focalLength = GetStringTable(kFocalTable, cameraControls->locationFE2E);
        printf("Focale: %s\r\n", focalLength);
    } else if (dl.getCameraType() == CameraType::cameraN90) {
        char *iso = GetStringTable(kISOTable, cameraControls->locationFD9D);
        printf("ISO: %s\r\n", iso);
        lensInfos(cameraControls->locationFE32, cameraControls->locationFE2C,
					cameraControls->locationFE2D, cameraControls->locationFE2E, cameraControls->locationFE2F);
        char *focalLength = GetStringTable(kFocalTable, cameraControls->locationFE2B);
        printf("Focale: %s\r\n", focalLength);

    }

    // if (cameraControls->locationFE51 > NULL) max aperture etc.
    

    dl.endSession();
}
