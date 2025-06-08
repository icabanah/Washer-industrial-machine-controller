// program_controller.cpp
#include "program_controller.h"

// Instancia global
ProgramControllerClass ProgramController;

void ProgramControllerClass::init() {
  // Inicializar variables de estado
  _currentState = ESTADO_SELECCION;
  _previousState = ESTADO_SELECCION;
  _currentProgram = 1;
  _currentPhase = 0;
  
  // Inicializar variables de temporizador
  _remainingMinutes = 0;
  _remainingSeconds = 0;
  _totalMinutes = 0;
  _totalSeconds = 0;
  _timerRunning = false;
  
  // Inicializar variables de edición
  _editingProgram = 1;
  _editingPhase = 0;
  _editingParameter = 0;
  _editingParameterValue = 0;
  _isEditing = false;
  
  // Cargar datos de programa desde almacenamiento
  _loadProgramData();
  
  Utils.debug("ProgramControllerClass::init| Program Controller inicializado");
}

/// @brief 
/// Carga los datos de los programas desde el almacenamiento.
/// Este método se encarga de cargar los niveles de agua, temperaturas, tiempos y rotaciones para cada programa y fase.
/// Utiliza la clase `Storage` para acceder a los datos almacenados.
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
  
  Utils.debug("ProgramControllerClass::_loadProgramData| Datos de programa cargados");
}

/// @brief 
/// Cambia el estado del controlador de programas.
/// Este método actualiza el estado actual del controlador de programas y realiza las acciones
/// necesarias al cambiar de estado, como actualizar la interfaz de usuario y configurar los actuadores.
/// @param newState 
/// El nuevo estado al que se desea cambiar.
/// Debe ser uno de los siguientes valores:
/// - ESTADO_SELECCION: Selección de programa.
/// - ESTADO_EDICION: Edición de parámetros del programa.
/// - ESTADO_EJECUCION: Ejecución del programa.
/// - ESTADO_PAUSA: Pausa del programa.
/// - ESTADO_ERROR: Estado de error.
/// - ESTADO_EMERGENCIA: Estado de emergencia.
/// @details
/// Este método es utilizado para gestionar el flujo del controlador de programas, permitiendo que el sistema reaccione a eventos del usuario o condiciones del sistema.
/// Al cambiar de estado, se actualiza la interfaz de usuario y se configuran los actuadores según el nuevo estado.
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
        UIController.showEditScreen(_editingProgram, _editingPhase);
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

/// @brief 
/// Obtiene el estado actual del controlador de programas.
/// @return 
/// El estado actual del controlador de programas.
/// Los estados posibles son:
/// - ESTADO_SELECCION: Selección de programa.
/// - ESTADO_EDICION: Edición de parámetros del programa.
/// - ESTADO_EJECUCION: Ejecución del programa.
/// - ESTADO_PAUSA: Pausa del programa.
/// - ESTADO_ERROR: Estado de error.
/// - ESTADO_EMERGENCIA: Estado de emergencia.
////// Este método es utilizado para obtener el estado actual del controlador de programas,
/// permitiendo que otros componentes del sistema puedan reaccionar a los cambios de estado
/// y actualizar la interfaz de usuario o realizar acciones específicas según el estado.
/// @note
/// Este método es parte de la clase ProgramControllerClass y debe ser llamado después de
/// haber inicializado el controlador de programas con el método init().
/// @see ProgramControllerClass::init() 
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

/// @brief 
/// Obtiene el programa actual seleccionado.
/// Este método devuelve el número del programa que está actualmente seleccionado.
/// El programa se selecciona mediante el método `selectProgram(uint8_t program)`.
/// @note
/// Este método es útil para saber qué programa está activo en el controlador de programas,
/// permitiendo que otros componentes del sistema puedan reaccionar a los cambios de programa.
/// Por ejemplo, la interfaz de usuario puede actualizarse para mostrar los detalles del programa seleccionado.
/// @return 
/// El número del programa actualmente seleccionado, que debe estar en el rango de 1 a `NUM_PROGRAMAS - 1`.
/// Si no se ha seleccionado ningún programa, el valor por defecto es 0.
uint8_t ProgramControllerClass::getCurrentProgram() {
  return _currentProgram;
}

void ProgramControllerClass::startProgram() {
  if (_currentState == ESTADO_SELECCION) {
    setState(ESTADO_EJECUCION);
    Utils.debug("ProgramControllerClass::startProgram| Programa iniciado: " + String(_currentProgram));
  }
}

void ProgramControllerClass::pauseProgram() {
  if (_currentState == ESTADO_EJECUCION) {
    setState(ESTADO_PAUSA);
    Utils.debug("ProgramControllerClass::startProgram| Programa pausado: " + String(_currentProgram));
  }
}

void ProgramControllerClass::resumeProgram() {
  if (_currentState == ESTADO_PAUSA) {
    setState(ESTADO_EJECUCION);
    Utils.debug("ProgramControllerClass::startProgram| Programa reanudado: " + String(_currentProgram));
  }
}

void ProgramControllerClass::stopProgram() {
  if (_currentState == ESTADO_EJECUCION || _currentState == ESTADO_PAUSA) {
    Actuators.emergencyStop(); // Detener todos los actuadores de forma segura
    setState(ESTADO_SELECCION);
    Utils.debug("ProgramControllerClass::stopProgram| Programa detenido: " + String(_currentProgram));
  }
}

void ProgramControllerClass::setPhase(uint8_t phase) {
  if (phase < NUM_FASES) {
    _currentPhase = phase;
    Storage.savePhase(phase);
    Utils.debugValue("ProgramControllerClass::setPhase| Fase establecida a: ", phase);
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
    Utils.debugValue("ProgramControllerClass::nextPhase| Avanzado a la fase: ", _currentPhase);
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
      Utils.debug("ProgramControllerClass::_checkSensorConditions| Condiciones alcanzadas, iniciando temporizador de fase");
    }
  }
}

void ProgramControllerClass::_completePhase() {
  Utils.debugValue("ProgramControllerClass::_completePhase| Fase completada: ", _currentPhase);
  
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
  Utils.debug("ProgramControllerClass::_completeProgram| Programa completado");
  
  // Llevar el sistema a un estado seguro
  Actuators.stopAutoRotation();
  Actuators.closeSteamValve();
  Actuators.closeWaterValve();
  Actuators.openDrainValve();
  
  // Incrementar contador de uso
  Storage.incrementUsageCounter();
  
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
  
  Utils.debug("ProgramControllerClass::_initializeProgram| Programa inicializado");
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
  
  Utils.debug("ProgramControllerClass::_configureActuatorsForPhase| Actuadores configurados para la fase");
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
    _editingParameter = PARAM_NIVEL; // Comenzar editando el nivel
    _editingParameterValue = _waterLevels[program][phase]; // Cargar valor actual
    _isEditing = true;
    
    setState(ESTADO_EDICION);
    
    Utils.debug("ProgramControllerClass::startEditing| ✏️ Modo edición iniciado:");
    Utils.debug("ProgramControllerClass::startEditing|   Programa: " + String(program + 22));
    Utils.debug("ProgramControllerClass::startEditing|   Fase: " + String(phase + 1));
    Utils.debug("ProgramControllerClass::startEditing|   Parámetro inicial: NIVEL");
    Utils.debug("ProgramControllerClass::startEditing|   Valor inicial: " + String(_editingParameterValue));
    
    // Mostrar pantalla de edición
    _updateEditDisplay();
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
  if (!_isEditing) return;
  
  // Guardar el valor del parámetro editado en la estructura de datos
  switch (_editingParameter) {
    case PARAM_NIVEL:
      _waterLevels[_editingProgram][_editingPhase] = _editingParameterValue;
      Utils.debug("💾 Nivel guardado: " + String(_editingParameterValue));
      break;
    case PARAM_TEMPERATURA:
      _temperatures[_editingProgram][_editingPhase] = _editingParameterValue;
      Utils.debug("💾 Temperatura guardada: " + String(_editingParameterValue) + "°C");
      break;
    case PARAM_TIEMPO:
      _times[_editingProgram][_editingPhase] = _editingParameterValue;
      Utils.debug("💾 Tiempo guardado: " + String(_editingParameterValue) + " min");
      break;
    case PARAM_ROTACION:
      _rotations[_editingProgram][_editingPhase] = _editingParameterValue;
      Utils.debug("💾 Rotación guardada: " + String(_editingParameterValue));
      break;
  }
  
  // Guardar en almacenamiento persistente usando métodos individuales
  Storage.saveWaterLevel(_editingProgram, _editingPhase, _waterLevels[_editingProgram][_editingPhase]);
  Storage.saveTemperature(_editingProgram, _editingPhase, _temperatures[_editingProgram][_editingPhase]);
  Storage.saveTime(_editingProgram, _editingPhase, _times[_editingProgram][_editingPhase]);
  Storage.saveRotation(_editingProgram, _editingPhase, _rotations[_editingProgram][_editingPhase]);
  
  _isEditing = false;
  setState(ESTADO_SELECCION);
  
  // Mostrar pantalla de selección actualizada
  UIController.showSelectionScreen(_editingProgram);
  
  Utils.debug("✅ Edición guardada exitosamente");
}

void ProgramControllerClass::cancelEditing() {
  _isEditing = false;
  // Recargar valores originales
  _loadProgramData();
  setState(ESTADO_SELECCION);
  Utils.debug("Edición cancelada");
}

void ProgramControllerClass::processUserEvent(const String& event) {
  // Verificar si hay un evento táctil válido
  if (!Hardware.hasValidTouchEvent()) {
    return; // No hay evento táctil válido
  }
  
  uint8_t touchPage = Hardware.getTouchEventPage();
  uint8_t touchComponent = Hardware.getTouchEventComponent();
  uint8_t touchType = Hardware.getTouchEventType();
  
  // Solo procesar eventos de botón presionado (touchType == 1)
  if (touchType != 1) {
    return;
  }
  
  Utils.debug("Procesando evento táctil - Página: " + String(touchPage) + 
              ", Componente: " + String(touchComponent) + 
              ", Estado: " + String(_currentState));
  
  // Procesar eventos según la página actual
  switch (touchPage) {
    case NEXTION_PAGE_SELECTION:
      _handleSelectionPageEvents(touchComponent);
      break;
      
    case NEXTION_PAGE_EDIT:
      _handleEditPageEvents(touchComponent);
      break;
      
    case NEXTION_PAGE_EXECUTION:
      _handleExecutionPageEvents(touchComponent);
      break;
      
    default:
      Utils.debug("Página no manejada: " + String(touchPage));
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

// ===== IMPLEMENTACIÓN DE MANEJO DE EVENTOS TÁCTILES =====

void ProgramControllerClass::_handleSelectionPageEvents(uint8_t componentId) {
  Utils.debug("🔘 Evento en página de selección - Componente: " + String(componentId));
  
  switch (componentId) {
    case NEXTION_ID_BTN_PROG_ANTERIOR:
      // Cambiar al programa anterior
      if (_currentProgram > 0) {
        _currentProgram--;
      } else {
        _currentProgram = NUM_PROGRAMAS - 1; // Circular: ir al último programa
      }
      Utils.debug("📋 Programa seleccionado: " + String(_currentProgram + 22));
      UIController.showSelectionScreen(_currentProgram);
      break;
      
    case NEXTION_ID_BTN_PROG_SIGUIENTE:
      // Cambiar al programa siguiente
      if (_currentProgram < NUM_PROGRAMAS - 1) {
        _currentProgram++;
      } else {
        _currentProgram = 0; // Circular: ir al primer programa
      }
      Utils.debug("📋 Programa seleccionado: " + String(_currentProgram + 22));
      UIController.showSelectionScreen(_currentProgram);
      break;
      
    case NEXTION_ID_BTN_COMENZAR:
      // Iniciar el programa seleccionado
      Utils.debug("▶️ Iniciando programa " + String(_currentProgram + 22));
      startProgram();
      break;
      
    case NEXTION_ID_BTN_EDITAR:
      // Entrar en modo de edición para el programa seleccionado
      Utils.debug("✏️ Editando programa " + String(_currentProgram + 22));
      startEditing(_currentProgram, 0); // Comenzar editando la primera fase
      break;
      
    default:
      Utils.debug("⚠️ Componente no reconocido en página de selección: " + String(componentId));
      break;
  }
}

void ProgramControllerClass::_handleEditPageEvents(uint8_t componentId) {
  Utils.debug("🔧 Evento en página de edición - Componente: " + String(componentId));
  
  switch (componentId) {
    case NEXTION_ID_BTN_PARAM_MENOS:
      // Disminuir valor del parámetro actual
      _decreaseCurrentParameter();
      break;
      
    case NEXTION_ID_BTN_PARAM_MAS:
      // Aumentar valor del parámetro actual
      _increaseCurrentParameter();
      break;
      
    case NEXTION_ID_BTN_PARAM_ANTERIOR:
      // Cambiar al parámetro anterior
      _selectPreviousParameter();
      break;
      
    case NEXTION_ID_BTN_PARAM_SIGUIENTE:
      // Cambiar al parámetro siguiente
      _selectNextParameter();
      break;
      
    case NEXTION_ID_BTN_GUARDAR:
      // Guardar cambios y volver a selección
      Utils.debug("💾 Guardando cambios de edición");
      saveEditing();
      break;
      
    case NEXTION_ID_BTN_CANCELAR:
      // Cancelar edición y volver a selección
      Utils.debug("❌ Cancelando edición");
      cancelEditing();
      break;
      
    default:
      Utils.debug("⚠️ Componente no reconocido en página de edición: " + String(componentId));
      break;
  }
}

void ProgramControllerClass::_handleExecutionPageEvents(uint8_t componentId) {
  Utils.debug("⚙️ Evento en página de ejecución - Componente: " + String(componentId));
  
  switch (componentId) {
    case NEXTION_ID_BTN_PAUSAR:
      // Pausar/reanudar programa
      if (_currentState == ESTADO_EJECUCION) {
        Utils.debug("⏸️ Pausando programa");
        pauseProgram();
      } else if (_currentState == ESTADO_PAUSA) {
        Utils.debug("▶️ Reanudando programa");
        resumeProgram();
      }
      break;
      
    case NEXTION_ID_BTN_PARAR:
      // Detener programa completamente
      Utils.debug("⏹️ Deteniendo programa");
      stopProgram();
      break;
      
    default:
      Utils.debug("⚠️ Componente no reconocido en página de ejecución: " + String(componentId));
      break;
  }
}

/// @brief 
/// Actualiza el controlador de programa.
/// Esta función debe ser llamada periódicamente desde el loop principal del programa.
/// Se encarga de procesar eventos de usuario, actualizar el estado del controlador y manejar la lógica de la máquina de estados.
/// @details
/// - Procesa eventos de interfaz de usuario, como toques en la pantalla.
/// - Actualiza el estado del controlador de programa según la lógica de la máquina de estados.
/// - Llama a métodos auxiliares para manejar la edición de parámetros y la ejecución del programa.
/// - Debe ser llamada en el loop principal para asegurar que el controlador de programa funcione correctamente.
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


// ===== MÉTODOS AUXILIARES PARA EDICIÓN DE PARÁMETROS =====

void ProgramControllerClass::_decreaseCurrentParameter() {
  // Disminuir el valor del parámetro actual respetando límites
  switch (_editingParameter) {
    case PARAM_NIVEL:
      if (_editingParameterValue > MIN_NIVEL) {
        _editingParameterValue--;
        Utils.debug("🔽 Nivel disminuido a: " + String(_editingParameterValue));
      } else {
        Utils.debug("⚠️ Nivel ya está en el mínimo: " + String(MIN_NIVEL));
      }
      break;
      
    case PARAM_TEMPERATURA:
      if (_editingParameterValue > MIN_TEMPERATURA) {
        _editingParameterValue--;
        Utils.debug("🔽 Temperatura disminuida a: " + String(_editingParameterValue) + "°C");
      } else {
        Utils.debug("⚠️ Temperatura ya está en el mínimo: " + String(MIN_TEMPERATURA) + "°C");
      }
      break;
      
    case PARAM_TIEMPO:
      if (_editingParameterValue > MIN_TIEMPO) {
        _editingParameterValue--;
        Utils.debug("🔽 Tiempo disminuido a: " + String(_editingParameterValue) + " min");
      } else {
        Utils.debug("⚠️ Tiempo ya está en el mínimo: " + String(MIN_TIEMPO) + " min");
      }
      break;
      
    case PARAM_ROTACION:
      if (_editingParameterValue > MIN_ROTACION) {
        _editingParameterValue--;
        Utils.debug("🔽 Rotación disminuida a: " + String(_editingParameterValue));
      } else {
        Utils.debug("⚠️ Rotación ya está en el mínimo: " + String(MIN_ROTACION));
      }
      break;
  }
  
  _updateEditDisplay();
}

void ProgramControllerClass::_increaseCurrentParameter() {
  // Aumentar el valor del parámetro actual respetando límites
  switch (_editingParameter) {
    case PARAM_NIVEL:
      if (_editingParameterValue < MAX_NIVEL) {
        _editingParameterValue++;
        Utils.debug("🔼 Nivel aumentado a: " + String(_editingParameterValue));
      } else {
        Utils.debug("⚠️ Nivel ya está en el máximo: " + String(MAX_NIVEL));
      }
      break;
      
    case PARAM_TEMPERATURA:
      if (_editingParameterValue < MAX_TEMPERATURA) {
        _editingParameterValue++;
        Utils.debug("🔼 Temperatura aumentada a: " + String(_editingParameterValue) + "°C");
      } else {
        Utils.debug("⚠️ Temperatura ya está en el máximo: " + String(MAX_TEMPERATURA) + "°C");
      }
      break;
      
    case PARAM_TIEMPO:
      if (_editingParameterValue < MAX_TIEMPO) {
        _editingParameterValue++;
        Utils.debug("🔼 Tiempo aumentado a: " + String(_editingParameterValue) + " min");
      } else {
        Utils.debug("⚠️ Tiempo ya está en el máximo: " + String(MAX_TIEMPO) + " min");
      }
      break;
      
    case PARAM_ROTACION:
      if (_editingParameterValue < MAX_ROTACION) {
        _editingParameterValue++;
        Utils.debug("🔼 Rotación aumentada a: " + String(_editingParameterValue));
      } else {
        Utils.debug("⚠️ Rotación ya está en el máximo: " + String(MAX_ROTACION));
      }
      break;
  }
  
  _updateEditDisplay();
}

void ProgramControllerClass::_selectPreviousParameter() {
  // Cambiar al parámetro anterior en orden cíclico
  if (_editingParameter > 0) {
    _editingParameter--;
  } else {
    _editingParameter = 3; // Ir al último parámetro (PARAM_ROTACION)
  }
  
  // Cargar el valor actual del nuevo parámetro
  switch (_editingParameter) {
    case PARAM_NIVEL:
      _editingParameterValue = _waterLevels[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando NIVEL - Valor actual: " + String(_editingParameterValue));
      break;
    case PARAM_TEMPERATURA:
      _editingParameterValue = _temperatures[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando TEMPERATURA - Valor actual: " + String(_editingParameterValue) + "°C");
      break;
    case PARAM_TIEMPO:
      _editingParameterValue = _times[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando TIEMPO - Valor actual: " + String(_editingParameterValue) + " min");
      break;
    case PARAM_ROTACION:
      _editingParameterValue = _rotations[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando ROTACIÓN - Valor actual: " + String(_editingParameterValue));
      break;
  }
  
  _updateEditDisplay();
}

void ProgramControllerClass::_selectNextParameter() {
  // Cambiar al parámetro siguiente en orden cíclico
  if (_editingParameter < 3) {
    _editingParameter++;
  } else {
    _editingParameter = 0; // Ir al primer parámetro (PARAM_NIVEL)
  }
  
  // Cargar el valor actual del nuevo parámetro
  switch (_editingParameter) {
    case PARAM_NIVEL:
      _editingParameterValue = _waterLevels[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando NIVEL - Valor actual: " + String(_editingParameterValue));
      break;
    case PARAM_TEMPERATURA:
      _editingParameterValue = _temperatures[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando TEMPERATURA - Valor actual: " + String(_editingParameterValue) + "°C");
      break;
    case PARAM_TIEMPO:
      _editingParameterValue = _times[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando TIEMPO - Valor actual: " + String(_editingParameterValue) + " min");
      break;
    case PARAM_ROTACION:
      _editingParameterValue = _rotations[_editingProgram][_editingPhase];
      Utils.debug("📝 Editando ROTACIÓN - Valor actual: " + String(_editingParameterValue));
      break;
  }
  
  _updateEditDisplay();
}

void ProgramControllerClass::_updateEditDisplay() {
  // Actualizar la pantalla de edición con los valores actuales
  UIController.showEditScreen(_editingProgram, _editingPhase);
  
  Utils.debug("🖥️ Pantalla de edición actualizada:");
  Utils.debug("   Programa: " + String(_editingProgram + 22));
  Utils.debug("   Fase: " + String(_editingPhase + 1));
}
