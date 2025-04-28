// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <AsyncTaskLib.h>

class UtilsClass {
public:
  // Inicialización
  void init();
  
  // Gestión de temporizadores
  void startMainTimer();
  void stopMainTimer();
  bool isMainTimerRunning();
  
  // Manejo y conversión de tiempo
  void updateTimers();
  void formatTime(uint8_t minutes, uint8_t seconds, char* buffer, size_t bufferSize);
  uint16_t convertToSeconds(uint8_t minutes, uint8_t seconds);
  void convertFromSeconds(uint16_t totalSeconds, uint8_t& minutes, uint8_t& seconds);
  
  // Utilidades para depuración
  void debug(const String& message);
  void debugValue(const String& label, int value);
  
  // Funciones auxiliares generales
  bool isInRange(int value, int target, int range);
  int mapValue(int value, int inMin, int inMax, int outMin, int outMax);
  uint8_t calculateProgress(uint16_t current, uint16_t total);

private:
  // Variables para control de temporizadores
  bool _mainTimerRunning;
  AsyncTask* _mainTimer;
  
  // Métodos internos
  void _setupMainTimer();
};

// Instancia global
extern UtilsClass Utils;

#endif // UTILS_H