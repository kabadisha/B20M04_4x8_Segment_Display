#ifndef B20M04_4x8_h
#define B20M04_4x8_h

#include "Arduino.h"

#define NO_DIGITS 4
#define NO_SEGMENTS 8 // The 8th is a decimal point

class B20M04_4x8 {
	public:
		B20M04_4x8(int DATAOUT, int CLOCK, int SLAVESELECT);
		void displayInteger(int input);
		void displayDouble(double input);
		void displayText(String input);
		void toggleAN1(bool enabled);
		void toggleAN2(bool enabled);
	private:
		int _DATAOUT;
		int _CLOCK;
		int _SLAVESELECT;
		double _MAX_POSITIVE;
		double _MAX_NEGATIVE;
		bool _AN1_ENABLED;
		bool _AN2_ENABLED;
		void pulseClock(int noPulses);
		void prepareDisplayForUpdate();
		void clockOut(int data);
		void outputDigit(int digit[]);
		void displayDigits(int digits[][NO_SEGMENTS]);
		void displayNumber(String input);
};
#endif