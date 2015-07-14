
/*
  LiquidCrystal Library - Hello World
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 
 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int inputPin = A0;
int ledPin = 13;

// for smoothing
/*const int numReadings=100;
int readings[numReadings];
int index = 0;
unsigned long total= 0;
float average= 1023;
*/// gnihtooms rof

void setup()
{
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // initialize serial communication with computer:
////  Serial.begin(9600);                   
  // set analog pin A0 to INPUT
  pinMode(inputPin,INPUT);
  // set digital pin 13 to OUTPUT
  pinMode(ledPin,OUTPUT);
  // for smoothing //
//  for (int thisReading = 0; thisReading < numReadings; thisReading++)
//    readings[thisReading] = 1023;          
}

float sensorReading= 10;
int maxV= 200;
int minV= 1023;// min = 0;
float toLED=0;
unsigned long time_last=0;
int interval= 1000;//milliseconds
void loop() {
  // it takes 100μicroseconds to perform a read
  sensorReading= analogRead(inputPin);

/*  // subtract the last reading:
  total= total - readings[index];        
  // read from the sensor:
  readings[index] = sensorReading;
  // add the reading to the total:
  total= total + readings[index];      
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning:
    index = 0;                          

  // calculate the average:
  average = total / numReadings;        
  // send it to the computer as ASCII digits
*/
  if (millis()-time_last> interval){
    lcd.clear();
    lcd.print(sensorReading);
    lcd.print (" /1023");
    lcd.setCursor(0,1);
    lcd.print("min=");
    lcd.print(minV);
    lcd.print("|max=");
    lcd.print(maxV);
    time_last=millis();
  }
  // find minimum 0-1023
  minV=min(sensorReading,minV);

  if (sensorReading> maxV){
    sensorReading= maxV;
    toLED= 255;
  }else
    toLED= (sensorReading-minV)/(maxV-minV)*255.0;
  analogWrite(ledPin,toLED);

  // delay for stability
  delay(1);
}


