//We always have to include the library
#include "LedControlMS.h"

/*
Now we need a LedControl to work with.
***** These pin numbers will probably not work with your hardware *****
pin 12 is connected to the DataIn
pin 11 is connected to the CLK
pin 10 is connected to LOAD
We have only a single MAX72XX.
*/
#define NBR_MTX 4
int animType = 0;
int animDelay = 30;
LedControl lc=LedControl(12,11,10, NBR_MTX);


/* we always wait a bit between updates of the display */
String message = "HEHEHE";
String space = "   ";
String scrollString= message + space;
int stringLength=scrollString.length();
char ch0, ch1, ch2, ch3;
int nextCharIndex=4;


void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  Serial.begin (9600);
  Serial.println("Setup");
  Serial.println(scrollString);
  Serial.println(stringLength);

  for (int i=0; i< NBR_MTX; i++){
    lc.shutdown(i,false);
  /* Set the brightness to a low value */
    lc.setIntensity(i,1);
  /* and clear the display */
    lc.clearDisplay(i);
  }
  delay(100);
  lc.clearAll();
  ch0= scrollString[0];
  ch1= scrollString[1];
  ch2= scrollString[2];
  ch3= scrollString[3];
}

void loop(){
  displayMsg();
  displayAnim();
}

void displayMsg(){
  while (nextCharIndex<=stringLength){
  lc.displayChar(0, lc.getCharArrayPosition(ch0));
  lc.displayChar(1, lc.getCharArrayPosition(ch1));
  lc.displayChar(2, lc.getCharArrayPosition(ch2));
  lc.displayChar(3, lc.getCharArrayPosition(ch3));
  ch0=ch1;
  ch1=ch2;
  ch2=ch3;
  ch3=scrollString[nextCharIndex++];
  delay(300);
  }
    if (nextCharIndex>=stringLength) nextCharIndex=0;
}

void displayAnim() {
  switch (animType) {
    case 0: 
      for (int row = 0; row <= 7; row++) {
        for (int matrix = 0; matrix <= 3; matrix++) {
          lc.setRow(matrix, 7 - row, B11111111);
        }
        delay(animDelay);
      }
      delay(400);
      break;
      
    case 1:
      for (int row = 0; row <= 7; row++) {
        for (int matrix = 0; matrix <= 3; matrix++) {
          lc.setRow(matrix, row, B11111111);
        }
        delay(animDelay);
      }
      delay(400);
      break;
  }

  lc.clearAll();
  animType++;
  if (animType > 1)   
    animType = 0;

}
