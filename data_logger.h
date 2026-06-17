#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <time.h>
#include "config.h"

struct SensorEntry {
  time_t timestamp;
  float temperature;
  float humidity;
};

struct EventEntry {
  time_t timestamp;
  char message[48];
};

class DataLogger {
private:
  SensorEntry _sensors[MAX_SENSOR_ENTRIES];
  EventEntry  _events[MAX_EVENT_ENTRIES];
  int _sHead = 0, _sCount = 0;
  int _eHead = 0, _eCount = 0;

public:
  void logSensor(float temp, float hum) {
    _sensors[_sHead].timestamp   = time(nullptr);
    _sensors[_sHead].temperature = temp;
    _sensors[_sHead].humidity    = hum;
    _sHead = (_sHead + 1) % MAX_SENSOR_ENTRIES;
    if (_sCount < MAX_SENSOR_ENTRIES) _sCount++;
  }

  void logEvent(const char* msg) {
    _events[_eHead].timestamp = time(nullptr);
    strncpy(_events[_eHead].message, msg, 47);
    _events[_eHead].message[47] = '\0';
    _eHead = (_eHead + 1) % MAX_EVENT_ENTRIES;
    if (_eCount < MAX_EVENT_ENTRIES) _eCount++;
  }

  String getSensorJSON() {
    String j = "[";
    int start = (_sCount < MAX_SENSOR_ENTRIES) ? 0 : _sHead;
    for (int i = 0; i < _sCount; i++) {
      int idx = (start + i) % MAX_SENSOR_ENTRIES;
      if (i > 0) j += ",";
      j += "{\"t\":";
      j += String((unsigned long)_sensors[idx].timestamp);
      j += ",\"tp\":";
      j += String(_sensors[idx].temperature, 1);
      j += ",\"hm\":";
      j += String(_sensors[idx].humidity, 1);
      j += "}";
    }
    j += "]";
    return j;
  }

  String getEventJSON() {
    String j = "[";
    int start = (_eCount < MAX_EVENT_ENTRIES) ? 0 : _eHead;
    for (int i = 0; i < _eCount; i++) {
      int idx = (start + i) % MAX_EVENT_ENTRIES;
      if (i > 0) j += ",";
      j += "{\"t\":";
      j += String((unsigned long)_events[idx].timestamp);
      j += ",\"m\":\"";
      j += String(_events[idx].message);
      j += "\"}";
    }
    j += "]";
    return j;
  }

  int getSensorCount() { return _sCount; }
  int getEventCount()  { return _eCount; }
};

#endif
