#include "debug.h"

// Instancia global
DebugClass Debug;

void DebugClass::init() {
  _enabled = true;
}

void DebugClass::print(const String& msg) {
  if (_enabled) {
    Serial.println(msg);
  }
}

void DebugClass::enable(bool state) {
  _enabled = state;
}

bool DebugClass::isEnabled() {
  return _enabled;
}