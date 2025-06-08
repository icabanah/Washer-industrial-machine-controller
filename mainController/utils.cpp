// utils.cpp
#include "utils.h"
#include "actuators.h"
#include "program_controller.h"

// Instancia global
UtilsClass Utils;

void UtilsClass::init()
{
  // Inicializar variables de control
  _mainTimerRunning = false;
  _mainTimerLastRun = 0;
  _taskCount = 0;
  _nextTaskId = 1; // Empezamos con ID 1, el 0 lo reservamos para indicar error

  // Inicializar array de tareas
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++)
  {
    _tasks[i].active = false;
    _tasks[i].callback = nullptr;
    _tasks[i].id = 0;
  }

  // Inicializar array de callbacks del temporizador principal
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++)
  {
    _timerCallbacks[i].active = false;
    _timerCallbacks[i].callback = nullptr;
    _timerCallbacks[i].id = 0;
  }
  _timerCallbackCount = 0;
  _nextTimerCallbackId = 1;

  // Configurar temporizador principal
  _setupMainTimer();

  debug("UtilsClass::init()| Sistema de utils inicializado sin dependencias externas");
}

/// @brief
/// Registra un callback para el temporizador principal.
/// @details
/// Este método permite registrar una función callback que se ejecutará periódicamente en el temporizador principal.
/// Es útil para tareas que necesitan ejecutarse en intervalos regulares, como actualizaciones de estado o monitoreo.
/// @param callback
/// La función que se desea registrar como callback.
/// @return
/// El ID del callback registrado, o -1 si no se pudo registrar.
int UtilsClass::registerTimerCallback(TaskCallback callback)
{
  if (callback == nullptr)
  { // Verificar si el callback es nulo
    debug("UtilsClass::registerTimerCallback| Error: Intento de registrar callback nulo para temporizador principal");
    return -1;
  }

  // Buscar un slot libre
  int freeSlot = -1;
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++)
  {
    if (i < _timerCallbackCount)
    { // Si ya hay callbacks registrados, buscar uno inactivo
      if (!_timerCallbacks[i].active)
      {
        freeSlot = i;
        break;
      }
    }
    else
    { // Si no hay callbacks registrados, usar el primer slot libre
      freeSlot = i;
      break;
    }
  }

  if (freeSlot < 0)
  { // No hay espacio para más callbacks
    debug("UtilsClass::registerTimerCallback| Error: No se puede registrar más callbacks para el temporizador principal");
    return -1;
  }

  // Registrar callback
  _timerCallbacks[freeSlot].callback = callback;
  _timerCallbacks[freeSlot].id = _nextTimerCallbackId++;
  _timerCallbacks[freeSlot].active = true;

  // Incrementar contador si es necesario
  if (freeSlot == _timerCallbackCount)
  {
    _timerCallbackCount++;
  }

  debug("UtilsClass::registerTimerCallback| Nuevo callback registrado para temporizador principal con ID " + String(_timerCallbacks[freeSlot].id));
  return _timerCallbacks[freeSlot].id;
}

bool UtilsClass::unregisterTimerCallback(int callbackId)
{
  for (uint8_t i = 0; i < _timerCallbackCount; i++)
  {
    if (_timerCallbacks[i].active && _timerCallbacks[i].id == callbackId)
    {
      _timerCallbacks[i].active = false;
      debug("Callback con ID " + String(callbackId) + " eliminado del temporizador principal");
      return true;
    }
  }

  debug("UtilsClass::unregisterTimerCallback| Error: Intento de eliminar callback inexistente con ID " + String(callbackId));
  return false;
}

/// @brief 
/// Crea una tarea periódica que se ejecutará en intervalos regulares.
/// @note
/// Asegúrate de que el callback no bloquee el hilo principal, ya que esto podría afectar el rendimiento del sistema.
void UtilsClass::_setupMainTimer()
{
  // Configurar el temporizador principal basado en millis()
  _mainTimerLastRun = millis();
  debug("UtilsClass::_setupMainTimer| Temporizador principal configurado con intervalo de " + String(INTERVALO_TEMPORIZADOR) + " ms");
}

void UtilsClass::startMainTimer()
{
  if (!_mainTimerRunning)
  {
    _mainTimerRunning = true;
    _mainTimerLastRun = millis();
    debug("UtilsClass::startMainTimer| Temporizador principal iniciado");
  }
}

void UtilsClass::stopMainTimer()
{
  if (_mainTimerRunning)
  {
    _mainTimerRunning = false;
    debug("UtilsClass::stopMainTimer| UtilsClass::stopMainTimer| Temporizador principal detenido");
  }
}

bool UtilsClass::isMainTimerRunning()
{
  return _mainTimerRunning;
}


/// @brief 
/// Detiene una tarea específica por su ID.
/// @param taskId 
/// El ID de la tarea que se desea detener.
/// Este ID debe corresponder a una tarea previamente creada.
/// @return 
/// true si la tarea se detuvo correctamente, false si no se encontró la tarea o si ya estaba detenida.
int UtilsClass::_findTaskById(int taskId)
{
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++)
  {
    if (_tasks[i].active && _tasks[i].id == taskId)
    {
      return i;
    }
  }
  return -1;
}

/**
 * Busca un slot libre en el array de tareas
 * @return Índice del slot libre o -1 si no hay espacio
 */
int UtilsClass::_findFreeTaskSlot()
{
  // Primero buscamos slots que estén marcados como no activos
  for (uint8_t i = 0; i < MAX_ASYNC_TASKS; i++)
  {
    if (!_tasks[i].active)
    {
      return i;
    }
  }

  // Si no encontramos slots inactivos pero aún no llegamos al máximo, usar el próximo
  if (_taskCount < MAX_ASYNC_TASKS)
  {
    return _taskCount;
  }

  return -1; // No hay slots disponibles
}

/// @brief
/// Crea un temporizador que se ejecutará una sola vez después de un cierto tiempo.
/// @note
/// Este método permite crear un temporizador que ejecutará una función callback una sola vez después de un intervalo de tiempo especificado.
/// Es útil para tareas que necesitan ejecutarse una vez después de un retraso, como inicializaciones o eventos únicos.
/// @param duration
/// La duración en milisegundos que el temporizador debe esperar antes de ejecutar el callback.
/// Este valor debe ser positivo y mayor que cero.
/// Si se especifica un valor menor o igual a cero, el temporizador no se creará.
/// @param callback
/// La función que se desea ejecutar cuando el temporizador expire.
/// Esta función debe tener la firma `void()` y no debe recibir parámetros.
/// Se ejecutará una sola vez después del tiempo especificado.
/// @return
/// El ID del temporizador creado, o -1 si no se pudo crear.
int UtilsClass::createTimeout(unsigned long duration, TaskCallback callback)
{
  if (callback == nullptr)
  {
    debug("UtilsClass::createTimeout| Error: Intento de crear temporizador con callback nulo");
    return -1;
  }

  // Buscar un slot libre
  int slotIndex = _findFreeTaskSlot();
  if (slotIndex < 0)
  {
    debug("UtilsClass::createTimeout| Error: No se puede crear más temporizadores, límite alcanzado");
    return -1;
  }

  // Crear nueva tarea
  TimedTask &task = _tasks[slotIndex];
  task.interval = duration;
  task.lastExecuted = millis();
  task.callback = callback;
  task.recurring = false; // De un solo uso
  task.active = true;
  task.id = _nextTaskId++;

  // Incrementar contador si es necesario
  if (slotIndex == _taskCount)
  {
    _taskCount++;
  }

  debug("UtilsClass::createTimeout| Nuevo temporizador creado con ID " + String(task.id) + " y duración de " + String(duration) + " ms");
  return task.id;
}

/// @brief
/// Crea una tarea periódica que se ejecutará en intervalos regulares.
/// @note
/// Este método permite crear una tarea que se ejecutará periódicamente en intervalos de tiempo especificados.
/// Es útil para tareas que necesitan ejecutarse repetidamente, como actualizaciones de estado o monitoreo.
/// @param interval
/// El intervalo en milisegundos entre ejecuciones de la tarea.
/// Este valor debe ser positivo y mayor que cero.
/// Si se especifica un valor menor o igual a cero, la tarea no se creará.
/// @param callback
/// La función que se desea ejecutar en cada intervalo.
/// Esta función debe tener la firma `void()` y no debe recibir parámetros.
/// @param startNow
/// Si es true, la primera ejecución de la tarea será inmediata.
/// Si es false, la tarea comenzará a ejecutarse después del primer intervalo.
/// Este parámetro es opcional y por defecto es false.
/// @return
/// El ID de la tarea creada, o -1 si no se pudo crear.
int UtilsClass::createInterval(unsigned long interval, TaskCallback callback, bool startNow)
{
  if (callback == nullptr)
  {
    debug("UtilsClass::createInterval| Error: Intento de crear tarea periódica con callback nulo");
    return -1;
  }

  // Buscar un slot libre
  int slotIndex = _findFreeTaskSlot();
  if (slotIndex < 0)
  {
    debug("UtilsClass::createInterval| Error: No se puede crear más tareas, límite alcanzado");
    return -1;
  }

  // Crear nueva tarea
  TimedTask &task = _tasks[slotIndex];
  task.interval = interval;
  task.lastExecuted = startNow ? 0 : millis(); // Si startNow, asegura ejecución inmediata
  task.callback = callback;
  task.recurring = true; // Tarea periódica
  task.active = true;
  task.id = _nextTaskId++;

  // Incrementar contador si es necesario
  if (slotIndex == _taskCount)
  {
    _taskCount++;
  }

  debug("UtilsClass::createInterval| Nueva tarea periódica creada con ID " + String(task.id) + " e intervalo de " + String(interval) + " ms");
  return task.id;
}

/// @brief
/// Detiene una tarea por su ID
/// @param taskId
/// ID de la tarea a detener
/// @details
/// Esta función busca una tarea activa por su ID y la marca como inactiva.
/// Si la tarea no se encuentra, se registra un error en el log.
/// @return
/// true si se detuvo correctamente, false si no se encontró la tarea.
bool UtilsClass::stopTask(int taskId)
{
  int index = _findTaskById(taskId);
  if (index < 0)
  {
    debug("UtilsClass::stopTask| Error: Intento de detener tarea inexistente con ID " + String(taskId));
    return false;
  }

  _tasks[index].active = false;
  debug("UtilsClass::stopTask| Tarea con ID " + String(taskId) + " detenida");
  return true;
}

/// @brief
/// Reinicia una tarea por su ID
/// @details
/// Esta función busca una tarea activa por su ID y reinicia su temporizador.
/// Si la tarea no se encuentra, se registra un error en el log.
/// Esto es útil para reprogramar tareas que necesitan ejecutarse nuevamente.
/// @param taskId
/// ID de la tarea a reiniciar
/// @return
/// true si se reinició correctamente, false si no se encontró la tarea.
bool UtilsClass::restartTask(int taskId)
{
  int index = _findTaskById(taskId);
  if (index < 0)
  {
    debug("UtilsClass::restartTask| Error: Intento de reiniciar tarea inexistente con ID " + String(taskId));
    return false;
  }

  _tasks[index].lastExecuted = millis();
  _tasks[index].active = true;
  debug("UtilsClass::restartTask| Tarea con ID " + String(taskId) + " reiniciada");
  return true;
}

/// @brief
/// Actualiza todas las tareas temporizadas.
/// Este método debe ser llamado en cada iteración del loop principal.
/// Se encarga de verificar si es tiempo de ejecutar las tareas y ejecutar sus callbacks.
/// @note
/// Asegúrate de que este método se llame frecuentemente para que las tareas se ejecuten a tiempo.
/// @details
/// Este método recorre todas las tareas temporizadas registradas y verifica si ha pasado el intervalo
/// especificado desde la última ejecución. Si es así, ejecuta el callback asociado y actualiza
/// el tiempo de la última ejecución. Si la tarea no es recurrente, se marca como inactiva después de su ejecución.
/// También compacta el array de tareas si hay muchas inactivas para optimizar el uso de memoria.
void UtilsClass::updateTasks()
{
  unsigned long currentTime = millis();

  // Verificar si el temporizador principal debe ejecutarse
  if (_mainTimerRunning)
  {
    if (currentTime - _mainTimerLastRun >= INTERVALO_TEMPORIZADOR)
    {
      _mainTimerLastRun = currentTime;

      // Ejecutar la función de actualización de temporizadores
      updateTimers();

      // También actualizamos los temporizadores de otros módulos
      Actuators.updateTimers();
    }
  }

  // Procesar todas las tareas temporizadas
  for (uint8_t i = 0; i < _taskCount; i++)
  {
    TimedTask &task = _tasks[i];

    // Verificar si la tarea está activa
    if (task.active && task.callback != nullptr)
    {
      // Verificar si es tiempo de ejecutar la tarea
      if (currentTime - task.lastExecuted >= task.interval)
      {
        // Ejecutar la función callback
        task.callback();

        // Actualizar tiempo de ejecución
        task.lastExecuted = currentTime;

        // Si no es recurrente, marcarla como inactiva
        if (!task.recurring)
        {
          task.active = false;
          debug("Tarea no recurrente con ID " + String(task.id) + " completada");
        }
      }
    }
  }

  // Compactar el array de tareas si hay tareas inactivas
  // Solo lo hacemos ocasionalmente para no consumir tiempo en cada iteración
  static unsigned long lastCompaction = 0;
  if (currentTime - lastCompaction >= 10000)
  { // Cada 10 segundos
    lastCompaction = currentTime;

    // Contar cuántas tareas inactivas hay
    int inactiveTasks = 0;
    for (uint8_t i = 0; i < _taskCount; i++)
    {
      if (!_tasks[i].active)
      {
        inactiveTasks++;
      }
    }

    // Si hay muchas tareas inactivas, compactar el array
    if (inactiveTasks > 5)
    {
      uint8_t newIndex = 0;

      // Mover todas las tareas activas al principio del array
      for (uint8_t i = 0; i < _taskCount; i++)
      {
        if (_tasks[i].active)
        {
          if (i != newIndex)
          {
            _tasks[newIndex] = _tasks[i];
          }
          newIndex++;
        }
      }

      // Actualizar el contador
      _taskCount = newIndex;
      debug("Array de tareas compactado. Tareas activas: " + String(_taskCount));
    }

    // También compactar los callbacks del temporizador principal
    if (_timerCallbackCount > 0)
    {
      int inactiveCallbacks = 0;
      for (uint8_t i = 0; i < _timerCallbackCount; i++)
      {
        if (!_timerCallbacks[i].active)
        {
          inactiveCallbacks++;
        }
      }

      if (inactiveCallbacks > 3)
      {
        uint8_t newIndex = 0;

        // Mover todos los callbacks activos al principio del array
        for (uint8_t i = 0; i < _timerCallbackCount; i++)
        {
          if (_timerCallbacks[i].active)
          {
            if (i != newIndex)
            {
              _timerCallbacks[newIndex] = _timerCallbacks[i];
            }
            newIndex++;
          }
        }

        // Actualizar el contador
        _timerCallbackCount = newIndex;
        debug("Array de callbacks compactado. Callbacks activos: " + String(_timerCallbackCount));
      }
    }
  }
}

void UtilsClass::updateTimers()
{
  // Esta función es llamada por el temporizador principal
  // y actualiza los contadores y estados del sistema

  // Actualizar los temporizadores de otros módulos
  static unsigned long lastGlobalUpdate = 0;
  unsigned long currentTime = millis();

  // Actualización global cada 1000 ms (1 segundo)
  if (currentTime - lastGlobalUpdate >= 1000)
  {
    lastGlobalUpdate = currentTime;

    // Aquí actualizamos los temporizadores del sistema
    // Ejecutamos todos los callbacks registrados
    for (uint8_t i = 0; i < _timerCallbackCount; i++)
    {
      if (_timerCallbacks[i].active && _timerCallbacks[i].callback != nullptr)
      {
        _timerCallbacks[i].callback();
      }
    }

    // Actualizar temporizador del programa (si está en ejecución)
    if (ProgramController.getState() == ESTADO_EJECUCION)
    {
      ProgramController.updateTimers();
    }

    // Registrar tiempo de actualización (comentado para reducir ruido en consola)
    // debug("Actualizando temporizadores globales - " + String(currentTime));
  }
}

void UtilsClass::formatTime(uint8_t minutes, uint8_t seconds, char *buffer, size_t bufferSize)
{
  snprintf(buffer, bufferSize, "%02d:%02d", minutes, seconds);
}

uint16_t UtilsClass::convertToSeconds(uint8_t minutes, uint8_t seconds)
{
  return (minutes * 60) + seconds;
}

void UtilsClass::convertFromSeconds(uint16_t totalSeconds, uint8_t &minutes, uint8_t &seconds)
{
  minutes = totalSeconds / 60;
  seconds = totalSeconds % 60;
}

void UtilsClass::debug(const String &message)
{
  // En producción, esta función podría ser configurada
  // para imprimir por Serial solo en modo debug
  Serial.println(message);
}

void UtilsClass::debugValue(const String &label, int value)
{
  Serial.print(label);
  Serial.print(": ");
  Serial.println(value);
}

bool UtilsClass::isInRange(int value, int target, int range)
{
  return (value >= (target - range)) && (value <= (target + range));
}

int UtilsClass::mapValue(int value, int inMin, int inMax, int outMin, int outMax)
{
  return map(value, inMin, inMax, outMin, outMax);
}

uint8_t UtilsClass::calculateProgress(uint16_t current, uint16_t total)
{
  if (total == 0)
    return 0;
  return (current * 100) / total;
}