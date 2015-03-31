A Raspberry Pi based interface to the M24SR Dynamic NFC tag
====

This is a placeholder for some code that I am writing.

This code will run on a Raspberry pi, and interface via I2C to a ST Microelectronics M24SR Dynamic NFC tag.

These tags are great, you can write NDEF records to them, and then placing a modern smartphone on top will
redirect the phone to various destinations, such as URI's, phone numbers, etc.

It is possible to program the records from within a smartphone app.

The M24SR has energy harvesting, so a small amount of power can be drawn from it while it
is energised.

The M24SR also has an I2C host interface, so that a host processor (in this case a Raspberry Pi)
can read and write the contents of the records.

This is a great way to transfer small amounts of data.

David Whale

@whaleygeek

March 2015
