A Raspberry Pi based interface to the M24SR Dynamic NFC tag
====

This project is a dumping ground for some (in progress) code I am writing
for the ST Microelectronics M24SR Dynamic NFC Tag.

Prototypes and hardware test code for both RaspberryPi and Arduino
can be found here. They are in no way "neat" programs at the moment,
they are mainly just to excercise some test hardware and test that
NDEF URI's can be read-back from a dynamic tag.

On the Arduino Uno, just plug the X-NUCLEO-NFC01A1 evaluation board into
the socket, load the code, then wire up a serial terminal to see the
responses. My tag has a URI programmed into it, that was set by some
initial testing I did with some mbed code on a ST NUCLEO board at
a workshop.


Dynamic NFC Tags
----

These tags are great, you can write NDEF records to them, and then placing a modern 
smartphone on top will redirect the phone to various destinations, such as URI's, 
phone numbers, etc.

It is possible to program the records from within a smartphone app.

The M24SR has energy harvesting, so a small amount of power can be drawn from it while it
is energised.

The M24SR also has an I2C host interface, so that a host processor (in this case a Raspberry Pi)
can read and write the contents of the records.

This is a great way to transfer small amounts of data.


Wiring Up
----

Here is a rough braindump of how to read a NDEF record out of a M24SR
on a Raspberry Pi, with the ST M24SR X-Nucleo-NFC01A1 board.

Using Raspberry Pi Model B Rev 2.

GPIO 2 (SDA) goes to header CN1 pin 9

GPIO 3 (SCL) goes to header CN1 pin 10

3v3 goes to header CN2 pin 4

0V goes to header CN2 pin 7

The Nucleo eval board has 4k7 pullup resistors on SDA/SCL already.
The Matrix boards don't have pullups fitted so you have to fit them yourself.


Running it
----

sudo python m242r.py

Press return at each prompt.

When you get to the end, it assumes a valid URI NDEF record, and displays
the printable part of that NDEF record on the screen. If you don't have a
URI NDEF programmed, you might see garbage.


I2C addressing
----

I2C addresses are 7 bits (in b7..b1) with the R/W bit in b0.
Some I2C libraries take a 7 bit address, some take 8. The 7 bit
libraries shift the address you provide it left by one and append
the R/W bit.

For an 8 bit address, use 0xAC for write, 0xAD for read.

For a 7 bit address, use 0x56 and let the library add the R/W bit for you.



Useful sequences
----

These are better explained in the NDEF.ino file for the Arduino.
Most of these sequences can just be captured and send with no
modification. The ReadNDEFMessageLen is required to get the length of the
NDEF message, and you then have to adjust both the length byte and the CRC
inside ReadNDEFMessage for the device to respond correctly.


kill RF, select I2C: AC 52

select NFC application: AC 02 00 A4 04 00 07 D2 76 00 00 85 01 01 00 35 C0

select CC file: AC 03 00 A4 00 0C 02 E1 03 D2 AF

read CC file len: AC 02 00 B0 00 00 02 6B 7D

read CC file: AC 03 00 B0 00 00 0F A5 A2

select NDEF file: AC 02 00 A4 00 0C 02 00 01 3E FD

read NDEF message len: AC 03 00 B0 00 00 02 40 79

read NDEF message: AC 02 00 B0 00 02 0C A5 A7

deselect: AC C2 E0 B4

Note that when you send the read NDEF message len, the length of the NDEF message
comes back in the payload. In the example above, that length is sent to the next
read NDEF message command as 0C, as that is the value that came back in my tests
from my NDEF record - but you must not use a length any longer than the value that
comes back from the read NDEF message len (otherwise you'll get an error back).
Also note if the length is any different from 0C, you'll have to recalculate the
CRC (last two bytes) at the end of that request as they will be different. This is
the only tx sequence that would vary based on data stored in the card - i.e. all
the other messages you can probably just send from pre-stored messages, but this
one must be computed dynamically.

The NDEF record returned from my card came back as follows:

02 D1 01 08 55 01 61 62 63 2e 63 6f 6d 90 00 2c 42

The 02 D1 01 08 55 01 is part of the protocol header.

The 61 62 63 2e 63 6f 6d is "abc.com" (the URI I programmed)

The 90 00 2c 42 is the rest of the protocol wrapper (last 2 bytes CRC will vary
based on contents of payload)


Protocol Control Byte
----

In the sequences above, the first byte after the AC address is the PCB. This 
will toggle between 02 and 03 alternatively both from your end as the master,
and the NFC tag as the receiver - it is to detect out of sequence messages, 
and must be toggled on each message you send, for the message to be received
and processed.

You can probably ignore the returned CRC if you are short of implementation
time but it provides you with a way to verify the message is intact.



Useful code
----

If you are porting this to C (e.g. arduino), the c_proto folder has a crc.c file
that has a C implementation of the CRC generator. For most of the sequences you can
probably get away with hard coding the CRC, but the one that will vary is the
"read NDEF file" as it needs a valid length byte, and changing the length byte
will change the CRC for that frame.


Gemma
----

This now works on the Gemma. You can load in LED flash patterns over the NFC TEXT
record (ASCII '1'..'9' sets delay time in 100ms increments, the LED toggles
alternate ON and OFF using those flash times).

Use project arduino_proto/NFC_gemma

Note however you need to hack the TinyWireM library to extend the buffer size for
longer messages, there is a (1+5+2+4)=12 byte protocol overhead for the NFC
data record, so you need at least 12 bytes plus your longest NFC TEXT string
to prevent corruptions occuring inside the TinyWireM I2C driver:

//#define USI_BUF_SIZE    18              // bytes in message buffer

#define USI_BUF_SIZE     128

GetTinyWireM from here:

https://github.com/adafruit/TinyWireM





David Whale

@whaleygeek

March 2015
