/*
 * bouncing_ball.ino
 * 
 * Blinking the led(13) of the Arduino Leonardo like a falling/+bouncing 
 * ball.
 * 
 * Author: Spyros Papanastasiou spyridon.papanastasiou@gmail.com
 * 
 * License:
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*********************************************************
 * 
 *                      N O T E S 
 * 
 ********************************************************/

/* 
 *  on Leonardo and other ATMEGA boards "double" is the same as "float"
 *  
 *  also be VERY carefull on mult/div with integers! ALWAYS use #.0!!
 *  
 *  in this program, my main assumption is that at each bump, the kinetic
 *  energy lost is a fraction (a) of the kinetic energy it had before the
 *  bump.
*/
//#define DEBUG
//#define DEBUG2
/********************************************************* 
 *                   c o n s t a n t s: 
 ********************************************************/
#define gravity 9.806
#define ledPin  13
const int numOfBumps /* in 10 secs */ = 60;
const unsigned short totalSecs=20;
//const int maxHeight /* starting height in meters */ = 10;
//  const int startingHeight=maxHeight;
const double lastMaxHeightPercentage /* before the last bump/ in meters */ = 1/100.0/* of startingHeight  */;
/*******************************************************
 *                  V a r i a b l e s
 *******************************************************/
#ifdef DEBUG
  String debugStringOne;
  boolean printStringOneNow;
  String debugStringTwo;
#endif
double h/* h(t) */;
double localEquationTime/* t in h(t) and between each bump/ t1 -> t2 ...*/;
byte pwm/* the brightness value for the led */;
unsigned long loopsPerSec=0/* just a counter */;

double  v[numOfBumps]/* velocity after each bump */,
        t[numOfBumps]/* time from start to each bump */,
        H0/* starting height */, 
        a/* the percentage of lost kinetic energy on each bump/ calculated */;

/******************************************************************
 * 
 *           f u n c t i o n   i n i t i a l i z a t i o n
 * 
 ******************************************************************/
/*double startingHeight(void);
double velocityAfterCollision(unsigned short index);
double maxHeightAfterCollision(unsigned short index);
double timeBetweenCollisions(unsigned short index);
double scale(double value,double fromMax,double fromMin,double toMax,double toMin);
*//*******************************************************************
 * 
 *                          S E T U P
 * 
 *******************************************************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
#ifdef DEBUG2
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only only if you want to print
  }
#endif
  /*****************************************************************
   *              initialize variables for loop()
   *****************************************************************/
  H0=startingHeight();
  a= energyLossSlope();

  v[0]=velocityAfterCollision(0);
  t[0]=timeBetweenCollisions(0);
  for (int index=1;index<=(numOfBumps-1);index++){
    v[index]=velocityAfterCollision(index);
    t[index]=timeBetweenCollisions(index)+t[index-1];
#ifdef  DEBUG2
    debugStringTwo="v["+String(index)+"] : "+String(v[index],3)+\
    "\tt["+String(index)+"] : "+String(t[index],3);
    Serial.println(debugStringTwo);
    delay(1000);
#endif 
 }
}

/*double velocityPrime(double prevVelocity, unsigned short index){
  /* will use startingHeight, gravity */
  /*
  double returnVelocity=sqrt(pow(1-a,index)*2*gravity*startingHeight);
  return returnVelocity;
}
*/
/*****************************************************************
 * 
 *    a / P E R C E N T A G E   O F   L O S T   E N E R G Y
 *    
 *****************************************************************/
double energyLossSlope/* a */(void){
  double a=1-pow(lastMaxHeightPercentage,1.0/(numOfBumps-1));
  return a/* let's start with a 10% loss of kinetic energy */;
}
/*******************************************************************
 * 
 *                S T A R T I N G   H E I G H T 
 * 
 *******************************************************************/
double startingHeight(void){
  /* will use gravity, energyLossSlope/a, T=10sec */
  /* will square the denominator and nominator=T=10sec afterwards */
  double denominator=sqrt(2.0/gravity);
  for (int index=1;index<=(numOfBumps-1);index++){
    denominator+= 2.0*sqrt(pow(1-energyLossSlope(),index)) * sqrt(2.0/gravity);
  }
  unsigned short nominator=totalSecs/* sec */;
  double H0= pow(nominator/denominator,2);
  return H0;
}
/*******************************************************************
 * 
 *       V E L O C I T Y   A F T E R   C O L L I S I O N 
 * 
 *******************************************************************/
double velocityAfterCollision(unsigned short index){
  /* will use gravity, energyLossSlope/a, startingHeight */
  double velocity;
  if (index >= 1){
    velocity=sqrt(pow(1-energyLossSlope(),index)*2.0*gravity*startingHeight());
  }else if (index == 0){
    velocity= 0;
  }
  return velocity;
}
/*******************************************************************
 * 
 *      M A X   H E I G H T   A F T E R   C O L L I S I O N
 * 
 *******************************************************************/
double maxHeightAfterCollision(unsigned short index){
  /* will use velocityAftercollision and gravity */
  double maxH;
  if (index >=1){
    maxH=pow(velocityAfterCollision(index),2)/(2.0*gravity);
  }else if (index == 0 /* no collision yet/ just falling! */){
    maxH=startingHeight();
  }
  return maxH;
}
/*******************************************************************
 * 
 *          T I M E   B E T W E E N   C O L L I S I O N S
 * 
 *******************************************************************/
double timeBetweenCollisions(unsigned short index){
  /* will use startingHeight, gravity, energyLossSlope/a */
  double ti;
  if (index >=1 ){
    ti=2.0*sqrt(pow(1-energyLossSlope(),index)*2.0/gravity*startingHeight());
  }else if (index==0)/* time till first collision */{
    ti=sqrt(2.0/gravity*startingHeight());
  }
  return ti;
}

/*******************************************************************
 * 
 *                          S C A L E 
 * 
 *******************************************************************/
double scale(double value,double fromMax,double fromMin,double toMax,double toMin){
  double fromWidth=abs(fromMax-fromMin);
  double toWidth=abs(toMax - toMin);
/*
 * value          scaledValue
 * -----      ==  -----------
 * fromWidth        toWidth
*/
  double scaledValue=value*toWidth/fromWidth;

  /* i don't know about the following, i think i got it correct :/ */
  if (max(toMax,toMin)<0)// both are under zero //
    scaledValue=-1*scaledValue;
  if (toMax >=0 && toMin >=0)
    if (toMax < toMin)
      scaledValue= toMin-scaledValue;//!!
  return scaledValue;
}
/* code to keep track of 10 sec */
unsigned long prevTime=0,currTime=0,diffTime=0,
              prevTime2=0,currTime2=0,diffTime2=0;
double trackedTime=0/* 0 to 10 sec */;

/*******************************************************************
 * 
 *                          L O O P
 * 
 *******************************************************************/
void loop() {
  // put your main code here, to run repeatedly:

  /* code to keep track of 10 sec 
   *  trackedTime: 0 ---> 10 sec
  */
  currTime=millis();
  diffTime=abs(currTime- prevTime);
  if (diffTime >= totalSecs*1000/* 10secs * 1000millisecs */){
    Serial.println("          S T A R T             ");
    trackedTime=0;
    prevTime=currTime;
    Serial.print("\t\tloopsPerSecond = ");
    Serial.println(float(loopsPerSec/totalSecs));
    loopsPerSec=0;
  }else{
    trackedTime=diffTime/1000.0/* convert to secs */;
    loopsPerSec++;
  }

  /* 
   *  code to keep track of 100millisec 
   */
#ifdef  DEBUG
  currTime2=millis();
  diffTime2=abs(currTime2- prevTime2);
  printStringOneNow=false;
  if (diffTime2 >= 100/* millisecs */){
    prevTime2=currTime2;
    printStringOneNow=true;
  }
#endif
  /* remove the following */
//  delay(100);
  byte index=0;
  for (int i=1;i<=numOfBumps-1;i++){
    if(trackedTime > t[i-1] && trackedTime <= t[i]){
      index=i;
      break;
    }
  }
  /* set local time (between each bump) */
  /* height : h */
  if (index >= 1){
    localEquationTime=trackedTime-t[index-1]/* time since last bump */;
    h=v[index]*localEquationTime-1/2.0*gravity*pow(localEquationTime,2);
  }else if (index== 0){
    localEquationTime=trackedTime;
    h=H0-1/2.0*gravity*pow(localEquationTime,2);
  }
  /* set pwm */
  pwm=scale(h,H0,0,255,0);
  analogWrite(ledPin,pwm);
/*******************************************************************
 * 
 *                       T E S T/ D E B U G
 * 
 *******************************************************************/
#ifdef  DEBUG
  if(printStringOneNow==true){
    if(index>=1){
      debugStringOne=
      "trackedTime : " + String(trackedTime,3) + \
      "\tindex = " + index +\
      "\tv[index] : " + String(v[index],3) + \
      "\teqLocalTime : " + String(localEquationTime,3) + \
      "\theight : " + String(h,3) + \
      "\tmaxH : " + String(maxHeightAfterCollision(index),3) + \
      "\tStartingH : " + String(H0,3) + \
      "\ta = " + String(a,3) + \
      "\tPI = " + String(PI,DEC);
    }else if (index == 0){
      debugStringOne=
      "trackedTime : " + String(trackedTime,3) + \    
      "\tindex = " + index +\
      "\theight : " + String(h,3) + \
      "\tmaxH : " + String(maxHeightAfterCollision(index),3) + \
      "\tStartingH : " + String(H0,3) + \
      "\tPI = " + String(PI,DEC);
    }
    Serial.println(debugStringOne);
  }
#endif
  /*  debugStringTwo="t[0] = " + String(t[0],3);
    for (int i=1;i<=(numOfBumps-1);i++){
      debugStringTwo+="\tt["+String(i)+"] = " + String(t[i],3);
    }
  *///  Serial.println(debugStringTwo);
  /*  double vel=1;
    double x=vel*trackedTime-1/2.0*gravity*pow(trackedTime,2);
    debugStringTwo=
      "time = " + String(trackedTime,3) + \
      "\tx = " + String(x,3) + "m" + \
      "\t1/2*gravity*pow(trackedTime,2) = " + String(1/2.0*gravity,3);
    Serial.println(debugStringTwo);
  */
}

