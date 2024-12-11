// Calling required libraries 
#include <WiFi.h>
#include <ThingSpeak.h>

// Wifi details 
const char *ssid = "MDX welcomes you"; // wifi ssid  
const char *password = "MdxL0vesyou"; // wifi password 

// ThingSpeak channel details 
unsigned long chanNum = 2780135;
const char *keyAPI = "PV7UBKMN9L1K88EP";

// Pin Definitions
#define motionPin 15 

int itemCount = 100; // Inventory count

// Initiating client 
WiFiClient client;

// Time varibles 
unsigned long lastMotionTime = 0; // Tracks the last motion detected
const unsigned long motionCooldown = 15000; // Cooldown time between motions (15 seconds)

void setup() {
  // Starting serial connection 
  Serial.begin(9600);

  // Connecting to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  ThingSpeak.begin(client); // Initialize ThingSpeak
  pinMode(motionPin, INPUT); 
  Serial.println("System Ready!");
}

void loop() {
  // Read current time
  unsigned long currentTime = millis();

  // Check for motion and cooldown
  if (digitalRead(motionPin) == HIGH && (currentTime - lastMotionTime > motionCooldown)) {
    itemCount--; // Decrease item count
    Serial.print("Remaining quantity: ");
    Serial.println(itemCount);

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
      ThingSpeak.setField(1, itemCount);
      if (ThingSpeak.writeFields(chanNum, keyAPI) == 200) {
        Serial.println("Data sent to ThingSpeak.");
      } else {
        itemCount++;
        //Serial.println("Failed to send data.");
      }
    } else {
      Serial.println("WiFi disconnected!");
    }

    // Update the last motion time
    lastMotionTime = currentTime;
  }

  delay(100); // Small delay for stability
}
