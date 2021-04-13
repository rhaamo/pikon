# PIKON

Work in progress set of tools to use the Nikon DataLink present on the Nikon N90/N90s (F90/F90x).

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

## Features
### Working

- Nikon INQUIRY (camera identification)

### Broken
- Switch to 9600bps (don't work)
- Sending focusing command (segfault, might be related to baud switch)
- Everything else

## Resources

See the folder `docs/`.
- Text files about the DataLink protocol (packet format, etc.)
- `N90 Buddy for Palm Souce` source code for a PalmOS 3.x app
- `gicon-0.1.2` source code for a very old (gtk1-era) linux app, uses `tty` for serial comm

## Example

Example doing camera identification (N90) and failing on doing a focus command.

```
nikon@ubuntu-shared:~/pikon_new$ ./pikon -p /dev/ttyUSB0
09:17:04 INFO  src/nikonDatalink.cpp:36: Starting session.
09:17:04 INFO  src/nikonDatalink.cpp:142: libserialport version: 0.1.1
09:17:04 INFO  src/nikonDatalink.cpp:143: Port name: /dev/ttyUSB0
09:17:04 INFO  src/nikonDatalink.cpp:144: Description: FT232R USB UART - A50285BI
09:17:04 INFO  src/nikonDatalink.cpp:178: Identifying camera...
09:17:04 INFO  src/nikonDatalink.cpp:179: Sending wakeup string...
09:17:04 DEBUG src/nikonDatalink.cpp:262: writeDataSlow: '', size: 1
09:17:04 INFO  src/nikonDatalink.cpp:190: Sending nikon inquiry string...
09:17:04 DEBUG src/nikonDatalink.cpp:296: writeData: 'S1000'
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: '�', read: 0, expected: 1, err: -1
09:17:06 INFO  src/nikonDatalink.cpp:190: Sending nikon inquiry string...
09:17:06 DEBUG src/nikonDatalink.cpp:296: writeData: 'S1000'
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: '1', read: 1, expected: 1, err: 0
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: '0', read: 1, expected: 1, err: 0
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: '1', read: 1, expected: 1, err: 0
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: '0�', read: 1, expected: 1, err: 0
09:17:06 DEBUG src/nikonDatalink.cpp:349: readData: 'F', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '9', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '0', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '/', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: 'N', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '9', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '0', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: '', read: 1, expected: 1, err: 0
09:17:07 DEBUG src/nikonDatalink.cpp:349: readData: 'W', read: 2, expected: 2, err: 0
09:17:07 INFO  src/nikonDatalink.cpp:55: Camera is a N90
09:17:07 ERROR src/nikonDatalink.cpp:360: baudrateChange not enabled or camera type unknown. cannot switch baudrate
09:17:07 ERROR src/nikonDatalink.cpp:68: Baudrate switch failed
09:17:07 FATAL src/nikonDatalink.cpp:76: SessionError: -1
09:17:07 INFO  src/nikonDatalink.cpp:88: Ending session.
09:17:07 FATAL src/nikonDatalink.cpp:292: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:325: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:292: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:325: SessionError: -1
09:17:07 INFO  src/nikonDatalink.cpp:638: Triggering focus
09:17:07 DEBUG src/nikonDatalink.cpp:386: Sending command, mode: 0x80, address: 64825, buffer:, size: 1
09:17:07 DEBUG src/nikonDatalink.cpp:425: Sending command loop, mode: 0x80, address: 64825, buffer:, size: 1
09:17:07 DEBUG src/nikonDatalink.cpp:296: writeData: '�'
09:17:07 ERROR src/nikonDatalink.cpp:303: Cannot write buffer: Invalid argument
09:17:07 FATAL src/nikonDatalink.cpp:382: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:382: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:382: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:382: SessionError: -1
09:17:07 INFO  src/nikonDatalink.cpp:88: Ending session.
09:17:07 FATAL src/nikonDatalink.cpp:292: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:325: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:292: SessionError: -1
09:17:07 FATAL src/nikonDatalink.cpp:325: SessionError: -1
nikon@ubuntu-shared:~/pikon_new$
```

# Contact
dashie (at) otter (dot) sh
