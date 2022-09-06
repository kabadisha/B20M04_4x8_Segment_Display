#include "B20M04_4x8.h"
//avr pgmspace library for storing the LUT in program flash instead of sram
#include <avr/pgmspace.h>
// inslude the SPI library:
#include <SPI.h>

const String _CHARS = " 0123456789ABCDEFGHIJKLNOPQRSTUVXYZbcdghinortuv_-";

const byte _CHAR_SET[] PROGMEM = {
	B00000000, // = 
	B01111110, // = 0
	B00110000, // = 1
	B01101101, // = 2
	B01111001, // = 3
	B00110011, // = 4
	B01011011, // = 5
	B01011111, // = 6
	B01110000, // = 7
	B01111111, // = 8
	B01110011, // = 9
	B01110111, // = A
	B01111111, // = B
	B01001110, // = C
	B01111110, // = D
	B01001111, // = E
	B01000111, // = F
	B01011110, // = G
	B00110111, // = H
	B00110000, // = I
	B00111000, // = J
	B00110111, // = K
	B00001110, // = L
	B01110110, // = N
	B01111110, // = O
	B01100111, // = P
	B01110011, // = Q
	B01110111, // = R
	B01011011, // = S
	B00001111, // = T
	B00111110, // = U
	B00111110, // = V
	B00110111, // = X
	B00110011, // = Y
	B01101101, // = Z
	B00011111, // = b
	B00001101, // = c
	B00111101, // = d
	B01111011, // = g
	B00010111, // = h
	B00010000, // = i
	B00010101, // = n
	B00011101, // = o
	B00000101, // = r
	B00001111, // = t
	B00011100, // = u
	B00011100, // = v
	B00001000, // = _
	B00000001 // = -
};

/*
 * Constructor for the class
 */
B20M04_4x8::B20M04_4x8(int SLAVESELECT) {
	_SLAVESELECT = SLAVESELECT;
	_MAX_POSITIVE = pow(10, NO_DIGITS)-1; // 9999 for 4 segments
	_MAX_NEGATIVE = pow(-10, NO_DIGITS-1)+1; // -999 for 4 segments
	_AN1_ENABLED = false;
	_AN2_ENABLED = false;
	byte _BUFFER[BUFFER_SIZE];
	memset(_BUFFER, B00000000, BUFFER_SIZE);

	// Disable writing to the display
	digitalWrite(_SLAVESELECT, HIGH);

	// Initialise SPI:
  	SPI.begin();

  	// Initialise the display with the number zero.
	displayInteger(0);
}

/*
 * This function is called by both displayInteger and by displayDouble.
 * It contains functionality common to both:
 * - Right aligning short numbers.
 */
String B20M04_4x8::rightAlign(String input) {
	String inputMinusPoints = String(input);

	inputMinusPoints.remove(inputMinusPoints.indexOf('.'),1);
	
	int numLeadingSpaces = NO_DIGITS - inputMinusPoints.length();
	for (int i = 0; i < numLeadingSpaces; i++) {
		input = ' ' + input;
	}

	return input;
}

/*
 * This function applies upper and lower bounds to an integer and then 
 * converts to a string before displaying it.
 */
void B20M04_4x8::displayInteger(int input) {
	
	// Apply upper and lower bounds
	if (input > _MAX_POSITIVE) {
		displayText(String(_MAX_POSITIVE));
	} else if (input < _MAX_NEGATIVE) {
		displayText(String(_MAX_NEGATIVE));
	} else {
		displayText(rightAlign(String(input)));
	}
}

/*
 * This function applies upper and lower bounds to a double and then 
 * converts to a string before displaying it.
 */
void B20M04_4x8::displayDouble(double input) {
	
	// Apply upper and lower bounds
	if (input > _MAX_POSITIVE) {
		displayText(String(_MAX_POSITIVE));
	} else if (input < _MAX_NEGATIVE) {
		displayText(String(_MAX_NEGATIVE));
	} else {
		displayText(rightAlign(String(input)));
	}
}

/*
 * This function applies upper and lower bounds to a float and then 
 * converts to a string before displaying it.
 */
void B20M04_4x8::displayFloat(float input) {
	
	// Apply upper and lower bounds
	if (input > _MAX_POSITIVE) {
		displayText(String(_MAX_POSITIVE));
	} else if (input < _MAX_NEGATIVE) {
		displayText(String(_MAX_NEGATIVE));
	} else {
		char stringVal[NO_DIGITS + 1];

		dtostrf(input, NO_DIGITS + 1, 3, stringVal);

		displayText(rightAlign(stringVal));
	}
}

/*
 * This function calculates the component digits of a string and then delegates displaying them.
 */
void B20M04_4x8::displayText(String input) {
	
	// Reset the buffer to empty
	memset(_BUFFER, B00000000, BUFFER_SIZE);

	/*
	 * Separate all double full stops (..) with a space, as that is how it will be represented
	 * on the display.
	 */
	while (input.indexOf("..") > -1) {
		input.replace("..", ". .");
	}

	/*
	 * If the input starts with a full stop, then prefix it with a space as that is how it will
	 * be represented on the display.
	 */
	if (input.startsWith(".")) {
		input = ' ' + input;
	}

	/*
	 * We only loop once for each digit available. If the input string is too long it will be
	 * truncated as a result.
	 */
	for (int i = 0; i < NO_DIGITS; i++) {
		char thisChar = input.charAt(i);
		int charIndex = _CHARS.indexOf(thisChar);
		
		/*
		 * Here we calculate which byte this character needs to be in. Since the right most digit
		 * on the display is handled by the first byte, the order of the characters is effectively
		 * reversed in the buffer array.
		 */
		int byteIndex = (NO_DIGITS - 1) - i;


		if (thisChar == 0 || charIndex == -1) {
			/*
			 * If the string is shorter than the number of segments, thisChar will be null (zero)
			 * and we replace it with a space. This adds trailing whitespace to any input string
			 * as required.
			 * If the character is not supported, we also leave a blank space
			 */
			charIndex = _CHARS.indexOf(' ');
		}

		byte character = pgm_read_byte(&_CHAR_SET[charIndex]); // fetch character from program memory

		if ('.' == input.charAt(i + 1)) {
			/*
			 * If the next character is a full stop, then we need to add decimal point to the
			 * previous digit. Counter-intuitively, this requires adding 1, not subtracting 1
			 * due to the reversed nature of the display where the first byte represents the right
			 * most digit of the display.
			 *
			 * We then remove the  full stop from the string as we are dealing with it now and
			 * don't want it to count as a digit in it's own right.
			 */
			bitSet(_BUFFER[byteIndex + 1],7);
			input.remove(i + 1, 1);
		}

		/*
		 * The first character has to go in the 4th byte, and the last character goes in the first byte.
		 */
		_BUFFER[byteIndex] = character;
	}

	renderBuffer();
}

void B20M04_4x8::renderBuffer() {
	/*
	 * We must set the start bit, which is the most significant bit of the first byte, to 1.
	 */
	bitSet(_BUFFER[0],7);

	if (_AN1_ENABLED) {
		bitSet(_BUFFER[BUFFER_SIZE - 1], 6);
	} else {
		bitClear(_BUFFER[BUFFER_SIZE - 1], 6);
	}

	if (_AN2_ENABLED) {
		bitSet(_BUFFER[BUFFER_SIZE - 1], 5);
	} else {
		bitClear(_BUFFER[BUFFER_SIZE - 1], 5);
	}

	SPI.beginTransaction(SPISettings(500000, MSBFIRST, SPI_MODE0));
	
	// take the SS pin low to select the chip:
	digitalWrite(_SLAVESELECT, LOW);
	
	//  send in the address and value via SPI:
	SPI.transfer(_BUFFER, BUFFER_SIZE);
	
	// take the SS pin high to de-select the chip:
	digitalWrite(_SLAVESELECT, HIGH);
	
	SPI.endTransaction();
}

void B20M04_4x8::toggleAN1(bool enabled) {
	_AN1_ENABLED = enabled;
	renderBuffer();
}

void B20M04_4x8::toggleAN2(bool enabled) {
	_AN2_ENABLED = enabled;
	renderBuffer();
}
