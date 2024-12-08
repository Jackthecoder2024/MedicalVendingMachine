#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>

// Setting LCD screen 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED lights setting
const int redPin = 7;
const int yellowPin = 8;
const int greenPin = 13;

// PIR motion sensor 
const int motionPin = 6;

// Ultrasonic sensor 
const int SENSOR_IN  = 3;
const int SENSOR_OUT = 2;
const int user_distance = 50; 

// Special emoji 
byte Skull[] = {
  B00000,
  B01110, 
  B10101,
  B11011,
  B01110,
  B01110,
  B00000, 
  B00000,
};

// Setting keypad 
const byte ROWS = 4; // Rows 
const byte COLS = 4; // Columns 

// Item variable 
String item = "";
String passWord = "";

// Keypad characters for each button clicked 
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Keypad pin connections 
byte pinRow[ROWS] = {45, 43, 41, 39}; // Rows
byte pinCol[COLS] = {37, 35, 33, 31}; // Columns 

// Configuring keypad map
Keypad keypad = Keypad(makeKeymap(keys), pinRow, pinCol, ROWS, COLS);

// Setting stepper motors 
Stepper motor1(2048, 12, 10, 11, 9);
Stepper motor2(2048, 38, 42, 40, 44);
Stepper motor3(2048, 22, 26, 24, 28);
Stepper motor4(2048, 30, 34, 32, 36);

// Saving motor pins in an array 
int pinMotor[] = {12, 11, 10, 38, 40, 42, 44, 5, 22, 24, 26, 28, 30, 32, 34, 36};

// Creating a function that will turn off the pins 
void resetPins() {
  for (int i = 0; i < sizeof(pinMotor) / sizeof(pinMotor[0]); i++) {
    digitalWrite(pinMotor[i], LOW);
  }
}

// Stepper speed 
int motorSpeed = 10;

// Define pins for the RC522 module
#define RST_PIN 5  // Reset pin
#define SS_PIN 53  // Slave select pin

MFRC522 rfid(SS_PIN, RST_PIN); // Create an instance of the MFRC522 class

// Predefined authorized UID (replace with your card's UID)
byte authorizedUID[] = {0x43, 0x18, 0xFA, 0x29};


bool isAuthorized(byte *uid, byte uidSize) {
  if (uidSize != sizeof(authorizedUID)) return false; // Size mismatch
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != authorizedUID[i]) return false; // Byte mismatch
  }
  return true;
}

long checkDistance() {
  digitalWrite(SENSOR_OUT, LOW);
  delayMicroseconds(2);
  digitalWrite(SENSOR_OUT, HIGH);
  delayMicroseconds(10);
  digitalWrite(SENSOR_OUT, LOW);

  long duration = pulseIn(SENSOR_IN, HIGH);
  long distance = duration * 0.034 / 2; // Convert to centimeters
  return distance;
}


void setup() {
  lcd.init(); // Initializing LCD screen 
  lcd.backlight(); 
  lcd.setCursor(1, 0);
  lcd.print("-- Welcome! --"); // Printing a welcome message to the user 
  delay(2000);
  lcd.clear(); // Clearing screen after 2 seconds 

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  pinMode(motionPin, INPUT);

  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor3.setSpeed(motorSpeed);
  motor4.setSpeed(motorSpeed);

  SPI.begin();        // Initialize SPI bus
  rfid.PCD_Init();    // Initialize RFID module

  pinMode(SENSOR_IN, INPUT);
  pinMode(SENSOR_OUT, OUTPUT);
}

void loop() {

  long distance = checkDistance();
  if (distance > user_distance) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waiting for");
    lcd.setCursor(0, 1);
    lcd.print("user...");
    delay(500);
    return; // Wait until user is close
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan card: ");
  
  // Check for a card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; // No card detected
  }

  // Check if the detected UID matches the authorized UID
  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
    lcd.setCursor(0, 0);
    lcd.print("Enter pass:");
    passWord = passCheck();
    lcd.clear();

    if (passWord == "1523") {
      lcd.setCursor(0, 0);
      lcd.print("Access Granted!");
      delay(2000);

      lcd.setCursor(0, 0);
      lcd.print("Enter Item Code:");
      item = itemCode();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected item: ");
      lcd.setCursor(0, 1);
      lcd.print(item);
      delay(2000);
      lcd.clear();
    

      if (item == "12A") {
        dispenseItem(motor1); 

      }
      else if (item == "13B") {
        dispenseItem(motor2); 

      }
      else if (item == "12B") {
        dispenseItem(motor3); 

      }
      else if (item == "13A") {
       dispenseItem(motor4); 

      }
      else {
        lcd.setCursor(0, 0);
        lcd.print("Item not ");
        lcd.setCursor(0, 1);
        lcd.print("available");
        lcd.createChar(0, Skull);
        lcd.setCursor(10, 1);
        lcd.write(byte(0));
        delay(2000);
        lcd.clear();
      }
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Access Denied!");
      delay(2000);
    }
  }
    else {
    // Unauthorized card detected
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card not");
    lcd.setCursor(0, 1);
    lcd.print("recognized!");
    delay(2000);
    lcd.clear();
  }
    // Halt PICC to stop further detection
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    }
    


String itemCode() {
    String input = "";
    lcd.setCursor(0, 1);

    while (true) {
      char key = keypad.getKey();
      if (key) {
        if (key == '#') {
          // Submit button pressed
          break; 
        } else if (key == '*') {
          // Clear input if * is pressed
          input = "";
          lcd.setCursor(0, 1);
          lcd.print("                "); // Clear LCD row
          lcd.setCursor(0, 1);
        } else if (input.length() < 3) {
          // Add key to input if not full
          input += key;
          lcd.print(key);
        } 
      }
    }

    return input;
}

String passCheck() {
    String input = "";
    lcd.setCursor(0, 1);

    while (true) {
      char pass = keypad.getKey();
      if (pass) {
        if (pass == '#') {
          // Submit button pressed
          break; 
        } else if (pass == '*') {
          // Clear input if * is pressed
          input = "";
          lcd.setCursor(0, 1);
          lcd.print("                "); // Clear LCD row
          lcd.setCursor(0, 1);
        } else if (input.length() < 4) {
          // Add key to input if not full
          input += pass;
          lcd.print(pass);
        } 
      }
    }

    return input;
}

void dispenseItem(Stepper &motor) {
  lcd.print("dispensing...");
  motor.step(2048);
  lcd.clear();
  ledDispense();
  lcd.setCursor(0, 0);
  lcd.print("Done!");
  delay(2000);
  lcd.clear();

  resetPins();
}

void ledDispense() {

  lcd.setCursor(0, 0);
  lcd.print("Please collect");
  lcd.setCursor(0, 1);
  lcd.print("item :)");

  // Red
  digitalWrite(redPin, HIGH);
  delay(500);
  digitalWrite(redPin, LOW);

  // Yellow
  digitalWrite(yellowPin, HIGH);
  delay(500);
  digitalWrite(yellowPin, LOW);

  // Green 
  digitalWrite(greenPin, HIGH);
  delay(500);
  digitalWrite(greenPin, LOW);

  lcd.clear();
}