/*
 * Due to an oddity of the Arduino IDE, we have to also include the SPI library manually,
 * despite the fact that the B20M04_4x8 already includes it.
 */
#include <SPI.h>

#include <B20M04_4x8.h>

// Create an instance of the display with pin 10 as the SLAVESELECT pin.
B20M04_4x8 disp(10);

void setup()
{

}

void loop()
{

	disp.toggleAN1(false);
	disp.toggleAN2(false);

	disp.displayText("ABCD");
	delay(2000);

	disp.displayText("ABC");
	delay(2000);

	disp.toggleAN1(true);
	disp.toggleAN2(true);

	disp.displayText("-3.24");
	delay(2000);

	disp.displayInteger(-2);
	delay(2000);

	disp.displayInteger(12345);
	delay(2000);

	disp.displayDouble(1.2367);
	delay(2000);

	disp.displayFloat(12.2367);
	delay(2000);

	disp.displayFloat(-3.2367);
	delay(2000);

	disp.displayText("BOOP");
	delay(2000);

	disp.displayText("    ");
	delay(2000);

	for (int i=0; i < 10; i++) {
		disp.displayText("BANG");
		delay(100);
		disp.displayText("    ");
		delay(100);
	}

}


