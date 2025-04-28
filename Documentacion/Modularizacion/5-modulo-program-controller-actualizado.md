# Módulo de Controlador de Programas (program_controller.h / program_controller.cpp)

## Descripción

El módulo Controlador de Programas gestiona la lógica central del sistema, incluyendo la selección, edición y ejecución de programas de lavado. Este módulo coordina las transiciones entre estados, maneja los parámetros de los programas y controla el flujo general de la aplicación.

## Analogía: Director de Orquesta

Este módulo funciona como el director de una orquesta, quien coordina a todos los músicos (módulos) para que toquen la sinfonía (programa de lavado) correctamente. El director decide cuándo inicia cada sección, a qué ritmo tocan, y cómo transicionan entre las diferentes partes de la pieza musical. No toca ningún instrumento directamente, pero es quien determina cómo y cuándo actúa cada sección de la orquesta.

## Estructura del Módulo

El módulo de Controlador de Programas se divide en:

- **program_controller.h**: Define la interfaz pública del módulo
- **program_controller.cpp**: Implementa la funcionalidad interna

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
  
  // Control de programas
  void selectProgram(uint8_t programa);
  void startProgram();
  void pauseProgram();
  void resumeProgram();
  void stopProgram();
  void resetProgram();
  
  // Modo edición
  void enterEditMode();
  void exitEditMode();
  void editVariable(uint8_t numeroVariable, uint8_t valor);
  
  // Control de fase
  void nextPhase();
  void setPhase(uint8_t fase);
  
  // Acceso a parámetros
  uint8_t getCurrentProgram();
  uint8_t getCurrentPhase();
  bool isSystemBlocked();
  uint8_t getVariable(uint8_t numeroVariable);
  
  // Control de tiempo
  void updateTimers();
  
  // Acceso a datos
  uint8_t (*getNivelAguaData())[4];
  uint8_t (*getRotacionData())[4];
  uint8_t (*getTemperaturaData())[4];
  uint8_t (*getTemporizadorData())[4];
  
  // Control de agua
  void increaseWater();
  void decreaseWater();
  
  // Manejo de acciones de usuario desde la pantalla
  void handleUserAction(const String& action);
  
  // Manejo de emergencia
  void handleEmergency();
  void resetEmergency();

private:
  // Variables de estado
  uint8_t _currentState;
  uint8_t _currentProgram;
  uint8_t _currentPhase;
  uint8_t _currentVariable;
  uint8_t _editLevel;
  bool _programFinished;
  bool _programPaused;
  bool _emergencyActive;
  bool _editingProgram;
  uint16_t _blockCounter;
  
  // Temporizadores
  int16_t _timerSeconds;
  int16_t _motorSeconds;
  uint8_t _betweenPhaseSeconds;
  
  // Parámetros de programas
  uint8_t _nivelAgua[3][4];
  uint8_t _rotacionTam[3][4];
  uint8_t _temperaturaLim[3][4];
  uint8_t _temporizadorLim[3][4];
  uint8_t _tiempoRotacion[3][2];
  uint8_t _tiempoEntreFase[3][4];
  
  // Control de motor
  uint8_t _motorDirection;
  bool _motorPause;
  
  // Métodos internos
  void _initializeTimers();
  void _checkTimers();
  void _startPhase();
  void _updateVariableValue();
  void _parseUserAction(const String& action);
  void _getVariableValue();
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
  
  // Si estamos en estado de ejecución, actualizar temporizadores
  if (_currentState == ESTADO_EJECUCION) {
    updateTimers();
  }
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
      // Programa 3: Motor + Temperatura + Nivel
      Actuators.startMotor();
      Sensors.startTemperatureMonitoring(_temperaturaLim[_currentProgram - 1][_currentPhase - 1]);
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
    
    // Iniciar temporizador con una función de callback
    Utils.startTimer([]() {
      ProgramController.updateTimers();
    }, INTERVALO_TEMPORIZADOR);
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
    Actuators.openDrainValve(false);
    
    Sensors.stopTemperatureMonitoring();
    Sensors.stopPressureMonitoring();
    
    // Activar buzzer brevemente
    Actuators.soundBuzzer(true);
    delay(TIEMPO_BUZZER);
    Actuators.soundBuzzer(false);
    
    // Detener el temporizador
    Utils.stopTimer();
    
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
  Actuators.openDrainValve(true);  // Abrir desagüe para vaciar agua
  
  Sensors.stopTemperatureMonitoring();
  Sensors.stopPressureMonitoring();
  
  // Detener temporizadores
  Utils.stopTimer();
  
  // Activar alarma
  Actuators.soundBuzzer(true);
  delay(TIEMPO_BUZZER * 3);  // Alarma más larga
  Actuators.soundBuzzer(false);
}

void ProgramControllerClass::updateTimers() {
  // Esta función es llamada periódicamente para actualizar temporizadores
  
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
  // Procesar acción recibida de la interfaz de usuario
  
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
  else if (action.startsWith("VALUE_CHANGE_")) {
    // Formato: VALUE_CHANGE_[ComponentID]_[PageID]_[Value]
    // Procesar cambio de valor en controles deslizantes o numéricos
    String valueStr = action.substring(action.lastIndexOf('_') + 1);
    uint8_t value = valueStr.toInt();
    
    if (_currentState == ESTADO_EDICION) {
      editVariable(_currentVariable, value);
    }
  }
}

void ProgramControllerClass::enterEditMode() {
  if (_currentState == ESTADO_SELECCION) {
    _currentState = ESTADO_EDICION;
    _currentVariable = 1;
    _editLevel = 1;
    _editingProgram = true;
    
    // Obtener valor actual de la variable seleccionada
    _getVariableValue();
    
    // Mostrar pantalla de edición
    UIController.showEditScreen(_currentProgram, _currentPhase, _currentVariable, getVariable(_currentVariable));
  }
}

void ProgramControllerClass::exitEditMode() {
  if (_currentState == ESTADO_EDICION) {
    _currentState = ESTADO_SELECCION;
    _editingProgram = false;
    
    // Volver a pantalla de selección
    UIController.showSelectionScreen(_currentProgram);
  }
}

uint8_t ProgramControllerClass::getCurrentProgram() {
  return _currentProgram;
}

uint8_t ProgramControllerClass::getCurrentPhase() {
  return _currentPhase;
}

bool ProgramControllerClass::isSystemBlocked() {
  return _blockCounter >= LIMITE_BLOQUEO;
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

## Adaptaciones para el Nuevo Hardware

El módulo ha sido actualizado para adaptarse a los cambios de hardware:

1. **Interfaz Táctil**: Se ha implementado un sistema para manejar acciones de usuario recibidas desde la pantalla Nextion, reemplazando la lógica basada en botones físicos.
2. **Manejo de Emergencia**: Se ha añadido soporte para situaciones de emergencia activadas por el botón de emergencia.
3. **Integración con ESP32**: Se ha adaptado para aprovechar las capacidades del ESP32, como mejores opciones de temporización.

Al centralizar la lógica del sistema en este módulo, se facilita la comprensión y mantenimiento del sistema como un todo, permitiendo a otros módulos enfocarse en sus responsabilidades específicas sin preocuparse por la lógica general de la aplicación.
