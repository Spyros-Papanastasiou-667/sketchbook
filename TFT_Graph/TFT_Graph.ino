/*
 * bouncing_ball.ino
 * 
 * Blinking the led(13) of the Arduino Leonardo like a falling/+bouncing 
 * ball.
 * 
 * Author: Spyros Papanastasiou spyridon.papanastasiou@gmail.com
 * 
 * License:
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
 *  
 *  This program was made for modules with a screen resolution of 320x240 pixels
 *  
 *  This program requires the UTFT library.
 *  
 */
#include <UTFT.h>
//#define TOUCH
#ifdef TOUCH
  #include <UTouch.h>
#endif
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
/*
 *                      T F T
 */
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
extern unsigned char SmallFont[];
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

/* code to keep track of 10 sec */
unsigned long prevTime=0,currTime=0,diffTime=0,
              prevTime2=0,currTime2=0,diffTime2=0;
double trackedTime=0/* 0 to 10 sec */;

/*
 *                T  F  T
 */
unsigned short displayHeight, displayWidth, midHeight, midWidth;
byte tftBrightness;
unsigned short xPos,yPos,zPos;
String  TFTStringOne,TFTStringTwo;
/* variables about the time on which to draw the TFT */
unsigned long diffTFTTime=0,currTFTTime=0,prevTFTTime=0;
boolean drawTFTNow=true;
String TFTLoopsPerSecString;
unsigned long TFTLoopsPerSec;
unsigned short randomData[TFT_WIDTH],randomDataCounter=0,previousRandomDataCounter=0,randomDataIndex;

#ifdef TOUCH
  unsigned short xTouch,yTouch;
#endif
/******************************************************************
 * 
 *           f u n c t i o n   i n i t i a l i z a t i o n
 * 
 ******************************************************************/
/* Set the pins to the correct ones for your development shield
 * ------------------------------------------------------------
 * Standard Arduino Mega/Due shield            : <display model>,38,39,40,41
 * CTE TFT LCD/SD Shield for Arduino Due       : <display model>,25,26,27,28
 * Teensy 3.x TFT Test Board                   : <display model>,23,22, 3, 4
 * ElecHouse TFT LCD/SD Shield for Arduino Due : <display model>,22,23,31,33
 * 
 * Remember to change the model parameter to suit your display module!
 */
UTFT myGLCD(ITDB32S,25,26,27,28);
/* Initialize touchscreen
 * ----------------------
 * Set the pins to the correct ones for your development board
 * -----------------------------------------------------------
 * Standard Arduino Uno/2009 Shield            : 15,10,14, 9, 8
 * Standard Arduino Mega/Due shield            :  6, 5, 4, 3, 2
 * CTE TFT LCD/SD Shield for Arduino Due       :  6, 5, 4, 3, 2
 * Teensy 3.x TFT Test Board                   : 26,31,27,28,29
 * ElecHouse TFT LCD/SD Shield for Arduino Due : 25,26,27,29,30
 */
#ifdef TOUCH
  UTouch  myTouch( 6, 5, 4, 3, 2);
#endif
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
  randomSeed(analogRead(A0));
  // Setup the LCD
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.setFont(SmallFont);
  myGLCD.fillScr(VGA_WHITE);

  for (int index=0;index<=TFT_WIDTH-1 ;index++){
//    randomData[index]=random(1,TFT_HEIGHT+1);
    randomData[index]=scale( sin(2.0*PI*index/TFT_WIDTH)+1 ,2,0,1,TFT_HEIGHT-1);
/*    if (randomData[index]>TFT_HEIGHT-1)
      randomData[index]=TFT_HEIGHT-1;
    else if (randomData[index]<1)
      randomData[index]=1;
*/  }
#ifdef TOUCH
  myTouch.InitTouch(LANDSCAPE);
#endif

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
 /* 
  *  TFT variables
  */
  displayHeight=myGLCD.getDisplayYSize();
  displayWidth=myGLCD.getDisplayXSize();
  midHeight=( 1 + (TFT_HEIGHT-1) )/2;
  midWidth=(1 + (TFT_WIDTH -1) )/2;
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

/*******************************************************************
 * 
 *                    D R A W   P I X E L
 * 
 *******************************************************************/
int drawPixel(unsigned short x, unsigned short y){
  if ((x>=1)&&(x<=TFT_WIDTH-1)&&(y>=1)&&(y<=TFT_HEIGHT-1)){
    myGLCD.drawPixel(x,y);
    return 0;
  }else
    return -1;
}
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
    Serial.println(float(loopsPerSec/totalSecs),2);
    loopsPerSec=0;
    TFTLoopsPerSecString="Loops per second : " + String(float(TFTLoopsPerSec/totalSecs),2);
    TFTLoopsPerSec=0;
  }else{
    trackedTime=diffTime/1000.0/* convert to secs */;
    loopsPerSec++;
    TFTLoopsPerSec++;
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
  {/* blink led(pin 13) like the bouncing ball */
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
  }
  /*
   * TFT code
  */
//  tftBrightness=scale(h,H0,0,16,0);
//  myGLCD.setBrightness(tftBrightness);
  /*          when to draw          */
  currTFTTime=millis();
  diffTFTTime=abs(currTFTTime- prevTFTTime);
  drawTFTNow=false;
  if (diffTFTTime >= 1000/* millisecs */){
    prevTFTTime=currTFTTime;
    drawTFTNow=true;
  }
  
  if (drawTFTNow== true){
    {/* shift random data across the screen */
      /* index : 0      <---      TFT_WIDTH */
      /* randomDataCounter ++ */
      for (int index=TFT_WIDTH;index>0;index--){
        randomDataIndex=(index-1)+randomDataCounter;
        if (randomDataIndex>=TFT_WIDTH)
          randomDataIndex-=TFT_WIDTH;
        
        myGLCD.setColor(VGA_WHITE);
        int indexToErase=randomDataIndex-1;
        if (indexToErase<0)
          indexToErase=TFT_WIDTH-1;
        drawPixel(index,randomData[indexToErase]);

//        delayMicroseconds(1000);
        myGLCD.setColor(VGA_BLACK);
        drawPixel(index,randomData[randomDataIndex]);
//        delayMicroseconds(1000);
      }
      randomDataCounter++;
      if (randomDataCounter==TFT_WIDTH)
        randomDataCounter=0;
    }
    myGLCD.drawLine(1,midHeight,TFT_WIDTH-1,midHeight);
//    delay(2000);
/*    yPos=scale(h,H0,0,0,displayHeight-12);
    TFTStringOne="yPos = " +String(yPos);
    myGLCD.print(TFTStringOne,CENTER,0);
//    myGLCD.print(TFTLoopsPerSecString,CENTER,yPos);
    myGLCD.setColor(VGA_OLIVE);
    myGLCD.fillCircle(displayWidth/2-1,yPos,16);
*/
/*  Print coordinates of/on the four corners : */
/*    xPos=0;
    yPos=0;
    TFTStringOne="(" + String(xPos) + "," + String(yPos) + ")";
    myGLCD.print(TFTStringOne, xPos,yPos);

    xPos=0;
    yPos=displayHeight;
    TFTStringOne="(" + String(xPos) + "," + String(yPos) + ")";
    yPos-=12/* pixels *//*;
    myGLCD.print(TFTStringOne, xPos,yPos);

    xPos=displayWidth;
    yPos=0;
    TFTStringOne="(" + String(xPos) + "," + String(yPos) + ")";
    xPos-=TFTStringOne.length()*8/* pixels *//*;
    myGLCD.print(TFTStringOne, xPos,yPos);

    xPos=displayWidth;
    yPos=displayHeight;
    TFTStringOne="(" + String(xPos) + "," + String(yPos) + ")";
    xPos-=TFTStringOne.length()*8/* pixels *//*;
    yPos-=12/* pixels *//*;
    myGLCD.print(TFTStringOne, xPos,yPos);
*/
#ifdef TOUCH
    while(myTouch.dataAvailable()==true){
/*      myTouch.read();
      xTouch=myTouch.getX();
      yTouch=myTouch.getY();
      String toPrint="x,y = " + String(xTouch,DEC) + ", " + String(yTouch,DEC);
      myGLCD.print(toPrint,CENTER,y);
*//*      xTouch=myTouch.TP_X;
      yTouch=myTouch.TP_Y;
      String toPrint="TP_X, TP_Y = " + String(xTouch,DEC) + ", " + String(yTouch,DEC);
//      y-=6;
      myGLCD.print(toPrint,CENTER,y);
*/    };
#endif
  }
  

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

