# Módulo de Controlador de Programas (program_controller.h / program_controller.cpp)

## Descripción

El módulo Controlador de Programas gestiona la lógica central del sistema, implementando una máquina de estados completa que controla la selección, configuración y ejecución de los tres programas específicos de lavado (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda). Este módulo coordina las interacciones entre sensores, actuadores e interfaz de usuario, y maneja situaciones de emergencia. Desarrollado para el contexto específico de una lavadora industrial en Arequipa, Perú, utiliza el ESP32-WROOM como controlador principal.

## Analogía: Director de Orquesta y Compositor

Este módulo funciona como un director de orquesta que además es compositor. Como director, coordina a todos los músicos (módulos) para interpretar correctamente la partitura. Como compositor, define la estructura de la obra musical (máquina de estados) y crea las diferentes composiciones (programas 22, 23 y 24), cada una con su propio estilo y complejidad. Determina cuándo y cómo se ejecuta cada movimiento (fase), en qué momento entra cada sección de instrumentos (actuadores), y cómo responder ante interrupciones o imprevistos (emergencias).

## Estructura del Módulo

El módulo de Controlador de Programas se divide en:

- **program_controller.h**: Define la interfaz pública del módulo
- **program_controller.cpp**: Implementa la funcionalidad interna

### Máquina de Estados

El controlador implementa una máquina de estados completa con siete estados principales:

```cpp
// Definición de estados del sistema
#define ESTADO_INICIO      0  // Inicialización del sistema
#define ESTADO_SELECCION   1  // Selección de programa
#define ESTADO_ESPERA      2  // Esperando confirmación de inicio
#define ESTADO_EJECUCION   3  // Programa en ejecución
#define ESTADO_PAUSA       4  // Programa en pausa
#define ESTADO_FINALIZACION 5  // Completado y finalización
#define ESTADO_ERROR       6  // Error detectado
#define ESTADO_EMERGENCIA  7  // Emergencia activada
```

### Programas Específicos

El controlador está diseñado para gestionar tres programas específicos:

```cpp
// Configuración de programas
#define PROGRAMA_22  0  // Agua Caliente
#define PROGRAMA_23  1  // Agua Fría
#define PROGRAMA_24  2  // Multitanda (4 tandas)
```

### Interfaz (program_controller.h)

```cpp
// program_controller.h
#ifndef PROGRAM_CONTROLLER_H
#define PROGRAM_CONTROLLER_H

#include "config.h"
#include "sensors.h"
#include "actuators.h"

class ProgramControllerClass {
public:
  // Inicialización
  void init();
  
  // Actualización general (llamada desde loop)
  void update();
  
  // Gestión de máquina de estados
  void setState(uint8_t newState);
  uint8_t getCurrentState();
  
  // Control de programas
  void selectProgram(uint8_t programId);
  void startProgram();
  void pauseProgram();
  void resumeProgram();
  void stopProgram();
  void completeProgram();
  
  // Modo edición
  void enterEditMode();
  void exitEditMode();
  void updateParameter(const String& paramName, int value);
  
  // Control de fase
  void moveToNextPhase();
  void setupPhase(uint8_t phase);
  
  // Acceso a parámetros
  uint8_t getCurrentProgram();
  uint8_t getCurrentPhase();
  uint8_t getCurrentCycle(); // Solo para Programa 24
  
  // Gestión de temperatura
  void manageTemperature();
  
  // Control de tiempo
  void updateTimers();
  
  // Manejo de acciones de usuario desde la pantalla
  void handleUserAction(const String& action);
  
  // Manejo de emergencia
  void handleEmergency();
  void resetEmergency();

private:
  // Variables de estado
  uint8_t _currentState;
  uint8_t _previousState;
  uint8_t _currentProgram;
  uint8_t _currentPhase;
  uint8_t _currentCycle;     // Para Programa 24
  uint8_t _numberOfCycles;   // Para Programa 24
  bool _programFinished;
  bool _programPaused;
  bool _emergencyActive;
  
  // Parámetros de ejecución
  bool _useHotWater;
  bool _enableTemperatureControl;
  bool _centrifugeEnabled;
  uint8_t _centrifugeLevel;
  bool _temperatureAdjustmentInProgress;
  
  // Temporizadores
  int16_t _phaseTimer;
  int16_t _motorTimer;
  int16_t _cycleTimer;       // Para Programa 24
  
  // Valores objetivo
  float _targetTemperature;
  int _targetWaterLevel;
  int _rotationLevel;
  
  // Métodos internos
  void _initializeProgram();
  void _startCurrentPhase();
  void _executePhase();
  void _executeFilling();
  void _executeWashing();
  void _executeDraining();
  void _executeCentrifuge();
  void _monitorAndControlTemperature();
  void _manageLowTemperature();
  void _checkPhaseCompletion();
  void _manageCycles();      // Para Programa 24
  void _shutdownAllSystems();
};

// Instancia global
extern ProgramControllerClass ProgramController;

#endif // PROGRAM_CONTROLLER_H
```

### Implementación (program_controller.cpp)

```cpp
// program_controller.cpp
#include "program_controller.h"
#include "ui_controller.h"
#include "storage.h"
#include "utils.h"

// Definición de la instancia global
ProgramControllerClass ProgramController;

void ProgramControllerClass::init() {
  _currentState = ESTADO_SELECCION;
  _currentProgram = 1;
  _currentPhase = 1;
  _currentVariable = 1;
  _editLevel = 0;
  _programFinished = true;
  _programPaused = false;
  _emergencyActive = false;
  _editingProgram = false;
  _blockCounter = 0;
  _timerSeconds = 0;
  _motorSeconds = 0;
  _betweenPhaseSeconds = 0;
  _motorDirection = 1;
  _motorPause = false;
  
  // Los valores de los programas deberían ser cargados de EEPROM
  // en Storage.loadFromEEPROM()
}

void ProgramControllerClass::update() {
  // Si estamos en estado de emergencia, no hacer nada más
  if (_emergencyActive) {
    return;
  }
  
  // Esta función se llama continuamente desde loop
  
  // Si estamos en estado de ejecución, los temporizadores
  // se actualizan mediante callbacks del módulo Utils
  // No se utiliza polling aquí para evitar bloqueos
}

void ProgramControllerClass::selectProgram(uint8_t programa) {
  if (programa >= 1 && programa <= 3) {
    _currentProgram = programa;
    UIController.showSelectionScreen(programa);
  }
}

void ProgramControllerClass::startProgram() {
  if (_currentState == ESTADO_SELECCION || _currentState == ESTADO_PAUSA) {
    _currentState = ESTADO_EJECUCION;
    _programFinished = false;
    _programPaused = false;
    _currentPhase = 1;
    
    // Activar elementos según programa
    Actuators.lockDoor(true);
    Actuators.openWaterValve(true);
    Actuators.openDrainValve(false);  // Cerrado para retener agua
    
    // Reiniciar temporizadores
    _initializeTimers();
    
    // Iniciar actuadores y sensores según programa
    if (_currentProgram == 1) {
      // Programa 1: Motor + Temperatura
      Actuators.startMotor();
      Sensors.startTemperatureMonitoring(_temperaturaLim[_currentProgram - 1][_currentPhase - 1]);
    } else if (_currentProgram == 2) {
      // Programa 2: Temperatura
      Sensors.startTemperatureMonitoring(_temperaturaLim[_currentProgram - 1][_currentPhase - 1]);
    } else {
      // Programa 3: Motor + Temperatura + Nivel + Centrifugado
      Actuators.startMotor();
      Sensors.startTemperatureMonitoring(_temperaturaLim[_currentProgram - 1][_currentPhase - 1]);
      if (_currentPhase == 4) {
        // En la última fase activar centrifugado
        Actuators.activateCentrifuge(true);
      }
    }
    
    // Iniciar monitoreo de nivel de agua
    Sensors.startPressureMonitoring(_nivelAgua[_currentProgram - 1][_currentPhase - 1]);
    
    // Mostrar pantalla de ejecución
    UIController.showExecutionScreen(
      _currentProgram, 
      _currentPhase, 
      _nivelAgua[_currentProgram - 1][_currentPhase - 1],
      Sensors.getCurrentTemperature(),
      _rotacionTam[_currentProgram - 1][_currentPhase - 1]
    );
    
    // Usar el sistema no bloqueante de Utils para el temporizador
    _timerSeconds = _temporizadorLim[_currentProgram - 1][_currentPhase - 1] * 60;
    
    // Crear un temporizador periódico para actualizar cada segundo
    Utils.createPeriodicTask(1000, [this]() {
      this->updateTimers();
    });
  }
}

void ProgramControllerClass::pauseProgram() {
  if (_currentState == ESTADO_EJECUCION && !_programPaused) {
    _programPaused = true;
    _betweenPhaseSeconds = _tiempoEntreFase[_currentProgram - 1][_currentPhase - 1];
    
    // Detener actuadores
    Actuators.stopMotor();
    Actuators.openWaterValve(false);
    Actuators.activateSteam(false);
    Actuators.activateCentrifuge(false);
    Actuators.openDrainValve(true);  // Abrir para desagüe
    
    // Detener sensores
    Sensors.stopPressureMonitoring();
    Sensors.stopTemperatureMonitoring();
    
    // Notificar al usuario
    UIController.showMessage("Programa pausado", 2000);
  }
}

void ProgramControllerClass::resumeProgram() {
  if (_currentState == ESTADO_EJECUCION && _programPaused) {
    _programPaused = false;
    _currentPhase++;
    
    if (_currentPhase > 4) {
      stopProgram();
    } else {
      // Reiniciar para nueva fase
      _motorDirection = 1;
      _motorSeconds = 0;
      Actuators.startMotor();
      
      // Configurar temporizador para nueva fase
      _timerSeconds = _temporizadorLim[_currentProgram - 1][_currentPhase - 1] * 60;
      
      // Activar centrifugado si corresponde (fase 4 del programa 3)
      if (_currentProgram == 3 && _currentPhase == 4) {
        Actuators.activateCentrifuge(true);
      }
      
      // Reactivar válvulas y sensores
      Actuators.openWaterValve(true);
      Sensors.startTemperatureMonitoring(_temperaturaLim[_currentProgram - 1][_currentPhase - 1]);
      Sensors.startPressureMonitoring(_nivelAgua[_currentProgram - 1][_currentPhase - 1]);
      
      // Actualizar UI
      UIController.updatePhase(_currentPhase);
      UIController.showMessage("Fase " + String(_currentPhase) + " iniciada", 2000);
    }
  }
}

void ProgramControllerClass::stopProgram() {
  if (_currentState == ESTADO_EJECUCION) {
    _currentState = ESTADO_SELECCION;
    _programFinished = true;
    _programPaused = false;
    _currentPhase = 1;
    _blockCounter++;
    
    // Detener todo
    Actuators.stopMotor();
    Actuators.lockDoor(false);
    Actuators.openWaterValve(false);
    Actuators.activateSteam(false);
    Actuators.activateCentrifuge(false);
    Actuators.openDrainValve(false);
    
    Sensors.stopTemperatureMonitoring();
    Sensors.stopPressureMonitoring();
    
    // No usar delay para el buzzer, usar Utils
    // Esto es un ejemplo ilustrativo - normalmente usaríamos otras formas de notificación
    // ya que el modelo actualizado no parece tener buzzer
    Utils.createTimeout(TIEMPO_BUZZER, []() {
      // Finalización del sonido o alguna otra acción
    });
    
    // Volver a pantalla de selección
    UIController.showSelectionScreen(_currentProgram);
    UIController.showMessage("Programa completado", 3000);
  }
}

void ProgramControllerClass::handleEmergency() {
  if (!_emergencyActive) {
    _emergencyActive = true;
    _currentState = ESTADO_EMERGENCIA;
    
    // Detener todos los sistemas
    _shutdownAllSystems();
    
    // Mostrar pantalla de emergencia
    UIController.showEmergencyScreen();
  }
}

void ProgramControllerClass::resetEmergency() {
  if (_emergencyActive) {
    _emergencyActive = false;
    
    // Volver a pantalla de selección
    _currentState = ESTADO_SELECCION;
    UIController.showSelectionScreen(_currentProgram);
    UIController.showMessage("Sistema restablecido", 2000);
  }
}

void ProgramControllerClass::_shutdownAllSystems() {
  // Detener todos los sistemas de forma segura
  Actuators.stopMotor();
  Actuators.lockDoor(false);  // Desbloquear puerta en caso de emergencia
  Actuators.openWaterValve(false);
  Actuators.activateSteam(false);
  Actuators.activateCentrifuge(false);
  Actuators.openDrainValve(true);  // Abrir desagüe para vaciar agua
  
  Sensors.stopTemperatureMonitoring();
  Sensors.stopPressureMonitoring();
  
  // No utilizar delay para las señales de alarma
  Utils.createTimeout(TIEMPO_BUZZER * 3, []() {
    // Finalizar alarma o alguna otra acción
  });
}

void ProgramControllerClass::updateTimers() {
  // Esta función es llamada periódicamente mediante la tarea creada con Utils
  
  if (!_programFinished && !_emergencyActive) {
    if (!_programPaused) {
      // Temporizador principal
      if (_timerSeconds > 0) {
        _timerSeconds--;
        uint8_t minutos = _timerSeconds / 60;
        uint8_t segundos = _timerSeconds % 60;
        UIController.updateTime(minutos, segundos);
      } else {
        // Tiempo cumplido, pausar programa
        pauseProgram();
      }
      
      // Temporizador del motor si está activo
      if (Actuators.isMotorRunning()) {
        _motorSeconds++;
        
        // Control de dirección del motor según el tiempo
        switch (_motorDirection) {
          case 1: // Dirección A
            if (_motorSeconds >= _tiempoRotacion[_rotacionTam[_currentProgram - 1][_currentPhase - 1] - 1][0]) {
              _motorDirection = 2; // Pausa
              _motorPause = true;
              _motorSeconds = 0;
              Actuators.stopMotor();
            }
            break;
            
          case 2: // Pausa
            if (_motorSeconds >= _tiempoRotacion[_rotacionTam[_currentProgram - 1][_currentPhase - 1] - 1][1]) {
              if (_motorPause) {
                _motorDirection = 3; // Dirección B
                Actuators.reverseMotor();
              } else {
                _motorDirection = 1; // Dirección A
                Actuators.startMotor();
              }
              _motorSeconds = 0;
            }
            break;
            
          case 3: // Dirección B
            if (_motorSeconds >= _tiempoRotacion[_rotacionTam[_currentProgram - 1][_currentPhase - 1] - 1][0]) {
              _motorDirection = 2; // Pausa
              _motorPause = false;
              _motorSeconds = 0;
              Actuators.stopMotor();
            }
            break;
        }
      }
    } else {
      // Estamos en pausa entre fases
      if (_betweenPhaseSeconds > 0) {
        _betweenPhaseSeconds--;
        uint8_t minutos = _betweenPhaseSeconds / 60;
        uint8_t segundos = _betweenPhaseSeconds % 60;
        UIController.updateTime(minutos, segundos);
      } else {
        // Tiempo de pausa cumplido
        if (_currentProgram == 3) {
          // Para programa 3, pasar a siguiente fase
          resumeProgram();
        } else {
          // Para programas 1 y 2, terminar
          stopProgram();
        }
      }
    }
  }
}

void ProgramControllerClass::handleUserAction(const String& action) {
  // Procesar acción recibida de la interfaz de usuario basada en nombres de botones
  
  if (action.startsWith("PROGRAM_")) {
    uint8_t program = action.substring(8).toInt();
    selectProgram(program);
  } 
  else if (action == "START") {
    startProgram();
  } 
  else if (action == "STOP") {
    stopProgram();
  } 
  else if (action == "EDIT") {
    enterEditMode();
  } 
  else if (action == "SAVE") {
    exitEditMode();
    Storage.saveToEEPROM();
    UIController.showMessage("Configuración guardada", 2000);
  } 
  else if (action == "CANCEL") {
    // Restaurar datos originales
    Storage.loadFromEEPROM();
    exitEditMode();
    UIController.showMessage("Edición cancelada", 2000);
  } 
  else if (action == "INCREASE") {
    // Aumentar valor de la variable en edición
    uint8_t currentValue = getVariable(_currentVariable);
    editVariable(_currentVariable, currentValue + 1);
  }
  else if (action == "DECREASE") {
    // Disminuir valor de la variable en edición
    uint8_t currentValue = getVariable(_currentVariable);
    if (currentValue > 0) {
      editVariable(_currentVariable, currentValue - 1);
    }
  }
}

// Resto de la implementación...
```

## Responsabilidades

El módulo Controlador de Programas tiene las siguientes responsabilidades:

1. **Gestión de Estados**: Controlar las transiciones entre los diferentes estados del sistema (selección, edición, ejecución, pausa, emergencia).
2. **Control de Programas**: Manejar la selección, inicio, pausa y finalización de programas de lavado.
3. **Gestión de Parámetros**: Almacenar y proporcionar acceso a los parámetros de cada programa y fase.
4. **Control de Tiempo**: Gestionar los temporizadores para las fases y las rotaciones del motor.
5. **Coordinación**: Coordinar la interacción entre sensores, actuadores e interfaz de usuario.
6. **Manejo de Eventos**: Procesar las acciones del usuario recibidas desde la interfaz táctil.
7. **Gestión de Emergencias**: Manejar situaciones de emergencia provocadas por el botón de emergencia.

## Ventajas de este Enfoque

1. **Centralización Lógica**: Toda la lógica de control del sistema está centralizada en un solo módulo.
2. **Separación de Responsabilidades**: La lógica de control está separada de la interfaz de usuario y del control directo de hardware.
3. **Mantenibilidad**: Facilita la adición o modificación de programas y comportamientos sin afectar otros módulos.
4. **Encapsulamiento**: Los detalles internos del control del programa están encapsulados, exponiendo solo una interfaz clara.
5. **Flexibilidad**: Permite cambiar fácilmente las reglas de transición entre estados o el comportamiento de los programas.
6. **No Bloqueo**: Implementa funciones asíncronas mediante el módulo Utils en lugar de usar delay() para operaciones de tiempo.

## Adaptaciones para el Contexto y Hardware Específico

El módulo ha sido actualizado para el contexto de Arequipa, Perú, y adaptado al ESP32-WROOM con pantalla Nextion:

1. **Interfaz Táctil Nextion**: Se ha implementado un sistema para manejar acciones de usuario recibidas desde la pantalla Nextion, reemplazando la lógica basada en botones físicos.
2. **Manejo de Emergencia**: Se ha añadido soporte para situaciones de emergencia activadas por el botón de emergencia conectado al pin 15.
3. **Control de Centrifugado**: Incorpora manejo específico del control de centrifugado en el motor a través del pin 27.
4. **Integración con ESP32**: Se ha adaptado para aprovechar las capacidades del ESP32-WROOM, utilizando temporizadores asíncronos para evitar bloqueos.
5. **Sensores Específicos**: Integración con sensores de temperatura Dallas OneWire (pin 23) y sensor de presión HX710B (pines 5 y 4).

Al centralizar la lógica del sistema en este módulo, se facilita la comprensión y mantenimiento del sistema como un todo, permitiendo a otros módulos enfocarse en sus responsabilidades específicas sin preocuparse por la lógica general de la aplicación.
