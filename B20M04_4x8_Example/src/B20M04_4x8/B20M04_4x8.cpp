#include "B20M04_4x8.h"
//avr pgmspace library for storing the LUT in program flash instead of sram
#include <avr/pgmspace.h>

// This is the full list of supported characters
// Full-stops/periods/decimal points are handled by adding one to the previous
// Byte before it is sent to the display. Notice how all the bytes below end
// with a zero. That's the decimal point bit.
const String _VALID_CHARS = " 0123456789ABCDEFGHIJKLNOPQRSTUVXYZbcdghinortuv_-";

const byte _CHAR_SET[] PROGMEM = {
	B00000000, // =
	B11111100, // = 0
	B01100000, // = 1
	B11011010, // = 2
	B11110010, // = 3
	B01100110, // = 4
	B10110110, // = 5
	B10111110, // = 6
	B11100000, // = 7
	B11111110, // = 8
	B11100110, // = 9
	B11101110, // = A
	B11111110, // = B
	B10011100, // = C
	B11111100, // = D
	B10011110, // = E
	B10001110, // = F
	B10111100, // = G
	B01101110, // = H
	B01100000, // = I
	B01110000, // = J
	B01101110, // = K
	B00011100, // = L
	B11101100, // = N
	B11111100, // = O
	B11001110, // = P
	B11100110, // = Q
	B11101110, // = R
	B10110110, // = S
	B00011110, // = T
	B01111100, // = U
	B01111100, // = V
	B01101110, // = X
	B01100110, // = Y
	B11011010, // = Z
	B00111110, // = b
	B00011010, // = c
	B01111010, // = d
	B11110110, // = g
	B00101110, // = h
	B00100000, // = i
	B00101010, // = n
	B00111010, // = o
	B00001010, // = r
	B00011110, // = t
	B00111000, // = u
	B00111000, // = v
	B00010000, // = _
	B00000010  // = -
};

/*
 * Constructor for the class
 */
B20M04_4x8::B20M04_4x8(int SLAVESELECT_PIN, int MOSI_PIN, int CLOCK_PIN) {
	_SLAVESELECT_PIN = SLAVESELECT_PIN;
	_MOSI_PIN = MOSI_PIN;
	_CLOCK_PIN = CLOCK_PIN;
	_MAX_POSITIVE = pow(10, NO_DIGITS)-1; // 9999 for 4 segments
	_MAX_NEGATIVE = pow(-10, NO_DIGITS-1)+1; // -999 for 4 segments
	_AN1_ENABLED = false;
	_AN2_ENABLED = false;

  pinMode(_SLAVESELECT_PIN, OUTPUT);
  pinMode(MOSI_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

	digitalWrite(_SLAVESELECT_PIN, HIGH);  // Start with SS high
	digitalWrite(MOSI_PIN, HIGH);
  digitalWrite(CLOCK_PIN, LOW);  // SCK low

  // Initialise the display blank.
	clearDisplay();
}

// This will clear the display and reset the cursor
void B20M04_4x8::clearDisplay() {
	_AN1_ENABLED = false;
	_AN2_ENABLED = false;
	displayText("    ");
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
	 * We must send the start bit as 1.
	 */
	sendBit(HIGH);

	/*
	 * We only loop once for each digit available. If the input string is too long it will be
	 * truncated as a result.
	 * Due to the reversed nature of the display where the first byte represents the right
	 * most digit of the display, we have to add digits to a buffer, then iterate over that
	 * buffer to update the display.
	 */
	byte _buffer[NO_DIGITS];
	for (int i = 0; i < NO_DIGITS; i++) {
		char thisChar = input.charAt(i);
		int charIndex = _VALID_CHARS.indexOf(thisChar);

		/*
		 * Here we calculate which byte this character needs to be in. Since the right most digit
		 * on the display is handled by the first byte, the order of the characters is effectively
		 * reversed.
		 */
		int byteIndex = (NO_DIGITS - 1) - i;


		if (thisChar == 0 || charIndex == -1) {
			/*
			 * If the string is shorter than the number of segments, thisChar will be null (zero)
			 * and we replace it with a space. This adds trailing whitespace to any input string
			 * as required.
			 * If the character is not supported, we also leave a blank space
			 */
			charIndex = _VALID_CHARS.indexOf(' ');
		}

		byte character = pgm_read_byte(&_CHAR_SET[charIndex]); // fetch character from program memory

		if ('.' == input.charAt(i + 1)) {
			/*
			 * If the next character is a full stop, then we need to add decimal point to this digit
			 * by setting bit zero (least-significant, rightmost bit) to 1.
			 * We then remove the  full stop from the string as we are dealing with it now and
			 * don't want it to count as a digit in it's own right.
			 */
			bitSet(character, 0);
			input.remove(i + 1, 1);
		}
		_buffer[byteIndex] = character;
	}

	// Now we output the buffered display value.
	for (int i = 0; i < NO_DIGITS; i++) {
		sendByte(_buffer[i]);
	}

	if (_AN1_ENABLED) {
		sendBit(HIGH);
	} else {
		sendBit(LOW);
	}

	if (_AN2_ENABLED) {
		sendBit(HIGH);
	} else {
		sendBit(LOW);
	}
	sendBit(LOW); // Send the final, closing bit.
}

void B20M04_4x8::sendBit(int _value)  // Bitbang a single bit
{
  digitalWrite(_SLAVESELECT_PIN, LOW); // SlaveSelect low

  digitalWrite(_MOSI_PIN, _value);
  digitalWrite(_CLOCK_PIN, HIGH);
  digitalWrite(_CLOCK_PIN, LOW);

  digitalWrite(_SLAVESELECT_PIN, HIGH);  // SlaveSelect high again
}

void B20M04_4x8::sendByte(byte _send)  // Bitbang a while byte
{
  digitalWrite(_SLAVESELECT_PIN, LOW);  // SlaveSelect low

  for(int i=7; i >= 0; i--)  // There are 8 bits in a byte
  {
    digitalWrite(_MOSI_PIN, bitRead(_send, i));    // Set MOSI
    digitalWrite(_CLOCK_PIN, HIGH);                // SCK high
    digitalWrite(_CLOCK_PIN, LOW);                 // SCK low
  }

  digitalWrite(_SLAVESELECT_PIN, HIGH);  // SlaveSelect high again
}

void B20M04_4x8::toggleAN1(bool enabled) {
	_AN1_ENABLED = enabled;
}

void B20M04_4x8::toggleAN2(bool enabled) {
	_AN2_ENABLED = enabled;
}
