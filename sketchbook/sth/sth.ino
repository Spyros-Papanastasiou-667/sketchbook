int inputPin = A0;
int ledPin = 13;

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);                   
  // set analog pin A0 to INPUT
  pinMode(inputPin,INPUT);
  // set digital pin 13 to OUTPUT
  pinMode(ledPin,OUTPUT);
}

float sensorReading= 0;
int maxV= 200;
// min = 0;
float toLED=0;
void loop() {
  // it takes 100μicroseconds to perform a read
  sensorReading= analogRead(inputPin);
  if (sensorReading> maxV){
    sensorReading= maxV;
    toLED= 255;
  }else
    toLED= sensorReading/maxV*255.0;
  analogWrite(ledPin,toLED);
  // delay for stability
  delay(1);
}


