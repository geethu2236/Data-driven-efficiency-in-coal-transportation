#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Insert Firebase project API Key
#define API_KEY ""

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "" 
//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
// Define pin numbers
const int thermistorPin = A0;  // Analog pin for thermistor
const int relayPin = 2;       // Digital pin for relay control
const int waterSprinklePin = 3;  // Digital pin for water sprinkle
String waterSprikledValue;

// Define threshold temperature
const int thresholdTemperature = 2;  // Set your threshold temperature here

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set pin modes
  pinMode(thermistorPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(waterSprinklePin, OUTPUT);
}

void loop() {
  // Read the analog value from the thermistor
  int thermistorValue = analogRead(thermistorPin);

  // Convert the analog value to temperature (adjust this conversion based on your thermistor specifications)
  float temperature = map(thermistorValue, 0, 1023, 0, 100);

  // Print temperature to serial monitor (optional)
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Check if the temperature is above the threshold
  if (temperature > thresholdTemperature) {
    // Turn on the relay and water sprinkle
    digitalWrite(relayPin, LOW);
    waterSprikledValue = HIGH;
    digitalWrite(waterSprinklePin, HIGH);
     
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    
     if (Firebase.RTDB.setString(&fbdo, "water sprinkled", waterSprikledValue)) {
        Serial.println("PASSED");
    } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
    }
    // Set the string value in the "water sprinkled" path
    
}
  } else {
    // Turn off the relay and water sprinkle
    waterSprikledValue= LOW;
    digitalWrite(relayPin, HIGH);
    digitalWrite(waterSprinklePin, LOW);
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    
     if (Firebase.RTDB.setString(&fbdo, "water sprinkled", waterSprikledValue)) {
        Serial.println("PASSED");
    } else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
    }
    // Set the string value in the "water sprinkled" path
    
}
  }
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
     if (Firebase.RTDB.setFloat(&fbdo, "temperature", temperature)){
      Serial.println("PASSED");
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
   
  }


  // Add a delay to avoid rapid switching (adjust as needed)
  delay(1000);
}
