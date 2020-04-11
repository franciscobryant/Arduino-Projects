/* ------------ LED Library --------------- */

#include <Adafruit_NeoPixel.h>
#include <string.h>
#ifdef __AVR__
#include <avr/power.h>            // Required for 16 MHz Adafruit Trinket
#endif

/*-------------- Fixed Variables ------------ */

#define LEDPIN  3
#define TRIGGPIN 5
#define ECHOOPIN 6
#define TRIGPIN 7
#define ECHOPIN 8
#define NUMPIXELS 3                // Number of NEO LED Pins

/* -------------- Adjustable variables ----------- */

#define NUMCOLOUR 4
#define NUMINTENSITY 4
#define DELAYVAL 30                // Time (in milliseconds) to pause between cycles

/* -------------- Function declaration ------------ */

Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);
float distUltrasound1();
float distUltrasound2();
void colourDecode(int color, int inten);

/* -------------- Setup --------------- */

void setup() 
{
  Serial.begin(9600);
  pinMode (TRIGGPIN, OUTPUT);//Input for intensity
  pinMode (ECHOOPIN, INPUT);//Input for colour
  pinMode (TRIGPIN, OUTPUT);//Output for ultrasound sensor pulse
  pinMode (ECHOPIN, INPUT);//Input for reading ultrasound sensor pulse

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}

int inten = 0, flagInt = 0, col = 0, flagCol = 0;
int colCode[3];//0 for Red, 1 for Green, 2 for Blue, targetted value to achieve
int curColCode[3] = {0, 0, 0}; //the current value for the light

//Intensity Portion of the code
//Each number corresponds to each intensity level
int intensity[NUMINTENSITY] = {20, 80, 140, 200};   //goes from 0 to 100

/* ----------- Loop ----------- */

void loop()
{
  pixels.clear(); // Set all pixel colors to 'off'

  float cm2=distUltrasound2();
  float cm1=distUltrasound1();
  
  if (cm1<20 && !flagInt)
  {
    inten++;
    if (inten > 3)
      inten = 0;
    flagInt = 1;
  }
  if (cm1>=20)
    flagInt = 0;
 
  

  if (cm2<20 && !flagCol)
  {
    col++;
    if (col >= NUMCOLOUR)
      col = 0;
    flagCol = 1;
  }
  if (cm2>=20)
    flagCol = 0;
    
  Serial.println(String(cm1)+" "+String(cm2));
  
  colourDecode(col, intensity[inten]);
  while (curColCode[0] != colCode[0] || curColCode[1] != colCode[1] || curColCode[2] != colCode[2])
  {
    if (curColCode[0] > colCode[0])
      curColCode[0]--;
    else if (curColCode[0] < colCode[0])
      curColCode[0]++;
    if (curColCode[1] > colCode[1])
      curColCode[1]--;
    else if (curColCode[1] < colCode[1])
      curColCode[1]++;
    if (curColCode[2] > colCode[2])
      curColCode[2]--;
    else if (curColCode[2] < colCode[2])
      curColCode[2]++;
    for (int pixel = 0; pixel < NUMPIXELS; pixel++) // For loop to go through each pixel
    {
      pixels.setPixelColor(pixel, pixels.Color(curColCode[0], curColCode[1], curColCode[2]));
      pixels.show();
    }
    delay(30);
  }
  delay(DELAYVAL);
}



float distUltrasound1()         //recorded in cm
{
  long duration1;
  float cm1;

  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);

  duration1 = pulseIn(ECHOPIN, HIGH);
  cm1 = (duration1 / 2) / 29.1;
  return cm1;
}

float distUltrasound2()//recorded in cm
{
  long duration2;
  float cm2;

  digitalWrite(TRIGGPIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGPIN, LOW);

  duration2 = pulseIn(ECHOOPIN, HIGH);
  cm2 = (duration2 / 2) / 29.1;
  return cm2;
}

void colourDecode(int color, int inten)
{
  switch (color)
  {
    case 0: colCode[0] = 0;
      colCode[1] = 0;
      colCode[2] = 127;
      break;
    case 1: colCode[0] = 0;
      colCode[1] = 127;
      colCode[2] = 0;
      break;
    case 2: colCode[0] = 127;
      colCode[1] = 0;
      colCode[2] = 0;
      break;
    case 3: colCode[0] = 0;
      colCode[1] = 0;
      colCode[2] = 0;
  }

  colCode[0] = (colCode[0] * inten) / 100;
  colCode[1] = (colCode[1] * inten) / 100;
  colCode[2] = (colCode[2] * inten) / 100;
}
