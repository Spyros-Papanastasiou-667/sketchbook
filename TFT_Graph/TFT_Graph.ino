/*
 * bouncing_ball.ino
 * 
 * Blinking the led(13) of the Arduino Leonardo like a falling/+bouncing 
 * ball.
 * 
 * Author: Spyros Papanastasiou spyridon.papanastasiou@gmail.com
 * 
 * License:
 * This program is free software; you can redistribute it and/or modify
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
//#include <math.h>
#include <avr/dtostrf.h>
#include <UTFT.h>
//#define TOUCH

#include <SPI.h>
#include <SdFat.h>
#define SS  53
// store error strings in flash to save RAM
#define error(s) sd.errorHalt(F(s))
// create Serial stream
ArduinoOutStream cout(Serial);

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
/*
 *                   S D   C a r d
 */
// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = 53;/* change this to SS */
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
/* testing memory limits */
//#define testLength  40000
//unsigned short test[testLength];/* damn it! This keeps going down! */
double secs;
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

//#define randomDataLength  40000
//unsigned short randomData[randomDataLength],lastFirstPixel,randomDataCounter=0,previousRandomDataCounter=0,randomDataIndex;

#define graphLength 400/* about 10 Hours */
unsigned short graph[graphLength],graphCounter=0,lastFirstPixelY,graphIndex;

unsigned long TFTDrawCounter=-1;

#ifdef TOUCH
  unsigned short xTouch,yTouch;
#endif

/*
 *                   S D   C a r d
 */
String  SDStringOne;
// File system object.
SdFat sd;

SdFile file;
unsigned long SDCounter1=0;
/*
 *                  T   M   P
 */
float tmpFloat;
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

*/
 
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
  if ((x>=0)&&(x<=TFT_WIDTH-1)&&(y>=0)&&(y<=TFT_HEIGHT-1)){
    myGLCD.drawPixel(x,y);
    return 0;
  }else
    return -1;
}
/*******************************************************************
 * 
 *                         E L S E
 * 
 *******************************************************************/
String freeSpaceString(unsigned short precision){
  float fs = 0.000512 * sd.vol()->freeClusterCount() * sd.vol()->blocksPerCluster();
  String fsString=String(fs,precision)+" MiB";
  return fsString;
}
float freeSpace(){
  float fs = 0.000512 * sd.vol()->freeClusterCount() * sd.vol()->blocksPerCluster();
  return fs;
}
boolean buttonState(unsigned short button){
  pinMode(button,INPUT_PULLUP);
  int highLow=digitalRead(button);
  if (highLow==LOW)
    return true;
  else
    return false;
}
/*******************************************************************
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
  myGLCD.clrScr();
//  myGLCD.fillScr(VGA_WHITE);
  
  
    /*-----------------------------------------------------------------
     *      On random data array
     */
/*  for (int index=0;index<=randomDataLength-1 ;index++){
//    randomData[index]=random(1,TFT_HEIGHT+1);
    randomData[index]=scale( sin(2.0*PI*index/TFT_WIDTH)+1 ,2,0,0,TFT_HEIGHT-1);
/*    if (randomData[index]>TFT_HEIGHT-1)
      randomData[index]=TFT_HEIGHT-1;
    else if (randomData[index]<1)
      randomData[index]=1;
*//*  }
  lastFirstPixel=randomData[randomDataLength-1-TFT_WIDTH];
*/
/*===================================================================*/
  for (int index=graphLength-1;index>graphLength-1-TFT_WIDTH;index--)
    graph[index]=-1;
/* testing SRAM */
//  test[1]=1;
//  Serial.println(test[testLength-1]);
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
  /*=====================================*/
  myGLCD.setColor(VGA_RED);
  myGLCD.setBackColor(VGA_BLACK);
  TFTStringOne="Do you want to erase+format?";
  myGLCD.print(TFTStringOne,CENTER,midHeight-6);
  TFTStringOne="Hit button 8";
  myGLCD.print(TFTStringOne,CENTER,midHeight-6+12);

  prevTime=millis();
  boolean yesNo=buttonState(8);
  while( yesNo == false && diffTime<=3000){
    currTime=millis();
    diffTime=abs(currTime- prevTime);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.setColor(VGA_BLACK);
    TFTStringOne="10.1/3 secs";
    myGLCD.print(TFTStringOne,CENTER,TFT_HEIGHT-12);
    myGLCD.setColor(VGA_RED);
    TFTStringOne=String(diffTime/1000.0,1)+"/3 secs";
    myGLCD.print(TFTStringOne,CENTER,TFT_HEIGHT-12);
    yesNo = buttonState(8);
  }
  if (yesNo)
    format();
  else{
    TFTStringOne="doing nothing";
    myGLCD.print(TFTStringOne,CENTER,midHeight-6+24);
  }
//  char buff[sizeof("3.4028235E+38")];
//  dtostrf(fs,5,3,buff);
  // Setup the SD

  // Initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(SS, SPI_HALF_SPEED)) {
//    sd.initErrorHalt();
    myGLCD.clrScr();
    myGLCD.setColor(VGA_RED);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.print("Sth is wrong. Please fsck/format",CENTER,midHeight-12);
    myGLCD.print("after you backup!",CENTER,midHeight);
  }
  if (!file.open("test.txt", O_CREAT | O_WRITE | O_TRUNC)) {/* use O_TRUNC to erase first */
    error("opening file...");
  }

  myGLCD.clrScr();
  myGLCD.setColor(VGA_RED);
  myGLCD.setBackColor(VGA_BLACK);
  myGLCD.print(freeSpaceString(6),CENTER,midHeight-6);

    Serial.println(TFTStringOne);

  double freeSpaceInDays=freeSpace()*1000.0*1000.0/*bytes*//(17/*bytes per line*/*3600*24);
  TFTStringOne="free dayes : " + String(freeSpaceInDays);
  myGLCD.print(TFTStringOne,CENTER,midHeight-6+12);
  
  delay(5000);
  myGLCD.clrScr();

  /*
   *          SD Card
   */
   Serial.println("Press any key to stop and read");
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
    TFTLoopsPerSecString=String(TFTLoopsPerSec/totalSecs)+" lps";
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
  {/* * * * * * * * * * * * * * * * * * * * * * * * * * * 
    *     blink led(pin 13) like the bouncing ball
    * * * * * * * * * * * * * * * * * * * * * * * * * * */
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
  if (diffTFTTime >= 0/* millisecs */){
    prevTFTTime=currTFTTime;
    drawTFTNow=true;
    SDCounter1++;
    secs=millis()/1000.0;
    TFTDrawCounter++;
  }

  if (drawTFTNow== true){

      {
        for (int index=TFT_WIDTH-1;index>=0;index--){
          graphIndex=index+graphCounter;
          if (graphIndex>=graphLength)
            graphIndex-=graphLength;

          myGLCD.setColor(VGA_BLACK);
          short eraseIndex=graphIndex-1;
          if (eraseIndex<0)
            eraseIndex=graphLength-1;
          if (index==0){
            short temp=graphIndex-1;
            if (temp<0)
              temp=graphLength-1;
            lastFirstPixelY=graph[temp];
            drawPixel(index,lastFirstPixelY);
          }else
            drawPixel(index,graph[eraseIndex]);

          if (index==TFT_WIDTH-1){
            graph[graphIndex]=scale( sin(2.0*PI*TFTDrawCounter/TFT_WIDTH)+1 ,2,0,0,TFT_HEIGHT-1);
            tmpFloat=(sin(2.0*PI*TFTDrawCounter/TFT_WIDTH)+1)/2.0;
          }
          myGLCD.setColor(VGA_WHITE);
          drawPixel(index, graph[graphIndex]);
        }
        graphCounter++;
        if (graphCounter==graphLength)
          graphCounter=0;
        /*
         *    SD : write data
         */
        char buff[sizeof("3.4028235E+38")];
        dtostrf(secs,5,3,buff);
        Serial.println(buff);
        file.print(buff);file.print(',');
        //tmpFloat(sine...)
        dtostrf(tmpFloat,5,3,buff);
        file.println(buff)/* see also file.printField() */;
/*------------------------------------------------------------------
 *      remove this code
 ***********************************
    if (!file.open("test.txt", O_CREAT | O_WRITE | O_APPEND)) {/* use O_TRUNC to erase first *//*
      error("file.open");
    }
    char buff[sizeof("3.4028235E+38")];
    float num=millis()/1000.0;
    dtostrf(num,5,3,buff);
    file.print(SDCounter1);
    file.print(',');
    file.println(buff);/* see also file.printField() *//*
    Serial.println(buff);
    file.close();
====================================================================
*/
      }
      /* 
       *        on random data array    
       */
/*    {
      /* shift random data across the screen */
      /* index : 0      <---      TFT_WIDTH */
      /* randomDataCounter ++ */
/*      for (int index=TFT_WIDTH-1;index>=0;index--){
        randomDataIndex=index+randomDataCounter;/* correct! *//*
        if (randomDataIndex>=randomDataLength)
          randomDataIndex-=randomDataLength;
        
        myGLCD.setColor(VGA_BLACK);
        int indexToErase=randomDataIndex-1;
        if (indexToErase<0)
          indexToErase=randomDataLength-1;
        if(index==0){
          int temp=randomDataIndex-1;
          if (temp<0){
            temp=randomDataLength-1;
          }
          lastFirstPixel=randomData[temp];
          drawPixel(index,lastFirstPixel);
        }else
          drawPixel(index,randomData[indexToErase]);

        if (index==TFT_WIDTH-1){
          /* the following code line was meant for when randomDataLength == TFT_WIDTH.
           * Back then, the first pixel would become last and change, so we need a
           * variable pointing to its last value
           */
//          lastFirstPixel=randomData[randomDataIndex];
          /* randomize *//*
          randomData[randomDataIndex]=random(0,TFT_HEIGHT-1);
        }
        myGLCD.setColor(VGA_WHITE);
        drawPixel(index,randomData[randomDataIndex]);
      }
      randomDataCounter++;
      if (randomDataCounter==randomDataLength)
        randomDataCounter=0;
    }
*/
 
    myGLCD.drawLine(0,midHeight,TFT_WIDTH-1,midHeight);// shouldn't it be 1 instead of 0 ? nevermind
    myGLCD.setColor(VGA_RED);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.print(TFTLoopsPerSecString,0,0);// Now, WHY is it 0,0 and not 1,1 ? nevermind

    myGLCD.setColor(VGA_WHITE);
    TFTStringOne=String(analogRead(A0))+"/1023";
    TFTStringTwo=String(h,3)+"/"+String(H0,3);
    myGLCD.print(TFTStringOne,CENTER,midHeight-12);
    myGLCD.print(TFTStringTwo,CENTER,midHeight);

/*
 *      A T O F
 *
    myGLCD.setColor(VGA_WHITE);
    TFTStringOne="1.234";
    char buffChar[6];
    TFTStringOne.toCharArray(buffChar,6);
    Serial.println(buffChar);
    float testDouble=atof(buffChar);
    Serial.println(testDouble,3);
    myGLCD.print(String(testDouble,3),0,12);
*/
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
    /*
     *    SD : write data
     */
/*------------------------------------------------------------------
 *      remove this code
 ***********************************
    if (!file.open("test.txt", O_CREAT | O_WRITE | O_APPEND)) {/* use O_TRUNC to erase first *//*
      error("file.open");
    }
    char buff[sizeof("3.4028235E+38")];
    float num=millis()/1000.0;
    dtostrf(num,5,3,buff);
    file.print(SDCounter1);
    file.print(',');
    file.println(buff);/* see also file.printField() *//*
    Serial.println(buff);
    file.close();
====================================================================
*/
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

/*------------------------------------------------------------------
 *      remove this code
 ***********************************
  if (Serial.available()) {
    // Close file and stop.
//    SDWriteHere.close();
    ifstream sdRead("test.txt");
    // check for open error
    if (!sdRead.is_open()) {
      error("open");
    }
    int itemp;
    char hi;
    float temp;
    while (true){
      if (sdRead.fail())
        break;
      sdRead >> itemp >> hi >> temp;
      sdRead.skipWhite();
/*      if (sdRead.fail()) {
        error("bad input");
      }
*//*    Serial.print(itemp);
      Serial.print(hi);
      Serial.println(temp,3);
    }
//    if (!sdRead.eof())
//      error("not eof");
    Serial.println(F("Done"));
    while(1) {}
  }
  ====================================================================
  */
  if (Serial.available()||buttonState(11)==true){
    file.flush();
    Serial.print ("End free space: ");
    Serial.println(freeSpaceString(6));

    file.close();
    myGLCD.clrScr();
    myGLCD.setColor(VGA_RED);
    myGLCD.setBackColor(VGA_BLACK);
    myGLCD.print("file closed!",CENTER,midHeight-12);
    ifstream fileRead("test.txt");
    if (!fileRead.is_open())
      error("open");
    float index;
    char comma;
    float value;
    while (true){
      if (fileRead.fail())
        break;
      fileRead >> index >> comma >> value;
      fileRead.skipWhite();
      Serial.print(index,3);Serial.print(comma);Serial.println(value,3);
    }
    myGLCD.print("Done reading from SD",CENTER,midHeight);
    while(true){}
  }
}
/*-------------------------------------------------------------------
 * 
 *                          F O R M A T
 *                          
 *==================================================================*/
/*
 * This program will format an SD or SDHC card.
 * Warning all data will be deleted!
 *
 * For SD/SDHC cards larger than 64 MB this
 * program attempts to match the format
 * generated by SDFormatter available here:
 *
 * http://www.sdcard.org/consumers/formatter/
 *
 * For smaller cards this program uses FAT16
 * and SDFormatter uses FAT12.
 */
// Print extra info for debug if DEBUG_PRINT is nonzero
#define DEBUG_PRINT 0
#include <SPI.h>
#include <SdFat.h>
#if DEBUG_PRINT
#include <SdFatUtil.h>
#endif  // DEBUG_PRINT
//
// Change the value of chipSelect if your hardware does
// not use the default value, SS.  Common values are:
// Arduino Ethernet shield: pin 4
// Sparkfun SD shield: pin 8
// Adafruit SD shields and modules: pin 10
//const uint8_t chipSelect = 53;

// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
const uint8_t spiSpeed = SPI_HALF_SPEED;

// Serial output stream
//ArduinoOutStream cout(Serial);

Sd2Card card;
uint32_t cardSizeBlocks;
uint16_t cardCapacityMB;

// cache for SD block
cache_t cache;

// MBR information
uint8_t partType;
uint32_t relSector;
uint32_t partSize;

// Fake disk geometry
uint8_t numberOfHeads;
uint8_t sectorsPerTrack;

// FAT parameters
uint16_t reservedSectors;
uint8_t sectorsPerCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;

// constants for file system structure
uint16_t const BU16 = 128;
uint16_t const BU32 = 8192;

//  strings needed in file system structures
char noName[] = "NO NAME    ";
char fat16str[] = "FAT16   ";
char fat32str[] = "FAT32   ";
//------------------------------------------------------------------------------
#define sdError(msg) sdError_F(F(msg))

void sdError_F(const __FlashStringHelper* str) {
  cout << F("error: ");
  cout << str << endl;
  if (card.errorCode()) {
    cout << F("SD error: ") << hex << int(card.errorCode());
    cout << ',' << int(card.errorData()) << dec << endl;
  }
  while (1);
}
//------------------------------------------------------------------------------
#if DEBUG_PRINT
void debugPrint() {
  cout << F("FreeRam: ") << FreeRam() << endl;
  cout << F("partStart: ") << relSector << endl;
  cout << F("partSize: ") << partSize << endl;
  cout << F("reserved: ") << reservedSectors << endl;
  cout << F("fatStart: ") << fatStart << endl;
  cout << F("fatSize: ") << fatSize << endl;
  cout << F("dataStart: ") << dataStart << endl;
  cout << F("clusterCount: ");
  cout << ((relSector + partSize - dataStart)/sectorsPerCluster) << endl;
  cout << endl;
  cout << F("Heads: ") << int(numberOfHeads) << endl;
  cout << F("Sectors: ") << int(sectorsPerTrack) << endl;
  cout << F("Cylinders: ");
  cout << cardSizeBlocks/(numberOfHeads*sectorsPerTrack) << endl;
}
#endif  // DEBUG_PRINT
//------------------------------------------------------------------------------
// write cached block to the card
uint8_t writeCache(uint32_t lbn) {
  return card.writeBlock(lbn, cache.data);
}
//------------------------------------------------------------------------------
// initialize appropriate sizes for SD capacity
void initSizes() {
  if (cardCapacityMB <= 6) {
    sdError("Card is too small.");
  } else if (cardCapacityMB <= 16) {
    sectorsPerCluster = 2;
  } else if (cardCapacityMB <= 32) {
    sectorsPerCluster = 4;
  } else if (cardCapacityMB <= 64) {
    sectorsPerCluster = 8;
  } else if (cardCapacityMB <= 128) {
    sectorsPerCluster = 16;
  } else if (cardCapacityMB <= 1024) {
    sectorsPerCluster = 32;
  } else if (cardCapacityMB <= 32768) {
    sectorsPerCluster = 64;
  } else {
    // SDXC cards
    sectorsPerCluster = 128;
  }

  cout << F("Blocks/Cluster: ") << int(sectorsPerCluster) << endl;
  // set fake disk geometry
  sectorsPerTrack = cardCapacityMB <= 256 ? 32 : 63;

  if (cardCapacityMB <= 16) {
    numberOfHeads = 2;
  } else if (cardCapacityMB <= 32) {
    numberOfHeads = 4;
  } else if (cardCapacityMB <= 128) {
    numberOfHeads = 8;
  } else if (cardCapacityMB <= 504) {
    numberOfHeads = 16;
  } else if (cardCapacityMB <= 1008) {
    numberOfHeads = 32;
  } else if (cardCapacityMB <= 2016) {
    numberOfHeads = 64;
  } else if (cardCapacityMB <= 4032) {
    numberOfHeads = 128;
  } else {
    numberOfHeads = 255;
  }
}
//------------------------------------------------------------------------------
// zero cache and optionally set the sector signature
void clearCache(uint8_t addSig) {
  memset(&cache, 0, sizeof(cache));
  if (addSig) {
    cache.mbr.mbrSig0 = BOOTSIG0;
    cache.mbr.mbrSig1 = BOOTSIG1;
  }
}
//------------------------------------------------------------------------------
// zero FAT and root dir area on SD
void clearFatDir(uint32_t bgn, uint32_t count) {
  clearCache(false);
  if (!card.writeStart(bgn, count)) {
    sdError("Clear FAT/DIR writeStart failed");
  }
  for (uint32_t i = 0; i < count; i++) {
    if ((i & 0XFF) == 0) {
      cout << '.';
    }
    if (!card.writeData(cache.data)) {
      sdError("Clear FAT/DIR writeData failed");
    }
  }
  if (!card.writeStop()) {
    sdError("Clear FAT/DIR writeStop failed");
  }
  cout << endl;
}
//------------------------------------------------------------------------------
// return cylinder number for a logical block number
uint16_t lbnToCylinder(uint32_t lbn) {
  return lbn / (numberOfHeads * sectorsPerTrack);
}
//------------------------------------------------------------------------------
// return head number for a logical block number
uint8_t lbnToHead(uint32_t lbn) {
  return (lbn % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
}
//------------------------------------------------------------------------------
// return sector number for a logical block number
uint8_t lbnToSector(uint32_t lbn) {
  return (lbn % sectorsPerTrack) + 1;
}
//------------------------------------------------------------------------------
// format and write the Master Boot Record
void writeMbr() {
  clearCache(true);
  part_t* p = cache.mbr.part;
  p->boot = 0;
  uint16_t c = lbnToCylinder(relSector);
  if (c > 1023) {
    sdError("MBR CHS");
  }
  p->beginCylinderHigh = c >> 8;
  p->beginCylinderLow = c & 0XFF;
  p->beginHead = lbnToHead(relSector);
  p->beginSector = lbnToSector(relSector);
  p->type = partType;
  uint32_t endLbn = relSector + partSize - 1;
  c = lbnToCylinder(endLbn);
  if (c <= 1023) {
    p->endCylinderHigh = c >> 8;
    p->endCylinderLow = c & 0XFF;
    p->endHead = lbnToHead(endLbn);
    p->endSector = lbnToSector(endLbn);
  } else {
    // Too big flag, c = 1023, h = 254, s = 63
    p->endCylinderHigh = 3;
    p->endCylinderLow = 255;
    p->endHead = 254;
    p->endSector = 63;
  }
  p->firstSector = relSector;
  p->totalSectors = partSize;
  if (!writeCache(0)) {
    sdError("write MBR");
  }
}
//------------------------------------------------------------------------------
// generate serial number from card size and micros since boot
uint32_t volSerialNumber() {
  return (cardSizeBlocks << 8) + micros();
}
//------------------------------------------------------------------------------
// format the SD as FAT16
void makeFat16() {
  uint32_t nc;
  for (dataStart = 2 * BU16;; dataStart += BU16) {
    nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
    fatSize = (nc + 2 + 255)/256;
    uint32_t r = BU16 + 1 + 2 * fatSize + 32;
    if (dataStart < r) {
      continue;
    }
    relSector = dataStart - r + BU16;
    break;
  }
  // check valid cluster count for FAT16 volume
  if (nc < 4085 || nc >= 65525) {
    sdError("Bad cluster count");
  }
  reservedSectors = 1;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + 2 * fatSize + reservedSectors + 32;
  if (partSize < 32680) {
    partType = 0X01;
  } else if (partSize < 65536) {
    partType = 0X04;
  } else {
    partType = 0X06;
  }
  // write MBR
  writeMbr();
  clearCache(true);
  fat_boot_t* pb = &cache.fbs;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
    pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->rootDirEntryCount = 512;
  pb->mediaType = 0XF8;
  pb->sectorsPerFat16 = fatSize;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat16str, sizeof(pb->fileSystemType));
  // write partition boot sector
  if (!writeCache(relSector)) {
    sdError("FAT16 write PBS failed");
  }
  // clear FAT and root directory
  clearFatDir(fatStart, dataStart - fatStart);
  clearCache(false);
  cache.fat16[0] = 0XFFF8;
  cache.fat16[1] = 0XFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
      || !writeCache(fatStart + fatSize)) {
    sdError("FAT16 reserve failed");
  }
}
//------------------------------------------------------------------------------
// format the SD as FAT32
void makeFat32() {
  uint32_t nc;
  relSector = BU32;
  for (dataStart = 2 * BU32;; dataStart += BU32) {
    nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
    fatSize = (nc + 2 + 127)/128;
    uint32_t r = relSector + 9 + 2 * fatSize;
    if (dataStart >= r) {
      break;
    }
  }
  // error if too few clusters in FAT32 volume
  if (nc < 65525) {
    sdError("Bad cluster count");
  }
  reservedSectors = dataStart - relSector - 2 * fatSize;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + dataStart - relSector;
  // type depends on address of end sector
  // max CHS has lbn = 16450560 = 1024*255*63
  if ((relSector + partSize) <= 16450560) {
    // FAT32
    partType = 0X0B;
  } else {
    // FAT32 with INT 13
    partType = 0X0C;
  }
  writeMbr();
  clearCache(true);

  fat32_boot_t* pb = &cache.fbs32;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
    pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->mediaType = 0XF8;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->sectorsPerFat32 = fatSize;
  pb->fat32RootCluster = 2;
  pb->fat32FSInfo = 1;
  pb->fat32BackBootBlock = 6;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat32str, sizeof(pb->fileSystemType));
  // write partition boot sector and backup
  if (!writeCache(relSector)
      || !writeCache(relSector + 6)) {
    sdError("FAT32 write PBS failed");
  }
  clearCache(true);
  // write extra boot area and backup
  if (!writeCache(relSector + 2)
      || !writeCache(relSector + 8)) {
    sdError("FAT32 PBS ext failed");
  }
  fat32_fsinfo_t* pf = &cache.fsinfo;
  pf->leadSignature = FSINFO_LEAD_SIG;
  pf->structSignature = FSINFO_STRUCT_SIG;
  pf->freeCount = 0XFFFFFFFF;
  pf->nextFree = 0XFFFFFFFF;
  // write FSINFO sector and backup
  if (!writeCache(relSector + 1)
      || !writeCache(relSector + 7)) {
    sdError("FAT32 FSINFO failed");
  }
  clearFatDir(fatStart, 2 * fatSize + sectorsPerCluster);
  clearCache(false);
  cache.fat32[0] = 0x0FFFFFF8;
  cache.fat32[1] = 0x0FFFFFFF;
  cache.fat32[2] = 0x0FFFFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
      || !writeCache(fatStart + fatSize)) {
    sdError("FAT32 reserve failed");
  }
}
//------------------------------------------------------------------------------
// flash erase all data
uint32_t const ERASE_SIZE = 262144L;
void eraseCard() {
  cout << endl << F("Erasing\n");
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint16_t n = 0;

  do {
    lastBlock = firstBlock + ERASE_SIZE - 1;
    if (lastBlock >= cardSizeBlocks) {
      lastBlock = cardSizeBlocks - 1;
    }
    if (!card.erase(firstBlock, lastBlock)) {
      sdError("erase failed");
    }
    cout << '.';
    if ((n++)%32 == 31) {
      cout << endl;
    }
    firstBlock += ERASE_SIZE;
  } while (firstBlock < cardSizeBlocks);
  cout << endl;

  if (!card.readBlock(0, cache.data)) {
    sdError("readBlock");
  }
  cout << hex << showbase << setfill('0') << internal;
  cout << F("All data set to ") << setw(4) << int(cache.data[0]) << endl;
  cout << dec << noshowbase << setfill(' ') << right;
  cout << F("Erase done\n");
}
//------------------------------------------------------------------------------
void formatCard() {
  cout << endl;
  cout << F("Formatting\n");
  initSizes();
  if (card.type() != SD_CARD_TYPE_SDHC) {
    cout << F("FAT16\n");
    makeFat16();
  } else {
    cout << F("FAT32\n");
    makeFat32();
  }
#if DEBUG_PRINT
  debugPrint();
#endif  // DEBUG_PRINT
  cout << F("Format done\n");
}
//------------------------------------------------------------------------------
void format() {
/*  char c;
  Serial.begin(9600);
  while (!Serial) {} // wait for Leonardo

  cout << F(
         "\n"
         "This program can erase and/or format SD/SDHC cards.\n"
         "\n"
         "Erase uses the card's fast flash erase command.\n"
         "Flash erase sets all data to 0X00 for most cards\n"
         "and 0XFF for a few vendor's cards.\n"
         "\n"
         "Cards larger than 2 GB will be formatted FAT32 and\n"
         "smaller cards will be formatted FAT16.\n"
         "\n"
         "Warning, all data on the card will be erased.\n"
         "Enter 'Y' to continue: ");
  while (!Serial.available()) {}
  delay(400);  // catch Due restart problem

  c = Serial.read();
  cout << c << endl;
  if (c != 'Y') {
    cout << F("Quiting, you did not enter 'Y'.\n");
    return;
  }
  // read any existing Serial data
  while (Serial.read() >= 0) {}

  cout << F(
         "\n"
         "Options are:\n"
         "E - erase the card and skip formatting.\n"
         "F - erase and then format the card. (recommended)\n"
         "Q - quick format the card without erase.\n"
         "\n"
         "Enter option: ");

  while (!Serial.available()) {}
  c = Serial.read();
  cout << c << endl;
  if (!strchr("EFQ", c)) {
    cout << F("Quiting, invalid option entered.") << endl;
    return;
  }

*/  if (!card.begin(chipSelect, spiSpeed)) {
    cout << F(
           "\nSD initialization failure!\n"
           "Is the SD card inserted correctly?\n"
           "Is chip select correct at the top of this program?\n");
    sdError("card.begin failed");
  }

  myGLCD.clrScr();
  myGLCD.setColor(VGA_RED);
  myGLCD.setBackColor(VGA_BLACK);
  cardSizeBlocks = card.cardSize();
  if (cardSizeBlocks == 0) {
    Serial.println("cardSize");
    sdError("cardSize");
  }
  cardCapacityMB = (cardSizeBlocks + 2047)/2048;

  TFTStringOne="Card Size: "+String(cardCapacityMB)+" MiB";
  myGLCD.print(TFTStringOne,CENTER,midHeight-6);
  TFTStringOne="Erasing and formating...";
  myGLCD.print(TFTStringOne,CENTER,midHeight-6+12);  
  cout << F("Card Size: ") << cardCapacityMB;
  cout << F(" MiB, (MiB = 1,048,576 bytes)") << endl;
  char c='F';
  if (c == 'E' || c == 'F') {
    eraseCard();
  }
  if (c == 'F' || c == 'Q') {
    formatCard();
  }
  TFTStringOne="Done!";
  myGLCD.print(TFTStringOne,CENTER,midHeight-6+24);
  delay(5000);
}
