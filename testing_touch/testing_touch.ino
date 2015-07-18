#include <UTFT.h>
#include <UTouch.h>

// Initialize display
// ------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
// Standard Arduino Uno/2009 Shield            : <display model>,19,18,17,16
// Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
// CTE TFT LCD/SD Shield for Arduino Due       : <display model>,25,26,27,28
// Teensy 3.x TFT Test Board                   : <display model>,23,22, 3, 4
// ElecHouse TFT LCD/SD Shield for Arduino Due : <display model>,22,23,31,33
//
// Remember to change the model parameter to suit your display module!
UTFT    myGLCD(ITDB32S,25,26,27,28);

// Initialize touchscreen
// ----------------------
// Set the pins to the correct ones for your development board
// -----------------------------------------------------------
// Standard Arduino Uno/2009 Shield            : 15,10,14, 9, 8
// Standard Arduino Mega/Due shield            :  6, 5, 4, 3, 2
// CTE TFT LCD/SD Shield for Arduino Due       :  6, 5, 4, 3, 2
// Teensy 3.x TFT Test Board                   : 26,31,27,28,29
// ElecHouse TFT LCD/SD Shield for Arduino Due : 25,26,27,29,30
//
UTouch  myTouch( 6, 5, 4, 3, 2);
// Declare which fonts we will be using
extern uint8_t SmallFont[];

#define TFT_WIDTH 320
#define TFT_HEIGHT  240
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  myTouch.InitTouch(PORTRAIT);
}

int x,y,iteration=0;
String toDraw;
String toDraw2;
void loop() {
  // put your main code here, to run repeatedly:
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(toDraw2,CENTER,TFT_HEIGHT/2-6+12);

  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_RED);
  toDraw2=String(millis()/1000.0,1);
  myGLCD.print(toDraw2,CENTER,TFT_HEIGHT/2-6+12);
  
  while (myTouch.dataAvailable() == true){
    myTouch.calibrateRead();
    x=myTouch.TP_X;
    y=myTouch.TP_Y;

    toDraw=String(x)+","+String(y);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_RED);
    myGLCD.print(toDraw,CENTER,TFT_HEIGHT/2-6);

    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.print(toDraw2,CENTER,TFT_HEIGHT/2-6+12);
  
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_RED);
    toDraw2=String(millis()/1000.0,1);
    myGLCD.print(toDraw2,CENTER,TFT_HEIGHT/2-6+12);
  
  }
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(toDraw,CENTER,TFT_HEIGHT/2-6);

}
