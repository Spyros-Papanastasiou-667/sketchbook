/*
 * one should change ciclesPerSec and not freq or freqStep (externally)
 */

/* max eye can see: 100Hz */


int ledPin = 13;    // LED connected to digital pin 9

#define PI 3.14159
byte    fadeValue=0;
float   freq=0;
boolean start=true;
const float milli=0.001;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

const unsigned long pwmChangeInterval = 1000L; // in μicroseconds // 1000 pwm/sec
unsigned long previousMicros=0L;
unsigned long currentMicros=0L;
const float pwmsPerSec= 1.0/(pwmChangeInterval*0.000001);  //  == 1000
float ciclesPerSec=1.0/3.5; //starting value 
float maxCiclesPerSec=100;
float minCiclesPerSec=0;
float freqStep=1/(pwmsPerSec/ciclesPerSec);// 1/total_interupts

float gravity=9.806;//  meters/ sec^2
unsigned long fallingTime=0;// milliseconds
unsigned long maxFallingTime=10000;// milliseconds
const unsigned long totalMeters=0.5*gravity*pow(maxFallingTime*milli,2);
float currPos=totalMeters;
// falling equation currPos=totalMeters - 0.5*gravity*fallingTime^2

float scale(float value,float fromMax,float fromMin,float toMax,float toMin){
  float fromWidth=abs(fromMax-fromMin);
  float toWidth=abs(toMax - toMin);
/*
 * value          scaledValue
 * -----      ==  -----------
 * fromWidth        toWidth
*/
  float scaledValue=value*toWidth/fromWidth;

  if (max(toMax,toMin)<0)// both are under zero //
    scaledValue=-1*scaledValue;
  if (toMax >=0 && toMin >=0)
    if (toMax < toMin)
      scaledValue= toMin-scaledValue;//!!
  return scaledValue;
}

unsigned long currentTime=0;
unsigned long previousTime=0;
unsigned long timeDiff;

void loop() {
    /*/ wait for 30 milliseconds to see the dimming effect
    delay(30);*/
  // do stuff
  /* time */
  currentTime=millis();// in seconds
  timeDiff=abs(currentTime - previousTime);
  if (timeDiff >=maxFallingTime/* milliseconds */){
    fallingTime=0;
    currPos= totalMeters;    
    previousTime= currentTime;
  }else
    fallingTime=timeDiff;
  
  currentMicros=micros();
  if (abs(currentMicros - previousMicros) >= pwmChangeInterval) {
    // do stuff:

    Serial.print("fTime: ");
    Serial.print(fallingTime*milli,0);
    Serial.print("\t");
    currPos= totalMeters - 0.5*gravity*pow(fallingTime*milli/*in seconds */,2);
    Serial.print("cPos: ");
    Serial.print(currPos,0);
    Serial.print("\tciclesPS: ");
    ciclesPerSec= scale(currPos,totalMeters,0,minCiclesPerSec,maxCiclesPerSec);
    Serial.print(ciclesPerSec,2);
    Serial.print("\n");
    freqStep=1/(pwmsPerSec/ciclesPerSec);// 1/total_interupts
 
    {// step up freq for fadeValue
      freq+=freqStep;// look below at fadeValue
      if (freq>= 1)
        freq=0;
    }
    fadeValue=255*(cos(2*PI*freq) + 1.0)/2.0;
    analogWrite(ledPin,fadeValue);
    
    previousMicros=micros();
  }

  
}


