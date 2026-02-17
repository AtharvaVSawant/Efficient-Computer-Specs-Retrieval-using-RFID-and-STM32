/**
 * EFFICIENT COMPUTER SPECIFICATIONS RETRIEVAL SYSTEM
 * Using RFID and STM32
 * 
 * File: STM32_RFID_Reader.ino
 * Description: Main code for STM32 to read RFID tags and display computer specifications
 * 
 * Microcontroller: STM32F103C8T6
 * RFID Module: EM-18 (125kHz)
 * Display: 16x2 LCD with I2C
 * Communication: UART with NodeMCU
 * 
 * @authors: Ankit Nirmal, Umang Panchal, Pradyumn Sahu, Atharva Sawant
 * @guided_by: Prof. Jagdish Sarode
 * @college: Shah and Anchor Kutchhi Engineering College
 * @date: AY 2023-24
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "config.h"

// ==================== PIN DEFINITIONS ====================
#define RFID_RX PA10     // STM32 RX (connect to EM-18 TX)
#define RFID_TX PA9      // STM32 TX (connect to EM-18 RX)
#define NODE_RX PA3      // STM32 RX for NodeMCU communication
#define NODE_TX PA2      // STM32 TX for NodeMCU communication
#define LED_PIN PC13     // Built-in LED (active low)
#define BUZZER_PIN PB0   // Optional buzzer for feedback

// ==================== OBJECT INITIALIZATION ====================
// LCD Display (I2C address from config, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Software Serial for RFID and NodeMCU
SoftwareSerial rfidSerial(RFID_RX, RFID_TX);  // RX, TX
SoftwareSerial nodeSerial(NODE_RX, NODE_TX);  // RX, TX

// ==================== RFID TAG DATABASE ====================
struct ComputerSpec {
  char tagID[13];           // 12-digit RFID tag ID + null terminator
  char componentType[30];   // Type of component (Desktop, Laptop, Monitor, etc.)
  char processor[30];       // Processor model
  char ram[15];             // RAM size and type
  char storage[25];         // Storage type and capacity
  char graphics[30];        // Graphics card info
  char status[15];          // Status (Available, In Use, Under Repair)
  char location[20];        // Location/Room number
  char purchaseDate[15];    // Purchase date
  char notes[50];           // Additional notes
};

// Database entries - REPLACE WITH YOUR ACTUAL RFID TAG IDs AND COMPONENT SPECS
ComputerSpec specDatabase[] = {
  // Computer 1 - Lab Desktop
  {
    "123456789012",         // RFID Tag ID
    "Lab Desktop",          // Component Type
    "Intel Core i7-11700",  // Processor
    "16GB DDR4",            // RAM
    "512GB NVMe SSD",       // Storage
    "NVIDIA GTX 1660 6GB",  // Graphics
    "Available",            // Status
    "Lab 301",              // Location
    "15-06-2023",           // Purchase Date
    "Main lab computer"     // Notes
  },
  
  // Computer 2 - Student Laptop
  {
    "234567890123",         // RFID Tag ID
    "Student Laptop",       // Component Type
    "AMD Ryzen 5 5600H",    // Processor
    "8GB DDR4",             // RAM
    "1TB HDD + 256GB SSD",  // Storage
    "Integrated Radeon",    // Graphics
    "In Use",               // Status
    "Library",              // Location
    "10-01-2024",           // Purchase Date
    "Issued to student"     // Notes
  },
  
  // Computer 3 - Server
  {
    "345678901234",         // RFID Tag ID
    "Server",               // Component Type
    "Intel Xeon E-2234",    // Processor
    "32GB ECC RAM",         // RAM
    "2TB RAID 10",          // Storage
    "Integrated ASPEED",    // Graphics
    "Available",            // Status
    "Server Room",          // Location
    "05-03-2023",           // Purchase Date
    "Main server"           // Notes
  },
  
  // Computer 4 - Workstation
  {
    "456789012345",         // RFID Tag ID
    "Workstation",          // Component Type
    "AMD Ryzen 9 5950X",    // Processor
    "64GB DDR4",            // RAM
    "2TB NVMe SSD",         // Storage
    "NVIDIA RTX 3080 10GB", // Graphics
    "In Use",               // Status
    "Design Lab",           // Location
    "20-11-2023",           // Purchase Date
    "High-end workstation"  // Notes
  },
  
  // Computer 5 - Faculty Desktop
  {
    "567890123456",         // RFID Tag ID
    "Faculty Desktop",      // Component Type
    "Intel i5-12400",       // Processor
    "16GB DDR4",            // RAM
    "512GB SSD",            // Storage
    "Intel UHD 730",        // Graphics
    "Available",            // Status
    "Room 204",             // Location
    "02-09-2023",           // Purchase Date
    "Faculty office"        // Notes
  },
  
  // Computer 6 - Monitor only
  {
    "678901234567",         // RFID Tag ID
    "Monitor",              // Component Type
    "N/A",                  // Processor
    "N/A",                  // RAM
    "N/A",                  // Storage
    "24-inch 1080p 75Hz",   // Graphics (Display specs)
    "Available",            // Status
    "Lab 302",              // Location
    "12-12-2023",           // Purchase Date
    "Dell monitor"          // Notes
  },
  
  // Computer 7 - Printer (Non-computer device)
  {
    "789012345678",         // RFID Tag ID
    "Network Printer",      // Component Type
    "N/A",                  // Processor
    "N/A",                  // RAM
    "N/A",                  // Storage
    "HP LaserJet Pro",      // Graphics (Model)
    "Low Toner",            // Status
    "Office",               // Location
    "30-07-2023",           // Purchase Date
    "Needs toner soon"      // Notes
  },
  
  // Computer 8 - Additional Desktop
  {
    "890123456789",         // RFID Tag ID
    "Lab Desktop",          // Component Type
    "Intel i5-10400",       // Processor
    "8GB DDR4",             // RAM
    "256GB SSD",            // Storage
    "Integrated",           // Graphics
    "Available",            // Status
    "Lab 302",              // Location
    "18-01-2024",           // Purchase Date
    "Basic lab computer"    // Notes
  }
};

// Calculate database size
int databaseSize = sizeof(specDatabase) / sizeof(specDatabase[0]);

// ==================== GLOBAL VARIABLES ====================
String lastTagID = "";
unsigned long lastScanTime = 0;
int scanCount = 0;
bool systemReady = false;
String scannedTags[10];  // Store last 10 scanned tags
int scanIndex = 0;

// ==================== SYSTEM STATES ====================
enum SystemState {
  STATE_IDLE,
  STATE_READING_TAG,
  STATE_PROCESSING,
  STATE_DISPLAYING,
  STATE_UPLOADING,
  STATE_ERROR,
  STATE_SLEEP
};

SystemState currentState = STATE_IDLE;

// ==================== SETUP FUNCTION ====================
void setup() {
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off (active low)
  digitalWrite(BUZZER_PIN, LOW); // Buzzer off
  
  // Initialize serial for debugging
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);  // Wait for serial to initialize
  
  printHeader();
  Serial.println(F("System Initializing..."));
  
  // Initialize I2C
  Wire.begin();
  delay(100);
  
  // Initialize RFID serial communication
  rfidSerial.begin(RFID_BAUD_RATE);
  Serial.println(F("✓ RFID Module Initialized"));
  
  // Initialize NodeMCU serial communication
  nodeSerial.begin(NODE_BAUD_RATE);
  Serial.println(F("✓ NodeMCU Communication Ready"));
  
  // Initialize LCD
  if (!initializeLCD()) {
    Serial.println(F("⚠️ LCD not found! Check I2C address and connections"));
  } else {
    Serial.println(F("✓ LCD Initialized"));
  }
  
  // Scan I2C devices (debug)
  if (SERIAL_DEBUG) {
    scanI2CDevices();
  }
  
  // Display welcome message
  displayWelcomeScreen();
  
  // System ready
  Serial.println(F("✓ System Ready!"));
  Serial.println(F("======================================"));
  Serial.println(F("Waiting for RFID tags..."));
  
  systemReady = true;
  currentState = STATE_IDLE;
  
  // Beep to indicate ready
  beep(100);
}

// ==================== MAIN LOOP ====================
void loop() {
  switch(currentState) {
    case STATE_IDLE:
      checkForRFID();
      updateIdleDisplay();
      break;
      
    case STATE_READING_TAG:
      readRFIDTag();
      break;
      
    case STATE_PROCESSING:
      processTagData();
      break;
      
    case STATE_DISPLAYING:
      // Already handled in processTagData
      currentState = STATE_IDLE;
      break;
      
    case STATE_UPLOADING:
      // Handle data upload to NodeMCU
      currentState = STATE_IDLE;
      break;
      
    case STATE_ERROR:
      handleError();
      break;
      
    case STATE_SLEEP:
      // Low power mode
      delay(1000);
      break;
  }
  
  // Small delay to prevent overwhelming the system
  delay(50);
}

// ==================== PRINT HEADER ====================
void printHeader() {
  Serial.println(F("\n\n======================================"));
  Serial.println(F("EFFICIENT COMPUTER SPECIFICATIONS RETRIEVAL"));
  Serial.println(F("Using RFID and STM32"));
  Serial.println(F("======================================"));
  Serial.println(F("Team: Ankit | Umang | Pradyumn | Atharva"));
  Serial.println(F("Guide: Prof. Jagdish Sarode"));
  Serial.println(F("SAKEC | University of Mumbai | AY 2023-24"));
  Serial.println(F("======================================"));
}

// ==================== LCD INITIALIZATION ====================
bool initializeLCD() {
  // Try multiple I2C addresses if needed
  byte lcdAddresses[] = {0x27, 0x3F, 0x20, 0x26};
  
  for(int i = 0; i < 4; i++) {
    Wire.beginTransmission(lcdAddresses[i]);
    if(Wire.endTransmission() == 0) {
      lcd = LiquidCrystal_I2C(lcdAddresses[i], LCD_COLUMNS, LCD_ROWS);
      lcd.init();
      lcd.backlight();
      lcd.clear();
      
      if (SERIAL_DEBUG) {
        Serial.print(F("LCD found at 0x"));
        Serial.println(lcdAddresses[i], HEX);
      }
      return true;
    }
    delay(100);
  }
  return false;
}

// ==================== I2C SCANNER ====================
void scanI2CDevices() {
  Serial.println(F("\n🔍 Scanning I2C devices..."));
  byte error, address;
  int deviceCount = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print(F("✅ I2C device found at 0x"));
      if(address < 16) Serial.print(F("0"));
      Serial.print(address, HEX);
      Serial.println(F(" !"));
      deviceCount++;
    }
  }
  
  if(deviceCount == 0) {
    Serial.println(F("❌ No I2C devices found!"));
  } else {
    Serial.print(F("Found "));
    Serial.print(deviceCount);
    Serial.println(F(" I2C devices"));
  }
  Serial.println();
}

// ==================== WELCOME SCREEN ====================
void displayWelcomeScreen() {
  lcd.clear();
  
  // Line 1 - Project name
  lcd.setCursor(0, 0);
  lcd.print(F("RFID COMPUTER"));
  
  // Line 2 - System
  lcd.setCursor(2, 1);
  lcd.print(F("SPECS SYSTEM"));
  
  // Blink animation
  for(int i = 0; i < 3; i++) {
    lcd.noBacklight();
    delay(200);
    lcd.backlight();
    delay(200);
  }
  
  delay(1500);
  
  // Show team names
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Ankit | Umang"));
  lcd.setCursor(0, 1);
  lcd.print(F("Pradyumn|Atharva"));
  delay(2000);
  
  // Ready screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SCAN RFID TAG"));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait..."));
}

// ==================== UPDATE IDLE DISPLAY ====================
void updateIdleDisplay() {
  static unsigned long lastUpdate = 0;
  static int dotCount = 0;
  
  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    
    lcd.setCursor(13, 1);
    for (int i = 0; i < 3; i++) {
      if (i < dotCount) {
        lcd.print(".");
      } else {
        lcd.print(" ");
      }
    }
    
    dotCount = (dotCount + 1) % 4;
  }
}

// ==================== CHECK FOR RFID ====================
void checkForRFID() {
  // Check if RFID data is available
  if (rfidSerial.available() >= RFID_TAG_LENGTH) {
    // Prevent multiple scans of same tag
    unsigned long currentTime = millis();
    if (currentTime - lastScanTime > SCAN_COOLDOWN) {
      currentState = STATE_READING_TAG;
      lastScanTime = currentTime;
      digitalWrite(LED_PIN, LOW);  // LED on (scanning)
      beep(50);  // Short beep for scan
    } else {
      // Too soon, ignore and flush buffer
      while(rfidSerial.available() >= RFID_TAG_LENGTH) {
        rfidSerial.read();
      }
    }
  }
}

// ==================== READ RFID TAG ====================
void readRFIDTag() {
  String tagID = "";
  
  // Read the 12-digit RFID tag ID
  for (int i = 0; i < RFID_TAG_LENGTH; i++) {
    if (rfidSerial.available()) {
      char c = rfidSerial.read();
      // Only accept printable characters
      if (isPrintable(c)) {
        tagID += c;
      } else {
        i--; // Skip non-printable
      }
    } else {
      // Handle incomplete read
      Serial.println(F("⚠️ Incomplete tag read"));
      currentState = STATE_ERROR;
      return;
    }
    delay(5);
  }
  
  // Flush any remaining data
  while (rfidSerial.available()) {
    rfidSerial.read();
    delay(5);
  }
  
  // Store for processing
  lastTagID = tagID;
  scanCount++;
  
  // Store in history
  scannedTags[scanIndex % 10] = tagID;
  scanIndex++;
  
  Serial.print(F("\n📡 Tag Scanned #"));
  Serial.print(scanCount);
  Serial.print(F(" | ID: "));
  Serial.println(tagID);
  
  // Update LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SCANNED:"));
  lcd.setCursor(0, 1);
  lcd.print(tagID);
  delay(500);
  
  // Move to processing state
  currentState = STATE_PROCESSING;
}

// ==================== PROCESS TAG DATA ====================
void processTagData() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PROCESSING..."));
  lcd.setCursor(0, 1);
  lcd.print(F("ID: " + lastTagID.substring(0, 8)));
  
  Serial.print(F("🔍 Searching database for tag: "));
  Serial.println(lastTagID);
  
  // Search for tag in database
  bool found = false;
  int componentIndex = -1;
  
  for (int i = 0; i < databaseSize; i++) {
    if (lastTagID.equals(specDatabase[i].tagID)) {
      found = true;
      componentIndex = i;
      break;
    }
  }
  
  if (found) {
    Serial.println(F("✅ Component Found!"));
    Serial.print(F("   Type: ")); Serial.println(specDatabase[componentIndex].componentType);
    Serial.print(F("   CPU: ")); Serial.println(specDatabase[componentIndex].processor);
    
    beep(200);  // Success beep
    displayComponentSpecs(componentIndex);
    sendToNodeMCU(componentIndex);
    
    // Update LCD for next scan
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("SCAN RFID TAG"));
    lcd.setCursor(0, 1);
    lcd.print(F("Ready..."));
  } else {
    // Unknown tag
    Serial.println(F("❌ Unknown Tag! Not in database"));
    Serial.println(F("   Add this tag to database: \"") + lastTagID + F("\""));
    
    beep(500);  // Error beep
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("UNKNOWN TAG!"));
    lcd.setCursor(0, 1);
    lcd.print(F("ID: " + lastTagID));
    delay(3000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("SCAN RFID TAG"));
    lcd.setCursor(0, 1);
    lcd.print(F("Please wait..."));
  }
  
  digitalWrite(LED_PIN, HIGH);  // LED off
  currentState = STATE_IDLE;
}

// ==================== DISPLAY COMPONENT SPECS ====================
void displayComponentSpecs(int index) {
  // Display Component Type/Name first
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("COMPONENT:"));
  lcd.setCursor(0, 1);
  
  String compType = specDatabase[index].componentType;
  scrollTextIfNeeded(compType, 1, 1500);
  
  // Display Processor
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PROCESSOR:"));
  lcd.setCursor(0, 1);
  
  String processor = specDatabase[index].processor;
  scrollTextIfNeeded(processor, 1, 2000);
  
  // Display RAM
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("RAM:"));
  lcd.setCursor(0, 1);
  lcd.print(specDatabase[index].ram);
  delay(1500);
  
  // Display Storage
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("STORAGE:"));
  lcd.setCursor(0, 1);
  
  String storage = specDatabase[index].storage;
  scrollTextIfNeeded(storage, 1, 1500);
  
  // Display Graphics
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("GRAPHICS:"));
  lcd.setCursor(0, 1);
  
  String graphics = specDatabase[index].graphics;
  scrollTextIfNeeded(graphics, 1, 2000);
  
  // Display Status
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("STATUS:"));
  lcd.setCursor(0, 1);
  lcd.print(specDatabase[index].status);
  delay(1500);
  
  // Display Location
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("LOCATION:"));
  lcd.setCursor(0, 1);
  
  String location = specDatabase[index].location;
  scrollTextIfNeeded(location, 1, 1500);
  
  // Display Purchase Date
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("PURCHASE:"));
  lcd.setCursor(0, 1);
  lcd.print(specDatabase[index].purchaseDate);
  delay(1500);
}

// ==================== SCROLL TEXT IF NEEDED ====================
void scrollTextIfNeeded(String text, int row, int displayTime) {
  int length = text.length();
  
  if (length <= 16) {
    lcd.setCursor(0, row);
    lcd.print(text);
    delay(displayTime);
    return;
  }
  
  // Scroll text
  unsigned long startTime = millis();
  while (millis() - startTime < displayTime) {
    for (int i = 0; i <= length - 16; i++) {
      lcd.setCursor(0, row);
      lcd.print(text.substring(i, i + 16));
      delay(300);
      
      // Break if display time exceeded
      if (millis() - startTime >= displayTime) {
        break;
      }
    }
  }
  
  // Show truncated text at the end
  lcd.setCursor(0, row);
  lcd.print(text.substring(0, 13) + "...");
}

// ==================== SEND TO NODEMCU ====================
void sendToNodeMCU(int index) {
  // Format: TAGID|TYPE|PROCESSOR|RAM|STORAGE|GRAPHICS|STATUS|LOCATION
  String data = String(specDatabase[index].tagID) + "|" +
                String(specDatabase[index].componentType) + "|" +
                String(specDatabase[index].processor) + "|" +
                String(specDatabase[index].ram) + "|" +
                String(specDatabase[index].storage) + "|" +
                String(specDatabase[index].graphics) + "|" +
                String(specDatabase[index].status) + "|" +
                String(specDatabase[index].location);
  
  nodeSerial.println(data);
  Serial.println(F("📤 Data sent to NodeMCU:"));
  Serial.println(data);
  
  // Wait for acknowledgment (optional)
  delay(100);
  if (nodeSerial.available()) {
    String ack = nodeSerial.readStringUntil('\n');
    Serial.print(F("NodeMCU Response: "));
    Serial.println(ack);
  }
}

// ==================== BEEP FUNCTION ====================
void beep(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

// ==================== ERROR HANDLER ====================
void handleError() {
  Serial.println(F("⚠️ Error occurred!"));
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("ERROR!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Check system"));
  
  // Flash LED
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(200);
  }
  
  // Flush buffers
  while(rfidSerial.available()) {
    rfidSerial.read();
  }
  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("SCAN RFID TAG"));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait..."));
  
  digitalWrite(LED_PIN, HIGH);  // LED off
  currentState = STATE_IDLE;
}

// ==================== GET LAST SCANNED TAG ====================
String getLastScannedTag() {
  if (scanIndex > 0) {
    return scannedTags[(scanIndex - 1) % 10];
  }
  return "None";
}

// ==================== PRINT DATABASE INFO ====================
void printDatabaseInfo() {
  Serial.println(F("\n📋 Database Summary:"));
  Serial.print(F("Total components: "));
  Serial.println(databaseSize);
  
  int available = 0, inUse = 0, repair = 0;
  for (int i = 0; i < databaseSize; i++) {
    if (strcmp(specDatabase[i].status, "Available") == 0) available++;
    else if (strcmp(specDatabase[i].status, "In Use") == 0) inUse++;
    else repair++;
  }
  
  Serial.print(F("Available: ")); Serial.println(available);
  Serial.print(F("In Use: ")); Serial.println(inUse);
  Serial.print(F("Other: ")); Serial.println(repair);
}

// ==================== SERIAL COMMAND HANDLER ====================
void handleSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'd':
      case 'D':
        printDatabaseInfo();
        break;
      case 's':
      case 'S':
        scanI2CDevices();
        break;
      case 'l':
      case 'L':
        Serial.println(F("Last scanned tag: ") + getLastScannedTag());
        break;
      case 'h':
      case 'H':
        Serial.println(F("\nCommands:"));
        Serial.println(F("d - Print database info"));
        Serial.println(F("s - Scan I2C devices"));
        Serial.println(F("l - Show last scanned tag"));
        Serial.println(F("h - Show this help"));
        break;
    }
    // Clear remaining input
    while (Serial.available()) Serial.read();
  }
}
