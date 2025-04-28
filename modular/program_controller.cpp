// program_controller.cpp
#include "program_controller.h"

// Instancia global
ProgramControllerClass ProgramController;

void ProgramControllerClass::init() {
  // Inicializar variables de estado
  _currentState = ESTADO_SELECCION;
  _previousState = ESTADO_SELECCION;
  _currentProgram = 0;
  _currentPhase = 0;
  
  // Inicializar variables de temporizador
  _remainingMinutes = 0;
  _remainingSeconds = 0;
  _totalMinutes = 0;
  _totalSeconds = 0;
  _timerRunning = false;
  
  // Inicializar variables de edición
  _editingProgram = 0;
  _editingPhase = 0;
  _isEditing = false;
  
  // Cargar datos de programa desde almacenamiento
  _loadProgramData();
  
  Utils.debug("Program Controller inicializado");
}

void ProgramControllerClass::_loadProgramData() {
  // Cargar todos los parámetros para todos los programas
  for (uint8_t p = 0; p < NUM_PROGRAMAS; p++) {
    for (uint8_t f = 0; f < NUM_FASES; f++) {
      _waterLevels[p][f] = Storage.loadWaterLevel(p, f);
      _temperatures[p][f] = Storage.loadTemperature(p, f);
      _times[p][f] = Storage.loadTime(p, f);
      _rotations[p][f] = Storage.loadRotation(p, f);
    }
  }
  
  // Inicialmente cargar el último programa utilizado
  _currentProgram = Storage.loadProgram();
  _currentPhase = Storage.loadPhase();
  
  Utils.debug("Datos de programa cargados");
}

void ProgramControllerClass::setState(uint8_t newState) {
  if (newState != _currentState) {
    _previousState = _currentState;
    _currentState = newState;
    
    // Acciones específicas al cambiar de estado
    switch (newState) {
      case ESTADO_SELECCION:
        UIController.showSelectionScreen(_currentProgram);
        break;
      
      case ESTADO_EDICION:
        UIController.showEditScreen(_editingProgram, _editingPhase, 0, 0);
        break;
      
      case ESTADO_EJECUCION:
        _initializeProgram();
        _configureActuatorsForPhase();
        UIController.showExecutionScreen(
          _currentProgram,
          _currentPhase,
          _waterLevels[_currentProgram][_currentPhase],
          _temperatures[_currentProgram][_currentPhase],
          _rotations[_currentProgram][_currentPhase]
        );
        break;
      
      case ESTADO_PAUSA:
        Actuators.stopMotor();
        UIController.showMessage("Programa en pausa");
        break;
      
      case ESTADO_ERROR:
        Actuators.emergencyStop();
        UIController.showErrorScreen();
        break;
      
      case ESTADO_EMERGENCIA:
        Actuators.emergencyStop();
        UIController.showEmergencyScreen();
        break;
    }
    
    Utils.debugValue("Estado cambiado a", _currentState);
  }
}

uint8_t ProgramControllerClass::getState() {
  return _currentState;
}

void ProgramControllerClass::selectProgram(uint8_t program) {
  if (program < NUM_PROGRAMAS) {
    _currentProgram = program;
    Storage.saveProgram(program);
    Utils.debugValue("Programa seleccionado", program);
  }
}

uint8_t ProgramControllerClass::getCurrentProgram() {
  return _currentProgram;
}

void ProgramControllerClass::startProgram() {
  if (_currentState == ESTADO_SELECCION) {
    setState(ESTADO_EJECUCION);
    Utils.debug("Programa iniciado");
  }
}

void ProgramControllerClass::pauseProgram() {
  if (_currentState == ESTADO_EJECUCION) {
    setState(ESTADO_PAUSA);
    Utils.debug("Programa pausado");
  }
}

void ProgramControllerClass::resumeProgram() {
  if (_currentState == ESTADO_PAUSA) {
    setState(ESTADO_EJECUCION);
    Utils.debug("Programa reanudado");
  }
}

void ProgramControllerClass::stopProgram() {
  if (_currentState == ESTADO_EJECUCION || _currentState == ESTADO_PAUSA) {
    Actuators.emergencyStop(); // Detener todos los actuadores de forma segura
    setState(ESTADO_SELECCION);
    Utils.debug("Programa detenido");
  }
}

void ProgramControllerClass::setPhase(uint8_t phase) {
  if (phase < NUM_FASES) {
    _currentPhase = phase;
    Storage.savePhase(phase);
    Utils.debugValue("Fase establecida a", phase);
  }
}

uint8_t ProgramControllerClass::getCurrentPhase() {
  return _currentPhase;
}

void ProgramControllerClass::nextPhase() {
  if (_currentPhase < NUM_FASES - 1) {
    _currentPhase++;
    Storage.savePhase(_currentPhase);
    _updatePhaseParameters();
    Utils.debugValue("Avanzado a la fase", _currentPhase);
  } else {
    _completeProgram();
  }
}

bool ProgramControllerClass::isLastPhase() {
  return _currentPhase >= (NUM_FASES - 1);
}

void ProgramControllerClass::_updatePhaseParameters() {
  // Actualizar parámetros para la fase actual
  if (_currentState == ESTADO_EJECUCION) {
    _configureActuatorsForPhase();
    
    // Reiniciar temporizador para la nueva fase
    _totalMinutes = _times[_currentProgram][_currentPhase];
    _totalSeconds = _totalMinutes * 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;
    
    // Actualizar la interfaz de usuario
    UIController.updatePhase(_currentPhase);
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    UIController.updateProgressBar(0);
  }
}

void ProgramControllerClass::updateTimers() {
  if (_currentState == ESTADO_EJECUCION && _timerRunning) {
    _decrementTimer();
    
    // Actualizar actuadores según estado del programa
    Actuators.updateTimers();
    
    // Comprobar condiciones de los sensores
    _checkSensorConditions();
  }
}

void ProgramControllerClass::_decrementTimer() {
  // Decrementar el temporizador cada segundo
  if (_remainingSeconds > 0) {
    _remainingSeconds--;
  } else if (_remainingMinutes > 0) {
    _remainingMinutes--;
    _remainingSeconds = 59;
  } else {
    // Tiempo completado para la fase actual
    _completePhase();
    return;
  }
  
  // Actualizar la interfaz de usuario
  UIController.updateTime(_remainingMinutes, _remainingSeconds);
  
  // Calcular y actualizar progreso
  uint16_t totalSecs = _totalMinutes * 60;
  uint16_t elapsedSecs = totalSecs - ((_remainingMinutes * 60) + _remainingSeconds);
  uint8_t progress = (elapsedSecs * 100) / totalSecs;
  UIController.updateProgressBar(progress);
}

void ProgramControllerClass::_checkSensorConditions() {
  // Verificar las condiciones de temperatura y nivel de agua según la fase actual
  uint8_t targetTemp = _temperatures[_currentProgram][_currentPhase];
  uint8_t targetLevel = _waterLevels[_currentProgram][_currentPhase];
  
  // Actualizar la interfaz con los valores actuales
  UIController.updateTemperature(Sensors.getCurrentTemperature());
  UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
  
  // Control de llenado de agua
  if (Sensors.getCurrentWaterLevel() < targetLevel) {
    if (!Actuators.isWaterValveOpen()) {
      Actuators.openWaterValve();
    }
  } else {
    if (Actuators.isWaterValveOpen()) {
      Actuators.closeWaterValve();
    }
  }
  
  // Control de temperatura
  if (Sensors.getCurrentTemperature() < targetTemp) {
    if (!Actuators.isSteamValveOpen()) {
      Actuators.openSteamValve();
    }
  } else {
    if (Actuators.isSteamValveOpen()) {
      Actuators.closeSteamValve();
    }
  }
  
  // Condición para iniciar la rotación automática (cuando se alcanza nivel y temperatura)
  uint8_t rotLevel = _rotations[_currentProgram][_currentPhase];
  if (Sensors.isWaterLevelReached(targetLevel) && 
      Sensors.isTemperatureReached(targetTemp) && 
      rotLevel > 0 && 
      !Actuators.isAutoRotationActive()) {
    Actuators.startAutoRotation(rotLevel);
    // Iniciar el temporizador solo cuando se alcanzan las condiciones necesarias
    if (!_timerRunning) {
      _timerRunning = true;
      Utils.debug("Condiciones alcanzadas, iniciando temporizador de fase");
    }
  }
}

void ProgramControllerClass::_completePhase() {
  Utils.debugValue("Fase completada", _currentPhase);
  
  // Detener actuadores de la fase actual
  Actuators.stopAutoRotation();
  Actuators.closeSteamValve();
  Actuators.closeWaterValve();
  
  // Avanzar a la siguiente fase o completar el programa
  if (isLastPhase()) {
    _completeProgram();
  } else {
    nextPhase();
  }
}

void ProgramControllerClass::_completeProgram() {
  Utils.debug("Programa completado");
  
  // Llevar el sistema a un estado seguro
  Actuators.stopAutoRotation();
  Actuators.closeSteamValve();
  Actuators.closeWaterValve();
  Actuators.openDrainValve();
  
  // Incrementar contador de uso
  Storage.incrementUsageCounter();
  
  // Reproducir señal de finalización
  Actuators.startBuzzer(TIEMPO_BUZZER);
  
  // Volver a la pantalla de selección
  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::_initializeProgram() {
  // Inicializar temporizador para la fase actual
  _totalMinutes = _times[_currentProgram][_currentPhase];
  _totalSeconds = _totalMinutes * 60;
  _remainingMinutes = _totalMinutes;
  _remainingSeconds = 0;
  _timerRunning = false; // Se activará cuando se alcancen las condiciones necesarias
  
  // Preparar el sistema para el inicio del programa
  Actuators.lockDoor();
  
  Utils.debug("Programa inicializado");
}

void ProgramControllerClass::_configureActuatorsForPhase() {
  // Configurar actuadores según la fase actual
  uint8_t targetLevel = _waterLevels[_currentProgram][_currentPhase];
  uint8_t targetTemp = _temperatures[_currentProgram][_currentPhase];
  
  // Configurar válvulas según la fase
  if (_currentPhase == 0) {
    // Primera fase: llenar con agua
    Actuators.closeDrainValve();
    if (Sensors.getCurrentWaterLevel() < targetLevel) {
      Actuators.openWaterValve();
    }
  } else if (_currentPhase == NUM_FASES - 1) {
    // Última fase: drenar
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
    Actuators.openDrainValve();
  } else {
    // Fases intermedias
    if (Sensors.getCurrentWaterLevel() < targetLevel) {
      Actuators.openWaterValve();
      Actuators.closeDrainValve();
    }
    
    if (Sensors.getCurrentTemperature() < targetTemp) {
      Actuators.openSteamValve();
    }
  }
  
  Utils.debug("Actuadores configurados para la fase");
}

uint8_t ProgramControllerClass::getRemainingMinutes() {
  return _remainingMinutes;
}

uint8_t ProgramControllerClass::getRemainingSeconds() {
  return _remainingSeconds;
}

uint8_t ProgramControllerClass::getTotalMinutes() {
  return _totalMinutes;
}

uint8_t ProgramControllerClass::getTotalSeconds() {
  return _totalSeconds;
}

uint8_t ProgramControllerClass::getProgressPercentage() {
  if (_totalSeconds == 0) return 0;
  uint16_t remainingTotal = (_remainingMinutes * 60) + _remainingSeconds;
  return 100 - ((remainingTotal * 100) / _totalSeconds);
}

void ProgramControllerClass::startEditing(uint8_t program, uint8_t phase) {
  if (program < NUM_PROGRAMAS && phase < NUM_FASES) {
    _editingProgram = program;
    _editingPhase = phase;
    _isEditing = true;
    setState(ESTADO_EDICION);
    Utils.debug("Modo edición iniciado");
  }
}

void ProgramControllerClass::editParameter(uint8_t paramType, uint8_t value) {
  if (!_isEditing) return;
  
  // Guardar el nuevo valor según el tipo de parámetro
  switch (paramType) {
    case 0: // Nivel de agua
      _waterLevels[_editingProgram][_editingPhase] = value;
      Storage.saveWaterLevel(_editingProgram, _editingPhase, value);
      break;
    
    case 1: // Temperatura
      _temperatures[_editingProgram][_editingPhase] = value;
      Storage.saveTemperature(_editingProgram, _editingPhase, value);
      break;
    
    case 2: // Tiempo
      _times[_editingProgram][_editingPhase] = value;
      Storage.saveTime(_editingProgram, _editingPhase, value);
      break;
    
    case 3: // Rotación
      _rotations[_editingProgram][_editingPhase] = value;
      Storage.saveRotation(_editingProgram, _editingPhase, value);
      break;
  }
  
  Utils.debug("Parámetro editado");
}

void ProgramControllerClass::saveEditing() {
  _isEditing = false;
  setState(ESTADO_SELECCION);
  Utils.debug("Edición guardada");
}

void ProgramControllerClass::cancelEditing() {
  _isEditing = false;
  // Recargar valores originales
  _loadProgramData();
  setState(ESTADO_SELECCION);
  Utils.debug("Edición cancelada");
}

void ProgramControllerClass::processUserEvent(const String& event) {
  // Procesar eventos según el estado actual
  switch (_currentState) {
    case ESTADO_SELECCION:
      _handleSelectionState();
      break;
    
    case ESTADO_EDICION:
      _handleEditingState();
      break;
    
    case ESTADO_EJECUCION:
      _handleExecutionState();
      break;
    
    case ESTADO_PAUSA:
      _handlePauseState();
      break;
    
    case ESTADO_ERROR:
      _handleErrorState();
      break;
    
    case ESTADO_EMERGENCIA:
      _handleEmergencyState();
      break;
  }
}

void ProgramControllerClass::_handleStateMachine() {
  // Esta función implementará la lógica principal de la máquina de estados
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handleSelectionState() {
  // Procesar eventos en el estado de selección
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handleEditingState() {
  // Procesar eventos en el estado de edición
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handleExecutionState() {
  // Procesar eventos en el estado de ejecución
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handlePauseState() {
  // Procesar eventos en el estado de pausa
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handleErrorState() {
  // Procesar eventos en el estado de error
  // Se implementará en versiones futuras
}

void ProgramControllerClass::_handleEmergencyState() {
  // Procesar eventos en el estado de emergencia
  // Se implementará en versiones futuras
}

void ProgramControllerClass::handleEmergency() {
  // Manejar situación de emergencia
  if (_currentState != ESTADO_EMERGENCIA) {
    Utils.debug("EMERGENCIA DETECTADA");
    setState(ESTADO_EMERGENCIA);
    Actuators.emergencyStop();
  }
}

void ProgramControllerClass::resetEmergency() {
  // Restablecer el sistema después de una emergencia
  if (_currentState == ESTADO_EMERGENCIA) {
    Utils.debug("Restableciendo sistema después de emergencia");
    Actuators.emergencyReset();
    setState(ESTADO_SELECCION);
  }
}

void ProgramControllerClass::_triggerError(uint8_t errorCode, const String& errorMessage) {
  // Activar estado de error con código y mensaje específicos
  Utils.debug("ERROR: " + errorMessage);
  setState(ESTADO_ERROR);
  UIController.showErrorScreen(errorCode, errorMessage);
}

void ProgramControllerClass::update() {
  // Actualización periódica del controlador de programa
  // Esta función debe ser llamada desde el loop principal

  // Procesar eventos de interfaz de usuario
  if (UIController.hasUserAction()) {
    String action = UIController.getUserAction();
    processUserEvent(action);
  }
  
  // Actualizar según el estado actual
  _handleStateMachine();
}
