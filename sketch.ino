#include <Arduino.h>
#include <TinyGPS++.h> // NMEA GPS Parsing Library
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

#define DATABASE_URL "https://upetbackendapi.onrender.com/api/v1/add_smart_collar"

#define HRS_PIN 33
#define BT_PIN 32

#define GPS_BAUDRATE 9600  // Default baudrate of NEO-6M

#define RXD2 16 // Serial2 RX Pin 
#define TXD2 17 // Serial2 TX Pin

TinyGPSPlus gps;  // the TinyGPS++ object instance

// Create HTTP Client
HTTPClient client;

// Operation Variables
String sensorID = "IOTDEV-01";

void setup() {
  Serial.begin(115200);
  Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RXD2, TXD2);

  pinMode(HRS_PIN, INPUT);
  pinMode(BT_PIN, INPUT);
    
  // WiFi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");

  // Perform connection test
  client.begin(DATABASE_URL);
  int httpResponseCode = client.GET();
  if (httpResponseCode > 0) {
    Serial.println("Endpoint reachable");
  } else {
    Serial.print("Endpoint error: ");
    Serial.println(httpResponseCode);
  }

  // Timer Client Configuration
  configTime(0, 0, "pool.ntp.org");
}

void loop() {
  // Default static values
  float latitude = -12.105186;
  float longitude = -76.970540;
  int battery = 100;
  int pet_id = 7;

  // Try to read GPS data
  if (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        Serial.println("Valid GPS data received");
      } else {
        Serial.println("Invalid GPS data, using static values");
      }
    }
  }

  // Read sensor data
  int lpm = map(analogRead(HRS_PIN), 0, 4095, 0, 200);
  int temperature = map(analogRead(BT_PIN), 0, 4095, 20, 45);

  // Create JSON object
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["id"] = "1"; // Static ID as per requirement
  jsonDoc["serial_number"] = sensorID;
  jsonDoc["temperature"] = temperature;
  jsonDoc["lpm"] = lpm;
  jsonDoc["battery"] = battery;
  
  JsonObject location = jsonDoc.createNestedObject("location");
  location["latitude"] = latitude;
  location["longitude"] = longitude;
  jsonDoc["pet_id"] = pet_id;

  String jsonData;
  serializeJson(jsonDoc, jsonData);

  // Send information to Database
  client.begin(DATABASE_URL);
  client.addHeader("Content-Type", "application/json");
  int httpResponseCode = client.POST(jsonData);

  if (httpResponseCode > 0) {
    String response = client.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response from server: ");
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    Serial.print("Error details: ");
    Serial.println(client.errorToString(httpResponseCode).c_str());
  }

  client.end(); // Free resources

  delay(300000); // Send data every 5 minutes
}
