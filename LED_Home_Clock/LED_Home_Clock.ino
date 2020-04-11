/*
 * PROTOTYPE 1
 * 32 x 8 LED matrix date-clock-temperature-humidity display
 *
 * components: 
 * - DS3231 real time clock
 * - DHT11 temperature and humidity sensor 
 * - 4 x MAX7219 8x8 LED matrix
 * - Arduino Nano
*/


                                // Include libraries


#include <stdio.h>                          // C standard input output library
#include <DS3231.h>                         // library for DS3231 RealTimeClock
#include <LedControlMS.h>                   // library for 8x8 LED matrix
#include <Wire.h>                           // library for communicating with I2C / TWI devices
#include <DHT.h>                            // library for DHT11 Temperature Sensor




                                    // Define pins


#define DHTPIN 8                            // temperature sensor pin
#define DHTTYPE DHT11                       // temperature sensor type

#define CLOCKSDAPIN 5                       // DS3231 SDA pin
#define CLOCKSCLPIN 6                       // DS3231 Serial Clock pin

#define LCDINPIN 12                         // LC DataIn pin
#define LCCLKPIN 10                         // LC CLK pin
#define LCLOADPIN 11                        // LC LOAD pin
#define NBR_MTX 4                           // Number of Matrixes


                    

                                    // Setup objects


LedControl lc = LedControl(LCDINPIN, LCCLKPIN, LCLOADPIN, NBR_MTX);   // Create a lc object

DS3231  rtc(CLOCKSDAPIN, CLOCKSCLPIN);      // Create a rtc object

DHT dht(DHTPIN, DHTTYPE);                   // Create a dht object




                                  // Initialize variables


int ledBrightness = 5;                      // Can go from 0 -15
int numOfTimeLoops = 5;                     // num of loops the time will display
int delayTimeStatic = 2000;                 // display time for the static texts (temp, hum, etc. )
int animDelay = 55;                         // delay time for animations
int delayAnimStatic = 500;
int ledUpdateDelay = 20;                    // delay time between drawing LED columns
int nextLineDelay = 1500;                   // delay time between multiple line displays
int destMatrix, destCol;
int animType = 0;
String message = "Hello brother!";            // message that YOU want to show
String space = "   ";
String scrollString = message + space;       // make the message to look good
int stringLength = scrollString.length();    // length of your message
char ch0, ch1, ch2, ch3;
int nextCharIndex = 4;                       // variables for the displayMsg() function

const char * DS3231ErrorMessage = "*RTC ERROR";   // error message



                                          // Setup


void setup() {

  rtc.begin();                                // Start the rtc
//  setTime();                                // Run once to initialize the date and time of the rtc
  dht.begin();                                // Start the dht
    
  int devices = lc.getDeviceCount();          // Initialize the LED Matrix
  for(int address = 0; address < devices; address++) {
    lc.shutdown(address, false);
    // Set the brightness
    lc.setIntensity(address, ledBrightness);
    lc.clearDisplay(address);
  }

  ch0= scrollString[0];                       // variables for the displayMsg() function
  ch1= scrollString[1];
  ch2= scrollString[2];
  ch3= scrollString[3];

  Serial.begin(9600);                         // for debugging purpose
}




                                          // Main loop


void loop() {

    displayAnim();
    
    displayMsg();
    
    displayAnim();
  
    displayDate();
    
    displayTime();
    
    displayTemp();
    
}




                                           // Functions


void setTime() {
  // Initialize a new chip by turning off write protection and clearing the
  // clock halt flag. These methods needn't always be called. See the DS1302
  // datasheet for details.
  rtc.setDOW(MONDAY);
  rtc.setTime(22, 8, 00);
  rtc.setDate(9, 3, 2020);
}

void displayDate() {
  Time t = rtc.getTime();

  String day = dayAsString(rtc.getDOWStr());
  char dayString[20];
  snprintf(dayString, sizeof(dayString), "%s ", day.c_str());
  displayStringCondensed(1, dayString);  
  delayAndClearLed();

  String month = monthAsString(t.mon);
  char dateString[8];
  snprintf(dateString, sizeof(dateString), "%02d||%s", t.date, month.c_str());
  displayStringCondensed(1, dateString);  
  delayAndClearLed();

  char yearString[5];
  snprintf(yearString, sizeof(yearString), "%04d", t.year);
  displayStringCondensed(4, yearString);  
  delayAndClearLed();
}

void displayTime() {
  for (int i = 0; i <= numOfTimeLoops; i++) {    
    displayStringCondensed(1, rtc.getTimeStr(FORMAT_SHORT));  

    lc.setLed(1,4,0,false);
    lc.setLed(1,1,0,false);
    delay(800);
    lc.setLed(1,4,0,true);
    lc.setLed(1,1,0,true);
    delay(800);
  }  
}

void displayTemp () {
  int intValue, fracValue;
 
    char tempString[10];
    intValue = dht.readTemperature();
    fracValue = dht.readTemperature() * 10 - intValue * 10;
    
    snprintf(tempString, sizeof(tempString), "%02d.|%01d||C", intValue, fracValue);
    displayStringCondensed(2, tempString);
    delayAndClearLed();
 

    char humString[10];    
    intValue = dht.readHumidity();

    snprintf(humString, sizeof(humString), "%02d||/ ", intValue);
    displayStringCondensed(7, humString);
    delayAndClearLed();
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
      delay(delayAnimStatic);
      break;
      
    case 1:
      for (int row = 0; row <= 7; row++) {
        for (int matrix = 0; matrix <= 3; matrix++) {
          lc.setRow(matrix, row, B11111111);
        }
        delay(animDelay);
      }
      delay(delayAnimStatic);
      break;
  }
  lc.clearAll();
  animType++;
  if (animType > 1)   
    animType = 0;
  
}

void displayStringCondensed (int startCol, char * displayString) {
  int i;
  char c;
  destMatrix = 0;
  destCol = 7 - startCol;

  for (i = 7; i > (7 - startCol); i--)                      // clear first columns if the startCol > 0
    lc.setColumn(destMatrix, i, B00000000);

  while (displayString[0] != 0) {
    c = displayString[0];

    if (destMatrix == 4 || displayString[0] == '~') {       // next line when we're out of matrices or a newline character is sent: ~
      clearLastColumns();
      destMatrix = 0;
      destCol = 7 - startCol;

      if (displayString[0] == '~') {
        displayString++;
        c = displayString[0];
      }
      delay(nextLineDelay);
    }

    if (displayString[0] == '|') {                          // display one blank column
      lc.setColumn(destMatrix, destCol, B00000000);
      increaseColCounter();
    }
    else if (displayString[0] == '.') {                     // display a point in a single column
      lc.setColumn(destMatrix, destCol, B10000000);
      increaseColCounter();
    }
    else if (displayString[0] == '!') {                     // display a !
      lc.setColumn(destMatrix, destCol, B10111111);
      increaseColCounter();
    }
    else if (displayString[0] == '/') {                     // display a % sign, % sign won't work because of the snprintf function
      lc.setColumn(destMatrix, destCol, B11000011);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B00110011);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B11001100);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B11000011);
      increaseColCounter();
    }
    else if (displayString[0] == '*') {                     // display a *
      lc.setColumn(destMatrix, destCol, B00100010);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B00010100);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B01111111);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B00010100);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B00100010);
      increaseColCounter();
      lc.setColumn(destMatrix, destCol, B00000000);
      increaseColCounter();
    }
    else {
      if (destMatrix == 3 && destCol < 4) {                 // the character does not fit so enforce a new line
        clearLastColumns();
        destMatrix = 0;
        destCol = 7 - startCol;
        displayString--;
        delay(nextLineDelay);
      }
      else {
        int pos = lc.getCharArrayPosition(c);
    
        for (i = 0; i < 6; i++) {
          lc.setColumn(destMatrix, destCol, alphabetBitmap[pos][i]);          
          increaseColCounter();
        }
      }
    }
    
    displayString++;
  } 

  clearLastColumns();
}

void increaseColCounter() {
  destCol--;
  
  if (destCol < 0) {
    destMatrix++;
    destCol = 7;
  }
  
  delay(ledUpdateDelay);
}

void clearLastColumns() {
  for (int i = destMatrix * 8 + destCol; i < 4 * 8; i++) {
    lc.setColumn(destMatrix, destCol, B00000000);
    increaseColCounter();
  }
}

void delayAndClearLed() {
  delay(delayTimeStatic);
}

String dayAsString(char *day) {
  if (strcmp(day,"Monday") == 0)  {
    return "|||Mon- day";
  }
  else if (strcmp(day,"Tuesday") == 0)  {
    return "Tues- day";
  }
  else if (strcmp(day,"Wednesday") == 0)  {
    return "|||Wed-ness- day";
  }
  else if (strcmp(day,"Thursday") == 0)  {
    return "Thurs|||-day";
  }
  else if (strcmp(day,"Friday") == 0)  {
    return "|||Fri- day";
  }
  else if (strcmp(day,"Saturday") == 0)  {
    return "Satur|||-day";
  }
  else if (strcmp(day,"Sunday") == 0)  {
    return "|||Sun- day";
  }
  else
    return DS3231ErrorMessage ;
}

String monthAsString(int month) {
  switch (month) {
    case 1:  return "Jan";
    case 2:  return "Feb";
    case 3:  return "Mar";
    case 4:  return "Apr";
    case 5:  return "May";
    case 6:  return "Jun";
    case 7:  return "Jul";
    case 8:  return "Aug";
    case 9:  return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
  }
  return DS3231ErrorMessage ;
}

void displayMsg(){

    while (nextCharIndex<=stringLength)
    {
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
    if (nextCharIndex>=stringLength) 
      nextCharIndex=0;
}
