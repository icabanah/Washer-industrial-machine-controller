// utils.cpp
#include "utils.h"

// Instancia global
UtilsClass Utils;

void UtilsClass::init() {
  // Inicializar variables de control
  _mainTimerRunning = false;
  _mainTimer = nullptr;
  _taskCount = 0;
  
  // Inicializar array de tareas asíncronas
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++) {
    _asyncTasks[i] = nullptr;
  }
  
  // Configurar temporizador principal
  _setupMainTimer();
  
  debug("Utils inicializado");
}

/** 
  * Configuración del temporizador principal
  * Este temporizador se encargará de actualizar los contadores y estados
  * del sistema cada cierto intervalo de tiempo
  * Se utiliza AsyncTaskLib para evitar bloqueos en el loop principal
  * y permitir una ejecución más fluida de otras tareas
  * El temporizador se configura para ejecutarse cada INTERVALO_TEMPORIZADOR
  * y se detiene automáticamente si no se necesita
  * En la implementación final, se llamará a ProgramController.updateTimers()
  * para actualizar los contadores y estados del sistema
  */

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

/**
 * Registra una tarea asíncrona para ser gestionada automáticamente
 * Esto permite centralizar la actualización de todas las tareas en un solo lugar
 * @param task Puntero a la tarea AsyncTask a registrar
 */
void UtilsClass::registerAsyncTask(AsyncTask* task) {
  if (task == nullptr || _taskCount >= MAX_ASYNC_TASKS) {
    return;
  }
  
  // Verificar si la tarea ya está registrada
  for (uint8_t i = 0; i < _taskCount; i++) {
    if (_asyncTasks[i] == task) {
      return;  // La tarea ya está registrada
    }
  }
  
  // Registrar la nueva tarea
  _asyncTasks[_taskCount++] = task;
  debug("Nueva tarea asíncrona registrada");
}

/**
 * Elimina una tarea asíncrona del gestor
 * @param task Puntero a la tarea AsyncTask a eliminar
 */
void UtilsClass::unregisterAsyncTask(AsyncTask* task) {
  if (task == nullptr || _taskCount == 0) {
    return;
  }
  
  // Buscar la tarea en el array
  for (uint8_t i = 0; i < _taskCount; i++) {
    if (_asyncTasks[i] == task) {
      // Eliminar la tarea desplazando las demás
      for (uint8_t j = i; j < _taskCount - 1; j++) {
        _asyncTasks[j] = _asyncTasks[j + 1];
      }
      _asyncTasks[--_taskCount] = nullptr;
      debug("Tarea asíncrona eliminada");
      return;
    }
  }
}

/**
 * Actualiza todas las tareas asíncronas registradas
 * Esta función debe ser llamada en cada iteración del loop principal
 */
void UtilsClass::updateAsyncTasks() {
  // Actualizar el temporizador principal si está activo
  if (_mainTimer && _mainTimerRunning) {
    _mainTimer->Update();
  }
  
  // Actualizar todas las tareas registradas
  for (uint8_t i = 0; i < _taskCount; i++) {
    if (_asyncTasks[i] && _asyncTasks[i]->IsActive()) {
      _asyncTasks[i]->Update();
    }
  }
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
