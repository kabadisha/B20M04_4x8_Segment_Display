#include "Arduino.h"
#include "B20M04_4x8.h"

static String _CHARS = " 0123456789ABCDEFGHIJKLNOPQRSTUVXYZbcdghinortuv_-";

static int _CHAR_BYTES[][NO_SEGMENTS] = {
	{0,0,0,0,0,0,0,0}, // = 
	{1,1,1,1,1,1,0,0}, // = 0
	{0,1,1,0,0,0,0,0}, // = 1
	{1,1,0,1,1,0,1,0}, // = 2
	{1,1,1,1,0,0,1,0}, // = 3
	{0,1,1,0,0,1,1,0}, // = 4
	{1,0,1,1,0,1,1,0}, // = 5
	{1,0,1,1,1,1,1,0}, // = 6
	{1,1,1,0,0,0,0,0}, // = 7
	{1,1,1,1,1,1,1,0}, // = 8
	{1,1,1,0,0,1,1,0}, // = 9
	{1,1,1,0,1,1,1,0}, // = A
	{1,1,1,1,1,1,1,0}, // = B
	{1,0,0,1,1,1,0,0}, // = C
	{1,1,1,1,1,1,0,0}, // = D
	{1,0,0,1,1,1,1,0}, // = E
	{1,0,0,0,1,1,1,0}, // = F
	{1,0,1,1,1,1,0,0}, // = G
	{0,1,1,0,1,1,1,0}, // = H
	{0,1,1,0,0,0,0,0}, // = I
	{0,1,1,1,0,0,0,0}, // = J
	{0,1,1,0,1,1,1,0}, // = K
	{0,0,0,1,1,1,0,0}, // = L
	{1,1,1,0,1,1,0,0}, // = N
	{1,1,1,1,1,1,0,0}, // = O
	{1,1,0,0,1,1,1,0}, // = P
	{1,1,1,0,0,1,1,0}, // = Q
	{1,1,1,0,1,1,1,0}, // = R
	{1,0,1,1,0,1,1,0}, // = S
	{0,0,0,1,1,1,1,0}, // = T
	{0,1,1,1,1,1,0,0}, // = U
	{0,1,1,1,1,1,0,0}, // = V
	{0,1,1,0,1,1,1,0}, // = X
	{0,1,1,0,0,1,1,0}, // = Y
	{1,1,0,1,1,0,1,0}, // = Z
	{0,0,1,1,1,1,1,0}, // = b
	{0,0,0,1,1,0,1,0}, // = c
	{0,1,1,1,1,0,1,0}, // = d
	{1,1,1,1,0,1,1,0}, // = g
	{0,0,1,0,1,1,1,0}, // = h
	{0,0,1,0,0,0,0,0}, // = i
	{0,0,1,0,1,0,1,0}, // = n
	{0,0,1,1,1,0,1,0}, // = o
	{0,0,0,0,1,0,1,0}, // = r
	{0,0,0,1,1,1,1,0}, // = t
	{0,0,1,1,1,0,0,0}, // = u
	{0,0,1,1,1,0,0,0}, // = v
	{0,0,0,1,0,0,0,0}, // = _
	{0,0,0,0,0,0,1,0} // = -
};
/*
 * Constructor for the class
 */
B20M04_4x8::B20M04_4x8(int DATAOUT, int CLOCK, int SLAVESELECT) {
	_DATAOUT = DATAOUT;
	_CLOCK = CLOCK;
	_SLAVESELECT = SLAVESELECT;
	_MAX_POSITIVE = pow(10, NO_DIGITS)-1; // 9999 for 4 segments
	_MAX_NEGATIVE = pow(-10, NO_DIGITS-1)+1; // -999 for 4 segments
	_AN1_ENABLED = false;
	_AN2_ENABLED = false;
	pinMode(_DATAOUT, OUTPUT);
	pinMode(_CLOCK, OUTPUT);
	pinMode(_SLAVESELECT, OUTPUT);

	// Disable writing to the display
	digitalWrite(_SLAVESELECT, HIGH);
	
	// Initialize the clock and data pins low
	digitalWrite(_CLOCK, LOW);
	digitalWrite(_DATAOUT, LOW);
	displayInteger(0);
}

void B20M04_4x8::pulseClock(int noPulses) {
	for (int i=0; i<noPulses; i++) {
		digitalWrite(_CLOCK, HIGH);
		digitalWrite(_CLOCK, LOW);
	}
}

/*
 * This function sets the data pin to high or low as required, then pulses the clock so that
 * the display reads the data pin on the rising edge of the clock.
 * We then set the data pin back to low for good measure.
 */
void B20M04_4x8::clockOut(int data) {
	digitalWrite(_DATAOUT, data);
	pulseClock(1);
	digitalWrite(_DATAOUT, LOW);
}

/*
 * This function is required to get the clocks in sync between the controller and the display.
 * It pulses the clock with the data pin low - no change will be seen on the display until 
 * the data pin goes high to indicate the start of data transmission.
 */
void B20M04_4x8::prepareDisplayForUpdate() {
	digitalWrite(_DATAOUT, LOW);
	digitalWrite(_SLAVESELECT, LOW);
	/*
	 * We have to send a few clock pulses with data low. This seems to synchronise the start of the data.
	 * Otherwise the data bits get offset and the display looks like garbage.
	 * Originally I actually pulsed the clock 36 times, but experimented and found 5 is enough.
	 */
	pulseClock(36);
	clockOut(HIGH); // Start bit
}

/*
 * This function selects and applies the appropriate byte to display a digit on one of the
 * seven segments. The decimal point is turned off in all cases.
 */
void B20M04_4x8::outputDigit(int digit[NO_SEGMENTS]) {
	for (int segCount = 0; segCount < NO_SEGMENTS; ++segCount) {
		clockOut(digit[segCount]);
	}
}

/*
 * This function sets up the data transfer, delegates the setting of each digit and then
 * closes the data transfer.
 */
void B20M04_4x8::displayDigits(int digits[NO_DIGITS][NO_SEGMENTS]) {
	
	prepareDisplayForUpdate();
	
	for (int i=NO_DIGITS-1; i > -1; i--) {
		outputDigit(digits[i]);
	}
	
	clockOut(_AN1_ENABLED);
	clockOut(_AN2_ENABLED);
	clockOut(LOW); // Null unused bit
	digitalWrite(_SLAVESELECT, HIGH);
}

/*
 * This function is called by both displayInteger and by displayDouble.
 * It contains functionality common to both:
 * - Right aligning short numbers.
 */
void B20M04_4x8::displayNumber(String input) {
	String inputMinusPoints = input;
	inputMinusPoints.remove(inputMinusPoints.indexOf('.'),1);
	
	int numLeadingSpaces = NO_DIGITS - inputMinusPoints.length();
	for (int i = 0; i < numLeadingSpaces; i++) {
		input = ' ' + input;
	}
	displayText(input);
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
		displayNumber(String(input));
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
	}
	displayNumber(String(input));
}

/*
 * This function calculates the component digits of a string and then delegates displaying them.
 */
void B20M04_4x8::displayText(String input) {
	int digits[NO_DIGITS][NO_SEGMENTS];
	
	/*
	 * We only loop once for each digit available. If the input string is too long it will be
	 * truncated as a result.
	 */
	for (int i=0; i < NO_DIGITS; i++) {
		char thisChar = input.charAt(i);
		int letterIndex = _CHARS.indexOf(thisChar);
		bool decimalPointNeeded = false;
		
		if (thisChar == '.') {
			// If the current char is a point, we need a decimal point
			decimalPointNeeded = true;
		} else if (input.charAt(i+1) == '.') {
			/*
			 * If the current char is not a point, but the next one is, we need a point,
			 * and we need to remove the point from the string, as we are dealing with it now.
			 */
			decimalPointNeeded = true;
			input.remove(i+1, 1);
		}
		
		
		if (thisChar == 0 || letterIndex == -1) {
			/*
			 * If the string is shorter than the number of segments, thisChar will be zero
			 * and we replace it with a space. This adds trailing whitespace to any input string
			 * as required.
			 * If the character is not supported, we also leave a blank space
			 */
			letterIndex = _CHARS.indexOf(' ');
		}
		/*
		 * Copy the array of integers that represent this character
		 * N.B. memcpy does not work. Not sure why, but is screws up the last 4 bits
		 */
		for (int j=0; j<NO_SEGMENTS; j++) {
			digits[i][j] = _CHAR_BYTES[letterIndex][j];
		}
		
		// Add the decimal point if needed.
		if (decimalPointNeeded) {
			digits[i][7] = 1;
		}
	}
	displayDigits(digits);
}

void B20M04_4x8::toggleAN1(bool enabled) {
	_AN1_ENABLED = enabled;
}

void B20M04_4x8::toggleAN2(bool enabled) {
	_AN2_ENABLED = enabled;
}