// utils.cpp
#include "utils.h"
#include "actuators.h"

// Instancia global
UtilsClass Utils;

void UtilsClass::init() {
  // Inicializar variables de control
  _mainTimerRunning = false;
  _mainTimerLastRun = 0;
  _taskCount = 0;
  _nextTaskId = 1; // Empezamos con ID 1, el 0 lo reservamos para indicar error
  
  // Inicializar array de tareas
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++) {
    _tasks[i].active = false;
    _tasks[i].callback = nullptr;
    _tasks[i].id = 0;
  }
  
  // Configurar temporizador principal
  _setupMainTimer();
  
  debug("Sistema de utils inicializado sin dependencias externas");
}

/** 
  * Configuración del temporizador principal
  * Este temporizador se encargará de actualizar los contadores y estados
  * del sistema cada cierto intervalo de tiempo
  * Se utiliza millis() para evitar bloqueos en el loop principal
  */
void UtilsClass::_setupMainTimer() {
  // Configurar el temporizador principal basado en millis()
  _mainTimerLastRun = millis();
  debug("Temporizador principal configurado con intervalo de " + String(INTERVALO_TEMPORIZADOR) + " ms");
}

void UtilsClass::startMainTimer() {
  if (!_mainTimerRunning) {
    _mainTimerRunning = true;
    _mainTimerLastRun = millis();
    debug("Temporizador principal iniciado");
  }
}

void UtilsClass::stopMainTimer() {
  if (_mainTimerRunning) {
    _mainTimerRunning = false;
    debug("Temporizador principal detenido");
  }
}

bool UtilsClass::isMainTimerRunning() {
  return _mainTimerRunning;
}

/**
 * Busca una tarea por su ID
 * @param taskId ID de la tarea a buscar
 * @return Índice de la tarea en el array o -1 si no se encontró
 */
int UtilsClass::_findTaskById(int taskId) {
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++) {
    if (_tasks[i].active && _tasks[i].id == taskId) {
      return i;
    }
  }
  return -1;
}

/**
 * Busca un slot libre en el array de tareas
 * @return Índice del slot libre o -1 si no hay espacio
 */
int UtilsClass::_findFreeTaskSlot() {
  // Primero buscamos slots que estén marcados como no activos
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++) {
    if (!_tasks[i].active) {
      return i;
    }
  }
  
  // Si no encontramos slots inactivos pero aún no llegamos al máximo, usar el próximo
  if (_taskCount < MAX_ASYNC_TASKS) {
    return _taskCount;
  }
  
  return -1; // No hay slots disponibles
}

/**
 * Crea un temporizador de un solo uso (ejecuta una vez y se elimina)
 * @param duration Duración en milisegundos
 * @param callback Función a ejecutar cuando expire el temporizador
 * @return ID de la tarea creada o -1 si falló
 */
int UtilsClass::createTimeout(unsigned long duration, TaskCallback callback) {
  if (callback == nullptr) {
    debug("Error: Intento de crear temporizador con callback nulo");
    return -1;
  }
  
  // Buscar un slot libre
  int slotIndex = _findFreeTaskSlot();
  if (slotIndex < 0) {
    debug("Error: No se puede crear más temporizadores, límite alcanzado");
    return -1;
  }
  
  // Crear nueva tarea
  TimedTask& task = _tasks[slotIndex];
  task.interval = duration;
  task.lastExecuted = millis();
  task.callback = callback;
  task.recurring = false;  // De un solo uso
  task.active = true;
  task.id = _nextTaskId++;
  
  // Incrementar contador si es necesario
  if (slotIndex == _taskCount) {
    _taskCount++;
  }
  
  debug("Nuevo temporizador creado con ID " + String(task.id) + " y duración de " + String(duration) + " ms");
  return task.id;
}

/**
 * Crea una tarea periódica que se ejecutará cada cierto intervalo
 * @param interval Intervalo en milisegundos entre ejecuciones
 * @param callback Función a ejecutar
 * @param startNow Si es true, la primera ejecución es inmediata
 * @return ID de la tarea creada o -1 si falló
 */
int UtilsClass::createInterval(unsigned long interval, TaskCallback callback, bool startNow) {
  if (callback == nullptr) {
    debug("Error: Intento de crear tarea periódica con callback nulo");
    return -1;
  }
  
  // Buscar un slot libre
  int slotIndex = _findFreeTaskSlot();
  if (slotIndex < 0) {
    debug("Error: No se puede crear más tareas, límite alcanzado");
    return -1;
  }
  
  // Crear nueva tarea
  TimedTask& task = _tasks[slotIndex];
  task.interval = interval;
  task.lastExecuted = startNow ? 0 : millis();  // Si startNow, asegura ejecución inmediata
  task.callback = callback;
  task.recurring = true;  // Tarea periódica
  task.active = true;
  task.id = _nextTaskId++;
  
  // Incrementar contador si es necesario
  if (slotIndex == _taskCount) {
    _taskCount++;
  }
  
  debug("Nueva tarea periódica creada con ID " + String(task.id) + " e intervalo de " + String(interval) + " ms");
  return task.id;
}

/**
 * Detiene una tarea por su ID
 * @param taskId ID de la tarea a detener
 * @return true si se detuvo correctamente, false si no se encontró
 */
bool UtilsClass::stopTask(int taskId) {
  int index = _findTaskById(taskId);
  if (index < 0) {
    debug("Error: Intento de detener tarea inexistente con ID " + String(taskId));
    return false;
  }
  
  _tasks[index].active = false;
  debug("Tarea con ID " + String(taskId) + " detenida");
  return true;
}

/**
 * Reinicia una tarea por su ID
 * @param taskId ID de la tarea a reiniciar
 * @return true si se reinició correctamente, false si no se encontró
 */
bool UtilsClass::restartTask(int taskId) {
  int index = _findTaskById(taskId);
  if (index < 0) {
    debug("Error: Intento de reiniciar tarea inexistente con ID " + String(taskId));
    return false;
  }
  
  _tasks[index].lastExecuted = millis();
  _tasks[index].active = true;
  debug("Tarea con ID " + String(taskId) + " reiniciada");
  return true;
}

/**
 * Actualiza todas las tareas temporizadas
 * Esta función debe ser llamada en cada iteración del loop principal
 */
void UtilsClass::updateTasks() {
  unsigned long currentTime = millis();
  
  // Verificar si el temporizador principal debe ejecutarse
  if (_mainTimerRunning) {
    if (currentTime - _mainTimerLastRun >= INTERVALO_TEMPORIZADOR) {
      _mainTimerLastRun = currentTime;
      
      // Ejecutar la función de actualización de temporizadores
      updateTimers();
      
      // También actualizamos los temporizadores de otros módulos
      Actuators.updateTimers();
      // Cuando estén implementados:
      // ProgramController.updateTimers();
      // Sensors.updateTimers();
    }
  }
  
  // Procesar todas las tareas temporizadas
  for (uint8_t i = 0; i < _taskCount; i++) {
    TimedTask& task = _tasks[i];
    
    // Verificar si la tarea está activa
    if (task.active && task.callback != nullptr) {
      // Verificar si es tiempo de ejecutar la tarea
      if (currentTime - task.lastExecuted >= task.interval) {
        // Ejecutar la función callback
        task.callback();
        
        // Actualizar tiempo de ejecución
        task.lastExecuted = currentTime;
        
        // Si no es recurrente, marcarla como inactiva
        if (!task.recurring) {
          task.active = false;
          debug("Tarea no recurrente con ID " + String(task.id) + " completada");
        }
      }
    }
  }
  
  // Compactar el array de tareas si hay tareas inactivas
  // Solo lo hacemos ocasionalmente para no consumir tiempo en cada iteración
  static unsigned long lastCompaction = 0;
  if (currentTime - lastCompaction >= 10000) { // Cada 10 segundos
    lastCompaction = currentTime;
    
    // Contar cuántas tareas inactivas hay
    int inactiveTasks = 0;
    for (uint8_t i = 0; i < _taskCount; i++) {
      if (!_tasks[i].active) {
        inactiveTasks++;
      }
    }
    
    // Si hay muchas tareas inactivas, compactar el array
    if (inactiveTasks > 5) {
      uint8_t newIndex = 0;
      
      // Mover todas las tareas activas al principio del array
      for (uint8_t i = 0; i < _taskCount; i++) {
        if (_tasks[i].active) {
          if (i != newIndex) {
            _tasks[newIndex] = _tasks[i];
          }
          newIndex++;
        }
      }
      
      // Actualizar el contador
      _taskCount = newIndex;
      debug("Array de tareas compactado. Tareas activas: " + String(_taskCount));
    }
  }
}

void UtilsClass::updateTimers() {
  // Esta función será llamada por el temporizador principal
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