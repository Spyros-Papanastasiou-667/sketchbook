#include <UTFT.h>
#include <UTouch.h>

UTFT myGLCD(ITDB32S,25,26,27,28);
UTouch  myTouch(6,5,4,3,2);

void setup()
{
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.clrScr();

  myTouch.InitTouch(PORTRAIT);
  myTouch.setPrecision(PREC_MEDIUM);
}

void loop()
{
    while (myTouch.dataAvailable() == true)
    {
      myTouch.read();
      myGLCD.drawPixel (myTouch.getX(), myTouch.getY());
    }
}
