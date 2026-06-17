#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <time.h>

#include "config.h"
#include "data_logger.h"
#include "humidity_control.h"
#include "wifi_manager.h"
#include "web_dashboard.h"

WebServer server(80);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
DHT dht(PIN_DHT, DHT22);
WiFiManager wifiMgr;
HumidityController ctrl;
DataLogger logger;

float gTemp     = 0.0f;
float gHum      = 0.0f;
bool  gDHTValid = false;
unsigned long gBootTime       = 0;
unsigned long gLastDHT        = 0;
unsigned long gLastSensorLog  = 0;
unsigned long gLastLCD        = 0;
unsigned long gLastLCDReinit  = 0;
String gPrevState             = "";

void setupRoutes();
void setupAPRoutes();
void updateLCD();
void reinitLCD();
String buildStatusJSON();

void setup() {
  Serial.begin(115200);
  Serial.println("\n==============================");
  Serial.println(" Mist-Ify Controller v" FW_VERSION);
  Serial.println("==============================");

  ctrl.begin();

  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   Mist-Ify   ");
  lcd.setCursor(0, 1);
  lcd.print("  Loading...  ");

  dht.begin();

  bool connected = wifiMgr.begin();

  if (wifiMgr.isAPMode()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Setup Mode");
    lcd.setCursor(0, 1);
    lcd.print(wifiMgr.getIP());
    setupAPRoutes();
    server.begin();
    Serial.println("[Server] AP setup server started");
    while (wifiMgr.isAPMode()) {
      server.handleClient();
      delay(10);
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(wifiMgr.getIP());
  delay(5000);

  if (MDNS.begin(OTA_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] http://%s.local/\n", OTA_HOSTNAME);
  }

  configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
  Serial.println("[NTP] Syncing time...");
  struct tm ti;
  int attempts = 0;
  while (!getLocalTime(&ti) && attempts < 10) {
    delay(500);
    attempts++;
  }
  if (getLocalTime(&ti)) {
    Serial.printf("[NTP] Time: %04d-%02d-%02d %02d:%02d:%02d\n",
      ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
      ti.tm_hour, ti.tm_min, ti.tm_sec);
  }

  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    Serial.println("[OTA] Update starting...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("OTA Updating...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[OTA] Update complete!");
    lcd.setCursor(0, 1);
    lcd.print("Done! Restart..");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int pct = progress / (total / 100);
    Serial.printf("[OTA] Progress: %u%%\r", pct);
    lcd.setCursor(0, 1);
    lcd.printf("Progress: %3d%%  ", pct);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]\n", error);
    lcd.setCursor(0, 1);
    lcd.print("OTA Error!      ");
  });
  ArduinoOTA.begin();

  setupRoutes();
  server.begin();

  gBootTime = millis();
  logger.logEvent("System boot OK");
  Serial.printf("[Ready] Dashboard: http://%s/\n", wifiMgr.getIP().c_str());

  lcd.clear();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  unsigned long now = millis();

  if (now - gLastDHT >= DHT_READ_INTERVAL) {
    gLastDHT = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      gTemp = t;
      gHum  = h;
      gDHTValid = true;
    } else {
      Serial.println("[DHT] Read error");
    }
  }

  if (gDHTValid) {
    bool changed = ctrl.update(gHum);
    if (changed) {
      String st = ctrl.stateStr();
      if (st != gPrevState) {
        String msg = "State: " + st;
        if (ctrl.isPumpOn())  msg += " | Pump ON";
        if (ctrl.isMistOn())  msg += " | Mist ON";
        if (ctrl.isBuzzerOn()) msg += " | BUZZER!";
        logger.logEvent(msg.c_str());
        Serial.println("[Ctrl] " + msg);
        gPrevState = st;
        reinitLCD();
      }
    }
  }

  if (now - gLastSensorLog >= SENSOR_LOG_INTERVAL) {
    gLastSensorLog = now;
    if (gDHTValid) {
      logger.logSensor(gTemp, gHum);
    }
  }

  if (now - gLastLCDReinit >= 10000UL) {
    gLastLCDReinit = now;
    reinitLCD();
  }

  if (now - gLastLCD >= LCD_UPDATE_INTERVAL) {
    gLastLCD = now;
    updateLCD();
  }

  delay(10);
}

void reinitLCD() {
  lcd.init();
  lcd.backlight();
}

void updateLCD() {
  lcd.setCursor(0, 0);
  if (gDHTValid) {
    char line1[17];
    snprintf(line1, sizeof(line1), "T:%-5.1fC H:%4.1f%%", gTemp, gHum);
    lcd.print(line1);
  } else {
    lcd.print("Sensor Error!   ");
  }

  lcd.setCursor(0, 1);
  char line2[17];
  if (ctrl.getState() == STATE_ALARM) {
    snprintf(line2, sizeof(line2), "!! ALARM !!     ");
  } else if (ctrl.isManual()) {
    snprintf(line2, sizeof(line2), "M P:%s  M:%s  ",
      ctrl.isPumpOn() ? "ON " : "OFF",
      ctrl.isMistOn() ? "ON " : "OFF");
  } else {
    snprintf(line2, sizeof(line2), "A P:%s  M:%s  ",
      ctrl.isPumpOn() ? "ON " : "OFF",
      ctrl.isMistOn() ? "ON " : "OFF");
  }
  lcd.print(line2);
}

String buildStatusJSON() {
  String j = "{";
  j += "\"temp\":" + String(gTemp, 1);
  j += ",\"hum\":" + String(gHum, 1);
  j += ",\"state\":\"" + String(ctrl.stateStr()) + "\"";
  j += ",\"pump\":" + String(ctrl.isPumpOn() ? "true" : "false");
  j += ",\"mist\":" + String(ctrl.isMistOn() ? "true" : "false");
  j += ",\"buzzer\":" + String(ctrl.isBuzzerOn() ? "true" : "false");
  j += ",\"manual\":" + String(ctrl.isManual() ? "true" : "false");
  j += ",\"uptime\":" + String((millis() - gBootTime) / 1000);
  j += ",\"rssi\":" + String(WiFi.RSSI());
  j += ",\"ip\":\"" + wifiMgr.getIP() + "\"";
  j += ",\"heap\":" + String(ESP.getFreeHeap());
  j += ",\"ver\":\"" FW_VERSION "\"";
  j += "}";
  return j;
}

void setupAPRoutes() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", wifiMgr.getSetupHTML());
  });

  server.on("/wifi-scan", HTTP_GET, []() {
    server.send(200, "application/json", wifiMgr.scanNetworksJSON());
  });

  server.on("/wifi-save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    if (ssid.length() > 0) {
      wifiMgr.saveCredentials(ssid, pass);
      server.send(200, "application/json", "{\"ok\":true}");
      delay(1500);
      ESP.restart();
    } else {
      server.send(400, "application/json", "{\"ok\":false}");
    }
  });
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", DASHBOARD_HTML);
  });

  server.on("/api/status", HTTP_GET, []() {
    server.send(200, "application/json", buildStatusJSON());
  });

  server.on("/api/sensors", HTTP_GET, []() {
    server.send(200, "application/json", logger.getSensorJSON());
  });

  server.on("/api/events", HTTP_GET, []() {
    server.send(200, "application/json", logger.getEventJSON());
  });

  server.on("/api/pump", HTTP_POST, []() {
    if (ctrl.isManual()) {
      ctrl.toggleManualPump();
      logger.logEvent(ctrl.isPumpOn() ? "Manual: Pump ON" : "Manual: Pump OFF");
    }
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/mist", HTTP_POST, []() {
    if (ctrl.isManual()) {
      ctrl.toggleManualMist();
      logger.logEvent(ctrl.isMistOn() ? "Manual: Mist ON" : "Manual: Mist OFF");
    }
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/manual", HTTP_POST, []() {
    ctrl.setManualMode(true);
    logger.logEvent("Mode: MANUAL");
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/auto", HTTP_POST, []() {
    ctrl.setManualMode(false);
    logger.logEvent("Mode: AUTO");
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/alarm", HTTP_POST, []() {
    ctrl.resetAlarm();
    logger.logEvent("Alarm reset");
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.on("/api/wifi-reset", HTTP_POST, []() {
    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    wifiMgr.clearCredentials();
    ESP.restart();
  });

  server.on("/api/ota", HTTP_POST,
    []() {
      server.send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      if (!Update.hasError()) {
        logger.logEvent("OTA update success");
        delay(1000);
        ESP.restart();
      }
    },
    []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA-Web] File: %s\n", upload.filename.c_str());
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Web OTA Update");
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("[OTA-Web] Success: %u bytes\n", upload.totalSize);
          lcd.setCursor(0, 1);
          lcd.print("Done! Restart..");
        } else {
          Update.printError(Serial);
          lcd.setCursor(0, 1);
          lcd.print("OTA Failed!     ");
        }
      }
    }
  );
}
