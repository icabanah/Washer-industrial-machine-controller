#ifndef MATH_UTILS_H
#define MATH_UTILS_H

class MathUtils {
public:
  // Verificar si un valor está dentro de un rango
  static bool isInRange(int value, int target, int range);
  
  // Mapear un valor de un rango a otro
  static int mapValue(int value, int inMin, int inMax, int outMin, int outMax);
  
  // Calcular progreso como porcentaje
  static uint8_t calculateProgress(uint16_t current, uint16_t total);
};

#endif // MATH_UTILS_H