# PIKON

Work in progress set of tools to use the Nikon DataLink remote control protocol present on the Nikon N90/N90s (F90/F90x).

## Installation

Install the `libserialport` (https://sigrok.org/api/libserialport/unstable/index.html) lib on your computer, then checkout the repository:

```
git clone --recursive https://github.com/rhaamo/pikon/
# or
git clone https://github.com/rhaamo/pikon/
cd pikon
git submodule init
git submodule update
```

## Build

Using GCC:
```
make
```

Using Clang:
```
make CC=clang++ CXX=clang++
```

## Features
### Working

- Nikon INQUIRY (camera identification)
- Fire shutter (tested on N90s)

### Broken
- Switch to 9600bps (don't work)
- Sending focusing command (does nothing, sessionError)
- Everything else

### TODO
- Focus, commands, etc. (might be N90s only, waiting to receive it for testing)
- Export of the 'exif-like' datas (the memo I think)
- Setting various camera infos (exposure, ISO, etc.)

## Resources

See the folder `docs/`.
- Text files about the DataLink protocol (packet format, etc.)
- `N90 Buddy for Palm Souce` source code for a PalmOS 3.x app
- `gicon-0.1.2` source code for a very old (gtk1-era) linux app, uses `tty` for serial comm

## Example

Example doing camera identification (N90) and triggering the shutter.

```
nikon@ubuntu-shared:~/pikon$ ./pikon fire_shutter
11:47:17 INFO  src/nikonDatalink.cpp:36: Starting session.
11:47:17 INFO  src/nikonDatalink.cpp:133: libserialport version: 0.1.1
11:47:17 INFO  src/nikonDatalink.cpp:134: Port name: /dev/ttyUSB0
11:47:17 INFO  src/nikonDatalink.cpp:135: Description: FT232R USB UART - A50285BI
11:47:17 INFO  src/nikonDatalink.cpp:169: Identifying camera...
11:47:17 INFO  src/nikonDatalink.cpp:170: Sending wakeup string...
11:47:17 INFO  src/nikonDatalink.cpp:181: Sending nikon inquiry string...
11:47:19 INFO  src/nikonDatalink.cpp:181: Sending nikon inquiry string...
11:47:19 INFO  src/nikonDatalink.cpp:57: Camera is a N90s/F90x
11:47:19 INFO  src/nikonDatalink.cpp:644: Triggering shutter
11:47:19 INFO  src/nikonDatalink.cpp:79: Ending session.
nikon@ubuntu-shared:~/pikon$
```

# Contact
dashie (at) otter (dot) sh

# Copyright

Everything under docs/ are under their own (GPL2 for gicon, non-specified for the rest) and out of the scope of the utils.

Everything under ext/ are external libraries not packaged that we uses.

For our code:
- MIT licensed
- Some files has been used from 'N90 Buddy for Palm' by Ken Hancock
