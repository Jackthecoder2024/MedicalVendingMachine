/* 

// FILE NOT USED

#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ThingSpeak.h>

// Wi-Fi Credentials
const char* ssid = "YourWiFiSSID";
const char* pass = "YourWiFiPassword";

// ThingSpeak API Key
String writeAPIKey = "WEF3HL1FXU89TT1Z";
unsigned long channelID = "2778937";

WiFiClient client;

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID
#define RST_PIN 5
#define SS_PIN 21
MFRC522 rfid(SS_PIN, RST_PIN);

// LED Pins
const int redPin = 7;
const int yellowPin = 8;
const int greenPin = 13;

// Ultrasonic Sensor
const int SENSOR_IN = 3;
const int SENSOR_OUT = 2;
const int user_distance = 50;

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pinRow[ROWS] = {45, 43, 41, 39};
byte pinCol[COLS] = {37, 35, 33, 31};
Keypad keypad = Keypad(makeKeymap(keys), pinRow, pinCol, ROWS, COLS);

// Stepper Motors
Stepper motor1(2048, 12, 10, 11, 9);
Stepper motor2(2048, 38, 42, 40, 44);

// Authorized UID
byte authorizedUID[] = {0x43, 0x18, 0xFA, 0x29};

String item = "";
String passWord = "";
String rfidID = "";

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  motor1.setSpeed(10);
  motor2.setSpeed(10);

  SPI.begin();
  rfid.PCD_Init();

  // Wi-Fi Connection
  connectToWiFi();

  ThingSpeak.begin(client);
}

void loop() {
  if (checkDistance() > user_distance) {
    lcd.setCursor(0, 0);
    lcd.print("Waiting for user...");
    delay(500);
    return;
  }

  lcd.clear();
  lcd.print("Scan card:");
  
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
    rfidID = getUID(rfid.uid.uidByte, rfid.uid.size);
    lcd.clear();
    lcd.print("Enter Pass:");
    passWord = passCheck();

    if (passWord == "1523") {
      lcd.clear();
      lcd.print("Enter Item:");
      item = itemCode();
      lcd.clear();
      lcd.print("Dispensing...");
      
      if (item == "12A") dispenseItem(motor1);
      else if (item == "13B") dispenseItem(motor2);
      else lcd.print("Item not found");

      sendDataToThingSpeak(rfidID, item);
    } else {
      lcd.print("Wrong Password");
    }
  } else {
    lcd.print("Access Denied");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Functions

void connectToWiFi() {
  lcd.clear();
  lcd.print("Connecting...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.print(".");
  }
  lcd.clear();
  lcd.print("WiFi Connected");
}

long checkDistance() {
  digitalWrite(SENSOR_OUT, LOW);
  delayMicroseconds(2);
  digitalWrite(SENSOR_OUT, HIGH);
  delayMicroseconds(10);
  digitalWrite(SENSOR_OUT, LOW);

  long duration = pulseIn(SENSOR_IN, HIGH);
  return duration * 0.034 / 2;
}

bool isAuthorized(byte *uid, byte uidSize) {
  if (uidSize != sizeof(authorizedUID)) return false;
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != authorizedUID[i]) return false;
  }
  return true;
}

String getUID(byte *uid, byte uidSize) {
  String result = "";
  for (byte i = 0; i < uidSize; i++) {
    result += String(uid[i], HEX);
  }
  return result;
}

String passCheck() {
  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') break;
      if (key == '*') input = "";
      else if (input.length() < 4) input += key;
    }
  }
  return input;
}

String itemCode() {
  String input = "";
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#') break;
      if (key == '*') input = "";
      else if (input.length() < 3) input += key;
    }
  }
  return input;
}

void dispenseItem(Stepper &motor) {
  motor.step(2048);
  delay(1000);
}

void sendDataToThingSpeak(String rfid, String item) {
  String jsonData = "{\"rfid\":\"" + rfid + "\",\"item\":\"" + item + "\"}";
  ThingSpeak.setField(1, jsonData);
  ThingSpeak.writeFields(channelID, writeAPIKey.c_str());
}
 */