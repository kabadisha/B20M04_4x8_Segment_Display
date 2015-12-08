#include <B20M04_4x8.h>
// Setup pins: DATAOUT (1), CLOCK(2), SLAVESELECT (3)
  B20M04_4x8 disp(1, 2, 3);
  
void setup()
{

}

void loop()
{
  delay(3000);
  
  disp.toggleAN1(false);
  disp.toggleAN2(false);
  
  disp.displayInteger(6.6);
  delay(2000);

  disp.displayDouble(6.1);
  delay(2000);

  disp.displayDouble(-9.7);
  delay(2000);
  
  disp.displayText("bcdg");
  delay(2000);

  disp.displayText("hino");
  delay(2000);

  disp.displayText("rtuv");
  delay(2000);

  disp.toggleAN1(true);
  disp.toggleAN2(true);
  
  for (int i=0; i < 10; i++) {
    disp.displayText("BANG");
    delay(100);
    disp.displayText("    ");
    delay(100);
  }
}


