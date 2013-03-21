rfidoor
=======

Arduino RFID door lock
I Heart Engineering (2013)
All rights reserved.


Copyright(c) Adafruit Industries regarding readMifare.pde
Credit to Bill Greiman regarding the SDFat Library


This is a code utilizing the power of both the Arduino Ethernet and the PN-532 breakout board.
It reads a MiFare ISO14443A card or tag which has a 4-byte UID and 16-bytes of data.

Using storage on an SD card, the Arduino checks if the UID exists on the SD card, and checks if the data matches.
If the card is accepted, it will power a relay, which opens a RCI S65 electromagnetic door lock.

Disclaimer:
No insurance will be granted to any users. If you use this program and your Arduino spontaneously combusts, it must be caused by your own account.
