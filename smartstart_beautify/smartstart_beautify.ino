#include "Arduino.h"
#include "LiquidCrystal.h"
#include <Adafruit_Fingerprint.h> #include <SoftwareSerial.h>

// Pin Definitionen
#define LCD_PIN_RS 7
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
#define NEWRS 13
#define PCRS 9
#define POWERON_LED 12

//sonstige Definitionen
# define MASTER_ID 1

// Objekte werden initialisiert
LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_E, LCD_PIN_DB4, LCD_PIN_DB5, LCD_PIN_DB6, LCD_PIN_DB7); //LCD
SoftwareSerial mySerial(FINGERPRINT_TX, FINGERPRINT_RX); //Konsole
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); //Fingerprintsensor mit Adafruit Library

//Variablen initialisierung

uint8_t getid = -1;
uint8_t id = 0;
uint8_t erk = 0;

int buttonResetPushCounter = 0;
int buttonResetState = 0;
int lastButtonResetState = LOW;
int buttonMenuState = 0;
int lastButtonMenuState = LOW;

//millis Definitionen

unsigned long m_time;
unsigned long passedtime;
unsigned long interval = 1000;

void setup() {
    pinMode(NEWRS, INPUT);
    pinMode(PCRS, INPUT);
    pinMode(POWERON_LED, INPUT);
    Serial.begin(9600);
    while (!Serial);
    delay(100);

    lcd.begin(LCD_COLUMNS, LCD_ROWS);

    finger.begin(57600);

    //Sensor Status
    if (finger.verifyPassword()) {
        Serial.println("Sensor gefunden!");
    } else {
        Serial.println("Keinen Sensor gefunden!");
        while (1) {
            delay(1);
        }
    }

    //Informatinen zum Speicherstand
    finger.getTemplateCount();
    id = finger.templateCount;
    Serial.print("Der Sensor enthält ");
    Serial.print(finger.templateCount);
    Serial.println(" Vorlagen");
    Serial.println("Es wird auf einen berechtigten Finger gewartet...");
}

void loop() {
    buttonResetState = digitalRead(PCRS);
    buttonMenuState = digitalRead(NEWRS);
    lcd.setCursor(0, 0);
    checkFPDatabase();
    m_time = millis();
    passedtime = millis() - m_time;

    if (digitalRead(PCRS) && digitalRead(NEWRS) == HIGH) {
        clearFPDatabase();
        delay(3000);
    }

    if (m_time - passedtime > interval) {
        getFingerprintIDez();
        if (erk == 1) {
            startComputer();
        }
        lcd.clear();

    }
    if (m_time - passedtime > interval) {
        if (buttonResetState != lastButtonResetState) {
            if (buttonResetState == HIGH) {
                resetComputer();
            }
            delay(50);

        }
    }
    lastButtonResetState = buttonResetState;

    if (m_time - passedtime > interval) {
        if (buttonMenuState != lastButtonMenuState) {
            if (buttonMenuState == HIGH) {
                newid();
                erk = 0;
            }
            delay(50);
        }
    }
    lastButtonMenuState = buttonMenuState;
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
    Serial.print("Erkannte ID #");
    Serial.print(finger.fingerID);
    Serial.print("\n");
    lcd.print("Erkannte ID: #");
    lcd.print(finger.fingerID);
    erk = 1;
    return finger.fingerID;
}

//return -1 (Abbruch) bei Fehler, sonst wird Fingerabdruck-ID wiedergegeben
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
    Serial.print("Erkannte ID #");
    Serial.print(finger.fingerID);
    Serial.print("\n");
    lcd.print("Erkannte ID: #");
    lcd.print(finger.fingerID);
    erk = 1;
    return finger.fingerID;
}


//Master (Admin) Fingerabdruck wird beim ersten Start (oder nach Reset) gefordert
uint8_t getFingerprintEnrollMaster() {
    delay(2000);
    int p = -1;
    Serial.print("Keinen Fingerabdruck gefunden! Bitte Master-Fingerabdruck einscannen.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kein Finger-");
    lcd.setCursor(0, 1);
    lcd.print("abdruck gefunden");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bitte Master-");
    lcd.setCursor(0, 1);
    lcd.print("Finger scannen");
    delay(2000);
    lcd.clear();

    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Scan erfasst! :)");
            lcd.clear();
            lcd.print("Scan erfasst!");
            delay(2000);
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("Kein Finger erkannt!");
            return p;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Verbindungsfehler");
            lcd.clear();
            lcd.print("Verbindungsfehler");
            return p;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Scanfehler");
            lcd.clear();
            lcd.print("Scanfehler");
            return p;
        default:
            Serial.println("Error 404");
            lcd.clear();

            lcd.print("Error 404");
            return p;
        }
    }

    p = finger.image2Tz(1);
    switch (p) {
    case FINGERPRINT_OK:
        Serial.println("Scan umgewandelt");
        lcd.clear();
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
        lcd.clear();    
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
        lcd.clear();
        lcd.print("Error 404");
        return p;
    }

    Serial.println("Remove finger");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Finger entfernen");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(MASTER_ID);
    p = -1;
    Serial.println("Selben Finger wieder auflegen");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Selben Finger");
    lcd.setCursor(0, 1);
    lcd.print("erneut auflegen");

    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Fingerabdruck");
            lcd.setCursor(0, 1);
            lcd.print("gescannt!");
            delay(2000);
            lcd.clear();
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }


    p = finger.image2Tz(2);
    switch (p) {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("Creating model for #");
    Serial.println(MASTER_ID);

    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
        Serial.println("Prints matched!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(MASTER_ID);
    p = finger.storeModel(MASTER_ID);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
}

//
// Methoden
//

//Computer bootet mit den definierten Werten
void startComputer() {
    Serial.println("COMPUTER AN!");
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("PC gestartet");
    analogWrite(POWERPC, VALUEPCV);
    delay(400);
    analogWrite(POWERPC, 0);
    erk = 0;
}

//Hardreset durch zehn-sekuendige Powerzufuhr nach Eingabe eines gueltigen Fingerabdrucks
void resetComputer() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Finger scannen");
    lcd.setCursor(0, 1);
    lcd.print("zum resetten");
    Serial.println(erk);
    delay(1000);
    while (erk == 0) {
        getFingerprintIDez();

        buttonResetState = digitalRead(PCRS);

        if (buttonResetState == HIGH) {
            return -1;
        }
        if (erk == 1) {
            Serial.println("COMPUTER RESET!");
            analogWrite(POWERPC, VALUEPCV);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Computer wird");
            lcd.setCursor(0, 1);
            lcd.print("resettet (10s)");
            delay(10000);
            analogWrite(POWERPC, 0);
            erk = 0;
        }
    }

}

//Prueft beim Start, ob die Datenbank leer ist. Falls ja, wird auf eine Methode zum Erstellen verwiesen
void checkFPDatabase() {
    finger.getTemplateCount();
    if (finger.templateCount == 0) {
        getFingerprintEnrollMaster();
    }

}

//Datenbank wird nach Eingabe eines gueltigen Fingerabdrucks komplett gelöscht
void clearFPDatabase() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SmartStart");
    lcd.setCursor(0, 1);
    lcd.print("   Reset-Menu");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bitte Master-");
    lcd.setCursor(0, 1);
    lcd.print("Finger scannen");
    delay(2000);
    uint8_t t_id = 0;
    while (t_id != 1) {
        getFingerprintIDez();

        buttonMenuState = digitalRead(NEWRS);

        if (buttonMenuState == HIGH) {
            return -1;
        }

        t_id = finger.fingerID;

        if (t_id == 1) {
            finger.emptyDatabase();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("   SmartStart");
            lcd.setCursor(0, 1);
            lcd.print("    resettet");
            delay(1000);
        }
    }

}


//Hilfsmethode zum Erstellen eines weiteren Fingerabdrucks
void newid() {
    delay(1000);
    uint8_t t_id = 0;
    while (t_id != 1) {
        getFingerprintIDez();

        buttonMenuState = digitalRead(NEWRS);
        buttonResetState = digitalRead(PCRS);

        if (buttonMenuState == HIGH) {
            return -1;
        }

        if (buttonResetState == HIGH) {
            clearFPDatabase();
            return -1;
        }

        t_id = finger.fingerID;

        if (erk == 1) {
            delay(4000);
            getFingerprintEnroll();

        }
    }
}


//Neuer Fingerabdruck wird gescannt und gespeichert. 
uint8_t getFingerprintEnroll() {
    id = finger.templateCount + 1;
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");
    Serial.println(id);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bitte neuen");
    lcd.setCursor(0, 1);
    lcd.print("Finger auflegen");
    delay(1000);
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }


    p = finger.image2Tz(1);
    switch (p) {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Finger jetzt");
    lcd.setCursor(0, 1);
    lcd.print("wieder entfernen");
    delay(4000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
        p = finger.getImage();
    }
    Serial.print("ID ");
    Serial.println(id);
    p = -1;
    Serial.println("Place same finger again");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Selben Finger");
    lcd.setCursor(0, 1);
    lcd.print("erneut auflegen");
    delay(1500);
    while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
        case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
        case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
    }


    p = finger.image2Tz(2);
    switch (p) {
    case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
    case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
    default:
        Serial.println("Unknown error");
        return p;
    }

    // OK converted!
    Serial.print("Creating model for #");
    Serial.println(id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
        Serial.println("Prints matched!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Finger stimmen");
        lcd.setCursor(0, 1);
        lcd.print("ueberein!");
        delay(1500);
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID ");
    Serial.println(id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");
        lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Neuer Abdruck");
      lcd.setCursor(0, 1);
      lcd.print("gespeichert");
      delay(1500);
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FINGERPRINT_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
}
