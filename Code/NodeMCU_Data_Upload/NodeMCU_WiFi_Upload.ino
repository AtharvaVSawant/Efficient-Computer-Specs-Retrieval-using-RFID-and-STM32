/**
 * EFFICIENT COMPUTER SPECIFICATIONS RETRIEVAL SYSTEM
 * NodeMCU WiFi Upload Module
 * 
 * File: NodeMCU_WiFi_Upload.ino
 * Description: Receives data from STM32 and uploads to cloud server
 *              Also handles offline storage when WiFi is unavailable
 * 
 * Board: NodeMCU 1.0 (ESP-12E Module)
 * Communication: UART with STM32
 * 
 * @authors: Ankit Nirmal, Umang Panchal, Pradyumn Sahu, Atharva Sawant
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

// ==================== PIN DEFINITIONS ====================
#define STM32_RX D5        // Connect to STM32 TX (PA2)
#define STM32_TX D6        // Connect to STM32 RX (PA3)
#define LED_BUILTIN 2      // Built-in LED (active low)
#define BUTTON_PIN D3      // Optional button for manual upload

// ==================== WIFI CREDENTIALS ====================
// IMPORTANT: Create a wifi_config.h file with your actual credentials
// DO NOT commit wifi_config.h to GitHub!
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// Replace with your actual WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server URL for data upload
const char* serverUrl = "http://your-server.com/api/upload_specs.php";
const char* backupServerUrl = "http://backup-server.com/api/upload.php";

#endif

// ==================== SOFTWARE SERIAL ====================
#include <SoftwareSerial.h>
SoftwareSerial stm32Serial(STM32_RX, STM32_TX);  // RX, TX

// ==================== STRUCTURES ====================
struct ComputerData {
  String tagID;
  String componentType;
  String processor;
  String ram;
  String storage;
  String graphics;
  String status;
  String location;
  unsigned long timestamp;
  bool uploaded;
};

// ==================== GLOBAL VARIABLES ====================
WiFiClient client;
HTTPClient http;

// EEPROM Configuration
const int EEPROM_SIZE = 4096;  // 4KB EEPROM
const int MAX_OFFLINE_RECORDS = 50;

// Offline storage
ComputerData offlineBuffer[MAX_OFFLINE_RECORDS];
int offlineCount = 0;
int nextRecordIndex = 0;

// System state
bool wifiConnected = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 30000;  // 30 seconds
unsigned long lastUploadAttempt = 0;
const unsigned long uploadInterval = 60000;     // 60 seconds

// Statistics
int totalScans = 0;
int successfulUploads = 0;
int failedUploads = 0;
int offlineStored = 0;

// ==================== SETUP FUNCTION ====================
void setup() {
  // Initialize pins
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off
  
  // Initialize serial for debugging
  Serial.begin(9600);
  delay(1000);
  
  printHeader();
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.println(F("✓ EEPROM Initialized"));
  
  // Initialize communication with STM32
  stm32Serial.begin(9600);
  Serial.println(F("✓ STM32 Communication Ready"));
  
  // Load offline data from EEPROM
  loadOfflineData();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Print system info
  printSystemInfo();
  
  Serial.println(F("\n✅ System Ready"));
  Serial.println(F("Waiting for data from STM32..."));
  Serial.println();
  
  // Blink LED to indicate ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
}

// ==================== MAIN LOOP ====================
void loop() {
  // Check for data from STM32
  if (stm32Serial.available()) {
    String receivedData = stm32Serial.readStringUntil('\n');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
      processReceivedData(receivedData);
    }
  }
  
  // Check WiFi connection periodically
  checkWiFiConnection();
  
  // Attempt to upload offline data periodically
  if (wifiConnected && offlineCount > 0 && millis() - lastUploadAttempt > uploadInterval) {
    uploadOfflineData();
    lastUploadAttempt = millis();
  }
  
  // Check for button press (manual upload)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
  
  // Handle serial commands
  handleSerialCommands();
  
  delay(10);
}

// ==================== PRINT HEADER ====================
void printHeader() {
  Serial.println(F("\n\n====================================="));
  Serial.println(F("NODEMCU WIFI UPLOAD MODULE"));
  Serial.println(F("Efficient Computer Specs Retrieval System"));
  Serial.println(F("====================================="));
  Serial.println(F("Team: Ankit | Umang | Pradyumn | Atharva"));
  Serial.println(F("SAKEC | University of Mumbai | AY 2023-24"));
  Serial.println(F("====================================="));
}

// ==================== PRINT SYSTEM INFO ====================
void printSystemInfo() {
  Serial.println(F("\n📊 System Information:"));
  Serial.print(F("   Chip ID: ")); Serial.println(ESP.getChipId(), HEX);
  Serial.print(F("   Flash Size:
