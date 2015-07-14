/*
 Fading

 This example shows how to fade an LED using the analogWrite() function.

 The circuit:
 * LED attached from digital pin 9 to ground.

 Created 1 Nov 2008
 By David A. Mellis
 modified 30 Aug 2011
 By Tom Igoe

 http://www.arduino.cc/en/Tutorial/Fading

 This example code is in the public domain.

 */

/* max eye can see: 100Hz */


int ledPin = 13;    // LED connected to digital pin 9

#define PI 3.14159
byte    fadeValue=0;
float   wmega=0;
float ciclesPerSec=0.5; // 1/2 cicle per second.
const float maxCiclesPerSec=100;
unsigned int stepsPerSec=1000*ciclesPerSec;
float   wmegaStep=2*PI/(4.0*stepsPerSec);
boolean start=true;
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

const unsigned long interval = 1000; // in μicroseconds
unsigned long intervalForStepChange = interval/ciclesPerSec; // in μicroseconds. intervalForStep is such that a full cicle has occured.
unsigned long previousMicros=0;
unsigned long currentMicros=0;
unsigned long previousMicrosForStep=0;
unsigned long currentMicrosForStep=0;

void loop() {
    /*/ wait for 30 milliseconds to see the dimming effect
    delay(30);*/

  currentMicros=micros();
  if (abs(currentMicros - previousMicros) >= interval) {
    // do stuff:
    
    {// step up wmega
      wmega+=wmegaStep;
      if (wmega>= 2*PI)
        wmega=0;
    }
    fadeValue=255*(cos(wmega) + 1.0)/2.0;
    analogWrite(ledPin,fadeValue);

    Serial.println(fadeValue,DEC);
    previousMicros=micros();
  }

  currentMicrosForStep=micros();
  if (abs(currentMicrosForStep - previousMicrosForStep) >= intervalForStepChange) {
    // do stuff:
    
/*    ciclesPerSec += 0.2;
    if (ciclesPerSec >= maxCiclesPerSec)
      ciclesPerSec = 0;    
*/    intervalForStepChange = interval/ciclesPerSec;
    stepsPerSec=1000*ciclesPerSec;
    wmegaStep=2*PI/(4.0*stepsPerSec);
//    Serial.println(fadeValue,DEC);

    previousMicrosForStep=micros();
  }

/*  if (start){

    start=false;
  }else{
  }
*/
  // do stuff
}


