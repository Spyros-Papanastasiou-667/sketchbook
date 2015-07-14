/*
 * one should change ciclesPerSec and not freq or freqStep (externally)
 */

/* max eye can see: 100Hz */


int ledPin = 13;    // LED connected to digital pin 9

#define PI 3.14159
byte    fadeValue=0;
double   freq=0;
boolean start=true;
const double milli=0.001;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

const unsigned long pwmChangeInterval = 1000L; // in μicroseconds // 22727.27 pwm/sec
unsigned long previousMicros=0L;
unsigned long currentMicros=0L;
const unsigned long pwmsPerSec= 1.0/(pwmChangeInterval*0.000001);  //  == 22727.27
double ciclesPerSec=0; //starting value 
double maxCiclesPerSec=30;
double minCiclesPerSec=0;
double freqStep=1/(pwmsPerSec/ciclesPerSec);// 1/total_interupts

double gravity=9.806;//  meters/ sec^2
unsigned long fallingTime=0;// milliseconds
unsigned long maxFallingTime=10000;// milliseconds
const unsigned long totalMeters=0.5*gravity*pow(maxFallingTime*milli,2);
double currPos=totalMeters;
// falling equation currPos=totalMeters - 0.5*gravity*fallingTime^2

double scale(double value,double fromMax,double fromMin,double toMax,double toMin){
  double fromWidth=abs(fromMax-fromMin);
  double toWidth=abs(toMax - toMin);
/*
 * value          scaledValue
 * -----      ==  -----------
 * fromWidth        toWidth
*/
  double scaledValue=value*toWidth/fromWidth;

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

unsigned long currPWMs=0,T;

double t=0;
void loop() {
    /*/ wait for 30 milliseconds to see the dimming effect
    delay(30);*/
  // do stuff
  /* time */
  currentTime=millis();
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

//    Serial.print("fTime: ");
//    Serial.print(fallingTime*milli,3);
//    Serial.print("\t");
    currPos= totalMeters - 0.5*gravity*pow(fallingTime*milli/*in seconds */,2);
    Serial.print("cPos: ");
    Serial.print(currPos,0);
    Serial.print("\tciclesPS: ");
    ciclesPerSec= scale(currPos,totalMeters,0,minCiclesPerSec,maxCiclesPerSec);
    Serial.print(ciclesPerSec,2);

    
    t=fallingTime/1000.0;
    Serial.print("\tt: ");
    Serial.print(t,3);
    fadeValue=255*(cos(2*ciclesPerSec*t) + 1.0)/2.0;// i don't know what the 2 stands for neither why no PI is used
    Serial.print("\tfadeValue: ");
    Serial.println(fadeValue,DEC);

    analogWrite(ledPin,fadeValue);
    
    previousMicros=currentMicros;
  }

  
}


