# Módulo de Utilidades (utils.h / utils.cpp)

## Descripción

El módulo de Utilidades proporciona funciones y herramientas auxiliares que pueden ser utilizadas por cualquier parte del sistema. Contiene funcionalidades genéricas que no pertenecen a un dominio específico, pero que son útiles para diferentes módulos.

## Analogía: Caja de Herramientas y Sistema de Programación

Este módulo es como una combinación de una caja de herramientas y un sistema de programación de tareas en un taller. La caja de herramientas contiene instrumentos genéricos útiles para diferentes trabajos (funciones de validación, conversión de tiempo, etc.), mientras que el sistema de programación coordina cuándo se deben realizar ciertas tareas sin bloquear el trabajo principal. Este sistema permite que múltiples trabajos progresen simultáneamente, similar a cómo un equipo de trabajo puede realizar varias tareas en paralelo, cada una según su propio cronograma, sin tener que esperar a que una tarea se complete antes de comenzar otra.

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
#include <vector>

// Definición de la instancia global
UtilsClass Utils;

void UtilsClass::init() {
  _mainTimer = nullptr;
  
  // Eliminar cualquier tarea previa
  for (auto task : _asyncTasks) {
    if (task) {
      task->Stop();
      delete task;
    }
  }
  _asyncTasks.clear();
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
}

void UtilsClass::createTimeout(unsigned long ms, std::function<void()> callback) {
  // Crear una tarea asíncrona que se ejecuta una vez después de ms milisegundos
  AsyncTask* task = new AsyncTask(ms, false, [this, callback]() {
    callback(); // Ejecutar la función de callback
    
    // Buscar la tarea actual en el vector y eliminarla
    for (size_t i = 0; i < _asyncTasks.size(); i++) {
      if (_asyncTasks[i] && !_asyncTasks[i]->IsRunning()) {
        delete _asyncTasks[i];
        _asyncTasks[i] = nullptr;
        break;
      }
    }
  });
  
  task->Start();
  _asyncTasks.push_back(task);
}

void UtilsClass::createPeriodicTask(unsigned long interval, std::function<void()> callback) {
  // Crear una tarea asíncrona que se ejecuta periódicamente cada interval milisegundos
  AsyncTask* task = new AsyncTask(interval, true, callback);
  task->Start();
  _asyncTasks.push_back(task);
}

void UtilsClass::updateTasks() {
  // Actualizar todas las tareas asíncronas
  if (_mainTimer) {
    _mainTimer->Update();
  }
  
  for (auto task : _asyncTasks) {
    if (task && task->IsRunning()) {
      task->Update();
    }
  }
  
  // Limpiar tareas finalizadas
  auto it = _asyncTasks.begin();
  while (it != _asyncTasks.end()) {
    if (*it == nullptr || !(*it)->IsRunning()) {
      delete *it;
      it = _asyncTasks.erase(it);
    } else {
      ++it;
    }
  }
}

void UtilsClass::splitTime(unsigned long seconds, uint8_t &minutes, uint8_t &secs) {
  minutes = seconds / 60;
  secs = seconds % 60;
}

unsigned long UtilsClass::combineTime(uint8_t minutes, uint8_t seconds) {
  return (minutes * 60) + seconds;
}

uint8_t UtilsClass::constrainValue(uint8_t value, uint8_t min, uint8_t max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

bool UtilsClass::isInRange(uint8_t value, uint8_t target, uint8_t range) {
  return (value >= (target - range) && value <= (target + range));
}

void UtilsClass::debugPrint(const char* message) {
#ifdef DEBUG_MODE
  Serial.println(message);
#endif
}

void UtilsClass::debugPrintValue(const char* label, int value) {
#ifdef DEBUG_MODE
  Serial.print(label);
  Serial.print(": ");
  Serial.println(value);
#endif
}
```

## Responsabilidades

El módulo de Utilidades tiene las siguientes responsabilidades:

1. **Gestión de Tareas Asíncronas**: Proporcionar un sistema no bloqueante para programar tareas temporizadas y periódicas, eliminando la necesidad de usar delay().
2. **Gestión de Tiempo**: Proporcionar herramientas para manejar y convertir unidades de tiempo.
3. **Validación**: Proporcionar funciones para validar y restringir valores dentro de rangos específicos.
4. **Depuración**: Facilitar la impresión de mensajes de depuración de manera controlada.
5. **Operaciones Comunes**: Implementar operaciones de uso general que pueden ser necesarias en diferentes partes del sistema.

## Ventajas de este Enfoque

1. **Reutilización**: Evita la duplicación de código al centralizar funciones comunes.
2. **Consistencia**: Garantiza que las operaciones comunes se realicen de la misma manera en todo el sistema.
3. **Mantenibilidad**: Facilita la corrección y mejora de funcionalidades genéricas en un solo lugar.
4. **Modularidad**: Permite a otros módulos centrarse en su lógica específica, delegando operaciones auxiliares.
5. **Depuración**: Proporciona mecanismos centralizados para la depuración y el registro de actividades.
6. **Flexibilidad**: Las funciones de utilidad se pueden adaptar fácilmente a diferentes contextos y necesidades.
7. **No Bloqueante**: Elimina la necesidad de usar delay() que bloquea la ejecución, permitiendo que el sistema responda a eventos mientras se ejecutan tareas temporizadas.

Al separar estas funciones genéricas en un módulo dedicado, se promueve la reutilización de código y se reduce la redundancia, lo que a su vez mejora la mantenibilidad y la consistencia del sistema como un todo. El módulo de Utilidades actúa como un conjunto de bloques de construcción básicos que pueden combinarse de diferentes maneras para resolver problemas específicos en cualquier parte del sistema.
