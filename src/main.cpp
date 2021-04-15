#include "nikonDatalink.h"

#include <stdio.h>
#include <CLI/CLI.hpp>

struct Options {
    std::string serialPort;
    bool debug;
};

void identify_camera(Options const &opt);
void focus(Options const &opt);
void fireShutter(Options const &opt);

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