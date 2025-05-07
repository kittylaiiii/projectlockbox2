//dont wire IRQ
//rst 9, miso 12, mosi 11, sca 13, sda 10
#include <MFRC522.h>

#include <MFRC522Extended.h>

#include <deprecated.h>

#include <require_cpp11.h>


//wire with 3.3 v not 5v because that will fry it


#include <SPI.h>

#include <MFRC522.h>


#include <Servo.h>
#define SPIN_PIN 2
Servo spin;  //make a servo object called spin

#define RST_PIN 9  // Configurable, see typical pin layout above

#define SS_PIN 10  // Configurable, see typical pin layout above


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance


//bluetooth stuff:
int const RX_PIN = 8;  //receiving bluetooth
int const TX_PIN = 7;  //transmitting bluetooth
#define GREEN_LED 5
#define BUZZER 6


#include <SoftwareSerial.h>  //include library

SoftwareSerial tooth(TX_PIN, RX_PIN);  //create a softwareserial object, set tx and rx pins, tx goes first then rx

String input = " ";  //an empty string to store


//ultrasonic sensor stuff:
#define TRIGGER_PIN 4  //this is the pin that sends out the pulse
#define ECHO_PIN 3     //this is the pin that reads the distance

void setup() {

  Serial.begin(9600);  // Initialize serial communications with the PC
  Serial.setTimeout(10);

  while (!Serial)
    ;  // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  SPI.begin();  // Init SPI bus

  mfrc522.PCD_Init();  // Init MFRC522

  delay(4);  // Optional delay. Some board do need more time after init to be ready, see Readme

  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details

  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));


  tooth.begin(9600);  //can print on device


  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);


  //ultrasonic sensor stuff:
  pinMode(TRIGGER_PIN, OUTPUT);  //output bc it sends pulse out
  pinMode(ECHO_PIN, INPUT);      //bc it is reading what is coming in
  spin.attach(2);
}


void loop() {

  // spin.write(60);  //close
  // delay(8900);
  // spin.write(120);  //open
  // delay(8900);






  //ultrasonic sensor stuff

  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);  //this is how long it has to wait to send out a pulse

  float duration = pulseIn(ECHO_PIN, HIGH);  // tell us the time from pulse sent to pulse received
  Serial.println(duration);

  //v=s/t
  float speed = 0.034;  //measured in cm/microseconds
  float distance = (speed * duration) / 2;


  delay(100);




  if (Serial.available() > 0) {
    String userInput = "";
    userInput = Serial.readString();
    Serial.print("User input: " + userInput);
    if (userInput.startsWith("lock")) {
      Serial.println("Locking...Locking...");
      spin.write(60);  //close
      delay(8900);
    }
  }


  if (distance <= 10) {
    Serial.println("Step back! You're too close!");
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(100);
  } else {
    Serial.println("Enter entrance code on phone: ");
    tooth.print("Type something");
    if (tooth.available() > 0) {  //if something is inputed on device
      input = tooth.read();
      Serial.println("Device Verified");
      Serial.println("Password correct! Swipe RFID");
      digitalWrite(GREEN_LED, HIGH);
      delay(1000);
      digitalWrite(GREEN_LED, LOW);
      Serial.println("Access granted.");
      delay(1000);
      spin.write(120);
      delay(8900);
    }
  }






  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.

  if (!mfrc522.PICC_IsNewCardPresent()) {

    return;
  }


  // Select one of the cards

  if (!mfrc522.PICC_ReadCardSerial()) {

    return;
  }




  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);



  Serial.print(F("RFID Tag UID:"));

  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);

  Serial.println("");



  mfrc522.PICC_HaltA();  // Halt PICC

  byte authorizedUID[] = { 0x96, 0xDE, 0x29, 0x03 };  // Example UID (change to your real UID)
  byte uidLength = 4;                                 // Length of the UID
  if (mfrc522.uid.size == uidLength) {
    boolean authorized = true;
    for (byte i = 0; i < uidLength; i++) {
      if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
        authorized = false;
        break;
      }
    }

    if (authorized) {
      Serial.println("Access granted.");
      Serial.println("hi");
      spin.write(60);  //close
      delay(8900);


    } else {
      Serial.println("Wrong RFID. Access not granted.");
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
      delay(100);
    }
  }
}

void printHex(byte* buffer, byte bufferSize) {



  //Serial.begin("reading?");

  for (byte i = 0; i < bufferSize; i++) {

    Serial.print(buffer[i] < 0x10 ? " 0" : " ");

    Serial.print(buffer[i], HEX);
  }
}
