// Calling on all required librairies 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>

// LCD screen settings 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED lights settings
const int redPin = 7;
const int yellowPin = 8;
const int greenPin = 13;

// PIR motion sensor settings
const int motionPin = 6;

// Buzzer settings 
const int buzPin = 23;

// Ultrasonic sensor settings 
const int SENSOR_IN  = 3;
const int SENSOR_OUT = 2;
const int user_distance = 80; 

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

// Stepper motor settings 
Stepper motor1(2048, 12, 10, 11, 9);
Stepper motor2(2048, 38, 42, 40, 44);
Stepper motor3(2048, 22, 26, 24, 28);
Stepper motor4(2048, 30, 34, 32, 36);

// Saving motor pins in an array 
int pinMotor[] = {12, 11, 10, 38, 40, 42, 44, 5, 22, 24, 26, 28, 30, 32, 34, 36};

// Stepper moptor speed 
int motorSpeed = 10;

// Define pins for the RC522 module
#define RST_PIN 5  // Reset pin
#define SS_PIN 53  // Slave pin 

MFRC522 rfid(SS_PIN, RST_PIN); // Creating an instance of the MFRC522 class

// UID of the authorized card to be used 
byte authorizedUID[] = {0x43, 0x18, 0xFA, 0x29};

// Creating a function that will check for the authorized card 
bool isAuthorized(byte *uid, byte uidSize) {
  if (uidSize != sizeof(authorizedUID)) return false; // Size mismatch
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] != authorizedUID[i]) return false; // Byte mismatch
  }
  return true;
}

// Creating a function that will check for the user distance 
long checkDistance() {
  digitalWrite(SENSOR_OUT, LOW);
  delayMicroseconds(2);
  digitalWrite(SENSOR_OUT, HIGH);
  delayMicroseconds(10);
  digitalWrite(SENSOR_OUT, LOW);

  long duration = pulseIn(SENSOR_IN, HIGH);
  long distance = duration * 0.034 / 2; // Converting to centimeters
  return distance;
}


void setup() {
  lcd.init(); // Initializing LCD screen 
  lcd.backlight(); // Turning LCD backlight on
  lcd.setCursor(1, 0);
  lcd.print("-- Welcome! --"); // Printing a welcome message to the user 
  delay(2000);
  lcd.clear(); // Clearing screen after 2 seconds 

  // Setting LED light pins 
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  // Setting PIR motion sensor pin
  pinMode(motionPin, INPUT);

  // Setting buzzer pin
  pinMode(buzPin, OUTPUT);

  // Setting stepper motors speed 
  motor1.setSpeed(motorSpeed);
  motor2.setSpeed(motorSpeed);
  motor3.setSpeed(motorSpeed);
  motor4.setSpeed(motorSpeed);

  // RFID settings 
  SPI.begin();        // Initializing SPI bus
  rfid.PCD_Init();    // Initializing RFID module

  // Setting ultrasonic sensor pins 
  pinMode(SENSOR_IN, INPUT);
  pinMode(SENSOR_OUT, OUTPUT);

}

void loop() {
  // Checking if the user is close enough for the machine
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
  lcd.print("Scan card: "); // If the user is close enough, they will be asked to scan their card 
  
    
  // Checking if any card has been detected 
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return; 
  }

  // Checking if the detected UID matches the authorized UID
  if (isAuthorized(rfid.uid.uidByte, rfid.uid.size)) {
    // A message informing the user how to select and clear the input field is displayed for 5 seconds 
    lcd.setCursor(0, 0);
    lcd.print("# to submit"); 
    lcd.setCursor(0, 1);
    lcd.print("* to clear");
    delay(3500);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Enter pin:"); // If the authorized card has been detected then the user is asked to enter their password
    passWord = passCheck(); // Getting the password entered by the user 
    lcd.clear();

    // Checking if the password entered is correct 
    if (passWord == "1523") {
      lcd.setCursor(0, 0);
      lcd.print("Access Granted!"); // If entered password is correct then the user is guarenteed access 
      delay(2000);
      lcd.clear();
        
      lcd.setCursor(0, 0);
      lcd.print("Enter item code:"); // The user is asked to select an item 
      item = itemCode(); // Getting the user input 

      // Confirming the user choice 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Selected item: "); 
      lcd.setCursor(0, 1);
      lcd.print(item);
      delay(2000);
      lcd.clear();
      
      // Checking for which item the user has selected that will then be dispensed 
      if (item == "12B") dispenseItem(motor1); 
      else if (item == "13B") dispenseItem(motor2); 
      else if (item == "12A") dispenseItem(motor3); 
      else if (item == "13A") dispenseItem(motor4); 
      // A message will be displayed to the user if the item selected is not available 
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
    }
    
    // If the enterd password is incorrect then the access is denied to the user 
    else {
      lcd.setCursor(0, 0);
      lcd.print("Access Denied!");
      delay(2000);
    }
  }
  // If an unauthorized UID is detected, then a message will be displayed to the user letting them know 
  else {
    digitalWrite(buzPin, HIGH); // Turn on the buzzer
    delay(500);                 // Wait for 200 ms
    digitalWrite(buzPin, LOW);  // Turn off the buzzer    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card not");
    lcd.setCursor(0, 1);
    lcd.print("recognized!");
    delay(2000);
    lcd.clear();
  }  
    // Stopping RFID from further detection 
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
} 


// --- Functions ---

// Function that will save user input when selecting their item 
String itemCode() {
    String input = "";
    lcd.setCursor(0, 1);

    while (true) {
      char key = keypad.getKey();
      if (key) {
        buzSound(); // Emiting sound when a button is pressed 
        // If # button is selected then the input will be submitted 
        if (key == '#') {
          break; 
        } 
        // If * is selected then the input area will be cleared
        else if (key == '*') {
          input = "";
          lcd.setCursor(0, 1);
          lcd.print("                "); // Clearing row 
          lcd.setCursor(0, 1);
        } 
        // Getting a 3 character input 
        else if (input.length() < 3) {
          // Add key to input if not full
          input += key;
          lcd.print(key);
        } 
      }
    }
    return input;
}

// Function to get password input of the user 
String passCheck() {
    String input = "";
    lcd.setCursor(0, 1);

    while (true) {
      char key = keypad.getKey();
      if (key) {
        buzSound(); // Emiting sound when a button is pressed 
        // If # button is selected then the input will be submitted 
        if (key == '#') {
          break; 
        } 
        // If * is selected then the input area will be cleared
        else if (key == '*') {
          input = "";
          lcd.setCursor(0, 1);
          lcd.print("                "); // Clearing row 
          lcd.setCursor(0, 1);
        } 
        // Getting a 4 character input 
        else if (input.length() < 4) {
          // Add key to input if not full
          input += key;
          lcd.print(key);
        } 
      }
    }
  return input;
}

// Creating a function that will turn off the motors after dispencing the item 
void resetPins() {
  for (int i = 0; i < sizeof(pinMotor) / sizeof(pinMotor[0]); i++) {
    digitalWrite(pinMotor[i], LOW);
  }
}

// Creating function that will dispense the item selected by the user 
void dispenseItem(Stepper &motor) {
  lcd.print("dispensing...");
  motor.step(2048); // Dispensing the item
  lcd.clear();

  // Displaying a light sequence after the item is dispensed
  ledDispense();

  resetPins(); // Setting motor pins to low 
}

// Function to play light sequence 
void ledDispense() {

  lcd.setCursor(0, 0);
  lcd.print("Please collect");
  lcd.setCursor(0, 1);
  lcd.print("item :)");

  delay(3000);

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

// Function to emit sound when button on keypad is clicked
void buzSound() {
  digitalWrite(buzPin, HIGH); // Turn on the buzzer
  delay(200);                 // Wait for 200 ms
  digitalWrite(buzPin, LOW);  // Turn off the buzzer
  delay(100);                 // Short delay before next action
}
