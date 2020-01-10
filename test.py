import serial
import time
import sys

port = "/dev/tty.usbserial-00000000"

# 1200 BAUD
ser = serial.Serial(port, 1200, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, write_timeout=2000, timeout=2000)

print(f"Using {port}")

# Send wakeup 'null byte' \x00
ser.write(b'\x00')

ser.flush()

# Wait 200ms
time.sleep(.200)

# Send unit inquiry "S1000."
# \x53 \x31 \x30 \x30 \x30 \x05
inquiry = b'S1000\x05'
sent = ser.write(inquiry)
time.sleep(.1)
a= ser.read(size=13)
print(f"Sent {sent} bytes ({inquiry!r})")
print(f"Read: {a!r} {len(a)} bytes")

if b"1010F90/N90" in a:
    print("N90 detected")
elif b"1020F90X/N90S" in a:
    print("N90s detected")
else:
    print("WTF ?")
    sys.exit()

"""
Sometimes it returns \x15\x83 before the inquiry answer
"""

# request to switch to 9600 bps
ser.write(b"\x01\x20\x87\x05\x00\x00\x00\x00")
a = ser.read(size=2)
print(f"Switch to 9600 response (should be \\x06\\x00): {a!r}")

# 06 00 == ok
# 06 15
# 00 W

time.sleep(.200)

ser.baudrate = 9600

# Send command for focus camera
ser.write(b'\x02\x20\x86\x00\x00\x00\x00\x00\x03')

# signoff
ser.write(b'\x04\x04')

print(ser.read(size=2))