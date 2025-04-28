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

private:
  // Variables de estado
  uint8_t _currentState;
  uint8_t _currentProgram;
  uint8_t _currentPhase;
  uint8_t _currentVariable;
  uint8_t _editLevel;
  bool _programFinished;
  bool _programPaused;
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
  void _getVariableValue();
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
    
    // Volver a pantalla de selección
    UIController.showSelectionScreen(_currentProgram);
  }
}

void ProgramControllerClass::resetProgram() {
  // Reiniciar para nueva fase sin cambiar el estado
  if (_currentProgram == 3) {
    _programPaused = false;
    _motorDirection = 1;
    _motorSeconds = 0;
    
    _initializeTimers();
    
    // Reactivar para nueva fase
    Actuators.startMotor();
    Actuators.openWaterValve(true);
    Actuators.openDrainValve(false);
    
    // Actualizar sensores con nuevos valores objetivo
    uint8_t tempTarget = _temperaturaLim[_currentProgram - 1][_currentPhase - 1];
    if (tempTarget > 0) {
      Sensors.startTemperatureMonitoring(tempTarget);
      Actuators.activateSteam(true);
    }
    
    uint8_t levelTarget = _nivelAgua[_currentProgram - 1][_currentPhase - 1];
    if (levelTarget > 0) {
      Sensors.startPressureMonitoring(levelTarget);
      Actuators.openWaterValve(true);
    }
  }
}

void ProgramControllerClass::enterEditMode() {
  if (_currentState == ESTADO_SELECCION) {
    _currentState = ESTADO_EDICION;
    _currentVariable = 1;
    _editLevel = 1;
    
    // Obtener valor actual de la variable seleccionada
    _getVariableValue();
    
    // Mostrar pantalla de edición
    UIController.showEditScreen(_currentProgram, _currentPhase, _currentVariable, getVariable(_currentVariable));
    UIController.enableEditMode(true);
    UIController.setEditCursor(_editLevel);
  }
}

void ProgramControllerClass::exitEditMode() {
  if (_currentState == ESTADO_EDICION) {
    _currentState = ESTADO_SELECCION;
    UIController.enableEditMode(false);
    
    // Guardar cambios en EEPROM
    Storage.saveToEEPROM();
    
    // Volver a pantalla de selección
    UIController.showSelectionScreen(_currentProgram);
  }
}

void ProgramControllerClass::editVariable(uint8_t numeroVariable, uint8_t valor) {
  if (_currentState == ESTADO_EDICION) {
    _currentVariable = numeroVariable;
    
    // Actualizar el valor
    switch (_currentVariable) {
      case 1:
        _nivelAgua[_currentProgram - 1][_currentPhase - 1] = valor;
        break;
      case 2:
        _temporizadorLim[_currentProgram - 1][_currentPhase - 1] = valor;
        break;
      case 3:
        _temperaturaLim[_currentProgram - 1][_currentPhase - 1] = valor;
        break;
      case 4:
        _rotacionTam[_currentProgram - 1][_currentPhase - 1] = valor;
        break;
    }
    
    // Actualizar pantalla
    UIController.showEditScreen(_currentProgram, _currentPhase, _currentVariable, getVariable(_currentVariable));
    UIController.setEditCursor(_editLevel);
  }
}

void ProgramControllerClass::nextPhase() {
  if (_currentPhase < 4) {
    _currentPhase++;
  } else {
    _currentPhase = 1;
  }
}

void ProgramControllerClass::setPhase(uint8_t fase) {
  if (fase >= 1 && fase <= 4) {
    _currentPhase = fase;
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

uint8_t ProgramControllerClass::getVariable(uint8_t numeroVariable) {
  switch (numeroVariable) {
    case 1: return _nivelAgua[_currentProgram - 1][_currentPhase - 1];
    case 2: return _temporizadorLim[_currentProgram - 1][_currentPhase - 1];
    case 3: return _temperaturaLim[_currentProgram - 1][_currentPhase - 1];
    case 4: return _rotacionTam[_currentProgram - 1][_currentPhase - 1];
    default: return 0;
  }
}

void ProgramControllerClass::updateTimers() {
  // Esta función es llamada periódicamente para actualizar temporizadores
  
  if (!_programFinished) {
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

void ProgramControllerClass::_initializeTimers() {
  _timerSeconds = _temporizadorLim[_currentProgram - 1][_currentPhase - 1] * 60;
  _motorSeconds = 0;
  _betweenPhaseSeconds = 0;
}

void ProgramControllerClass::_getVariableValue() {
  // Obtener el valor actual de la variable seleccionada
  // para el modo de edición
}

uint8_t (*ProgramControllerClass::getNivelAguaData())[4] {
  return _nivelAgua;
}

uint8_t (*ProgramControllerClass::getRotacionData())[4] {
  return _rotacionTam;
}

uint8_t (*ProgramControllerClass::getTemperaturaData())[4] {
  return _temperaturaLim;
}

uint8_t (*ProgramControllerClass::getTemporizadorData())[4] {
  return _temporizadorLim;
}

void ProgramControllerClass::increaseWater() {
  Actuators.openWaterValve(true);
}

void ProgramControllerClass::decreaseWater() {
  Actuators.openWaterValve(false);
}
```

## Responsabilidades

El módulo Controlador de Programas tiene las siguientes responsabilidades:

1. **Gestión de Estados**: Controlar las transiciones entre los diferentes estados del sistema (selección, edición, ejecución, pausa).
2. **Control de Programas**: Manejar la selección, inicio, pausa y finalización de programas de lavado.
3. **Gestión de Parámetros**: Almacenar y proporcionar acceso a los parámetros de cada programa y fase.
4. **Control de Tiempo**: Gestionar los temporizadores para las fases y las rotaciones del motor.
5. **Coordinación**: Coordinar la interacción entre sensores, actuadores e interfaz de usuario.
6. **Edición**: Gestionar el modo de edición de parámetros de programa.

## Ventajas de este Enfoque

1. **Centralización Lógica**: Toda la lógica de control del sistema está centralizada en un solo módulo.
2. **Separación de Responsabilidades**: La lógica de control está separada de la interfaz de usuario y del control directo de hardware.
3. **Mantenibilidad**: Facilita la adición o modificación de programas y comportamientos sin afectar otros módulos.
4. **Encapsulamiento**: Los detalles internos del control del programa están encapsulados, exponiendo solo una interfaz clara.
5. **Flexibilidad**: Permite cambiar fácilmente las reglas de transición entre estados o el comportamiento de los programas.

Al centralizar la lógica del sistema en este módulo, se facilita la comprensión y mantenimiento del sistema como un todo, permitiendo a otros módulos enfocarse en sus responsabilidades específicas sin preocuparse por la lógica general de la aplicación.
