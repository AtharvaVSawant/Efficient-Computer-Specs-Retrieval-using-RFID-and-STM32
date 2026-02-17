/**
 * Configuration File for STM32 RFID Project
 * 
 * File: config.h
 * Description: All configuration parameters for the STM32 RFID system
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==================== LCD CONFIGURATION ====================
#define LCD_ADDRESS     0x27    // I2C address (try 0x3F, 0x20, 0x26 if this doesn't work)
#define LCD_COLUMNS     16      // Number of columns
#define LCD_ROWS        2       // Number of rows

// ==================== RFID CONFIGURATION ====================
#define RFID_BAUD_RATE  9600    // EM-18 baud rate (usually 9600)
#define RFID_TAG_LENGTH 12      // Length of RFID tag ID (usually 12 digits)

// ==================== NODEMCU COMMUNICATION ====================
#define NODE_BAUD_RATE  9600    // Communication baud rate with NodeMCU

// ==================== SERIAL DEBUG CONFIGURATION ====================
#define SERIAL_BAUD_RATE 9600   // Serial monitor baud rate
#define SERIAL_DEBUG     true   // Enable/disable serial debug messages

// ==================== TIMING CONFIGURATION ====================
#define SCAN_COOLDOWN   2000    // Cooldown between scans (ms) - prevent multiple scans
#define DISPLAY_DELAY   2000    // Delay between LCD messages (ms)
#define WELCOME_DELAY   3000    // Welcome screen delay (ms)
#define SCROLL_SPEED    300     // Text scroll speed (ms per step)

// ==================== SYSTEM CONFIGURATION ====================
#define MAX_COMPONENTS  50      // Maximum number of components in database
#define BUZZER_ENABLED  true    // Enable/disable buzzer feedback
#define LED_INDICATOR   true    // Enable/disable LED indicator

// ==================== DATABASE CONFIGURATION ====================
#define DATABASE_VERSION "1.0"  // Database version

// ==================== SLEEP MODE CONFIGURATION ====================
#define SLEEP_TIMEOUT   300000  // Sleep after 5 minutes of inactivity (ms)
#define ENABLE_SLEEP     false  // Enable/disable sleep mode

// ==================== ERROR HANDLING ====================
#define MAX_RETRIES     3       // Maximum retries for failed operations
#define ERROR_BEEP_TIME 500     // Error beep duration (ms)
#define SUCCESS_BEEP_TIME 200   // Success beep duration (ms)

// ==================== WIFI CREDENTIALS (for reference) ====================
// These are just placeholders - actual credentials are in NodeMCU code
#define WIFI_SSID       "YourWiFiSSID"
#define WIFI_PASSWORD   "YourWiFiPassword"

#endif
