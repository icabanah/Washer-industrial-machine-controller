#include "math_utils.h"
#include "Arduino.h"

bool MathUtils::isInRange(int value, int target, int range) {
  return (value >= (target - range)) && (value <= (target + range));
}

int MathUtils::mapValue(int value, int inMin, int inMax, int outMin, int outMax) {
  return map(value, inMin, inMax, outMin, outMax);
}

uint8_t MathUtils::calculateProgress(uint16_t current, uint16_t total) {
  if (total == 0) {
    return 0;
  }
  return (current * 100) / total;
}