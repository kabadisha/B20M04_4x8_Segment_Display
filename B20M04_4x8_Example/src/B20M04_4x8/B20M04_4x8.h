#ifndef B20M04_4x8_h
#define B20M04_4x8_h

#include "Arduino.h"

// The number of bytes in the data buffer. The display requires 36 bits of
// segment data to be sent. Our buffer is 4 bytes (as there are 4 digits on the
// display), but will be prepended with a start bit (always zero) and appended
// with 3 bits (one for each 'annunciator' and an end bit which is always zero).
#define NO_DIGITS 4

class B20M04_4x8 {
  public:
    B20M04_4x8(int SLAVESELECT_PIN, int MOSI_PIN, int CLOCK_PIN);
    void clearDisplay();
    void displayInteger(int input);
    void displayDouble(double input);
    void displayFloat(float input);
    void displayText(String input);
    void toggleAN1(bool enabled);
    void toggleAN2(bool enabled);
  private:
    int _SLAVESELECT_PIN;
    int _MOSI_PIN;
    int _CLOCK_PIN;
    double _MAX_POSITIVE;
    double _MAX_NEGATIVE;
    bool _AN1_ENABLED;
    bool _AN2_ENABLED;
    String rightAlign(String input);
    void sendBit(int _value);
    void sendByte(byte _send);
};
#endif
