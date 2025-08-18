#ifndef DEBUG_H
#define DEBUG_H

#include "Arduino.h"

class DebugClass {
private:
  bool _enabled = true;
  
public:
  void init();
  void print(const String& msg);
  void enable(bool state);
  bool isEnabled();
};

extern DebugClass Debug;

#endif // DEBUG_H