/*
changed omega -> freq
        omegaStep -> freqStep
 */

/* max eye can see: 100Hz */


int ledPin = 13;    // LED connected to digital pin 9

#define PI 3.14159
byte    fadeValue=0;
float   freq=0;
boolean start=true;
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

const unsigned long pwmChangeInterval = 1000; // in μicroseconds // 1000 pwm/sec
unsigned long previousMicros=0;
unsigned long currentMicros=0;
const float pwmsPerSec= 1.0/(pwmChangeInterval*0.000001);  //  == 1000
float ciclesPerSec=0.5; // 1/2 cicle per second.
float maxCiclesPerSec=10;
float minCiclesPerSec=0;
float   freqStep=1/(pwmsPerSec/ciclesPerSec);// 1/total_interupts

unsigned long intervalForStepChange= (1/ciclesPerSec)*1000000;// in μicroseconds
//unsigned long intervalForStepChange = pwmChangeInterval/ciclesPerSec; // in μicroseconds. intervalForStep is such that a full cicle has occured.
unsigned long previousMicrosForStep=0;
unsigned long currentMicrosForStep=0;

void loop() {
    /*/ wait for 30 milliseconds to see the dimming effect
    delay(30);*/

  currentMicros=micros();
  if (abs(currentMicros - previousMicros) >= pwmChangeInterval) {
    // do stuff:
    
    {// step up freq for fadeValue
      freq+=freqStep;// look below at fadeValue
      if (freq>= 1)
        freq=0;
    }
    fadeValue=255*(cos(2*PI*freq) + 1.0)/2.0;
    analogWrite(ledPin,fadeValue);

    Serial.println(fadeValue,DEC);
    previousMicros=micros();
  }

  currentMicrosForStep=micros();
  if (abs(currentMicrosForStep - previousMicrosForStep) >= intervalForStepChange) {
    // do stuff:
    
    ciclesPerSec += 0.1;
    if (ciclesPerSec >= maxCiclesPerSec)
      ciclesPerSec = minCiclesPerSec;

    freqStep=1/(pwmsPerSec/ciclesPerSec);// 1/total_interupts
    intervalForStepChange= (1/ciclesPerSec)*1000000;// in μicroseconds
///    intervalForStepChange = pwmChangeInterval/ciclesPerSec;
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


