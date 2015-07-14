/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);

  float Vab= sensorValue*5/1023.0;
  const float E=5;//Volts
  const float R2=150;//KΩ
  float R1= E*R2/(E-Vab)-R2;
  
  // print out the value you read:
  Serial.println(R1);
  delay(500);        // delay in between reads for stability
}
