// program_controller.cpp
#include "program_controller.h"
#include "debug.h"

// Instancia global
ProgramControllerClass ProgramController;

/// @brief Actualizar solo botones de programa (función auxiliar rápida)
void ProgramControllerClass::_updateProgramButtons() {
  // Actualizar estado de botones con delays aumentados para mejor comunicación Nextion
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM1, (_currentProgram == 0) ? 1 : 0);
  delay(25); // Delay aumentado para mejor comunicación
  
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM2, (_currentProgram == 1) ? 1 : 0);
  delay(25); // Delay aumentado para mejor comunicación
  
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM3, (_currentProgram == 2) ? 1 : 0);
  delay(25); // Delay aumentado para mejor comunicación
  
  // Actualizar también el componente que muestra el programa seleccionado
  Hardware.nextionSetText(NEXTION_COMP_PROGRAMA_SEL, "P" + String(_currentProgram + 22));
  delay(25); // Delay aumentado
  
  // Actualizar NEXTION_COMP_SET_PROG usando la función generarTextoPrograma
  char buffer[20];
  generarTextoPrograma(_currentProgram, buffer, sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_SET_PROG, buffer);
  delay(25); // Delay final para asegurar todas las actualizaciones
}

void ProgramControllerClass::init() {
  // Inicializar variables de estado
  _currentState = ESTADO_SELECCION;
  _previousState = ESTADO_SELECCION;
  _currentProgram = 0; // Inicializar con programa P22 (índice 0) por defecto
  _currentPhase = 0; // fase llenado

  // Inicializar variables de temporizador
  _remainingMinutes = 0;
  _remainingSeconds = 0;
  _totalMinutes = 0;
  _totalSeconds = 0;
  _timerRunning = false;

  // Inicializar variables de pausa
  _pausedMinutes = 0;
  _pausedSeconds = 0;
  _pauseActuatorsStopped = false;
  _pausedPreparingPhase = false;

  // Inicializar variables de estado de fase
  _preparingPhase = false;
  _phaseStartTime = 0;
  _currentPhaseState = FASE_LLENANDO;

  // Inicializar variables de secuencia final
  _finalDrainMinutes = 0;
  _finalDrainSeconds = 0;
  _doorWaitMinutes = 0;
  _doorWaitSeconds = 0;

  // Inicializar variables de edición
  _editingProgram = 0; // Inicializar con programa P22 (índice 0) por defecto
  _editingPhase = 0;
  _editingTanda = 0; // Inicializar con tanda 0 por defecto
  _editingParameter = 0;
  _editingParameterValue = 0;
  _isEditing = false;
  
  // Inicializar temporizadores no bloqueantes
  _pauseBlinkTaskId = -1;
  _errorBlinkTaskId = -1;
  _emergencyBlinkTaskId = -1;
  _emergencyDoorUnlockTaskId = -1;
  _blinkState = false;
  
  // Inicializar control de emergencia
  _emergencyTriggeredByButton = false;

  // Cargar datos de programa desde almacenamiento
  _loadProgramData();

}

/// @brief
/// Carga el estado inicial del programa desde el almacenamiento.
/// Este método solo carga el programa y fase actual, los demás datos se
/// obtienen directamente de Storage.
void ProgramControllerClass::_loadCurrentProgramState() {
  // Cargar el último programa y fase utilizados
  _currentProgram = Storage.loadProgram();
  _currentPhase = Storage.loadPhase();

}

/// @brief
/// Cambia el estado del controlador de programas.
/// Este método actualiza el estado actual del controlador de programas y
/// realiza las acciones necesarias al cambiar de estado, como actualizar la
/// interfaz de usuario y configurar los actuadores.
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
/// Este método es utilizado para gestionar el flujo del controlador de
/// programas, permitiendo que el sistema reaccione a eventos del usuario o
/// condiciones del sistema. Al cambiar de estado, se actualiza la interfaz de
/// usuario y se configuran los actuadores según el nuevo estado.
void ProgramControllerClass::setState(uint8_t newState) {
  if (newState != _currentState) {
    _previousState = _currentState;
    _currentState = newState;
    
    // Detener todos los temporizadores de parpadeo al cambiar estado
    _stopAllBlinkTasks();

    // Acciones específicas al cambiar de estado
    switch (newState) {
    case ESTADO_SELECCION:
      // Asegurar que todos los actuadores estén detenidos al volver a selección
      Actuators.stopMotor();
      Actuators.stopAutoRotation();
      Actuators.stopCentrifuge();
      Actuators.closeWaterValve();
      Actuators.closeSteamValve();
      Actuators.closeDrainValve();
      UIController.showSelectionScreen(_currentProgram);
      break;

    case ESTADO_EDICION:
      UIController.showEditScreen(_editingProgram, _editingPhase);
      break;

    case ESTADO_EJECUCION: {
      _initializeProgram();
      _configureActuatorsForPhase();
      // Para P24 usar _tandaCounter (siempre inicia en 0), para P22/P23 usar 0
      uint8_t tandaEjecucion = (_currentProgram == 2) ? _tandaCounter : 0;
      UIController.showExecutionScreen(
          _currentProgram, _currentPhase,
          _waterLevels[_currentProgram][_currentPhase],
          _temperatures[_currentProgram][_currentPhase],
          _rotations[_currentProgram][_currentPhase],
          tandaEjecucion);
      // Resetear tiempo a "00:00" SOLO cuando se inicia programa nuevo (no reanudación)
      Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, "00:00");
      
      // NO activar temporizador inmediatamente - esperar a que se cumplan condiciones
      // El temporizador se activa en _checkSensorConditions() cuando se cumplen las condiciones
      _timerRunning = false;
      _preparingPhase = true;
      break;
    }

    case ESTADO_PAUSA:
      Actuators.stopMotor();
      Actuators.stopAutoRotation();
      Hardware.nextionSetText(NEXTION_COMP_MSG, "Programa pausado");
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

    // Estado cambiado
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
////// Este método es utilizado para obtener el estado actual del controlador de
/// programas,
/// permitiendo que otros componentes del sistema puedan reaccionar a los
/// cambios de estado y actualizar la interfaz de usuario o realizar acciones
/// específicas según el estado.
/// @note
/// Este método es parte de la clase ProgramControllerClass y debe ser llamado
/// después de haber inicializado el controlador de programas con el método
/// init().
/// @see ProgramControllerClass::init()
uint8_t ProgramControllerClass::getState() { return _currentState; }

void ProgramControllerClass::selectProgram(uint8_t program) {
  if (program < NUM_PROGRAMAS) {
    _currentProgram = program;
    Storage.saveProgram(program);
    
    // Actualizar interfaz inmediatamente
    _updateProgramButtons();
    Debug.print("Programa seleccionado: P" + String(program + 22) + " - Botones actualizados");
  }
}

/// @brief
/// Obtiene el programa actual seleccionado.
/// Este método devuelve el número del programa que está actualmente
/// seleccionado. El programa se selecciona mediante el método
/// `selectProgram(uint8_t program)`.
/// @note
/// Este método es útil para saber qué programa está activo en el controlador de
/// programas, permitiendo que otros componentes del sistema puedan reaccionar a
/// los cambios de programa. Por ejemplo, la interfaz de usuario puede
/// actualizarse para mostrar los detalles del programa seleccionado.
/// @return
/// El número del programa actualmente seleccionado, que debe estar en el rango
/// de 1 a `NUM_PROGRAMAS - 1`. Si no se ha seleccionado ningún programa, el
/// valor por defecto es 0.
uint8_t ProgramControllerClass::getCurrentProgram() { return _currentProgram; }


void ProgramControllerClass::pauseProgram() {
  if (_currentState == ESTADO_EJECUCION) {
    // Preservar tiempo restante para continuar después
    _pausedMinutes = _remainingMinutes;
    _pausedSeconds = _remainingSeconds;

    // Preservar estado de preparación
    _pausedPreparingPhase = _preparingPhase;

    // Detener temporizador
    _timerRunning = false;

    // Cambiar estado a pausa
    setState(ESTADO_PAUSA);

    // Cambiar texto del botón a "REANUDAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "REANUDAR");

    // Tiempo preservado para reanudar
  }
}

void ProgramControllerClass::resumeProgram() {
  if (_currentState == ESTADO_PAUSA) {
    // Detener parpadeo de pausa
    if (_pauseBlinkTaskId != -1) {
      Utils.stopTask(_pauseBlinkTaskId);
      _pauseBlinkTaskId = -1;
    }
    
    // Resetear flag de pausa para permitir reinicio de actuadores
    _pauseActuatorsStopped = false;

    // Restaurar tiempo restante desde donde se pausó
    _remainingMinutes = _pausedMinutes;
    _remainingSeconds = _pausedSeconds;

    // Restaurar estado de preparación desde donde se pausó
    _preparingPhase = _pausedPreparingPhase;

    // Solo reactivar temporizador si no estaba en preparación
    _timerRunning = !_preparingPhase;

    // Cambiar estado directamente sin llamar a setState() para evitar
    // _initializeProgram()
    _previousState = _currentState;
    _currentState = ESTADO_EJECUCION;

    // Reconfigurar actuadores para la fase actual sin reinicializar
    _configureActuatorsForPhase();

    // Asegurar que la máquina de estados de fases esté sincronizada
    // Convertir _currentPhase a _currentPhaseState correspondiente
    switch (_currentPhase) {
    case 0:
      _currentPhaseState = FASE_LLENANDO;
      break;
    case 1:
      _currentPhaseState = FASE_LAVADO;
      break;
    case 2:
      _currentPhaseState = FASE_CENTRIFUGA;
      break;
    case 3:
      _currentPhaseState = FASE_DRENAJE;
      break;
    case 4:
      _currentPhaseState = FASE_ENFRIAMIENTO;
      break;
    }

    // Ya estamos en página de ejecución, solo actualizar elementos necesarios
    
    // Actualizar tiempo restaurado directamente (sin cambiar página)
    UIController.updateTime(_remainingMinutes, _remainingSeconds);

    // Cambiar texto del botón de vuelta a "PAUSAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "PAUSAR");
    Hardware.nextionSetText(NEXTION_COMP_MSG, "Programa reanudado");
  }
}

void ProgramControllerClass::stopProgram() {
  if (_currentState == ESTADO_EJECUCION || _currentState == ESTADO_PAUSA) {
    Actuators.emergencyStop(); // Detener todos los actuadores de forma segura
    setState(ESTADO_SELECCION);
  }
}

uint8_t ProgramControllerClass::getCurrentPhase() { return _currentPhase; }

uint8_t ProgramControllerClass::getCurrentEditingTanda() { return _editingTanda; }

void ProgramControllerClass::setEditingTanda(uint8_t tanda) {
  if (_editingProgram == 2 && tanda <= 3) { // Solo para P24 y tanda válida (0-3)
    _editingTanda = tanda;
    _updateTandaDisplay();
  }
}

void ProgramControllerClass::nextPhase() {
  if (_currentPhase < NUM_FASES - 1) {
    _currentPhase++;

    // === VERIFICAR SI LA SIGUIENTE FASE ES CENTRIFUGADO Y ESTÁ DESHABILITADA
    // === Fase 2 = Centrifugado
    if (_currentPhase == 2 &&
        !_isCentrifugadoEnabled(_currentProgram, _currentPhase)) {
      // Centrifugado está deshabilitado en esta tanda, saltar al drenaje (fase
      // 3)
      Debug.print(
          "⏭️ Saltando centrifugado (deshabilitado) - Avanzando a drenaje");
      _currentPhase++; // Avanzar a fase 3 (drenaje)
      Storage.savePhase(_currentPhase);
      _updatePhaseParameters();
      return;
    }

    Storage.savePhase(_currentPhase);
    _updatePhaseParameters();
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
    // Para P24 usar tanda actual, para P22/P23 usar fase
    uint8_t timeIndex = (_currentProgram == 2) ? _tandaCounter : _currentPhase;
    _totalMinutes = Storage.loadTime(_currentProgram, timeIndex);
    _totalSeconds = _totalMinutes * 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;

    // Reiniciar estado de preparación
    _preparingPhase = true;
    _phaseStartTime = millis();
    _timerRunning = false;

    // Actualizar la interfaz de usuario
    UIController.updatePhase(_currentPhase); // Mostrar la fase actual en la UI
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    // Nota: Barra de progreso eliminada del HMI

  }
}

void ProgramControllerClass::updateTimers() {
  if (_currentState == ESTADO_EJECUCION) {
    // Siempre verificar condiciones de los sensores durante preparación
    if (_preparingPhase) {
      _checkSensorConditions();
    }

    // Decrementar temporizador principal si está corriendo
    if (_timerRunning) {
      _decrementTimer();
      UIController.updateTime(_remainingMinutes, _remainingSeconds);
    }

    // Siempre actualizar actuadores
    Actuators.updateTimers();
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
    // Transición manejada por la máquina de estados de fases
    return;
  }

  // Actualizar la interfaz de usuario
  UIController.updateTime(_remainingMinutes, _remainingSeconds);

  // Nota: El progreso se actualiza en _handleExecutionState() usando
  // getProgressPercentage()
}

/// @brief
/// Verifica las condiciones de los sensores y actualiza los actuadores según la
/// fase actual. Este método se encarga de verificar si se han alcanzado las
/// condiciones de temperatura y nivel de agua
void ProgramControllerClass::_checkSensorConditions() {
  // Actualizar la interfaz con los valores actuales
  UIController.updateTemperature(Sensors.getCurrentTemperature());
  UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());

  // Solo verificar condiciones si estamos preparando la fase
  if (!_preparingPhase) {
    return;
  }

  // Obtener valores objetivo para la fase actual
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, 0, _currentPhase);
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, 0, _currentPhase);

  // Determinar si se requiere control de temperatura
  bool requiresTempControl = false;
  switch (_currentProgram) {
  case 0: // P22 - Agua caliente
    requiresTempControl = true;
    break;
  case 1: // P23 - Agua fría
    requiresTempControl = false;
    break;
  case 2: // P24 - Configurable por fase
    requiresTempControl =
        (Storage.loadTipoAgua(_currentProgram, 0, _currentPhase) == 1);
    break;
  }

  // Verificar si se alcanzaron las condiciones
  bool levelReached = Sensors.isWaterLevelReached(targetLevel);
  bool tempReached =
      !requiresTempControl || Sensors.isTemperatureReached(targetTemp);

  if (levelReached && tempReached) {
    // Condiciones alcanzadas
    _preparingPhase = false;

    // FASE 0 (Llenado) → Automáticamente a FASE_LAVADO
    if (_currentPhaseState == FASE_LLENANDO) {
      _currentPhaseState = FASE_LAVADO;
      _initializePhaseState();
    } else {
      // Para otras fases, solo iniciar temporizador
      _timerRunning = true;
    }

    Hardware.nextionSetText(NEXTION_COMP_MSG,
                            "Fase " + String(_currentPhase + 1) + " iniciada");
  } else {
    // Mostrar estado de preparación
    String statusMsg = "Preparando: ";
    if (!levelReached)
      statusMsg += "Llenando... ";
    if (!tempReached && requiresTempControl)
      statusMsg += "Calentando... ";
    Hardware.nextionSetText(NEXTION_COMP_MSG, statusMsg);
  }
}

/// @brief
/// Verificaciones de seguridad crítica que pueden activar emergencia automática
/// Debe ser llamado periódicamente durante la ejecución del programa
void ProgramControllerClass::_checkCriticalSafety() {
  // Solo verificar durante ejecución - no interferir con otros estados
  if (_currentState != ESTADO_EJECUCION && _currentState != ESTADO_PAUSA) {
    return;
  }
  
  // 1. VERIFICACIÓN DE SOBRECALENTAMIENTO CRÍTICO
  float currentTemp = Sensors.getCurrentTemperature();
  if (currentTemp >= TEMPERATURA_EMERGENCIA) {
    Debug.print("🚨 EMERGENCIA: Sobrecalentamiento detectado - " + String(currentTemp) + "°C");
    handleEmergency(false); // Emergencia por software
    return;
  }
  
  // 2. VERIFICACIÓN DE FALLA CRÍTICA DE SENSORES
  // Si no podemos leer temperatura durante programa con agua caliente = peligroso
  if (currentTemp < -50 || currentTemp > 150) { // Valores imposibles = sensor fallando
    // Solo emergencia si es programa con agua caliente (crítico para seguridad)
    bool isHotWaterProgram = (_currentProgram == 0) || // P22 siempre caliente
                             (_currentProgram == 2 && Storage.loadTipoAgua(_currentProgram, _tandaCounter, _currentPhase) == 1); // P24 caliente
    
    if (isHotWaterProgram) {
      Debug.print("🚨 EMERGENCIA: Falla crítica sensor temperatura en programa agua caliente");
      handleEmergency(false); // Emergencia por software
      return;
    }
  }
  
  // 3. VERIFICACIÓN DE NIVEL DE AGUA CRÍTICO
  uint8_t currentLevel = Sensors.getCurrentWaterLevel();
  if (currentLevel >= 100) { // Nivel excesivo = posible desbordamiento
    Debug.print("🚨 EMERGENCIA: Nivel de agua crítico detectado - " + String(currentLevel) + "%");
    handleEmergency(false); // Emergencia por software
    return;
  }
  
  // 4. VERIFICACIÓN DE TIEMPO EXCESIVO EN FASE (posible bloqueo)
  unsigned long phaseTime = millis() - _phaseStartTime;
  if (phaseTime > 3600000) { // Más de 1 hora en una fase = anómalo
    Debug.print("🚨 EMERGENCIA: Tiempo excesivo en fase - " + String(phaseTime/60000) + " minutos");
    handleEmergency(false); // Emergencia por software
    return;
  }
}

void ProgramControllerClass::_controlActuatorsForPhase() {
  // Control de actuadores para las fases del programa

  // Obtener valores objetivo para la fase actual
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, 0, _currentPhase);
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, 0, _currentPhase);

  // Determinar si se requiere control de temperatura
  bool requiresTempControl = false;
  uint8_t tipoAgua = 0; // 0=fría, 1=caliente

  // Determinar si se requiere control según programa y fase
  switch (_currentProgram) {
  case 0: // P22 - Agua caliente
    requiresTempControl = true;
    tipoAgua = 1;
    break;

  case 1: // P23 - Agua fría
    requiresTempControl = false;
    tipoAgua = 0;
    break;

  case 2: // P24 - Configurable por fase
    // Verificar el tipo de agua configurado para esta fase
    tipoAgua = Storage.loadTipoAgua(_currentProgram, _tandaCounter, _currentPhase);
    requiresTempControl = (tipoAgua == 1); // Solo si usa agua caliente
    break;
  }

  // === CONTROL DE ACTUADORES SEGÚN FASE ===
  if (_currentPhase == 0) {
    // FASE 0: LLENADO - Control de nivel según tipo de agua

    if (tipoAgua == 1 && requiresTempControl) {
      // AGUA CALIENTE (P22): Usar válvula de vapor (agua caliente)
      if (Sensors.getCurrentWaterLevel() < targetLevel) {
        Actuators.openSteamValve();  // Válvula de agua caliente
        Actuators.closeWaterValve(); // Cerrar válvula de agua fría
      } else {
        Actuators.closeSteamValve(); // Cerrar válvula de agua caliente
      }
    } else {
      // AGUA FRÍA (P23): Usar válvula de agua fría
      if (Sensors.getCurrentWaterLevel() < targetLevel) {
        Actuators.openWaterValve();  // Válvula de agua fría
        Actuators.closeSteamValve(); // Cerrar válvula de agua caliente
      } else {
        Actuators.closeWaterValve(); // Cerrar válvula de agua fría
      }
    }
  } else if (_currentPhase == 1) {
    // FASE 1: LAVADO - Mantener condiciones
    // Durante lavado, ambas válvulas deben estar cerradas
    // El agua ya se llenó en la fase anterior
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
  } else if (_currentPhase == 3) {
    // FASE 3: DRENAJE - Mantener válvula abierta durante todo el tiempo configurado
    // Durante la fase de drenaje, la válvula debe permanecer abierta
    // independientemente del nivel de agua para asegurar drenaje completo
    Actuators.openDrainValve();
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
  }
  // Fase 2 (centrifugado) se maneja en la máquina de estados de fases
}

void ProgramControllerClass::_completeProgram() {
  // El enfriamiento ya se maneja en FASE_ENFRIAMIENTO de la máquina de estados
  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::_initializeProgram() {
  // === INICIALIZACIÓN SEGÚN DOCUMENTO DEL CLIENTE ===

  // 1. Bloquear puerta (ya verificada en startProgram)
  Actuators.lockDoor();

  // 2. Inicializar fase al comienzo (LLENADO)
  _currentPhase = 0; // Siempre comenzar desde fase 0 (llenado)
  Storage.savePhase(_currentPhase);

  // 3. Inicializar variables del programa
  // Para P24 usar tanda actual, para P22/P23 usar fase
  uint8_t timeIndex = (_currentProgram == 2) ? _tandaCounter : _currentPhase;
  _totalMinutes = Storage.loadTime(_currentProgram, timeIndex);
  _totalSeconds = _totalMinutes * 60;
  _remainingMinutes = _totalMinutes;
  _remainingSeconds = 0;
  _timerRunning = false;

  // 4. Cargar parámetros del programa según tipo
  // _configureProgramType();

  // 5. Inicializar estado de preparación
  _preparingPhase = true;
  _phaseStartTime = millis();

  // 6. Inicializar contador de tandas
  _tandaCounter = 0; // Tanda inicial (mostrada como "1" en pantalla)
  _maxTandas = (_currentProgram == 2) ? 4 : 1; // P24=4 tandas, P22/P23=1 tanda
  // Nota: La tanda se actualiza en showExecutionScreen() al inicio

  // 7. Actualizar UI para mostrar la fase inicial correcta (llenado)
  UIController.updatePhase(
      _currentPhase); // Asegurar que muestre fase 0 (llenado)

  // 8. Inicializar máquina de estados de fases
  _currentPhaseState = FASE_LLENANDO;
}

void ProgramControllerClass::_handleTemperatureControl() {
  // Solo para programas que requieren control de temperatura
  bool requiresTempControl = false;
  uint8_t tipoAgua = 0; // 0=fría, 1=caliente

  // Determinar si se requiere control según programa y fase
  switch (_currentProgram) {
  case 0: // P22 - Agua caliente
    requiresTempControl = true;
    tipoAgua = 1;
    break;

  case 1: // P23 - Agua fría
    requiresTempControl = false;
    tipoAgua = 0;
    break;

  case 2: // P24 - Configurable por fase
    // Verificar el tipo de agua configurado para esta fase
    tipoAgua = _tipoAguaPrograma[_currentProgram][_currentPhase];
    requiresTempControl = (tipoAgua == 1); // Solo si usa agua caliente
    break;
  }

  if (!requiresTempControl) {
    return; // No necesita control de temperatura
  }

  float currentTemp = Sensors.getCurrentTemperature();
  float targetTemp = Storage.loadTemperature(_currentProgram, 0, _currentPhase);

  if (currentTemp < targetTemp - 2) {
    // Utils.debug("🌡️ Temperatura baja: " + String(currentTemp) + "°C,
    // objetivo: " + String(targetTemp) + "°C");

    // Paso 1: Drenar parcialmente para hacer espacio al agua caliente
    if (Sensors.getCurrentWaterLevel() > 1) {
      // Utils.debug("💧 Drenando parcialmente para renovar agua...");
      Actuators.openDrainValve();

      // Esperar a que baje el nivel (implementación simplificada)
      static unsigned long drainStartTime = millis();
      if (millis() - drainStartTime > 5000) { // 5 segundos de drenaje
        Actuators.closeDrainValve();

        // Paso 2: Abrir válvula de agua caliente
        // Utils.debug("🔥 Abriendo válvula de agua caliente...");
        Actuators.openWaterValve();

        // Paso 3: Activar vapor si es necesario
        if (!Actuators.isSteamValveOpen()) {
          Actuators.openSteamValve();
          // Utils.debug("🔥 Activando vapor para acelerar calentamiento");
        }

        drainStartTime = millis(); // Reset timer
      }
    } else {
      // Si el nivel ya es bajo, solo activar calentamiento
      if (!Actuators.isSteamValveOpen()) {
        Actuators.openSteamValve();
        // Utils.debug("🔥 Activando calentamiento directo");
      }
    }
  } else if (currentTemp > targetTemp + 2) {
    // Temperatura alta - detener calentamiento
    if (Actuators.isSteamValveOpen()) {
      Actuators.closeSteamValve();
      // Utils.debug("❄️ Deteniendo calentamiento - Temp OK");
    }
  }

  // Cerrar válvula de agua si se alcanzó el nivel objetivo
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, 0, _currentPhase);
  if (Sensors.getCurrentWaterLevel() >= targetLevel) {
    Actuators.closeWaterValve();
  }
}

bool ProgramControllerClass::_isCentrifugadoEnabled(uint8_t programa,
                                                    uint8_t faseActual) {
  // Verificar si el centrifugado está habilitado para este programa en la tanda
  // actual El centrifugado es opcional en la fase 2 de cada tanda
  if (programa >= NUM_PROGRAMAS) {
    return false;
  }

  // Solo evaluar centrifugado en la fase 2 (fase de centrifugado)
  if (faseActual != 2) {
    return false; // El centrifugado solo aplica en la fase 2
  }

  uint8_t tandaActual;
  if (programa == 2) { // P24 - usar tanda actual (0, 1, 2)
    tandaActual = _tandaCounter;
  } else { // P22, P23 - solo tienen 1 tanda (índice 0)
    tandaActual = 0;
  }

  // Verificar que el índice de tanda sea válido
  if (tandaActual >= NUM_FASES) {
    return false;
  }

  bool enabled = _centrifugadoPorTanda[programa][tandaActual] == 1;

  return enabled;
}

void ProgramControllerClass::_configureActuatorsForPhase() {
  // Configurar actuadores según el flujo específico del Programa 22
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, 0, _currentPhase);
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, 0, _currentPhase);

  // ESTADO INICIAL: Al presionar "iniciar" (antes de que llegue agua y
  // temperatura)
  if (_preparingPhase) {
    // 1) PIN_VALVULA_DESFOGUE OFF - debe llenar agua
    Actuators.closeDrainValve();

    // 2) Temporizador OFF (ya manejado en _timerRunning = false)

    // 3) PIN_ELECTROV_VAPOR ON - ingresa agua caliente (solo Programa 22)
    if (_currentProgram == 0) { // P22 = agua caliente
      if (Sensors.getCurrentWaterLevel() < targetLevel)
        Actuators.openSteamValve();
    } else if (_currentProgram == 1) { // P23 = agua fría
      if (Sensors.getCurrentWaterLevel() < targetLevel)
        Actuators.openWaterValve(); // P23 no usa vapor, solo agua fría
    }

    // 4) El sensor de temperatura indica si llegó a temperatura seteada
    // (manejado en _checkSensorConditions)

    // 5) PIN_VALVULA_AGUA OFF cuando llegue al nivel (manejado en
    // _checkSensorConditions)

    // 6) PIN_CENTRIFUGADO OFF
    Actuators.stopCentrifuge();

    // 7) Motores OFF - comenzarán cuando llegue nivel de agua
    Actuators.stopMotor();

    return; // No continuar con lógica normal mientras se prepara
  }

  // CUANDO YA SE ALCANZARON LAS CONDICIONES (nivel + temperatura)
  if (_timerRunning) {
    // Configurar actuadores según la fase específica
    switch (_currentPhase) {
    case 1: // FASE LAVADO
    {
      Actuators.closeSteamValve();
      Actuators.closeWaterValve();
      Actuators.closeDrainValve(); // Mantener agua
      Actuators.stopCentrifuge();

      // Activar rotación
      uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
      if (rotLevel > 0 && !Actuators.isAutoRotationActive()) {
        Actuators.startAutoRotation(rotLevel);
      }
    } break;

    case 2: // FASE CENTRIFUGADO
    {
      Actuators.closeSteamValve();
      Actuators.closeWaterValve();
      Actuators.closeDrainValve();
      Actuators.stopAutoRotation();
      Actuators.stopMotor();
      Actuators.startCentrifuge();
    } break;

    case 3: // FASE DRENAJE
    {
      Actuators.closeSteamValve();
      Actuators.closeWaterValve();
      Actuators.openDrainValve(); // Activar drenaje
      Actuators.stopAutoRotation();
      Actuators.stopMotor();
      Actuators.stopCentrifuge();
    } break;

    case 4: // FASE ENFRIAMIENTO
    {
      Actuators.closeSteamValve();
      Actuators.closeWaterValve();
      Actuators.openDrainValve(); // Mantener drenaje abierto
      Actuators.stopAutoRotation();
      Actuators.stopMotor();
      Actuators.stopCentrifuge();
    } break;

    default:
      // Para otras fases, configuración conservadora
      {
        Actuators.stopAutoRotation();
        Actuators.stopMotor();
      }
      break;
    }
  }

  Utils.debug("✅ Actuadores configurados para P" +
              String(_currentProgram + 22) + " F" + String(_currentPhase));
}

uint8_t ProgramControllerClass::getProgressPercentage() {
  // Durante preparación de fase, mostrar 0%
  if (_preparingPhase) {
    return 0;
  }

  // Durante ejecución, calcular progreso basado en tiempo
  if (_totalSeconds == 0)
    return 0;

  uint16_t remainingTotal = (_remainingMinutes * 60) + _remainingSeconds;
  
  // Usar aritmética de 32 bits para evitar overflow en la multiplicación
  uint32_t progressCalc = (uint32_t)(remainingTotal) * 100;
  uint8_t progress = 100 - (progressCalc / _totalSeconds);

  return progress;
}

uint8_t ProgramControllerClass::getTotalProgramProgressPercentage() {
  // Progreso simplificado basado en fase actual
  if (_currentPhase >= NUM_FASES) {
    return 100;
  }

  uint8_t baseProgress = (_currentPhase * 100) / NUM_FASES;
  uint8_t phaseProgress = getProgressPercentage() / NUM_FASES;

  return baseProgress + phaseProgress;
}

/// @brief
/// Inicia el modo de edición para un programa y fase específicos.
/// Este método permite al usuario editar los parámetros de un programa
/// específico,
/// @param program
/// El número del programa a editar (0 a 2).
/// @param phase
/// El número de la fase a editar (0 a 3).
void ProgramControllerClass::startEditing(uint8_t program, uint8_t phase) {
  if (program < NUM_PROGRAMAS && phase < NUM_FASES) {
    // Sincronizar arrays internos con Storage antes de editar
    _loadProgramData();
    
    _editingProgram = program;
    _editingPhase = phase;
    _editingTanda = 0; // Inicializar con tanda 0 (P22/P23: solo 1 tanda, P24: primera de 3)
    _editingParameter = PARAM_NIVEL; // Comenzar editando el nivel
    _editingParameterValue =
        _waterLevels[program][phase]; // Cargar valor actual (ahora sincronizado)
    _isEditing = true;

    setState(ESTADO_EDICION);

    // Actualizar UI específica según el programa
    _updateEditScreenForProgram();
  }
}

void ProgramControllerClass::_updateEditScreenForProgram() {
  // Configurar pantalla de edición según el programa
  if (_editingProgram <= 1) {
    // P22/P23: Solo 1 tanda, mostrar "1" fijo
    Hardware.nextionSetText(NEXTION_COMP_SET_FASE, "1"); // Mostrar "1" (1 tanda)
    // Nota: Para deshabilitar visualmente el componente se podría cambiar color o usar otro método
  } else {
    // P24: 3 tandas, mostrar tanda actual
    Hardware.nextionSetText(NEXTION_COMP_SET_FASE, String(_editingTanda + 1)); // Mostrar tanda actual (1-3)
  }
  
  // Cargar y mostrar los valores de la tanda actual
  _loadEditingParametersForCurrentTanda();
}

void ProgramControllerClass::_loadEditingParametersForCurrentTanda() {
  // Cargar parámetros directamente desde Storage para evitar valores desactualizados
  uint8_t tandaIndex = _editingTanda;
  
  if (_editingProgram <= 1) {
    // P22/P23: usar _editingPhase como índice (solo 1 tanda)
    tandaIndex = _editingPhase;
  }
  
  // ELIMINADO: No actualizar aquí para evitar doble actualización
  // Los valores se actualizarán una sola vez cuando UIController.showEditScreen() 
  // llame a updateEditDisplay() -> updateEditPanel(UPDATE_FULL)
  
  // Solo cargar valores internos para uso de ProgramController
  uint8_t nivelStorage = Storage.loadWaterLevel(_editingProgram, 0, tandaIndex);
  uint8_t tempStorage = Storage.loadTemperature(_editingProgram, 0, tandaIndex);
  uint8_t tiempoStorage = Storage.loadTime(_editingProgram, tandaIndex);
  uint8_t rotacionStorage = Storage.loadRotation(_editingProgram, tandaIndex);
  uint8_t aguaStorage = Storage.loadTipoAgua(_editingProgram, 0, tandaIndex);
  uint8_t centrifugaStorage = Storage.loadCentrifugado(_editingProgram, tandaIndex);
  
  // Sincronizar variables internas sin actualizar pantalla
  _waterLevels[_editingProgram][tandaIndex] = nivelStorage;
  _temperatures[_editingProgram][tandaIndex] = tempStorage;
  _times[_editingProgram][tandaIndex] = tiempoStorage;
  _rotations[_editingProgram][tandaIndex] = rotacionStorage;
}

void ProgramControllerClass::editParameter(uint8_t paramType, uint8_t value) {
  if (!_isEditing)
    return;

  // Determinar el índice correcto según el programa
  uint8_t storageIndex = _editingPhase; // P22/P23 usan fase
  if (_editingProgram == 2) { // P24 usa tanda
    storageIndex = _editingTanda;
  }

  // Guardar el nuevo valor según el tipo de parámetro
  switch (paramType) {
  case PARAM_NIVEL: // Nivel de agua
    _waterLevels[_editingProgram][storageIndex] = value;
    Storage.saveWaterLevel(_editingProgram, 0, storageIndex, value);
    break;

  case PARAM_TEMPERATURA: // Temperatura
    _temperatures[_editingProgram][storageIndex] = value;
    Storage.saveTemperature(_editingProgram, 0, storageIndex, value);
    break;

  case PARAM_TIEMPO: // Tiempo
    _times[_editingProgram][storageIndex] = value;
    Storage.saveTime(_editingProgram, storageIndex, value);
    break;

  case PARAM_ROTACION: // Rotación
    _rotations[_editingProgram][storageIndex] = value;
    Storage.saveRotation(_editingProgram, storageIndex, value);
    break;

  case PARAM_AGUA: // Tipo de agua
    _tipoAguaPrograma[_editingProgram][storageIndex] = value;
    Storage.saveTipoAgua(_editingProgram, 0, storageIndex, value);
    break;

  case PARAM_CENTRIF: // Centrifugado
    _centrifugadoPorTanda[_editingProgram][storageIndex] = value;
    Storage.saveCentrifugado(_editingProgram, storageIndex, value);
    break;
  }

  // Actualizar la pantalla con el nuevo valor
  _loadEditingParametersForCurrentTanda();
  
  Utils.debug("Parámetro editado para P" + String(_editingProgram + 22) + 
             " Tanda " + String(storageIndex + 1) + ": " + String(value));
}

void ProgramControllerClass::saveEditing() {
  if (!_isEditing)
    return;

  // Guardar el valor del parámetro editado en la estructura de datos
  switch (_editingParameter) {
  case PARAM_NIVEL:
    _waterLevels[_editingProgram][_editingPhase] = _editingParameterValue;
    Utils.debug("💾 Nivel guardado: " + String(_editingParameterValue));
    break;
  case PARAM_TEMPERATURA:
    _temperatures[_editingProgram][_editingPhase] = _editingParameterValue;
    Utils.debug("💾 Temperatura guardada: " + String(_editingParameterValue) +
                "°C");
    break;
  case PARAM_TIEMPO:
    _times[_editingProgram][_editingPhase] = _editingParameterValue;
    Utils.debug("💾 Tiempo guardado: " + String(_editingParameterValue) +
                " min");
    break;
  case PARAM_ROTACION:
    _rotations[_editingProgram][_editingPhase] = _editingParameterValue;
    Utils.debug("💾 Rotación guardada: " + String(_editingParameterValue));
    break;
  }

  // Guardar en almacenamiento persistente usando métodos individuales
  Storage.saveWaterLevel(_editingProgram, 0, _editingPhase,
                         _waterLevels[_editingProgram][_editingPhase]);
  Storage.saveTemperature(_editingProgram, 0, _editingPhase,
                          _temperatures[_editingProgram][_editingPhase]);
  Storage.saveTime(_editingProgram, _editingPhase,
                   _times[_editingProgram][_editingPhase]);
  Storage.saveRotation(_editingProgram, _editingPhase,
                       _rotations[_editingProgram][_editingPhase]);

  _isEditing = false;

  // Recargar todos los datos para asegurar consistencia
  _loadProgramData();

  setState(ESTADO_SELECCION);

  // Mostrar pantalla de selección actualizada
  UIController.showSelectionScreen(
      _editingProgram); // Pasar índice directamente

  Utils.debug("✅ Edición guardada exitosamente");
}

void ProgramControllerClass::cancelEditing() {
  _isEditing = false;
  // Recargar valores originales
  _loadProgramData();
  setState(ESTADO_SELECCION);
  Utils.debug("Edición cancelada");
}

void ProgramControllerClass::endEditing() {
  _isEditing = false;

  // Asegurar que _currentProgram sea el programa que se estaba editando
  _currentProgram = _editingProgram;

  Utils.debug("✅ Edición terminada para P" + String(_editingProgram + 22) +
              ", volviendo a selección");
  Utils.debug("   Actualizando _currentProgram a: " + String(_currentProgram));

  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::processUserEvent(const String &event) {
  // Debug básico del evento recibido
  Utils.debug("📥 Evento recibido - Procesando: " + event);
  
  // Obtener datos del evento
  uint8_t touchPage = Hardware.getTouchEventPage();
  uint8_t touchComponent = Hardware.getTouchEventComponent();

  // FLUJO SIMPLIFICADO: Botón START → validateConditions() → executeStart()
  if (touchComponent == NEXTION_ID_BTN_START && touchPage == NEXTION_PAGE_SELECTION) {
    Utils.debug("🎯 BOTÓN START PRESIONADO");
    if (validateConditions()) {
      executeStart();
    }
    return; // Flujo simplificado completo
  }
  
  // Para otros eventos, usar lógica original simplificada
  if (!Hardware.hasValidTouchEvent()) {
    // Solo mostrar debug para eventos no-START 
    return;
  }

  // Procesar eventos según la página actual
  switch (touchPage) {
  case NEXTION_PAGE_SELECTION:
    _handleSelectionPageEvents(touchComponent);
    break;

  case NEXTION_PAGE_EDIT:
    _handleEditPageEvents(touchComponent);
    break;

  case NEXTION_PAGE_EXECUTION:
    if (touchComponent == NEXTION_ID_BTN_PARAR || touchComponent == NEXTION_ID_BTN_PAUSAR) {
      Utils.debug("🎯 Botón control - Comp:" + String(touchComponent) + 
                  " Estado:" + String(_currentState) + 
                  " Prep:" + String(_preparingPhase ? "Sí" : "No"));
    }
    _handleExecutionPageEvents(touchComponent);
    break;

  case NEXTION_PAGE_EMERGENCY:
    _handleEmergencyPageEvents(touchComponent);
    break;

  default:
    Utils.debug("⚠️ Página no manejada: " + String(touchPage));
    break;
  }
}

// ===== FUNCIONES DE VALIDACIÓN OPTIMIZADAS =====

bool ProgramControllerClass::_validateStartConditions(const String& context) {
  Utils.debug("🔍 VALIDANDO CONDICIONES DE INICIO" + (context.length() > 0 ? " (" + context + ")" : ""));
  
  // VALIDACIÓN 1: Evento táctil válido
  if (!Hardware.hasValidTouchEvent()) {
    Utils.debug("❌ VALIDACIÓN 1 FALLÓ: Evento Nextion inválido");
    return false;
  }
  
  // VALIDACIÓN 2: Página correcta
  uint8_t touchPage = Hardware.getTouchEventPage();
  if (touchPage != NEXTION_PAGE_SELECTION) {
    Utils.debug("❌ VALIDACIÓN 2 FALLÓ: Página incorrecta - Actual:" + String(touchPage) + 
                " Esperada:" + String(NEXTION_PAGE_SELECTION));
    return false;
  }
  
  // VALIDACIÓN 3: Componente correcto
  uint8_t touchComponent = Hardware.getTouchEventComponent();
  if (touchComponent != NEXTION_ID_BTN_START) {
    Utils.debug("❌ VALIDACIÓN 3 FALLÓ: Componente incorrecto - Actual:" + String(touchComponent) + 
                " Esperado:" + String(NEXTION_ID_BTN_START));
    return false;
  }
  
  // VALIDACIÓN 4: Tipo de evento correcto (presionado)
  uint8_t touchType = Hardware.getTouchEventType();
  if (touchType != 1) {
    Utils.debug("❌ VALIDACIÓN 4 FALLÓ: Tipo evento incorrecto - Actual:" + String(touchType) + " (debe ser 1)");
    return false;
  }
  
  // VALIDACIÓN 5: Estado del sistema correcto
  if (_currentState != ESTADO_SELECCION) {
    Utils.debug("❌ VALIDACIÓN 5 FALLÓ: Estado incorrecto - Actual:" + String(_currentState) + 
                " Esperado:" + String(ESTADO_SELECCION));
    return false;
  }
  
  Utils.debug("✅ TODAS LAS VALIDACIONES PASARON - Sistema listo para inicio");
  return true;
}

// ===== FLUJO SIMPLIFICADO (NUEVA IMPLEMENTACIÓN) =====

bool ProgramControllerClass::validateConditions() {
  // Validación 1: Estado del sistema
  if (_currentState != ESTADO_SELECCION) {
    Utils.debug("❌ Estado incorrecto: " + String(_currentState) + " (debe ser SELECCION)");
    return false;
  }
  
  // Validación 2: Puerta cerrada
  if (!Sensors.isDoorClosed()) {
    Utils.debug("❌ Puerta abierta - No se puede iniciar");
    return false;
  }
  
  // Validación 3: No hay errores críticos
  if (Sensors.getCurrentTemperature() >= TEMPERATURA_EMERGENCIA) {
    Utils.debug("❌ Temperatura crítica detectada");
    return false;
  }
  
  Utils.debug("✅ Todas las condiciones válidas - Listo para iniciar");
  return true;
}

void ProgramControllerClass::executeStart() {
  Utils.debug("🚀 EJECUTANDO INICIO DIRECTO - P" + String(_currentProgram + 22));
  
  // 1. Inicializar variables de programa
  _currentPhase = 0; // Comenzar desde llenado
  _preparingPhase = true;
  _phaseStartTime = millis();
  _currentPhaseState = FASE_LLENANDO;
  
  // 2. Inicializar temporizadores
  uint8_t timeIndex = (_currentProgram == 2) ? _tandaCounter : 0;
  _totalMinutes = Storage.loadTime(_currentProgram, timeIndex);
  _totalSeconds = _totalMinutes * 60;
  _remainingMinutes = _totalMinutes;
  _remainingSeconds = 0;
  _timerRunning = false;
  
  // 3. Inicializar contador de tandas
  _tandaCounter = 0; // Tanda inicial
  _maxTandas = (_currentProgram == 2) ? 4 : 1;
  
  // 4. Bloquear puerta y configurar actuadores
  Actuators.lockDoor();
  _configureActuatorsForPhase();
  
  // 5. Cambiar estado y mostrar pantalla
  setState(ESTADO_EJECUCION);
  
  uint8_t tandaEjecucion = (_currentProgram == 2) ? _tandaCounter : 0;
  UIController.showExecutionScreen(
      _currentProgram, _currentPhase,
      Storage.loadWaterLevel(_currentProgram, 0, _currentPhase),
      Storage.loadTemperature(_currentProgram, 0, _currentPhase),
      Storage.loadRotation(_currentProgram, _currentPhase),
      tandaEjecucion);
  
  // Reset tiempo en pantalla
  Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, "00:00");
  
  Utils.debug("✅ PROGRAMA INICIADO EXITOSAMENTE");
}

void ProgramControllerClass::_handleStateMachine() {
  // Máquina de estados principal que coordina todo el comportamiento del
  // sistema Se ejecuta en cada ciclo del loop principal

  switch (_currentState) {
  case ESTADO_IDLE:
    // Estado de reposo - Sistema esperando acción del usuario
    // No requiere procesamiento activo
    break;

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

  default:
    // Estado desconocido - regresar a IDLE por seguridad
    Utils.debug("⚠️ Estado desconocido: " + String(_currentState) +
                ", regresando a IDLE");
    setState(ESTADO_IDLE);
    break;
  }
}

void ProgramControllerClass::_handleSelectionState() {
  // Estado de selección - El usuario está navegando entre programas
  // Verificar si hay timeout de inactividad (opcional)

  // En este estado, el procesamiento principal se hace mediante eventos
  // táctiles que ya están siendo manejados por processUserEvent()

  // Actualizar display si es necesario
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) { // Actualizar cada segundo
    // Actualizar información del programa actual en la UI
    UIController.updateProgramInfo(_currentProgram +
                                   1); // Convertir a 1-3 para UI
    lastDisplayUpdate = millis();
  }
}

void ProgramControllerClass::_handleEditingState() {
  // Estado de edición - El usuario está modificando parámetros del programa

  // TIMEOUT DE EDICIÓN DESHABILITADO POR SOLICITUD DEL USUARIO
  // El usuario puede permanecer en edición indefinidamente
  // Solo sale manualmente con Guardar o Cancelar

  // El procesamiento principal de edición se hace mediante eventos táctiles
  // manejados en UIController
}

void ProgramControllerClass::_handleExecutionState() {
  // Estado de ejecución - El programa de lavado está activo

  // 1. Verificar condiciones de seguridad
  if (!Actuators.isDoorLocked()) {
    Utils.debug("⚠️ Puerta no asegurada durante ejecución");
    _triggerError(ERROR_PUERTA, "Puerta abierta durante ejecución");
    return;
  }

  // 2. Ejecutar máquina de estados de fases (maneja todo el flujo)
  _handlePhaseStateMachine();
}

void ProgramControllerClass::_handlePauseState() {
  // Estado de pausa - El programa está detenido temporalmente

  // Asegurar que todos los actuadores estén detenidos
  if (!_pauseActuatorsStopped) {
    Actuators.stopAutoRotation(); // Detener permutación durante pausa
    Actuators.stopMotor();
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
    // Mantener puerta bloqueada por seguridad
    _pauseActuatorsStopped = true;
    Utils.debug("⏸️ Sistema en pausa - actuadores detenidos");
    
    // Iniciar parpadeo no bloqueante
    if (_pauseBlinkTaskId == -1) {
      _pauseBlinkTaskId = Utils.createInterval(250, []() {
        ProgramController._togglePauseBlink();
      });
    }
  }
}

void ProgramControllerClass::_handleErrorState() {
  // Estado de error - Se detectó una condición anormal

  // Asegurar estado seguro del sistema
  static bool safetyMeasuresApplied = false;
  if (!safetyMeasuresApplied) {
    // Detener todos los actuadores
    Actuators.emergencyStop();

    // Abrir válvula de drenaje por seguridad
    Actuators.openDrainValve();

    // Desbloquear puerta después de un tiempo prudencial
    Utils.createTimeout(5000, []() {
      Actuators.unlockDoor();
      Utils.debug("🔓 Puerta desbloqueada después de error");
    });

    // Iniciar parpadeo no bloqueante
    if (_errorBlinkTaskId == -1) {
      _errorBlinkTaskId = Utils.createInterval(300, []() {
        ProgramController._toggleErrorBlink();
      });
    }

    safetyMeasuresApplied = true;
    Utils.debug("🛑 Medidas de seguridad aplicadas en estado de error");
  }

  // El sistema permanece en error hasta intervención manual
  // Requiere reinicio o acción específica del usuario
}

void ProgramControllerClass::_handleEmergencyState() {
  // Estado de emergencia - Botón de emergencia presionado

  // Aplicar medidas de emergencia inmediatas
  static bool emergencyMeasuresApplied = false;
  
  // Permitir resetear la variable desde resetEmergency()
  if (_currentState != ESTADO_EMERGENCIA) {
    emergencyMeasuresApplied = false;
    return;
  }
  
  if (!emergencyMeasuresApplied) {
    // Detener TODO inmediatamente
    Actuators.emergencyStop();

    // Abrir válvula de drenaje para evacuar agua
    Actuators.openDrainValve();

    // Programar desbloqueo de puerta después del tiempo de drenaje de seguridad
    if (_emergencyDoorUnlockTaskId == -1) {
      _emergencyDoorUnlockTaskId = Utils.createTimeout(TIEMPO_DRENAJE * 1000, []() {
        Actuators.unlockDoor();
        Utils.debug("🔓 Puerta desbloqueada después de drenaje de emergencia");
        ProgramController._emergencyDoorUnlockTaskId = -1;
      });
      Utils.debug("⏱️ Puerta se desbloqueará en " + String(TIEMPO_DRENAJE) + " segundos");
    }

    // Detener todos los temporizadores
    _timerRunning = false;

    // Iniciar alerta no bloqueante
    if (_emergencyBlinkTaskId == -1) {
      _emergencyBlinkTaskId = Utils.createInterval(250, []() {
        ProgramController._toggleEmergencyBlink();
      });
    }

    emergencyMeasuresApplied = true;
    Utils.debug("🚨 EMERGENCIA - Sistema detenido completamente");
  }

  // El sistema permanece en emergencia hasta reset manual
  // Solo se puede salir mediante resetEmergency() o reinicio del sistema
}

void ProgramControllerClass::handleEmergency(bool triggeredByButton) {
  // Manejar situación de emergencia
  if (_currentState != ESTADO_EMERGENCIA) {
    _emergencyTriggeredByButton = triggeredByButton;
    if (triggeredByButton) {
      Utils.debug("🚨 EMERGENCIA DETECTADA - Botón físico presionado");
    } else {
      Utils.debug("🚨 EMERGENCIA DETECTADA - Condición crítica de software");
    }
    setState(ESTADO_EMERGENCIA);
    Actuators.emergencyStop();
  }
}

void ProgramControllerClass::resetEmergency() {
  if (_currentState == ESTADO_EMERGENCIA) {
    // Verificar que el botón de emergencia ya no esté presionado
    if (!Hardware.isEmergencyButtonPressed()) {
      // Solo permitir reset automático si fue causada por botón físico
      if (!_emergencyTriggeredByButton) {
        Utils.debug("❌ RESET EMERGENCIA BLOQUEADO - Emergencia por software requiere reset manual");
        return;
      }
      
      Utils.debug("✅ RESET EMERGENCIA - Saliendo del estado de emergencia (botón físico)");
      
      // Detener parpadeo de emergencia
      if (_emergencyBlinkTaskId != -1) {
        Utils.stopTask(_emergencyBlinkTaskId);
        _emergencyBlinkTaskId = -1;
      }
      
      // Detener temporizador de desbloqueo de puerta si está activo
      if (_emergencyDoorUnlockTaskId != -1) {
        Utils.stopTask(_emergencyDoorUnlockTaskId);
        _emergencyDoorUnlockTaskId = -1;
      }
      
      // Resetear actuadores a estado seguro
      Actuators.emergencyReset();
      
      // Volver al estado de selección
      setState(ESTADO_SELECCION);
      
      // Resetear la variable static de emergencia llamando al handler
      _handleEmergencyState();
      
      // Mostrar pantalla de selección
      UIController.showSelectionScreen(_currentProgram + 1);
      
      Utils.debug("Sistema restablecido desde emergencia");
    } else {
      Utils.debug("RESET EMERGENCIA FALLIDO - Botón de emergencia aún presionado");
      // Mantener pantalla de emergencia (el mensaje se manejará en la UI)
      UIController.showEmergencyScreen();
    }
  }
}

void ProgramControllerClass::forceResetEmergency() {
  if (_currentState == ESTADO_EMERGENCIA) {
    if (_emergencyTriggeredByButton) {
      Utils.debug("🔄 Reset forzado - Emergencia era por botón físico (usar reset automático)");
      resetEmergency(); // Usar el reset normal
    } else {
      Utils.debug("🔧 RESET FORZADO - Emergencia por software, reset manual autorizado");
      
      // Para emergencias por software, permitir reset manual sin verificar botón físico
      // Detener parpadeo de emergencia
      if (_emergencyBlinkTaskId != -1) {
        Utils.stopTask(_emergencyBlinkTaskId);
        _emergencyBlinkTaskId = -1;
      }
      
      // Detener temporizador de desbloqueo de puerta si está activo
      if (_emergencyDoorUnlockTaskId != -1) {
        Utils.stopTask(_emergencyDoorUnlockTaskId);
        _emergencyDoorUnlockTaskId = -1;
      }
      
      // Resetear actuadores a estado seguro
      Actuators.emergencyReset();
      
      // Volver al estado de selección
      setState(ESTADO_SELECCION);
      
      // Mostrar pantalla de selección
      UIController.showSelectionScreen(_currentProgram + 1);
      
      Utils.debug("✅ Sistema restablecido desde emergencia por software");
    }
  }
}

void ProgramControllerClass::_triggerError(uint8_t errorCode,
                                           const String &errorMessage) {
  // Activar estado de error con código y mensaje específicos
  Utils.debug("ERROR: " + errorMessage);
  setState(ESTADO_ERROR);
  UIController.showErrorScreen(errorCode, errorMessage);
}

// ===== IMPLEMENTACIÓN DE MANEJO DE EVENTOS TÁCTILES =====

void ProgramControllerClass::_handleSelectionPageEvents(uint8_t componentId) {
  // Solo debug esencial - las validaciones ya se hicieron
  switch (componentId) {
  case NEXTION_ID_BTN_PROGRAM1:
    // Seleccionar programa 1 (P22) - índice interno 0
    Utils.debug("👆 Selección P22 - Botón presionado");
    selectProgram(0);
    // Delay antes de actualizar panel para evitar conflictos
    delay(50);
    UIController.updateProgramPanel(_currentProgram);
    break;

  case NEXTION_ID_BTN_PROGRAM2:
    // Seleccionar programa 2 (P23) - índice interno 1
    Utils.debug("👆 Selección P23 - Botón presionado");
    selectProgram(1);
    // Delay antes de actualizar panel para evitar conflictos
    delay(50);
    UIController.updateProgramPanel(_currentProgram);
    break;

  case NEXTION_ID_BTN_PROGRAM3:
    // Seleccionar programa 3 (P24) - índice interno 2
    Utils.debug("👆 Selección P24 - Botón presionado");
    selectProgram(2);
    // Delay antes de actualizar panel para evitar conflictos
    delay(50);
    UIController.updateProgramPanel(_currentProgram);
    break;

  case NEXTION_ID_BTN_START:
    // NOTA: El botón START ahora usa flujo simplificado en processUserEvent()
    // Esta sección ya no se ejecuta porque el evento se maneja directamente arriba
    Utils.debug("⚠️ Botón START manejado por flujo simplificado - esta línea no debería ejecutarse");
    break;

  case NEXTION_ID_BTN_EDIT:
    Utils.debug("✏️ Editando programa " + String(_currentProgram + 22));
    startEditing(_currentProgram, 0); // Comenzar editando la primera fase
    break;

  default:
    Utils.debug("⚠️ Componente no reconocido en página de selección: " +
                String(componentId));
    break;
  }
}

void ProgramControllerClass::_handleEditPageEvents(uint8_t componentId) {
  // Utils.debug("🔧 Manejando evento de edición - Componente: " + String(componentId));

  // Para P24: Solo manejar botones +/- cuando estemos editando tandas Y no se haya seleccionado otro parámetro
  if (_editingProgram == 2 && _editingParameter == PARAM_FASE && UIController.getCurrentParameter() == PARAM_FASE) {
    if (componentId == NEXTION_ID_BTN_PARAM_MAS) {
      _incrementTanda();
      return;
    } else if (componentId == NEXTION_ID_BTN_PARAM_MENOS) {
      _decrementTanda();
      return;
    }
  }
  
  // Para P24: Delegar botones +/- al UIController para todos los parámetros (incluye PARAM_FASE cuando no está en modo tanda)
  if (_editingProgram == 2 && (componentId == NEXTION_ID_BTN_PARAM_MAS || componentId == NEXTION_ID_BTN_PARAM_MENOS)) {
    UIController.handleEditPageEvent(componentId);
    return;
  }

  // DESACTIVADO: NEXTION_ID_PARAM_FASE_EDIT ya no se usa - ahora se usan botones TANDA1-4
  // Los eventos del panel derecho se delegan completamente al UIController

  // Delegar todos los demás eventos al UIController
  UIController.handleEditPageEvent(componentId);
}

void ProgramControllerClass::_handleTandaSelection() {
  // Ciclar entre las 4 tandas del P24
  _editingTanda = (_editingTanda + 1) % 4; // 0, 1, 2, 3
  
  Utils.debug("📝 P24 - Cambiando a tanda " + String(_editingTanda + 1));
  
  // Actualizar la pantalla para mostrar la nueva tanda
  Hardware.nextionSetText(NEXTION_COMP_SET_FASE, String(_editingTanda + 1));
  
  // Cargar y mostrar los parámetros de la nueva tanda
  _loadEditingParametersForCurrentTanda();
  
  // CRÍTICO: Resetear el parámetro en edición para permitir editar otros parámetros de la tanda seleccionada
  _editingParameter = -1; // No hay parámetro específico en edición - permitir selección libre
}

void ProgramControllerClass::_incrementTanda() {
  if (_editingProgram == 2) { // Solo para P24
    _editingTanda = (_editingTanda + 1) % 4; // 0→1→2→3→0
    Utils.debug("📝 P24 - Incrementando a tanda " + String(_editingTanda + 1));
    _updateTandaDisplay();
  }
}

void ProgramControllerClass::_decrementTanda() {
  if (_editingProgram == 2) { // Solo para P24
    _editingTanda = (_editingTanda + 3) % 4; // 0→3→2→1→0 (equivale a -1 pero sin negativos)
    Utils.debug("📝 P24 - Decrementando a tanda " + String(_editingTanda + 1));
    _updateTandaDisplay();
  }
}

void ProgramControllerClass::_updateTandaDisplay() {
  // Actualizar la pantalla para mostrar la nueva tanda
  Hardware.nextionSetText(NEXTION_COMP_SET_FASE, String(_editingTanda + 1));
  
  // Cargar y mostrar los parámetros de la nueva tanda
  _loadEditingParametersForCurrentTanda();
  
  // CRÍTICO: Cargar valores en UIController para que se puedan editar
  UIController._loadParametersFromStorage(_editingProgram, _editingTanda);
  
  // Notificar al UIController que actualice la visualización
  UIController.updateParameterDisplay();
  UIController.updateEditPanel(UPDATE_FULL); // Optimizado como página de selección
  
  // CRÍTICO: Actualizar estado visual de botones de tanda
  UIController.updateTandaButtons(_editingTanda);
  
  // CRÍTICO: Resetear el parámetro en edición para permitir editar otros parámetros de la tanda seleccionada
  _editingParameter = -1; // No hay parámetro específico en edición - permitir selección libre
}

void ProgramControllerClass::_handleExecutionPageEvents(uint8_t componentId) {
  Utils.debug("⚙️ Evento en página de ejecución - Componente: " +
              String(componentId));

  switch (componentId) {
  case NEXTION_ID_BTN_PAUSAR:
    // Pausar/reanudar programa
    if (_currentState == ESTADO_EJECUCION) {
      // Permitir pausa en cualquier momento, incluyendo fase de llenado
      Utils.debug(String("⏸️ Pausando programa") + 
                  (_preparingPhase ? " (durante llenado)" : ""));
      pauseProgram();
    } else if (_currentState == ESTADO_PAUSA) {
      Utils.debug("▶️ Reanudando programa");
      resumeProgram();
    }
    break;

  case NEXTION_ID_BTN_PARAR:
    // Detener programa completamente
    if (_currentState == ESTADO_EJECUCION || _currentState == ESTADO_PAUSA) {
      // Permitir detener en ejecución y pausa
      Utils.debug("⏹️ Deteniendo programa");
      stopProgram();
    } else {
      Utils.debug("⚠️ Detener no disponible en estado actual: " +
                  String(_currentState));
    }
    break;

  default:
    Utils.debug("⚠️ Componente no reconocido en página de ejecución: " +
                String(componentId));
    break;
  }
}

void ProgramControllerClass::_handleEmergencyPageEvents(uint8_t componentId) {
  Utils.debug("🚨 Evento en página de emergencia - Componente: " + String(componentId));

  switch (componentId) {
  case NEXTION_ID_BTN_REINICIAR:
    Utils.debug("🔄 Botón REINICIAR presionado - Derivando a ventana de selección");
    forceResetEmergency(); // Usar reset forzado que maneja ambos tipos
    break;

  default:
    Utils.debug("⚠️ Componente no reconocido en página de emergencia: " + String(componentId));
    break;
  }
}

/// @brief
/// Actualiza el controlador de programa.
/// Esta función debe ser llamada periódicamente desde el loop principal del
/// programa. Se encarga de procesar eventos de usuario, actualizar el estado
/// del controlador y manejar la lógica de la máquina de estados.
/// @details
/// - Procesa eventos de interfaz de usuario, como toques en la pantalla.
/// - Actualiza el estado del controlador de programa según la lógica de la
/// máquina de estados.
/// - Llama a métodos auxiliares para manejar la edición de parámetros y la
/// ejecución del programa.
/// - Debe ser llamada en el loop principal para asegurar que el controlador de
/// programa funcione correctamente.
void ProgramControllerClass::update() {
  // Verificaciones de seguridad crítica con máxima prioridad
  _checkCriticalSafety();
  
  // Eventos de interfaz de usuario se procesan directamente en main loop
  // para evitar procesamiento duplicado que causaba parpadeo
  // if (UIController.hasUserAction()) {
  //   String action = UIController.getUserAction();
  //   processUserEvent(action);
  // }

  // Actualizar según el estado actual
  _handleStateMachine();
}

void ProgramControllerClass::_loadProgramData() {
  // Cargar todos los datos de programa desde almacenamiento
  for (uint8_t prog = 0; prog < NUM_PROGRAMAS; prog++) {
    for (uint8_t fase = 0; fase < NUM_FASES; fase++) {
      _waterLevels[prog][fase] = Storage.loadWaterLevel(prog, 0, fase);
      _temperatures[prog][fase] = Storage.loadTemperature(prog, 0, fase);
      _times[prog][fase] = Storage.loadTime(prog, fase);
      _rotations[prog][fase] = Storage.loadRotation(prog, fase);
      _tipoAguaPrograma[prog][fase] = Storage.loadTipoAgua(prog, 0, fase);
      // Para centrifugado: [prog][fase] representa [prog][tanda]
      // P22/P23: solo tanda 0, P24: tandas 0,1,2
      _centrifugadoPorTanda[prog][fase] = Storage.loadCentrifugado(prog, fase);
    }
  }

  // Cargar el estado actual del programa
  _loadCurrentProgramState();

  Utils.debug("Datos de programa cargados desde almacenamiento");
}

void ProgramControllerClass::_finalizeProgramSequence() {
  Utils.debug("Finalizando P" + String(_currentProgram + 22));

  Actuators.unlockDoor();
  Actuators.closeSteamValve();
  Actuators.closeDrainValve();
  Actuators.closeWaterValve();
  Actuators.stopCentrifuge();
  Actuators.stopAutoRotation();
  Actuators.stopMotor();

  Storage.incrementUsageCounter();
  _timerRunning = false;
  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::_finalizeProgramWithDrainOpen() {
  Utils.debug("Finalizando P" + String(_currentProgram + 22) + " - Manteniendo drenaje abierto");

  Actuators.unlockDoor();
  Actuators.closeSteamValve();
  // NO cerrar drenaje - mantenerlo abierto
  Actuators.closeWaterValve();
  Actuators.stopCentrifuge();
  Actuators.stopAutoRotation();
  Actuators.stopMotor();

  Storage.incrementUsageCounter();
  _timerRunning = false;
  
  // Cambiar estado sin usar setState para evitar cerrar drenaje
  _previousState = _currentState;
  _currentState = ESTADO_SELECCION;
  
  // Mostrar pantalla de selección manualmente
  UIController.showSelectionScreen(_currentProgram);
}

void ProgramControllerClass::_handlePhaseStateMachine() {
  // MÁQUINA DE ESTADOS DE FASES
  // Controla el flujo: LLENANDO → LAVADO → DRENAJE → CENTRIFUGA (opcional) → ENFRIAMIENTO

  static unsigned long lastSensorUpdate = 0;
  unsigned long currentTime = millis();

  // Actualizar sensores en tiempo real cada 3 segundos en todas las fases
  if (currentTime - lastSensorUpdate >= 3000) {
    lastSensorUpdate = currentTime;
    UIController.updateTemperature(Sensors.getCurrentTemperature());
    UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
  }

  switch (_currentPhaseState) {
  case FASE_LLENANDO:
    // Controlar actuadores para llenado
    _controlActuatorsForPhase();

    // Verificar condiciones para pasar a lavado
    _checkSensorConditions();

    // La transición se maneja en _checkSensorConditions()
    break;

  case FASE_LAVADO:
    // Controlar actuadores para lavado
    _controlActuatorsForPhase();

    // El temporizador se actualiza desde updateTimers() llamado por el timer principal
    // Verificar si el lavado terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      // Lavado completo → Siempre ir a drenaje primero
      Utils.debug("Lavado completo → Drenaje");
      _currentPhaseState = FASE_DRENAJE;
      _initializePhaseState();
    }
    break;

  case FASE_CENTRIFUGA:
    // El temporizador se actualiza desde updateTimers() llamado por el timer principal
    // Verificar si el centrifugado terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      // Centrifugado completo: verificar si hay más tandas (P24) o finalizar
      if (_currentProgram == 2 && _tandaCounter < _maxTandas - 1) {
        // P24: Nueva tanda
        _tandaCounter++;
        Utils.debug("P24 - Centrifugado completo, iniciando tanda " + String(_tandaCounter + 1));
        // Actualizar display de tanda en página de ejecución
        UIController.updateTanda(_currentProgram, _tandaCounter);
        _currentPhaseState = FASE_LLENANDO;
        _initializePhaseState();
      } else {
        Utils.debug("Centrifugado completo → Enfriamiento");
        _currentPhaseState = FASE_ENFRIAMIENTO;
        _initializePhaseState();
      }
    }
    break;

  case FASE_DRENAJE:
    // Controlar actuadores para drenaje
    _controlActuatorsForPhase();

    // El temporizador se actualiza desde updateTimers() llamado por el timer principal
    // Verificar si el drenaje terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      // Verificar si hay centrifugado habilitado
      bool centrifugadoEnabled = false;
      if (_currentProgram <= 1) {
        // P22/P23: verificar centrifugado
        centrifugadoEnabled =
            (_centrifugadoPorTanda[_currentProgram][3] == 1);
      } else {
        // P24: usar tanda actual
        centrifugadoEnabled =
            (_centrifugadoPorTanda[_currentProgram][_tandaCounter] == 1);
      }

      if (centrifugadoEnabled) {
        Utils.debug("Drenaje completo → Centrifugado");
        _currentPhaseState = FASE_CENTRIFUGA;
        _initializePhaseState();
      } else {
        // Sin centrifugado: verificar si hay más tandas (P24) o finalizar
        if (_currentProgram == 2 && _tandaCounter < _maxTandas - 1) {
          // P24: Nueva tanda
          _tandaCounter++;
          Utils.debug("P24 - Iniciando tanda " + String(_tandaCounter + 1));
          // Actualizar display de tanda en página de ejecución
          UIController.updateTanda(_currentProgram, _tandaCounter);
          _currentPhaseState = FASE_LLENANDO;
          _initializePhaseState();
        } else {
          Utils.debug("Drenaje completo → Enfriamiento (sin centrifugado)");
          _currentPhaseState = FASE_ENFRIAMIENTO;
          _initializePhaseState();
        }
      }
    }
    break;

  case FASE_ENFRIAMIENTO:
    // El temporizador se actualiza desde updateTimers() llamado por el timer principal
    // Verificar si el enfriamiento terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      Utils.debug("Enfriamiento completo → Programa finalizado");
      _finalizeProgramWithDrainOpen();
    }
    break;

  default:
    Utils.debug("ERROR: Estado de fase desconocido " +
                String(_currentPhaseState));
    setState(ESTADO_ERROR);
    break;
  }
}

/// @brief 
/// Inicializa el estado de la fase actual.
/// Configura los parámetros y actuadores necesarios para la fase actual.
/// Debe ser llamado al iniciar una nueva fase o al reiniciar el programa.
void ProgramControllerClass::_initializePhaseState() {
  // Inicializar parámetros para la nueva fase
  switch (_currentPhaseState) {
  case FASE_LLENANDO:
    _currentPhase = 0;
    _preparingPhase = true;
    _timerRunning = false;
    
    // CRÍTICO: Detener actuadores de la fase anterior (centrifugado/drenaje)
    Actuators.stopCentrifuge();   // Apagar centrifugado de fase anterior
    Actuators.closeDrainValve();  // Cerrar desfogue de fase anterior
    Actuators.stopMotor();        // Asegurar motor parado
    
    Utils.debug("🔧 FASE_LLENANDO - Actuadores de fase anterior detenidos");
    break;

  case FASE_LAVADO: {
    _currentPhase = 1;
    // Para P24 usar tanda actual, para P22/P23 usar fase
    uint8_t timeIndex = (_currentProgram == 2) ? _tandaCounter : _currentPhase;
    _totalMinutes = Storage.loadTime(_currentProgram, timeIndex);
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;
    _totalSeconds = _totalMinutes * 60; // Calcular total en segundos
    _timerRunning = true;
    _preparingPhase = false;

    // Configurar actuadores para lavado
    Actuators.stopCentrifuge();   // Asegurar centrifugado apagado
    Actuators.closeDrainValve();  // Cerrar desfogue durante lavado
    Actuators.closeWaterValve();  // Cerrar agua (ya llenado)
    Actuators.closeSteamValve();  // Cerrar vapor (ya llenado)
    
    // Iniciar rotación en lavado
    uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
    if (rotLevel > 0) {
      Actuators.startAutoRotation(rotLevel);
    }
    
    Utils.debug("🔧 FASE_LAVADO - Actuadores configurados correctamente");
  } break;

  case FASE_CENTRIFUGA: {
    _currentPhase = 2;
    _totalMinutes = TIEMPO_CENTRIFUGADO / 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = TIEMPO_CENTRIFUGADO % 60;
    _totalSeconds = TIEMPO_CENTRIFUGADO; // Total en segundos
    _timerRunning = true;
    _preparingPhase = false;

    // Configurar actuadores para centrifugado
    Actuators.closeSteamValve();
    Actuators.openDrainValve(); // Mantener drenaje abierto durante centrifugado
    Actuators.closeWaterValve();
    Actuators.startCentrifuge();
    Actuators.stopAutoRotation();
    Actuators.stopMotor();
  } break;

  case FASE_DRENAJE: {
    _currentPhase = 3;
    _totalMinutes = TIEMPO_DRENAJE / 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = TIEMPO_DRENAJE % 60;
    _totalSeconds = TIEMPO_DRENAJE; // Total en segundos
    _timerRunning = true;
    _preparingPhase = false;

    // Configurar actuadores para drenaje
    Actuators.stopCentrifuge();
    Actuators.openDrainValve();
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
    Actuators.stopAutoRotation();
    Actuators.stopMotor();
  } break;

  case FASE_ENFRIAMIENTO: {
    _currentPhase = 4;
    _totalMinutes = TIEMPO_PUERTA_BLOQUEO / 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = TIEMPO_PUERTA_BLOQUEO % 60;
    _totalSeconds = TIEMPO_PUERTA_BLOQUEO; // Total en segundos
    _timerRunning = true;
    _preparingPhase = false;

    // Configurar actuadores para enfriamiento
    Actuators.stopCentrifuge();
    Actuators.openDrainValve(); // Mantener drenaje abierto
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
    Actuators.stopAutoRotation();
    Actuators.stopMotor();
  } break;
  }

  // Guardar fase y actualizar UI
  Storage.savePhase(_currentPhase);
  UIController.updatePhase(_currentPhase);
  UIController.updateTime(_remainingMinutes, _remainingSeconds);
  // Nota: Barra de progreso eliminada del HMI

  Utils.debug("Fase iniciada: " + String(_currentPhaseState) + " (Fase " +
              String(_currentPhase) + ")");
}

// Métodos para parpadeos no bloqueantes
void ProgramControllerClass::_togglePauseBlink() {
  _blinkState = !_blinkState;
  if (_blinkState) {
    // Mostrar tiempo normal
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
  } else {
    // Mostrar vacío para efecto de parpadeo
    Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, "");
  }
}

void ProgramControllerClass::_toggleErrorBlink() {
  _blinkState = !_blinkState;
  UIController.updateErrorDisplay(_blinkState);
}

void ProgramControllerClass::_toggleEmergencyBlink() {
  _blinkState = !_blinkState;
  UIController.updateEmergencyAlert(_blinkState);
}

void ProgramControllerClass::_stopAllBlinkTasks() {
  if (_pauseBlinkTaskId != -1) {
    Utils.stopTask(_pauseBlinkTaskId);
    _pauseBlinkTaskId = -1;
  }
  if (_errorBlinkTaskId != -1) {
    Utils.stopTask(_errorBlinkTaskId);
    _errorBlinkTaskId = -1;
  }
  if (_emergencyBlinkTaskId != -1) {
    Utils.stopTask(_emergencyBlinkTaskId);
    _emergencyBlinkTaskId = -1;
  }
  if (_emergencyDoorUnlockTaskId != -1) {
    Utils.stopTask(_emergencyDoorUnlockTaskId);
    _emergencyDoorUnlockTaskId = -1;
  }
}
