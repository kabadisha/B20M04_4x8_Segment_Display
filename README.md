# B20M04_4x8_Segment_Display
An Arduino library for driving the B20M04 4 digit, 8 segment display.

It supports the following characters:
`<space>0123456789ABCDEFGHIJKLNOPQRSTUVXYZbcdghinortuv_-.`

It also:
- handles left-aligning of text and right-aligning of numbers.
- handles rounding of decimals to fit on the display
- handles negative numbers

It uses bit-banging rather than SPI because the display explicitly requires 36 bits ONLY to be sent for each update and the SPI libraries can only send complete bytes, not individual bits.

I tried to make it work with SPI and got it pretty much working, but it would get out of sync and end up garbling the display.
