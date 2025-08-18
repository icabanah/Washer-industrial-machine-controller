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
// === SISTEMA UNIFICADO DE TAREAS ===
// Eliminando TimerCallbackEntry - usando solo AsyncTask para simplificar

class UtilsClass
{
public:
  // Inicialización
  void init();

  int createTimeout(unsigned long duration, TaskCallback callback);

  int createInterval(unsigned long interval, TaskCallback callback, bool startNow = false);

  bool stopTask(int taskId);

  bool restartTask(int taskId);

  void updateTasks();

  // === MÉTODOS DE TIMER CALLBACK ELIMINADOS ===
  // Sistema unificado - usar createInterval() en su lugar

  // Manejo y conversión de tiempo
  void formatTime(uint8_t minutes, uint8_t seconds, char *buffer, size_t bufferSize);
  uint16_t convertToSeconds(uint8_t minutes, uint8_t seconds);
  void convertFromSeconds(uint16_t totalSeconds, uint8_t &minutes, uint8_t &seconds);

  // === UTILIDADES DEBUG ELIMINADAS ===
  // Usar Debug.print() del sistema centralizado

  // === FUNCIONES MATEMÁTICAS MOVIDAS A MathUtils ===
  // Usar MathUtils::isInRange(), MathUtils::mapValue(), MathUtils::calculateProgress()

private:
  // === MAIN TIMER VARIABLES ELIMINADAS ===
  // Sistema simplificado sin timer principal centralizado

  // Array de tareas temporizadas
  TimedTask _tasks[MAX_ASYNC_TASKS];
  uint8_t _taskCount;
  int _nextTaskId;

  // === TIMER CALLBACKS ELIMINADOS ===
  // Sistema simplificado - solo AsyncTask

  // Métodos internos simplificados
  int _findTaskById(int taskId);
  int _findFreeTaskSlot();
};

// Instancia global
extern UtilsClass Utils;

#endif // UTILS_H