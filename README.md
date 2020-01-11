# PIKON

Work in progress.

Working:
- Nikon INQUIRY (camera identification)

Broken/Non working:
- Switch to 9600bps (don't work)
- Sending focusing command (segfault, might be related to baud switch)

# Protocol
- http://www.avernus.com/~gadams/photography/nikon/datalink-protocol/N90s-protocol-example.txt
- http://www.avernus.com/~gadams/photography/nikon/datalink-protocol/N90s-protocol.txt
- http://www.avernus.com/~gadams/photography/nikon/datalink-protocol/

# Dependencies
- https://sigrok.org/api/libserialport/unstable/index.html

# TODO
- Segfault at some point

# Copything
Based on `N90 Buddy for Palm`.
