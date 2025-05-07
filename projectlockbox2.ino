//rst 9, miso 12, mosi 11, sca 13, sda 10
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <SoftwareSerial.h>  //include library

#define SPIN_PIN 2
#define RST_PIN 9
#define SS_PIN 10
int const RX_PIN = 8;  //receiving bluetooth
int const TX_PIN = 7;  //transmitting bluetooth
#define GREEN_LED 5
#define BUZZER 6
#define TRIGGER_PIN 4  //the pin that sends out the pulse
#define ECHO_PIN 3     //the pin that reads the distance

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

SoftwareSerial tooth(TX_PIN, RX_PIN);  //create a softwareserial object, set tx and rx pins, tx goes first then rx

Servo spin;  //make a servo object called spin

void setup() {

  Serial.begin(9600);
  Serial.setTimeout(10);

  while (!Serial)
    ;  // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();                        // Init SPI bus
  mfrc522.PCD_Init();                 // Init MFRC522
  delay(4);                           // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  tooth.begin(9600);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);  //output because it sends pulse out
  pinMode(ECHO_PIN, INPUT);      //input because it is reading what is coming in

  spin.attach(2);
}


void loop() {

  digitalWrite(TRIGGER_PIN, LOW);  //trigger pin sends out pulse
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);  //this is how long it has to wait to send out a pulse

  float duration = pulseIn(ECHO_PIN, HIGH);  // tell us the time from pulse sent to pulse received
  Serial.println(duration);

  //v=s/t
  float speed = 0.034;  //measured in cm/microseconds
  float distance = (speed * duration) / 2;
  delay(100);


  if (distance <= 10) {  //if the distance is too close, the buzzer will beep and a warning will show
    Serial.println("Step back! You're too close!");
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  } else {  //else, if the distance is far enough, use will be told to enter something on their bluetooth verified device to grant access to unlock
    Serial.println("Enter entrance code on phone: ");
    tooth.print("Type something");
    if (tooth.available() > 0) {  //if something is inputed on the bluetooth device, the green LED will light and open the lock
      input = tooth.read();
      Serial.println("Device Verified");
      digitalWrite(GREEN_LED, HIGH);
      delay(1000);
      digitalWrite(GREEN_LED, LOW);
      Serial.println("Access granted.");
      delay(1000);
      spin.write(120);
      delay(8900);
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent()) {  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {  // Select one of the cards
    return;
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  Serial.print(F("RFID Tag UID:"));

  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);

  Serial.println("");

  mfrc522.PICC_HaltA();  // Halt PICC

  byte authorizedUID[] = { 0x96, 0xDE, 0x29, 0x03 };  // My UID
  byte uidLength = 4;                                 // Length of the UID
  if (mfrc522.uid.size == uidLength) {                //verify if the UID being read matches the one that is entered into the system
    boolean authorized = true;
    for (byte i = 0; i < uidLength; i++) {
      if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
        authorized = false;
        break;
      }
    }

    if (authorized) {  //if the UID read by RFID is the same as the pre-entered UID, then access will be given to close the lock
      Serial.println("Access granted.");
      spin.write(60);
      delay(8900);

    } else {  //else, if another UID is read, access will not be granted, and the buzzer will go off
      Serial.println("Wrong RFID. Access not granted.");
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
      delay(100);
    }
  }
}

void printHex(byte* buffer, byte bufferSize) {  //method to print the RFID
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
