// ---------- Upload to Cloud -----------------------

// Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <math.h>
#include <ArduinoJson.h>

// Configure WiFi access point
#define WLAN_SSID  "your wifi's SSID"
#define WLAN_PASS  "your wifi's password"

//--------------- LED SCREEN -----------------------------

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI  D6 //D1 Pin
#define OLED_CLK   D7 //D0 Pin
#define OLED_DC    D4
#define OLED_CS    D3
#define OLED_RESET D5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

//--------------- SENSOR -----------------------------

#include "DHT.h"

#define DHTPIN D1     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);


//------- Definitions for MQ135 Sensor ------------------

#define pin A0 //Analog input 0 of your arduino
#define type 135 //MQ135

//-------------------- SETUP -------------------------------

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println(); Serial.println();
  Serial.println(F("AQUA AIR - Upload to Database"));

  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  //--- SCREEN ---
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();  
  
  //--- SENSOR ---
  dht.begin();

  Serial.println(F("Ready!"));
}

void loop() {

  //--- READ TEMPERATURE ---
  
  int f = dht.readTemperature(true);
  Serial.print(F("Temperature = ")); Serial.println(f, DEC);
  
  //--- READ HUMIDITY ---
  
  int h = dht.readHumidity();
  Serial.print(F("Humidity = ")); Serial.println(h, DEC);
  
  //--- READ AIR QUALITY ---
  
  //AQI
  double sensorValue = analogRead(A0);
  double slopeAQI = 1.7432;
  double AQI = (slopeAQI*sensorValue);
  Serial.println(AQI,4);
  
  //HCHO
  double slopeHCHO = (4-0.02)/(1023);
  double HCHO = 0.02+(slopeHCHO*(sensorValue));
  Serial.print("HCHO: ");
  Serial.println(HCHO,4);
  
  //Benzene
  double slopeBenzene = (5-0.0 )/(1023);
  double Benzene =  0.0+(slopeBenzene*(sensorValue));
  Serial.print("Benzene: ");
  Serial.println(Benzene,4);

  //The slope values above can be seen from the MQ135 Datasheet    

  //------- DISPLAY STATS ON SCREEN -------
  
  displaystats(f,h,AQI);
  delay(2000);
  
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
   
     HTTPClient http;    //Declare object of class HTTPClient
   
     http.begin("http://the-aqua.herokuapp.com/api/sensor-data");      //Specify request destination
     http.addHeader("Content-Type", "application/json");  //Specify content-type header

    //--- Serialize the Data into JSON ---
    StaticJsonBuffer<200> jsonBuffer2;
    JsonObject& SensorValues = jsonBuffer2.createObject();
    JsonObject& arduinoFrame = jsonBuffer2.createObject();

    SensorValues["temperature"] = f;
    SensorValues["humidity"] = h;
    SensorValues["AQI"] = AQI;
    SensorValues["HCHO"] = HCHO;
    SensorValues["Benzene"] = Benzene;
    arduinoFrame["SensorValues"] = SensorValues;

    String string2send;

    SensorValues.printTo(string2send);
    Serial.print(string2send);
    Serial.print("\n");
   
     int httpCode = http.POST(string2send);   //Send the request
     uploadingdisplay();    //Shows that the data is being uploaded
     String payload = http.getString();    //Get the response payload
   
     Serial.print("\nHTTP Return Code: ");
     Serial.println(httpCode);   //Print HTTP return code
     Serial.println("HTTP Response Body: ");
     Serial.println(payload);    //Print request response payload

     if (httpCode == 200) {
      uploadeddisplay();
      delay(2000);      
     }
     else   {
      notuploadeddisplay();
      delay(2000);
     }
   
     http.end();  //Close connection
   
   }else{
   
      Serial.println("Error in WiFi connection");
      notconnecteddisplay();
      delay(2000);   
   
   }
  displaystats(f,h,AQI);
  delay(56000); // No need for delay, DHT sensor takes 2 seconds to gather data :-(
}

void displaystats(int f, int h, float AQI) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Temperature:"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(f); 

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.println(F("Humidity:"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(h);

  
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.println(F("AQI:"));

  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(AQI);

  display.display();
  
}

void uploadingdisplay(void) {
  display.clearDisplay();

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Uploading Data..."));

  display.display();
}


void uploadeddisplay(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Data Uploaded"));

  display.display();
}


void notuploadeddisplay(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Data Not Uploaded, Try Again"));

  display.display();
}

void notconnecteddisplay(void)  {
  display.clearDisplay();

  display.setTextSize(1);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("WiFi Not Connected"));

  display.display();
}
