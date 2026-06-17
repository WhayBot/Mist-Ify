#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Pin Definitions
// ============================================
#define PIN_PUMP        25
#define PIN_MIST        26
#define PIN_DHT         4
#define PIN_BUZZER      27
#define PIN_SDA         21
#define PIN_SCL         22

// ============================================
// Humidity Control Thresholds
// ============================================
#define HUMIDITY_LOW    88.0f
#define HUMIDITY_HIGH   90.0f

// ============================================
// Timing (milliseconds)
// ============================================
#define PUMP_DURATION         3000UL
#define SOAK_DURATION         3000UL
#define MIST_TIMEOUT          300000UL   // 5 minutes
#define DHT_READ_INTERVAL     2000UL
#define SENSOR_LOG_INTERVAL   300000UL   // 5 minutes
#define LCD_UPDATE_INTERVAL   1000UL
#define STATUS_POLL_INTERVAL  2000UL
#define WIFI_CONNECT_TIMEOUT  30000UL

// ============================================
// WiFi AP Mode
// ============================================
#define AP_SSID       "Mist-Ify-Setup"
#define AP_PASSWORD   "mistify1234"
#define AP_IP         IPAddress(192, 168, 4, 1)

// ============================================
// OTA
// ============================================
#define OTA_PASSWORD  "mistify"
#define OTA_HOSTNAME  "mist-ify-ctrl"

// ============================================
// NTP
// ============================================
#define NTP_SERVER    "pool.ntp.org"
#define GMT_OFFSET    25200    // UTC+7 WIB
#define DST_OFFSET    0

// ============================================
// LCD I2C
// ============================================
#define LCD_ADDR      0x27
#define LCD_COLS      16
#define LCD_ROWS      2

// ============================================
// Data Logger Limits
// ============================================
#define MAX_SENSOR_ENTRIES  288
#define MAX_EVENT_ENTRIES   100

// ============================================
// Firmware
// ============================================
#define FW_VERSION    "1.0.0"

#endif
