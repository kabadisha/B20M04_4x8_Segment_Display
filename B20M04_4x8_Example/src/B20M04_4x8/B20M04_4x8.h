#ifndef B20M04_4x8_h
#define B20M04_4x8_h

#include "Arduino.h"

#define NO_DIGITS 4
#define NO_SEGMENTS 8 // The 8th is a decimal point
#define BUFFER_SIZE 5 // The number of bytes in the data buffer

class B20M04_4x8 {
  public:
    B20M04_4x8(int SLAVESELECT);
    void displayInteger(int input);
    void displayDouble(double input);
    void displayFloat(float input);
    void displayText(String input);
    void toggleAN1(bool enabled);
    void toggleAN2(bool enabled);
  private:
    int _SLAVESELECT;
    double _MAX_POSITIVE;
    double _MAX_NEGATIVE;
    bool _AN1_ENABLED;
    bool _AN2_ENABLED;
    byte _BUFFER[BUFFER_SIZE];
    String rightAlign(String input);
    void renderBuffer();
};
#endif
