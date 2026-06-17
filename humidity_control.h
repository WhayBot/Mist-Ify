#ifndef HUMIDITY_CONTROL_H
#define HUMIDITY_CONTROL_H

#include <Arduino.h>
#include "config.h"

enum ControlState {
  STATE_IDLE,
  STATE_PUMPING,
  STATE_SOAKING,
  STATE_MISTING,
  STATE_ALARM
};

class HumidityController {
private:
  ControlState _state      = STATE_IDLE;
  unsigned long _stateTime = 0;
  bool _pumpOn   = false;
  bool _mistOn   = false;
  bool _buzzerOn = false;
  bool _manual   = false;
  bool _manPump  = false;
  bool _manMist  = false;

  void _setPump(bool on) {
    _pumpOn = on;
    digitalWrite(PIN_PUMP, on ? HIGH : LOW);
  }

  void _setMist(bool on) {
    _mistOn = on;
    digitalWrite(PIN_MIST, on ? HIGH : LOW);
  }

  void _setBuzzer(bool on) {
    _buzzerOn = on;
    digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
  }

  void _enterState(ControlState s) {
    _state = s;
    _stateTime = millis();
  }

public:
  void begin() {
    pinMode(PIN_PUMP,   OUTPUT);
    pinMode(PIN_MIST,   OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    _setPump(false);
    _setMist(false);
    _setBuzzer(false);
    _enterState(STATE_IDLE);
  }

  bool update(float humidity) {
    if (_manual) {
      _setPump(_manPump);
      _setMist(_manMist);
      _setBuzzer(false);
      return false;
    }

    ControlState prev = _state;
    unsigned long elapsed = millis() - _stateTime;

    switch (_state) {
      case STATE_IDLE:
        _setPump(false);
        _setMist(false);
        _setBuzzer(false);
        if (humidity > 0 && humidity <= HUMIDITY_LOW) {
          _enterState(STATE_PUMPING);
          _setPump(true);
        }
        break;

      case STATE_PUMPING:
        if (elapsed >= PUMP_DURATION) {
          _setPump(false);
          _enterState(STATE_SOAKING);
        }
        break;

      case STATE_SOAKING:
        if (elapsed >= SOAK_DURATION) {
          _setMist(true);
          _enterState(STATE_MISTING);
        }
        break;

      case STATE_MISTING:
        if (humidity >= HUMIDITY_HIGH) {
          _setMist(false);
          _enterState(STATE_IDLE);
        } else if (elapsed >= MIST_TIMEOUT) {
          _setMist(false);
          _setBuzzer(true);
          _enterState(STATE_ALARM);
        }
        break;

      case STATE_ALARM:
        digitalWrite(PIN_BUZZER, (millis() / 500) % 2 == 0 ? HIGH : LOW);
        break;
    }

    return _state != prev;
  }

  void resetAlarm() {
    if (_state == STATE_ALARM) {
      _setBuzzer(false);
      _enterState(STATE_IDLE);
    }
  }

  void setManualMode(bool on) {
    _manual = on;
    if (!on) {
      _manPump = false;
      _manMist = false;
      _setPump(false);
      _setMist(false);
      _enterState(STATE_IDLE);
    }
  }

  void toggleManualPump() { _manPump = !_manPump; }
  void toggleManualMist() { _manMist = !_manMist; }

  bool isManual()   { return _manual;   }
  bool isPumpOn()   { return _pumpOn;   }
  bool isMistOn()   { return _mistOn;   }
  bool isBuzzerOn() { return _buzzerOn; }
  ControlState getState() { return _state; }

  const char* stateStr() {
    switch (_state) {
      case STATE_IDLE:    return "IDLE";
      case STATE_PUMPING: return "PUMPING";
      case STATE_SOAKING: return "SOAKING";
      case STATE_MISTING: return "MISTING";
      case STATE_ALARM:   return "ALARM";
      default:            return "???";
    }
  }
};

#endif
