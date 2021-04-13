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

# Contact
dashie (at) otter (dot) sh
