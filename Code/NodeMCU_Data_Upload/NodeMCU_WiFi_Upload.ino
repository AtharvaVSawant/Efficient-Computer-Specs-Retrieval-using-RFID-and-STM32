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
  Serial.print(F("   Flash Size: ")); Serial.print(ESP.getFlashChipRealSize() / 1024 / 1024); Serial.println(F(" MB"));
  Serial.print(F("   Free Heap: ")); Serial.print(ESP.getFreeHeap() / 1024); Serial.println(F(" KB"));
  Serial.print(F("   SDK Version: ")); Serial.println(ESP.getSdkVersion());
  Serial.print(F("   WiFi Status: ")); Serial.println(wifiConnected ? F("Connected") : F("Disconnected"));
  Serial.print(F("   Offline Records: ")); Serial.println(offlineCount);
  Serial.println();
}

// ==================== CONNECT TO WIFI ====================
void connectToWiFi() {
  Serial.print(F("\n📶 Connecting to WiFi"));
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {  // 20 seconds timeout
    delay(500);
    Serial.print(F("."));
    attempts++;
    
    // Blink LED while connecting
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    digitalWrite(LED_BUILTIN, LOW);  // LED on when connected
    Serial.println(F("✅ WiFi Connected!"));
    Serial.print(F("   IP Address: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("   Signal Strength: "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
    Serial.print(F("   MAC Address: "));
    Serial.println(WiFi.macAddress());
  } else {
    wifiConnected = false;
    digitalWrite(LED_BUILTIN, HIGH);  // LED off when disconnected
    Serial.println(F("❌ WiFi Connection Failed!"));
    Serial.println(F("   Operating in offline mode"));
  }
}

// ==================== CHECK WIFI CONNECTION ====================
void checkWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConnected) {
      wifiConnected = true;
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println(F("\n✅ WiFi Reconnected"));
      Serial.print(F("   IP: ")); Serial.println(WiFi.localIP());
    }
  } else {
    if (wifiConnected) {
      wifiConnected = false;
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println(F("\n❌ WiFi Disconnected"));
    }
    
    // Attempt to reconnect periodically
    if (millis() - lastReconnectAttempt > reconnectInterval) {
      Serial.print(F("📶 Attempting to reconnect..."));
      WiFi.reconnect();
      lastReconnectAttempt = millis();
      
      // Quick check
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(100);
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println(F("✅ Reconnected!"));
      } else {
        Serial.println(F(" Failed"));
      }
    }
  }
}

// ==================== PROCESS RECEIVED DATA ====================
void processReceivedData(String data) {
  Serial.println(F("\n📩 Received from STM32:"));
  Serial.println(data);
  
  totalScans++;
  
  // Parse the data (format: TAGID|TYPE|PROCESSOR|RAM|STORAGE|GRAPHICS|STATUS|LOCATION)
  ComputerData newData;
  
  int startPos = 0;
  int endPos = data.indexOf('|');
  
  if (endPos > 0) {
    newData.tagID = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.componentType = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.processor = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.ram = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.storage = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.graphics = data.substring(startPos, endPos);
    startPos = endPos + 1;
    endPos = data.indexOf('|', startPos);
    
    newData.status = data.substring(startPos, endPos);
    startPos = endPos + 1;
    
    newData.location = data.substring(startPos);
    
    newData.timestamp = millis();
    newData.uploaded = false;
    
    // Print parsed data
    Serial.println(F("📋 Parsed Data:"));
    Serial.print(F("   Tag ID: ")); Serial.println(newData.tagID);
    Serial.print(F("   Type: ")); Serial.println(newData.componentType);
    Serial.print(F("   CPU: ")); Serial.println(newData.processor);
    Serial.print(F("   RAM: ")); Serial.println(newData.ram);
    Serial.print(F("   Storage: ")); Serial.println(newData.storage);
    Serial.print(F("   Graphics: ")); Serial.println(newData.graphics);
    Serial.print(F("   Status: ")); Serial.println(newData.status);
    Serial.print(F("   Location: ")); Serial.println(newData.location);
    
    // Try to upload immediately if WiFi is connected
    if (wifiConnected) {
      if (uploadToServer(newData)) {
        newData.uploaded = true;
        successfulUploads++;
        stm32Serial.println("UPLOAD_SUCCESS");
      } else {
        failedUploads++;
        storeOffline(newData);
        stm32Serial.println("UPLOAD_FAILED_STORED");
      }
    } else {
      storeOffline(newData);
      stm32Serial.println("OFFLINE_STORED");
    }
    
    // Send acknowledgment back to STM32
    stm32Serial.println("ACK");
    
  } else {
    Serial.println(F("❌ Invalid data format"));
    stm32Serial.println("INVALID_FORMAT");
  }
  
  printStats();
}

// ==================== UPLOAD TO SERVER ====================
bool uploadToServer(ComputerData &data) {
  Serial.print(F("📤 Uploading to server..."));
  
  // Create JSON payload
  StaticJsonDocument<300> doc;
  doc["tag_id"] = data.tagID;
  doc["component_type"] = data.componentType;
  doc["processor"] = data.processor;
  doc["ram"] = data.ram;
  doc["storage"] = data.storage;
  doc["graphics"] = data.graphics;
  doc["status"] = data.status;
  doc["location"] = data.location;
  doc["timestamp"] = millis();
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Configure HTTP
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  // Send POST request
  int httpCode = http.POST(jsonString);
  
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      String payload = http.getString();
      Serial.println(F("✅ Success!"));
      Serial.print(F("   Server Response: "));
      Serial.println(payload);
      http.end();
      return true;
    } else {
      Serial.print(F("❌ HTTP Error: "));
      Serial.println(httpCode);
    }
  } else {
    Serial.print(F("❌ Connection Failed: "));
    Serial.println(http.errorToString(httpCode));
  }
  
  http.end();
  return false;
}

// ==================== STORE OFFLINE ====================
void storeOffline(ComputerData &data) {
  if (offlineCount < MAX_OFFLINE_RECORDS) {
    offlineBuffer[offlineCount] = data;
    offlineCount++;
    offlineStored++;
    
    Serial.println(F("💾 Data stored offline"));
    Serial.print(F("   Offline records: "));
    Serial.println(offlineCount);
    
    // Save to EEPROM
    saveOfflineData();
  } else {
    Serial.println(F("⚠️ Offline buffer full! Oldest record will be overwritten"));
    offlineBuffer[nextRecordIndex] = data;
    nextRecordIndex = (nextRecordIndex + 1) % MAX_OFFLINE_RECORDS;
    offlineCount = MAX_OFFLINE_RECORDS;
    
    // Save to EEPROM
    saveOfflineData();
  }
}

// ==================== UPLOAD OFFLINE DATA ====================
void uploadOfflineData() {
  if (offlineCount == 0) return;
  
  Serial.println(F("\n📤 Attempting to upload offline data..."));
  
  int uploaded = 0;
  int i = 0;
  
  while (i < offlineCount && i < 10) {  // Upload max 10 per cycle
    Serial.print(F("   Uploading record "));
    Serial.print(i + 1);
    Serial.print(F("/"));
    Serial.print(offlineCount);
    Serial.print(F("..."));
    
    if (uploadToServer(offlineBuffer[i])) {
      // Remove from buffer by shifting
      for (int j = i; j < offlineCount - 1; j++) {
        offlineBuffer[j] = offlineBuffer[j + 1];
      }
      offlineCount--;
      uploaded++;
      successfulUploads++;
      Serial.println(F("✅"));
    } else {
      Serial.println(F("❌ Failed, will retry later"));
      i++;
    }
    
    delay(500);  // Small delay between uploads
  }
  
  if (uploaded > 0) {
    Serial.print(F("✅ Uploaded "));
    Serial.print(uploaded);
    Serial.println(F(" offline records"));
    
    // Save updated buffer to EEPROM
    saveOfflineData();
  }
}

// ==================== SAVE OFFLINE DATA TO EEPROM ====================
void saveOfflineData() {
  EEPROM.write(0, offlineCount);
  EEPROM.write(1, nextRecordIndex);
  
  int addr = 10;  // Start address for data
  
  for (int i = 0; i < offlineCount; i++) {
    // Save each field with delimiter
    String record = offlineBuffer[i].tagID + "|" +
                    offlineBuffer[i].componentType + "|" +
                    offlineBuffer[i].processor + "|" +
                    offlineBuffer[i].ram + "|" +
                    offlineBuffer[i].storage + "|" +
                    offlineBuffer[i].graphics + "|" +
                    offlineBuffer[i].status + "|" +
                    offlineBuffer[i].location;
    
    for (unsigned int j = 0; j < record.length() && addr < EEPROM_SIZE; j++) {
      EEPROM.write(addr++, record[j]);
    }
    EEPROM.write(addr++, '\0');  // Null terminator
  }
  
  EEPROM.commit();
  Serial.println(F("💾 Offline data saved to EEPROM"));
}

// ==================== LOAD OFFLINE DATA FROM EEPROM ====================
void loadOfflineData() {
  offlineCount = EEPROM.read(0);
  nextRecordIndex = EEPROM.read(1);
  
  if (offlineCount > MAX_OFFLINE_RECORDS) {
    offlineCount = 0;
    return;
  }
  
  int addr = 10;
  
  for (int i = 0; i < offlineCount; i++) {
    String record = "";
    char c = EEPROM.read(addr++);
    
    while (c != '\0' && addr < EEPROM_SIZE) {
      record += c;
      c = EEPROM.read(addr++);
    }
    
    // Parse record
    int startPos = 0;
    int endPos = record.indexOf('|');
    
    if (endPos > 0) {
      offlineBuffer[i].tagID = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].componentType = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].processor = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].ram = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].storage = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].graphics = record.substring(startPos, endPos);
      startPos = endPos + 1;
      endPos = record.indexOf('|', startPos);
      
      offlineBuffer[i].status = record.substring(startPos, endPos);
      startPos = endPos + 1;
      
      offlineBuffer[i].location = record.substring(startPos);
      
      offlineBuffer[i].uploaded = false;
      offlineBuffer[i].timestamp = 0;
    }
  }
  
  if (offlineCount > 0) {
    Serial.print(F("📂 Loaded "));
    Serial.print(offlineCount);
    Serial.println(F(" offline records from EEPROM"));
  }
}

// ==================== HANDLE BUTTON PRESS ====================
void handleButtonPress() {
  Serial.println(F("\n🔘 Button Pressed"));
  
  if (wifiConnected) {
    Serial.println(F("📤 Manual upload triggered"));
    uploadOfflineData();
  } else {
    Serial.println(F("📶 Attempting to connect WiFi..."));
    connectToWiFi();
  }
}

// ==================== PRINT STATISTICS ====================
void printStats() {
  Serial.println(F("\n📊 Statistics:"));
  Serial.print(F("   Total Scans: ")); Serial.println(totalScans);
  Serial.print(F("   Successful Uploads: ")); Serial.println(successfulUploads);
  Serial.print(F("   Failed Uploads: ")); Serial.println(failedUploads);
  Serial.print(F("   Offline Stored: ")); Serial.println(offlineStored);
  Serial.print(F("   Current Offline: ")); Serial.println(offlineCount);
}

// ==================== HANDLE SERIAL COMMANDS ====================
void handleSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 's':
      case 'S':
        printStats();
        break;
        
      case 'w':
      case 'W':
        Serial.print(F("WiFi Status: "));
        if (wifiConnected) {
          Serial.print(F("Connected to "));
          Serial.print(ssid);
          Serial.print(F(" ("));
          Serial.print(WiFi.RSSI());
          Serial.println(F(" dBm)"));
        } else {
          Serial.println(F("Disconnected"));
        }
        break;
        
      case 'c':
      case 'C':
        Serial.println(F("Attempting to connect WiFi..."));
        connectToWiFi();
        break;
        
      case 'u':
      case 'U':
        Serial.println(F("Manual upload triggered"));
        uploadOfflineData();
        break;
        
      case 'o':
      case 'O':
        Serial.print(F("Offline records: "));
        Serial.println(offlineCount);
        if (offlineCount > 0) {
          for (int i = 0; i < offlineCount; i++) {
            Serial.print(F("   "));
            Serial.print(i + 1);
            Serial.print(F(": "));
            Serial.print(offlineBuffer[i].tagID);
            Serial.print(F(" - "));
            Serial.println(offlineBuffer[i].componentType);
          }
        }
        break;
        
      case 'e':
      case 'E':
        Serial.println(F("Clearing EEPROM..."));
        for (int i = 0; i < EEPROM_SIZE; i++) {
          EEPROM.write(i, 0);
        }
        EEPROM.commit();
        offlineCount = 0;
        Serial.println(F("EEPROM cleared"));
        break;
        
      case 'i':
      case 'I':
        printSystemInfo();
        break;
        
      case 'h':
      case 'H':
        printHelp();
        break;
    }
    
    // Clear remaining input
    while (Serial.available()) {
      Serial.read();
    }
  }
}

// ==================== PRINT HELP ====================
void printHelp() {
  Serial.println(F("\n📋 Available Commands:"));
  Serial.println(F("   s - Show statistics"));
  Serial.println(F("   w - Show WiFi status"));
  Serial.println(F("   c - Connect to WiFi"));
  Serial.println(F("   u - Upload offline data"));
  Serial.println(F("   o - Show offline records"));
  Serial.println(F("   e - Clear EEPROM"));
  Serial.println(F("   i - Show system info"));
  Serial.println(F("   h - Show this help"));
}
