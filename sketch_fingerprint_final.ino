// Einbinden der Bibliotheken
#include "Arduino.h"
#include "LiquidCrystal.h"
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>


// Pin Definitionen
#define LCD_PIN_RS  7
#define LCD_PIN_E 6
#define LCD_PIN_DB4 2
#define LCD_PIN_DB5 3
#define LCD_PIN_DB6 4
#define LCD_PIN_DB7 5
#define FINGERPRINT_TX 10
#define FINGERPRINT_RX 11
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define POWERPC 8
#define VALUEPCV 168

// Objekte werden initialisiert
LiquidCrystal lcd(LCD_PIN_RS,LCD_PIN_E,LCD_PIN_DB4,LCD_PIN_DB5,LCD_PIN_DB6,LCD_PIN_DB7);
SoftwareSerial mySerial(FINGERPRINT_TX, FINGERPRINT_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//Variablen initialisierung

int erk = 0;

void setup()  
{
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("start");
  Serial.println("\n\nAdafruit Fingerscanner Test");

  // Setzten der Display Größe (Reihen und Zeilen)
     lcd.begin(LCD_COLUMNS, LCD_ROWS);
  // datenrate für den Seriellen Port
      finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Sensor gefunden!");
  } else {
    Serial.println("Keinen Sensor gefunden!");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("Der Sensor enthält "); Serial.print(finger.templateCount); Serial.println(" Vorlagen");
  Serial.println("Es wird auf einen berechtigten Finger gewartet...");
}

void loop()  {                   // run over and over again 
  lcd.setCursor(0, 0);
  getFingerprintIDez();
  Serial.println(getFingerprintIDez());
    if (erk == 1) {
      analogWrite(POWERPC, VALUEPCV);
      delay(400);
      analogWrite(POWERPC, 0);
      erk = 0;
    }
  
  delay(1000); //don't ned to run this at full speed.
  lcd.clear();
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Scan erfasst! :)");
      lcd.print("Scan erfasst! :)");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("Kein Finger erkannt!");
      lcd.print("Finger nicht");
      lcd.setCursor(0, 1);
      lcd.print("erkannt!");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Verbindungsfehler");
      lcd.print("Verbindungsfehler");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Scanfehler");
      lcd.print("Scanfehler");
      return p;
    default:
      Serial.println("Error 404");
      lcd.print("Error 404");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Scan umgewandelt");
      lcd.print("Scan umgewandelt");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Abdruck nicht eindeutig");
      lcd.print("Abdruck");
      lcd.setCursor(0, 1);
      lcd.print("nicht eindeutig");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Verbindungsfehler");
      lcd.print("Verbindungsfehler");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Keine übereinstimmenden Eigenschaften im Fingerabdruck");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Keine übereinstimmenden Eigenschaften im Fingerabdruck");
      return p;
    default:
      Serial.println("Error 404");
      lcd.print("Error 404");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerabdruck erkannt!");
    lcd.print("Abdruck erkannt!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Übertragungsfehler");
    lcd.print("Verbindungsfehler");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Kein Finger erkannt");
    lcd.print("Kein Finger");
    lcd.setCursor(0, 1);
    lcd.print("erkannt!");
    return p;
  } else {
    Serial.println("Unknown error");
    lcd.print("Error 404");
    return p;
  }   
  
  // Finger erkannt
  Serial.print("Erkannte ID #"); Serial.print(finger.fingerID); Serial.print("\n");
  lcd.print("Erkannte ID: #"); lcd.print(finger.fingerID);
  erk = 1;
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
      return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) { 
      return -1;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
      Serial.print("Fingerabdruck nicht erkannt! Bitte Finger entfernen und erneut auflegen.\n");
      lcd.print("Finger nicht");
      lcd.setCursor(0, 1);
      lcd.print("erkannt!");
      delay(1000); 
      lcd.setCursor(0, 0);
      lcd.print("Bitte Finger");
      lcd.setCursor(0, 1);
      lcd.print("neu auflegen!");
      return -1;
  }
  
   // Finger erkannt
  Serial.print("Erkannte ID #"); Serial.print(finger.fingerID); Serial.print("\n");
  lcd.print("Erkannte ID: #"); lcd.print(finger.fingerID);
//  Serial.print(" mit einer Zuverlässigkeit von "); Serial.println(finger.confidence); Serial.print("%");
  erk = 1;
  return finger.fingerID; 
}
