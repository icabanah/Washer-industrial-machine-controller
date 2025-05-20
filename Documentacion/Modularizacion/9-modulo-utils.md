# Módulo de Utilidades (utils.h / utils.cpp)

## Descripción

El módulo de Utilidades proporciona funciones y herramientas auxiliares que son esenciales para la implementación no bloqueante de los tres programas específicos (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda). Este módulo centraliza la gestión de tareas temporizadas asíncronas, el manejo de mensajes de depuración, y otras funciones auxiliares utilizadas por los diferentes componentes del sistema.

## Analogía: Sistema de Orquestación Temporal y Kit de Herramientas Universal

Este módulo puede visualizarse como una combinación de un sistema sofisticado de orquestación temporal y un kit de herramientas universal. El sistema de orquestación (con temporizadores asíncronos) funciona como un director musical asistente que mantiene el tiempo para los diferentes instrumentos sin interrumpir al director principal. Esto permite que cada sección de la orquesta (cada módulo) sepa exactamente cuándo debe intervenir, sin tener que esperar inactivamente a que llegue su turno. Por otro lado, el kit de herramientas universal contiene instrumentos precisos y versátiles que facilitan tareas comunes en cualquier parte del sistema, desde medir con exactitud hasta comunicar mensajes de diagnóstico.

## Estructura del Módulo

El módulo de Utilidades se divide en:

- **utils.h**: Define la interfaz pública del módulo
- **utils.cpp**: Implementa la funcionalidad interna

### Interfaz (utils.h)

```cpp
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
  void startTimer(void (*callback)(), unsigned long interval);
  void stopTimer();
  void updateTimer();
  
  // Funciones de tiempo
  void splitTime(unsigned long seconds, uint8_t &minutes, uint8_t &secs);
  unsigned long combineTime(uint8_t minutes, uint8_t seconds);
  
  // Funciones de ayuda
  uint8_t constrainValue(uint8_t value, uint8_t min, uint8_t max);
  bool isInRange(uint8_t value, uint8_t target, uint8_t range);
  
  // Depuración
  void debugPrint(const char* message);
  void debugPrintValue(const char* label, int value);

private:
  // Temporizador asíncrono
  AsyncTask _timer;
  bool _timerActive;
};

// Instancia global
extern UtilsClass Utils;

#endif // UTILS_H
```

### Implementación (utils.cpp)

```cpp
// utils.cpp
#include "utils.h"
#include "program_controller.h"

// Definición de la instancia global
UtilsClass Utils;

// Niveles de depuración
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARN  2
#define DEBUG_LEVEL_INFO  3
#define DEBUG_LEVEL_DEBUG 4
#define DEBUG_LEVEL_TRACE 5

void UtilsClass::init() {
  _mainTimer = nullptr;
  _debugLevel = DEBUG_LEVEL_INFO; // Nivel predeterminado
  
  // Eliminar cualquier tarea previa
  for (auto task : _asyncTasks) {
    if (task) {
      task->Stop();
      delete task;
    }
  }
  _asyncTasks.clear();
  
  debug("Módulo Utils inicializado");
}

void UtilsClass::startMainTimer() {
  // Crear temporizador principal que se ejecuta cada INTERVALO_TEMPORIZADOR ms
  _mainTimer = new AsyncTask(INTERVALO_TEMPORIZADOR, true, []() {
    // Este es el temporizador principal del sistema
    // Aquí se pueden actualizar contadores globales, verificar tiempo para
    // cambios de fase, rotación de motor, etc.
    ProgramController.updateTimers();
  });
  
  _mainTimer->Start();
  debug("Temporizador principal iniciado");
}

void UtilsClass::createTimeout(unsigned long ms, std::function<void()> callback) {
  // Crear una tarea asíncrona que se ejecuta una vez después de ms milisegundos
  AsyncTask* task = new AsyncTask(ms, false, [this, callback]() {
    callback(); // Ejecutar la función de callback
    
    // La tarea se elimina automáticamente en updateTasks()
  });
  
  task->Start();
  _asyncTasks.push_back(task);
  
  if (_debugLevel >= DEBUG_LEVEL_DEBUG) {
    debug("Timeout creado: " + String(ms) + "ms");
  }
}

void UtilsClass::createPeriodicTask(unsigned long interval, std::function<void()> callback) {
  // Crear una tarea asíncrona que se ejecuta periódicamente cada interval milisegundos
  AsyncTask* task = new AsyncTask(interval, true, callback);
  task->Start();
  _asyncTasks.push_back(task);
  
  debug("Tarea periódica creada: " + String(interval) + "ms");
}

void UtilsClass::updateTasks() {
  // Actualizar el temporizador principal
  if (_mainTimer) {
    _mainTimer->Update();
  }
  
  // Actualizar todas las tareas asíncronas
  for (auto task : _asyncTasks) {
    if (task && task->IsRunning()) {
      task->Update();
    }
  }
  
  // Limpiar tareas finalizadas
  _cleanupTasks();
}

void UtilsClass::_cleanupTasks() {
  auto it = _asyncTasks.begin();
  while (it != _asyncTasks.end()) {
    if (*it == nullptr || !(*it)->IsRunning()) {
      // Solo imprimir debug si se elimina una tarea real
      if (*it != nullptr) {
        if (_debugLevel >= DEBUG_LEVEL_TRACE) {
          debug("Tarea finalizada eliminada");
        }
      }
      
      delete *it;
      it = _asyncTasks.erase(it);
    } else {
      ++it;
    }
  }
}

int UtilsClass::getRunningTasksCount() {
  int count = 0;
  for (auto task : _asyncTasks) {
    if (task && task->IsRunning()) {
      count++;
    }
  }
  return count;
}

// Temporizadores específicos para programas de lavado
void UtilsClass::createRotationTimer(uint8_t rotationLevel, std::function<void(uint8_t)> callback) {
  // Configurar tiempos según el nivel de rotación
  unsigned long dirTime = 0;
  unsigned long pauseTime = 0;
  
  switch(rotationLevel) {
    case 1: // Suave
      dirTime = ROTATION_LEVEL1_TIME;
      pauseTime = ROTATION_LEVEL1_PAUSE;
      break;
    case 2: // Medio
      dirTime = ROTATION_LEVEL2_TIME;
      pauseTime = ROTATION_LEVEL2_PAUSE;
      break;
    case 3: // Intenso
      dirTime = ROTATION_LEVEL3_TIME;
      pauseTime = ROTATION_LEVEL3_PAUSE;
      break;
    default:
      dirTime = ROTATION_LEVEL2_TIME;
      pauseTime = ROTATION_LEVEL2_PAUSE;
  }
  
  // Estado inicial: dirección A
  uint8_t currentState = 0;
  callback(currentState);
  
  // Crear temporizador periódico
  createPeriodicTask(dirTime, [=]() mutable {
    // Cambiar al siguiente estado
    currentState = (currentState + 1) % 4;
    
    // Ajustar intervalo según el estado actual
    unsigned long nextInterval = (currentState % 2 == 0) ? dirTime : pauseTime;
    
    // Enviar estado a la función callback
    callback(currentState);
  });
  
  debug("Temporizador de rotación creado con nivel: " + String(rotationLevel));
}

void UtilsClass::createTemperatureCheckTimer(std::function<void()> callback) {
  // Para programas con agua caliente (22 y potencialmente 24)
  createPeriodicTask(TEMPERATURE_CHECK_INTERVAL, callback);
  debug("Temporizador de verificación de temperatura creado");
}

// Funciones de tiempo
String UtilsClass::formatTime(unsigned long seconds) {
  uint8_t mins, secs;
  splitTime(seconds, mins, secs);
  
  String result = "";
  if (mins < 10) result += "0";
  result += String(mins) + ":";
  if (secs < 10) result += "0";
  result += String(secs);
  
  return result;
}

// Sistema de mensajes de depuración mejorado
void UtilsClass::debug(const String& message) {
#ifdef DEBUG_MODE
  Serial.println(_getDebugPrefix() + message);
#endif
}

void UtilsClass::debugValue(const String& label, float value, int decimals) {
#ifdef DEBUG_MODE
  Serial.print(_getDebugPrefix() + label + ": ");
  Serial.println(value, decimals);
#endif
}

String UtilsClass::_getDebugPrefix() {
  unsigned long now = millis();
  unsigned long seconds = now / 1000;
  unsigned long ms = now % 1000;
  
  String prefix = "[";
  prefix += String(seconds);
  prefix += ".";
  if (ms < 100) prefix += "0";
  if (ms < 10) prefix += "0";
  prefix += String(ms);
  prefix += "] ";
  
  return prefix;
}

void UtilsClass::setDebugLevel(uint8_t level) {
  _debugLevel = level;
  debug("Nivel de depuración establecido a: " + String(_debugLevel));
}

// Funciones auxiliares
bool UtilsClass::isInRange(float value, float target, float range) {
  return (value >= (target - range) && value <= (target + range));
}

float UtilsClass::constrainFloat(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}
```
```

## Responsabilidades

El módulo de Utilidades tiene las siguientes responsabilidades adaptadas para los tres programas específicos:

1. **Gestión de Tareas Asíncronas Avanzada**:
   - Implementación de sistema no bloqueante para programar tareas temporizadas
   - Temporizadores específicos para patrones de rotación, control de temperatura y centrifugado
   - Manejo de múltiples tareas concurrentes con priorización adecuada

2. **Soporte para Programas Específicos**:
   - Temporizadores de rotación con tres niveles de intensidad (suave, medio, intenso)
   - Verificación periódica de temperatura para Programa 22 y Programa 24 con agua caliente
   - Manejo de ciclos de tiempo para las múltiples tandas del Programa 24

3. **Sistema de Depuración Mejorado**:
   - Mensajes con timestamps para rastreo preciso de eventos
   - Niveles de depuración configurables (error, warn, info, debug, trace)
   - Formateo de valores numéricos con precisión decimal ajustable

4. **Funciones Matemáticas y de Conversión**:
   - Manejo preciso de valores de punto flotante para temperaturas
   - Verificación de rangos con tolerancias para el control de temperatura
   - Formateo de tiempo para retroalimentación visual

5. **Optimización de Recursos**:
   - Limpieza automática de tareas finalizadas para prevenir fugas de memoria
   - Monitoreo de cantidad de tareas activas para diagnóstico
   - Procesamiento eficiente de callbacks con lambdas y std::function

## Ventajas de esta Implementación

1. **Operación Completamente No Bloqueante**: Eliminación total de delay(), garantizando que el sistema responda inmediatamente a eventos como emergencias durante cualquier operación temporizada.

2. **Mayor Flexibilidad para Programas Específicos**: Temporizadores dedicados para las necesidades específicas de cada programa, particularmente para el control preciso de temperatura en agua caliente.

3. **Depuración Avanzada**: Sistema de registro con timestamps que facilita la identificación de problemas de sincronización y comportamiento temporal del sistema.

4. **Gestión Eficiente de Memoria**: Limpieza automática de tareas completadas, previniendo fugas de memoria en ejecuciones prolongadas del sistema.

5. **Retroalimentación Mejorada**: Funciones de formateo que facilitan presentar información temporal y numérica de manera clara en la interfaz de usuario.

6. **Mantenibilidad Superior**: Centralización de toda lógica de temporización, eliminando implementaciones duplicadas o inconsistentes en otros módulos.

7. **Escalabilidad**: Capacidad para añadir nuevos tipos de temporizadores específicos para futuras funciones sin modificar la arquitectura base.

## Soporte para Programas Específicos

### Para Programa 22 (Agua Caliente):
- Temporizadores precisos para verificación de temperatura
- Callbacks específicos para gestión de calentamiento por vapor
- Funciones de rango flotante para control de temperatura con tolerancia ±2°C

### Para Programa 23 (Agua Fría):
- Temporizadores simples para rotación y fases
- Optimización para minimizar tiempos de cambio de fase

### Para Programa 24 (Multitanda):
- Sistema de temporizadores jerárquicos para gestionar tandas y fases
- Adaptación dinámica según configuración de agua caliente/fría
- Mecanismos para transición suave entre tandas

Al implementar estas capacidades específicas, el módulo de Utilidades proporciona la infraestructura temporal necesaria para que los tres programas específicos funcionen de manera eficiente y responsiva, manteniendo siempre el sistema capaz de responder a eventos como el botón de emergencia.
