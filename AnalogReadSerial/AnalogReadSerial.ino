//#include <QueueList.h>

/*
  AnalogReadSerial
 Reads an analog input on pin 0, prints the result to the serial monitor.
 Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */

/*class maxRl{
 public:
 float maxRls[100];
 int index;
 maxRl(){
 for (int i=0;i<=99;i++){
 maxRls[i]=0;
 }
 index=0;
 }
 float pop(){
 float output= maxRls[index];
 index++;
 if (index==100)
 index=0;
 return output;
 }
 void push(float number){
 maxRls[index]=number;
 index++;
 if (index==100)
 index=0;
 }
 float mo(){
 int mo=0;
 for (int i=0;i<=99;i++){
 mo+=maxRls[i];
 }
 mo/=100;
 return mo;
 }
 float s(){
 float s_2=0;
 float ave=mo();
 int n=100;
 for (int i=0;i<=99;i++){
 s_2+=pow(maxRls[i]-ave,2);
 }
 s_2/=n;
 return sqrt(s_2);
 }
 }test;
 */

//
const int pwmLED = 13;

// the setup routine runs once when you press reset:

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(pwmLED, OUTPUT);
  //
}

const float E=5;//in Volts
const float R=221000;//in Ohms
float maxRl =0;
float minRl = 10000000;//Ohms:i wonder what value should i put here?
long previousMillis= 0;
// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000;           // interval at which to blink (milliseconds)
float Rl;//lightresistor

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int AnalogInput0 = analogRead(A0);//0-1023
  /* but 1023=5V so*/
  float Vab = AnalogInput0*5/1023.0;//Volts
  if (E-Vab!= 0)
    Rl =(Vab*R)/(E-Vab);//RlightSensitive
  else//E=Vab=5V
  //i don't know what to put here either
    Rl= 0;

  //find max,min
  if (Rl > maxRl) maxRl = Rl;
   if (Rl < minRl) minRl = Rl;
   float eyros = maxRl-minRl;
   
   float brightness;
   if (eyros!= 0)
     brightness = Rl/eyros*255.0;
   else
     brightness = 255;

//  float brightness=Rl/maxRl*255.0;
  analogWrite(pwmLED,brightness);
  delay(2);
  
  
  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.
  unsigned long currentMillis = millis();

  //  test.push(Rl);
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  
    // print out the value you read:
    Serial.print("brightness = ");
    Serial.print(brightness);
    Serial.print(" \t ");
     Serial.print("AnalogInput0= ");
     Serial.print(AnalogInput0);
    Serial.print(" \t ");
     Serial.print("Rl= ");
     Serial.print(Rl);
     Serial.print(" \t ");
     Serial.print("maxRl= ");
     Serial.println(maxRl);
/*     Serial.print(" \t ");
     Serial.println(test.pop());
     */  }

}

