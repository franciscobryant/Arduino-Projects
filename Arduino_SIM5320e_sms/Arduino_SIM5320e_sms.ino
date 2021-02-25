#include "Adafruit_FONA.h"

#define FONA_RX 3
#define FONA_TX 2
#define FONA_RST 4 //sms



#include <SoftwareSerial.h>

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

#define phoneNumber "Your phone number"



void setup() {
  // initialize serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("SMS Messages Sender");

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));

}

void loop() {
  String location = gps();
  String messages = "hello";
  char txt[messages.length() + 1];
  messages.toCharArray(txt, messages.length());

  Serial.println(txt);
  flushSerial();
    Serial.println(phoneNumber);
    Serial.println(txt);
    fona.sendSMS(phoneNumber, txt);
    if (!fona.sendSMS(phoneNumber, txt)) {
    Serial.println(F("Failed"));
    } else {
    Serial.println(F("Sent!"));
    }

    Serial.println("\nCOMPLETE!\n");
    delay(3600000);

}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}


String gps() {


  String lat, lon;
  Serial.println(F("Enabling GPS..."));
  fona.enableGPS(true);

  delay(2000);
  float latitude, longitude;
  while (!fona.getGPS(&latitude, &longitude));
  // if you ask for an altitude reading, getGPS will return false if there isn't a 3D fix
  lat = String(latitude, 6);
  lon = String(longitude, 6);
  return "Latitude: " + lat + "\nLongitude: " + lon;
}
