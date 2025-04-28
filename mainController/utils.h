// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "config.h"

// Definimos un tipo para funciones callback
typedef void (*TaskCallback)();

// Estructura para representar una tarea temporizada
struct TimedTask {
  unsigned long interval;      // Intervalo en milisegundos
  unsigned long lastExecuted;  // Último tiempo de ejecución
  TaskCallback callback;       // Función a ejecutar
  bool recurring;              // Si es recurrente o de una sola vez
  bool active;                 // Si la tarea está activa
  int id;                      // Identificador único de la tarea
};

class UtilsClass {
public:
  // Inicialización
  void init();
  
  // Gestión de temporizadores
  void startMainTimer();
  void stopMainTimer();
  bool isMainTimerRunning();
  
  /**
   * Crea un temporizador de un solo uso (fire-and-forget)
   * @param duration Duración en milisegundos
   * @param callback Función a ejecutar cuando expire el temporizador
   * @return ID de la tarea creada o -1 si falló
   */
  int createTimeout(unsigned long duration, TaskCallback callback);
  
  /**
   * Crea una tarea periódica que se ejecutará cada cierto intervalo
   * @param interval Intervalo en milisegundos entre ejecuciones
   * @param callback Función a ejecutar
   * @param startNow Si es true, la primera ejecución es inmediata
   * @return ID de la tarea creada o -1 si falló
   */
  int createInterval(unsigned long interval, TaskCallback callback, bool startNow = false);
  
  /**
   * Detiene una tarea por su ID
   * @param taskId ID de la tarea a detener
   * @return true si se detuvo correctamente, false si no se encontró
   */
  bool stopTask(int taskId);
  
  /**
   * Reinicia una tarea por su ID
   * @param taskId ID de la tarea a reiniciar
   * @return true si se reinició correctamente, false si no se encontró
   */
  bool restartTask(int taskId);
  
  /**
   * Actualiza todas las tareas temporizadas
   * Esta función debe ser llamada en cada iteración del loop principal
   */
  void updateTasks();
  
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
  unsigned long _mainTimerLastRun;
  
  // Array de tareas temporizadas
  TimedTask _tasks[MAX_ASYNC_TASKS];
  uint8_t _taskCount;
  int _nextTaskId;
  
  // Métodos internos
  void _setupMainTimer();
  int _findTaskById(int taskId);
  int _findFreeTaskSlot();
};

// Instancia global
extern UtilsClass Utils;

#endif // UTILS_H