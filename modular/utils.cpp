// utils.cpp
#include "utils.h"

// Instancia global
UtilsClass Utils;

void UtilsClass::init() {
  _mainTimerRunning = false;
  _mainTimer = nullptr;
  _setupMainTimer();
  debug("Utils inicializado");
}

/* Configuración del temporizador principal
Este temporizador se encargará de actualizar los contadores y estados
del sistema cada cierto intervalo de tiempo
Se utiliza AsyncTaskLib para evitar bloqueos en el loop principal
y permitir una ejecución más fluida de otras tareas
El temporizador se configura para ejecutarse cada INTERVALO_TEMPORIZADOR
y se detiene automáticamente si no se necesita
En la implementación final, se llamará a ProgramController.updateTimers()
para actualizar los contadores y estados del sistema
**/

void UtilsClass::_setupMainTimer() {
  // Usar AsyncTaskLib para crear un temporizador no bloqueante
  _mainTimer = new AsyncTask(INTERVALO_TEMPORIZADOR, true, []() {
    // En la implementación final, esta llamada se actualizará
    // para llamar a ProgramController.updateTimers()
    Utils.updateTimers();
  });
}

void UtilsClass::startMainTimer() {
  if (_mainTimer && !_mainTimerRunning) {
    _mainTimer->Start();
    _mainTimerRunning = true;
    debug("Temporizador principal iniciado");
  }
}

void UtilsClass::stopMainTimer() {
  if (_mainTimer && _mainTimerRunning) {
    _mainTimer->Stop(); 
    _mainTimerRunning = false;
    debug("Temporizador principal detenido");
  }
}

bool UtilsClass::isMainTimerRunning() {
  return _mainTimerRunning;
}

void UtilsClass::updateTimers() {
  // Esta función será llamada por el temporizador asincrónico
  // En la implementación final, actualizará los contadores y 
  // estados del sistema
}

void UtilsClass::formatTime(uint8_t minutes, uint8_t seconds, char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%02d:%02d", minutes, seconds);
}

uint16_t UtilsClass::convertToSeconds(uint8_t minutes, uint8_t seconds) {
  return (minutes * 60) + seconds;
}

void UtilsClass::convertFromSeconds(uint16_t totalSeconds, uint8_t& minutes, uint8_t& seconds) {
  minutes = totalSeconds / 60;
  seconds = totalSeconds % 60;
}

void UtilsClass::debug(const String& message) {
  // En producción, esta función podría ser configurada
  // para imprimir por Serial solo en modo debug
  Serial.println(message);
}

void UtilsClass::debugValue(const String& label, int value) {
  Serial.print(label);
  Serial.print(": ");
  Serial.println(value);
}

bool UtilsClass::isInRange(int value, int target, int range) {
  return (value >= (target - range)) && (value <= (target + range));
}

int UtilsClass::mapValue(int value, int inMin, int inMax, int outMin, int outMax) {
  return map(value, inMin, inMax, outMin, outMax);
}

uint8_t UtilsClass::calculateProgress(uint16_t current, uint16_t total) {
  if (total == 0) return 0;
  return (current * 100) / total;
}
