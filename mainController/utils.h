// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "Arduino.h"
#include "config.h"

// Definimos un tipo para funciones callback
typedef void (*TaskCallback)();

// Estructura para representar una tarea temporizada
struct TimedTask
{
  unsigned long interval;     // Intervalo en milisegundos
  unsigned long lastExecuted; // Último tiempo de ejecución
  TaskCallback callback;      // Función a ejecutar
  bool recurring;             // Si es recurrente o de una sola vez
  bool active;                // Si la tarea está activa
  int id;                     // Identificador único de la tarea
};

/// @brief 
/// Estructura para representar una entrada de callback del temporizador
/// Esta estructura se utiliza para almacenar los callbacks registrados en el temporizador principal.
/// Cada entrada contiene un callback, un ID único, y un estado de actividad.
/// @details
/// Esta estructura es parte de la clase UtilsClass y se utiliza para gestionar los callbacks
/// que se ejecutan periódicamente en el temporizador principal del sistema.
/// Permite registrar, actualizar y eliminar callbacks de manera eficiente.
/// @note
/// Asegúrate de que el callback registrado tenga la firma `void()` y no reciba parámetros.
/// Los callbacks se ejecutarán en el contexto del temporizador principal, por lo que deben ser ligeros y no bloquear el hilo principal.
struct TimerCallbackEntry
{
  TaskCallback callback;
  int id;
  bool active;
};

class UtilsClass
{
public:
  // Inicialización
  void init();

  // 
  void startMainTimer();
  void stopMainTimer();
  bool isMainTimerRunning();

  int createTimeout(unsigned long duration, TaskCallback callback);

  int createInterval(unsigned long interval, TaskCallback callback, bool startNow = false);

  bool stopTask(int taskId);

  bool restartTask(int taskId);

  void updateTasks();

  /// @brief
  /// Registra un callback para el temporizador principal.
  /// Este método permite registrar una función que se ejecutará periódicamente
  /// en el temporizador principal del sistema. Es útil para tareas que necesitan
  /// ejecutarse en intervalos regulares sin necesidad de crear una tarea específica.
  /// @details
  /// Este método almacena el callback en un array de entradas de callbacks del temporizador.
  /// @param callback
  /// La función que se desea registrar como callback.
  /// Esta función debe tener la firma `void()` y no debe recibir parámetros.
  /// Se ejecutará cada vez que el temporizador principal se actualice.
  /// @warning
  /// Asegúrate de que el callback no consuma demasiado tiempo de ejecución,
  /// ya que esto podría afectar la capacidad de respuesta del sistema.
  /// Si el callback es demasiado pesado, considera dividir la tarea en partes más pequeñas
  /// o utilizar tareas temporizadas específicas para manejar la carga.
  /// @note
  /// Asegúrate de que el callback no consuma demasiado tiempo de ejecución,
  /// ya que esto podría afectar la capacidad de respuesta del sistema.
  /// Si el callback es demasiado pesado, considera dividir la tarea en partes más pequeñas
  /// o utilizar tareas temporizadas específicas para manejar la carga.
  /// @return
  /// El ID del callback registrado, o -1 si no se pudo registrar.
  /// Este ID puede ser utilizado posteriormente para eliminar el callback si es necesario.
  int registerTimerCallback(TaskCallback callback);

  /**
   * Elimina un callback previamente registrado
   * @param callbackId ID del callback a eliminar
   * @return true si se eliminó correctamente, false si no se encontró
   */
  bool unregisterTimerCallback(int callbackId);

  // Manejo y conversión de tiempo
  void updateTimers();
  void formatTime(uint8_t minutes, uint8_t seconds, char *buffer, size_t bufferSize);
  uint16_t convertToSeconds(uint8_t minutes, uint8_t seconds);
  void convertFromSeconds(uint16_t totalSeconds, uint8_t &minutes, uint8_t &seconds);

  // Utilidades para depuración
  void debug(const String &message);
  void debugValue(const String &label, int value);

  // Funciones auxiliares generales
  bool isInRange(int value, int target, int range);
  int mapValue(int value, int inMin, int inMax, int outMin, int outMax);
  uint8_t calculateProgress(uint16_t current, uint16_t total);

private:
  // Variables para control de temporizadores
  bool _mainTimerRunning;
  unsigned long _mainTimerLastRun;

  // Array de tareas temporizadas
  TimedTask _tasks[MAX_ASYNC_TASKS];
  uint8_t _taskCount;
  int _nextTaskId;

  // Array para temporizadores callbacks
  TimerCallbackEntry _timerCallbacks[MAX_ASYNC_TASKS];
  uint8_t _timerCallbackCount;
  int _nextTimerCallbackId;

  // Métodos internos
  void _setupMainTimer();
  int _findTaskById(int taskId);
  int _findFreeTaskSlot();
};

// Instancia global
extern UtilsClass Utils;

#endif // UTILS_H