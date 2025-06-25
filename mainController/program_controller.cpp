// program_controller.cpp
#include "program_controller.h"

// Instancia global
ProgramControllerClass ProgramController;

void ProgramControllerClass::init() {
  // Inicializar variables de estado
  _currentState = ESTADO_SELECCION;
  _previousState = ESTADO_SELECCION;
  _currentProgram = 0;  // Inicializar con programa P22 (índice 0) por defecto
  _currentPhase = 0;
  
  // Inicializar variables de temporizador
  _remainingMinutes = 0;
  _remainingSeconds = 0;
  _totalMinutes = 0;
  _totalSeconds = 0;
  _timerRunning = false;
  
  // Inicializar variables de pausa
  _pausedMinutes = 0;
  _pausedSeconds = 0;
  
  // Inicializar variables de estado de fase
  _preparingPhase = false;
  _phaseStartTime = 0;
  
  // Inicializar variables de edición
  _editingProgram = 0;  // Inicializar con programa P22 (índice 0) por defecto
  _editingPhase = 0;
  _editingParameter = 0;
  _editingParameterValue = 0;
  _isEditing = false;
  
  // Cargar datos de programa desde almacenamiento
  _loadProgramData();
  
  Utils.debug("Program Controller inicializado");
}

/// @brief 
/// Carga el estado inicial del programa desde el almacenamiento.
/// Este método solo carga el programa y fase actual, los demás datos se obtienen directamente de Storage.
void ProgramControllerClass::_loadCurrentProgramState() {
  // Cargar el último programa y fase utilizados
  _currentProgram = Storage.loadProgram();
  _currentPhase = Storage.loadPhase();
  
  Utils.debug("Estado del programa cargado: P" + String(_currentProgram + 22) + " F" + String(_currentPhase + 1));
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
    
    Utils.debug("🔄 CAMBIO DE ESTADO:");
    Utils.debug("   Estado anterior: " + String(_previousState));
    Utils.debug("   Estado nuevo: " + String(_currentState));
    
    // Acciones específicas al cambiar de estado
    switch (newState) {
      case ESTADO_SELECCION:
        Utils.debug("📋 Mostrando pantalla de selección");
        UIController.showSelectionScreen(_currentProgram); // Pasar índice directamente
        break;
      
      case ESTADO_EDICION:
        Utils.debug("✏️ Mostrando pantalla de edición");
        UIController.showEditScreen(_editingProgram, _editingPhase);
        break;
      
      case ESTADO_EJECUCION:
        Utils.debug("▶️ Mostrando pantalla de ejecución");
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
    // === VERIFICACIÓN INICIAL DE PUERTA SEGÚN DOCUMENTO DEL CLIENTE ===
    if (!Sensors.isDoorClosed()) {
      // Mostrar advertencia en componente de mensajes de Nextion
      Hardware.nextionSetText(NEXTION_COMP_MSG, "PUERTA ABIERTA");
      Utils.debug("❌ No se puede iniciar: Puerta abierta");
      return;
    } else {
      // Limpiar mensaje si la puerta está cerrada
      Hardware.nextionSetText(NEXTION_COMP_MSG, "");
    }
    
    setState(ESTADO_EJECUCION);
    
    // Asegurar que el botón pausar muestre "PAUSAR" al iniciar
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "PAUSAR");
    
    Utils.debug("ProgramControllerClass::startProgram| Programa iniciado: " + String(_currentProgram));
  }
}

void ProgramControllerClass::pauseProgram() {
  if (_currentState == ESTADO_EJECUCION) {
    // Preservar tiempo restante para continuar después
    _pausedMinutes = _remainingMinutes;
    _pausedSeconds = _remainingSeconds;
    
    // Detener temporizador
    _timerRunning = false;
    
    // Cambiar estado a pausa
    setState(ESTADO_PAUSA);
    
    // Cambiar texto del botón a "REANUDAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "REANUDAR");
    
    Utils.debug("⏸️ Programa pausado - Tiempo preservado: " + String(_pausedMinutes) + ":" + String(_pausedSeconds));
  }
}

void ProgramControllerClass::resumeProgram() {
  if (_currentState == ESTADO_PAUSA) {
    // Restaurar tiempo restante desde donde se pausó
    _remainingMinutes = _pausedMinutes;
    _remainingSeconds = _pausedSeconds;
    
    // Reactivar temporizador
    _timerRunning = true;
    
    // Cambiar estado a ejecución
    setState(ESTADO_EJECUCION);
    
    // Cambiar texto del botón de vuelta a "PAUSAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "PAUSAR");
    
    // Actualizar display con tiempo restaurado
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    
    Utils.debug("▶️ Programa reanudado - Tiempo restaurado: " + String(_remainingMinutes) + ":" + String(_remainingSeconds));
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
    
    // === VERIFICAR SI LA SIGUIENTE FASE ES CENTRIFUGADO Y ESTÁ DESHABILITADA ===
    if (_currentPhase == 3 && !_isCentrifugadoEnabled(_currentProgram, _currentPhase)) {
      // Fase 4 (centrifugado) está deshabilitada, saltar al final
      Utils.debug("⏭️ Saltando centrifugado (deshabilitado) - Completando programa");
      _completeProgram();
      return;
    }
    
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
    _totalMinutes = Storage.loadTime(_currentProgram, _currentPhase);
    _totalSeconds = _totalMinutes * 60;
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;
    
    // Reiniciar estado de preparación
    _preparingPhase = true;
    _phaseStartTime = millis();
    _timerRunning = false;
    
    // Actualizar la interfaz de usuario
    UIController.updatePhase(_currentPhase + 1); // Convertir índice interno (0-3) a número de fase para UI (1-4)
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    UIController.updateProgressBar(0);
    
    Utils.debug("📌 Nueva fase iniciada: " + String(_currentPhase + 1));
  }
}

void ProgramControllerClass::updateTimers() {
  if (_currentState == ESTADO_EJECUCION) {
    // Siempre verificar condiciones de los sensores durante preparación
    if (_preparingPhase) {
      _checkSensorConditions();
    }
    
    // NOTA: El decremento del temporizador se hace en _handleExecutionState() 
    // para sincronización exacta con la UI cada segundo
    
    // Solo actualizar actuadores si no se está manejando desde _handleExecutionState
    if (!_timerRunning) {
      Actuators.updateTimers();
    }
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
  
  // Nota: El progreso se actualiza en _handleExecutionState() usando getProgressPercentage()
}

/// @brief 
/// Verifica las condiciones de los sensores y actualiza los actuadores según la fase actual.
/// Este método se encarga de verificar si se han alcanzado las condiciones de temperatura y nivel de agua
void ProgramControllerClass::_checkSensorConditions() {
  // Verificar las condiciones de temperatura y nivel de agua según la fase actual
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);
  
  // Actualizar la interfaz con los valores actuales
  UIController.updateTemperature(Sensors.getCurrentTemperature());
  UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
  
  // Si estamos preparando la fase, mostrar el estado en el temporizador
  if (_preparingPhase) {
    unsigned long elapsedTime = (millis() - _phaseStartTime) / 1000; // segundos
    uint8_t prepMinutes = elapsedTime / 60;
    uint8_t prepSeconds = elapsedTime % 60;
    
    // Mostrar tiempo de preparación con indicador
    UIController.updateTime(prepMinutes, prepSeconds);
    UIController.updateProgressBar(0); // Barra en 0 durante preparación
    
    // Mostrar mensaje de estado
    String statusMsg = "Preparando: ";
    bool waterOk = Sensors.isWaterLevelReached(targetLevel);
    bool tempOk = Sensors.isTemperatureReached(targetTemp);
    
    if (!waterOk) {
      statusMsg += "Llenando... ";
    }
    if (!tempOk) {
      statusMsg += "Calentando... ";
    }
    if (waterOk && tempOk) {
      statusMsg = "Iniciando ciclo...";
    }
    
    // Actualizar estado en la UI (esto requerirá un método nuevo en UIController)
    // Por ahora, usar debug
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 5000) { // Cada 5 segundos
      lastStatusUpdate = millis();
      Utils.debug("📊 " + statusMsg + " Agua:" + String(Sensors.getCurrentWaterLevel()) + "/" + String(targetLevel) + 
                  " Temp:" + String(Sensors.getCurrentTemperature()) + "/" + String(targetTemp) + "°C");
    }
  }
  
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
  
  // Verificar si se han alcanzado las condiciones para iniciar el temporizador
  if (Sensors.isWaterLevelReached(targetLevel) && 
      Sensors.isTemperatureReached(targetTemp) && 
      !_timerRunning) {
    
    // Iniciar rotación automática si es necesaria para esta fase
    uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
    if (rotLevel > 0 && !Actuators.isAutoRotationActive()) {
      Actuators.startAutoRotation(rotLevel);
      Utils.debug("🔄 Iniciando rotación nivel: " + String(rotLevel));
    }
    
    // Iniciar el temporizador cuando se alcanzan las condiciones necesarias
    _timerRunning = true;
    _preparingPhase = false; // Ya no estamos preparando
    
    // Reiniciar el temporizador a los valores configurados
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;
    
    Utils.debug("✅ Condiciones alcanzadas, iniciando temporizador de fase");
    Utils.debug("⏱️ Tiempo de fase: " + String(_totalMinutes) + " minutos");
    Utils.debug("💧 Nivel: " + String(Sensors.getCurrentWaterLevel()) + "/" + String(targetLevel));
    Utils.debug("🌡️ Temp: " + String(Sensors.getCurrentTemperature()) + "/" + String(targetTemp) + "°C");
  }
}

void ProgramControllerClass::_completePhase() {
  Utils.debugValue("ProgramControllerClass::_completePhase| Fase completada: ", _currentPhase);
  
  // Detener actuadores de la fase actual
  Actuators.stopAutoRotation();
  Actuators.closeSteamValve();
  Actuators.closeWaterValve();
  
  // === SISTEMA DE TANDAS PARA PROGRAMA 24 SEGÚN DOCUMENTO DEL CLIENTE ===
  if (_currentProgram == 2 && isLastPhase()) { // P24 en última fase
    // Verificar si hay más tandas pendientes
    if (_tandaCounter < _maxTandas - 1) {
      _tandaCounter++;
      Utils.debug("🔄 P24 - Completando tanda " + String(_tandaCounter) + " de " + String(_maxTandas));
      
      // Reiniciar desde la primera fase para nueva tanda
      _currentPhase = 0;
      Storage.savePhase(_currentPhase);
      
      // Reinicializar para nueva tanda
      _updatePhaseParameters();
      _preparingPhase = true;
      _phaseStartTime = millis();
      
      Utils.debug("🔄 Iniciando tanda " + String(_tandaCounter + 1) + " - Volviendo a Fase 1");
      
      // Mostrar progreso de tandas en UI
      String tandaMsg = "Tanda " + String(_tandaCounter + 1) + "/" + String(_maxTandas);
      UIController.showMessage(tandaMsg, 3000);
      
      return;
    } else {
      Utils.debug("✅ P24 - Todas las tandas completadas (" + String(_maxTandas) + " tandas)");
    }
  }
  
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
  
  // Desbloquear puerta después de completar programa exitosamente
  Actuators.unlockDoor();
  Utils.debug("🔓 Puerta desbloqueada - Programa completado exitosamente");
  
  // Incrementar contador de uso
  Storage.incrementUsageCounter();
  
  // Volver a la pantalla de selección
  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::_initializeProgram() {
  // === INICIALIZACIÓN SEGÚN DOCUMENTO DEL CLIENTE ===
  
  // 1. Bloquear puerta (ya verificada en startProgram)
  Actuators.lockDoor();
  Utils.debug("🔒 Puerta bloqueada");
  
  // 2. Inicializar variables del programa
  _totalMinutes = Storage.loadTime(_currentProgram, _currentPhase);
  _totalSeconds = _totalMinutes * 60;
  _remainingMinutes = _totalMinutes;
  _remainingSeconds = 0;
  _timerRunning = false;
  
  // 3. Cargar parámetros del programa según tipo
  _configureProgramType();
  
  // 4. Inicializar estado de preparación
  _preparingPhase = true;
  _phaseStartTime = millis();
  
  // 5. Inicializar contador de tandas para P24
  if (_currentProgram == 2) { // Programa 24 (índice 2)
    _tandaCounter = 0;
    _maxTandas = 3; // Configurable según necesidades del cliente
  }
  
  Utils.debug("ProgramControllerClass::_initializeProgram| Programa inicializado");
  Utils.debug("📋 Programa: P" + String(_currentProgram + 22) + " | Fase: " + String(_currentPhase + 1));
  Utils.debug("⏳ Esperando condiciones: Nivel=" + String(_waterLevels[_currentProgram][_currentPhase]) + 
              ", Temp=" + String(_temperatures[_currentProgram][_currentPhase]) + "°C");
}

void ProgramControllerClass::_configureProgramType() {
  // === CONFIGURACIÓN DE PROGRAMAS SEGÚN DOCUMENTO DEL CLIENTE ===
  
  switch (_currentProgram) {
    case 0: // Programa 22 - Agua Caliente
      Utils.debug("🔥 Configurando Programa 22 - Agua Caliente");
      // Activar gestión de temperatura activa para todas las fases
      // P22 usa agua caliente con control de temperatura estricto
      break;
      
    case 1: // Programa 23 - Agua Fría  
      Utils.debug("❄️ Configurando Programa 23 - Agua Fría");
      // Desactivar gestión de temperatura (solo agua fría)
      // P23 usa agua fría sin calentamiento
      break;
      
    case 2: // Programa 24 - Multi-ciclo configurable
      Utils.debug("🔄 Configurando Programa 24 - Multi-ciclo");
      // Configurar según tipo de agua seleccionado por el usuario
      // El tipo de agua se define por fase en los arrays de configuración
      break;
      
    default:
      Utils.debug("❓ Programa desconocido: " + String(_currentProgram));
      break;
  }
  
  Utils.debug("✅ Configuración de programa completada");
}

void ProgramControllerClass::_handleTemperatureControl() {
  // === CONTROL DE TEMPERATURA CON DRENAJE PARCIAL SEGÚN DOCUMENTO DEL CLIENTE ===
  
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
  float targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);
  
  // === CONTROL CON DRENAJE PARCIAL SEGÚN DOCUMENTO ===
  if (currentTemp < targetTemp - 2) {
    // Utils.debug("🌡️ Temperatura baja: " + String(currentTemp) + "°C, objetivo: " + String(targetTemp) + "°C");
    
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
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);
  if (Sensors.getCurrentWaterLevel() >= targetLevel) {
    Actuators.closeWaterValve();
  }
}

bool ProgramControllerClass::_isCentrifugadoEnabled(uint8_t programa, uint8_t fase) {
  // Verificar si el centrifugado está habilitado para este programa y fase
  if (programa >= NUM_PROGRAMAS || fase >= NUM_FASES) {
    return false;
  }
  return _centrifugadoPrograma[programa][fase] == 1;
}

void ProgramControllerClass::_configureActuatorsForPhase() {
  // Configurar actuadores según la fase actual
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);
  
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
  
  // Utils.debug("Actuadores configurados para la fase");
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
  // Durante preparación de fase, mostrar 0%
  if (_preparingPhase) {
    return 0;
  }
  
  // Durante ejecución, calcular progreso basado en tiempo
  if (_totalSeconds == 0) return 0;
  
  uint16_t remainingTotal = (_remainingMinutes * 60) + _remainingSeconds;
  uint8_t progress = 100 - ((remainingTotal * 100) / _totalSeconds);
  
  // Debug cada 5 segundos para verificar sincronización
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 5000) { // Cada 5 segundos
    Utils.debug("📊 Progreso: " + String(progress) + "% | Restante: " + 
                String(_remainingMinutes) + ":" + String(_remainingSeconds) + 
                " | Total: " + String(_totalSeconds) + "s | Timer: " + 
                String(_timerRunning ? "ON" : "OFF"));
    lastDebug = millis();
  }
  
  return progress;
}

/// @brief
/// Inicia el modo de edición para un programa y fase específicos.
/// Este método permite al usuario editar los parámetros de un programa específico,
/// @param program
/// El número del programa a editar (0 a 2).
/// @param phase
/// El número de la fase a editar (0 a 3).
void ProgramControllerClass::startEditing(uint8_t program, uint8_t phase) {
  if (program < NUM_PROGRAMAS && phase < NUM_FASES) {
    _editingProgram = program;
    _editingPhase = phase;
    _editingParameter = PARAM_NIVEL; // Comenzar editando el nivel
    _editingParameterValue = _waterLevels[program][phase]; // Cargar valor actual
    _isEditing = true;
    
    setState(ESTADO_EDICION);
    
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
  
  // Recargar todos los datos para asegurar consistencia
  _loadProgramData();
  
  setState(ESTADO_SELECCION);
  
  // Mostrar pantalla de selección actualizada
  UIController.showSelectionScreen(_editingProgram); // Pasar índice directamente
  
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
  
  Utils.debug("✅ Edición terminada para P" + String(_editingProgram + 22) + ", volviendo a selección");
  Utils.debug("   Actualizando _currentProgram a: " + String(_currentProgram));
  
  setState(ESTADO_SELECCION);
}

void ProgramControllerClass::processUserEvent(const String& event) {
  // Verificar si hay un evento táctil válido
  if (!Hardware.hasValidTouchEvent()) {
    Utils.debug("⚠️ Evento táctil no válido recibido: " + event);
    return; // No hay evento táctil válido
  }
  
  uint8_t touchPage = Hardware.getTouchEventPage();
  uint8_t touchComponent = Hardware.getTouchEventComponent();
  uint8_t touchType = Hardware.getTouchEventType();
  
  // Solo procesar eventos de botón presionado (touchType == 1)
  if (touchType != 1) {
    return;
  }
  
  // Filtro anti-rebote: evitar procesar el mismo evento repetidamente
  static uint8_t lastPage = 255;
  static uint8_t lastComponent = 255;
  static unsigned long lastEventTime = 0;
  unsigned long currentTime = millis();
  
  if (touchPage == lastPage && touchComponent == lastComponent && 
      (currentTime - lastEventTime) < 200) { // 200ms de anti-rebote
    return; // Ignorar evento duplicado
  }
  
  lastPage = touchPage;
  lastComponent = touchComponent;
  lastEventTime = currentTime;
  
  // Debug solo para componentes importantes (botones de control)
  if (touchComponent == NEXTION_ID_BTN_PARAR || touchComponent == NEXTION_ID_BTN_PAUSAR || touchComponent == NEXTION_ID_BTN_START) {
    Utils.debug("🎯 Evento botón control - Página: " + String(touchPage) + ", Componente: " + String(touchComponent));
  }
  
  // Procesar eventos según la página actual
  switch (touchPage) {
    case NEXTION_PAGE_SELECTION:
      // Solo procesar eventos de selección si estamos en el estado correcto
      _handleSelectionPageEvents(touchComponent);
      break;
      
    case NEXTION_PAGE_EDIT:
      // Utils.debug("📄 Procesando eventos de página de edición");
      _handleEditPageEvents(touchComponent);
      break;
      
    case NEXTION_PAGE_EXECUTION:
      _handleExecutionPageEvents(touchComponent);
      break;
      
    default:
      Utils.debug("⚠️ Página no manejada: " + String(touchPage));
      break;
  }
}

void ProgramControllerClass::_handleStateMachine() {
  // Máquina de estados principal que coordina todo el comportamiento del sistema
  // Se ejecuta en cada ciclo del loop principal
  
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
      Utils.debug("⚠️ Estado desconocido: " + String(_currentState) + ", regresando a IDLE");
      setState(ESTADO_IDLE);
      break;
  }
}

void ProgramControllerClass::_handleSelectionState() {
  // Estado de selección - El usuario está navegando entre programas
  // Verificar si hay timeout de inactividad (opcional)
  
  // En este estado, el procesamiento principal se hace mediante eventos táctiles
  // que ya están siendo manejados por processUserEvent()
  
  // Actualizar display si es necesario
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) { // Actualizar cada segundo
    // Actualizar información del programa actual en la UI
    UIController.updateProgramInfo(_currentProgram + 1); // Convertir a 1-3 para UI
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
  
  // 2. Verificar si estamos preparando la fase (llenado/calentamiento)
  if (_preparingPhase) {
    // Mostrar estado de preparación en UI
    unsigned long prepTime = (millis() - _phaseStartTime) / 1000;
    UIController.updatePreparationStatus(prepTime);
    
    // Verificar si se alcanzaron las condiciones necesarias
    _checkSensorConditions();
    
    // Si las condiciones se cumplieron, iniciar el temporizador
    if (!_preparingPhase) {
      Utils.debug("✅ Condiciones alcanzadas - iniciando temporizador de fase");
      UIController.clearPreparationStatus();
    }
    return;
  }
  
  // 3. Actualizar temporizadores (solo si está corriendo)
  if (_timerRunning) {
    // El decremento se hace en updateTimers() llamado por el callback
    
    // Actualizar display y decrementar temporizador EXACTAMENTE cada segundo
    static unsigned long lastSecondUpdate = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastSecondUpdate >= 1000) {
      // Sincronizar exactamente cada segundo
      lastSecondUpdate = currentTime;
      
      // Decrementar temporizador aquí para sincronización exacta
      _decrementTimer();
      
      // Actualizar UI inmediatamente después del decremento
      UIController.updateTime(_remainingMinutes, _remainingSeconds);
      UIController.updateProgressBar(getProgressPercentage());
      
      // Actualizar sensores y actuadores en tiempo real durante ejecución
      UIController.updateTemperature(Sensors.getCurrentTemperature());
      UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
      UIController.updateRotation(Actuators.getCurrentRotationLevel());
      
      // Actualizar actuadores
      Actuators.updateTimers();
    }
    
    // Verificar si la fase terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      _completePhase();
    }
  }
  
  // 4. Control de temperatura con drenaje parcial según documento del cliente
  _handleTemperatureControl();
  
  // 5. Control del motor según nivel de rotación
  if (Storage.loadRotation(_currentProgram, _currentPhase) > 0) {
    // Asegurar que el motor esté funcionando con el patrón correcto
    if (!Actuators.isMotorRunning()) {
      uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
    Actuators.startAutoRotation(rotLevel);
      Utils.debug("🔄 Motor iniciado - Nivel: " + String(_rotations[_currentProgram][_currentPhase]));
    }
  } else {
    // Sin rotación en esta fase
    if (Actuators.isMotorRunning()) {
      Actuators.stopMotor();
      Utils.debug("⏹️ Motor detenido - Sin rotación en esta fase");
    }
  }
}

void ProgramControllerClass::_handlePauseState() {
  // Estado de pausa - El programa está detenido temporalmente
  
  // Asegurar que todos los actuadores estén detenidos
  static bool actuatorsStopped = false;
  if (!actuatorsStopped) {
    Actuators.stopMotor();
    Actuators.closeWaterValve();
    Actuators.closeSteamValve();
    // Mantener puerta bloqueada por seguridad
    actuatorsStopped = true;
    Utils.debug("⏸️ Sistema en pausa - actuadores detenidos");
  }
  
  // Mostrar estado de pausa en UI
  static unsigned long lastBlink = 0;
  static bool blinkState = false;
  if (millis() - lastBlink > 500) { // Parpadeo cada 500ms
    blinkState = !blinkState;
    // UIController.updatePauseIndicator(blinkState);
    lastBlink = millis();
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
    
    safetyMeasuresApplied = true;
    Utils.debug("🛑 Medidas de seguridad aplicadas en estado de error");
  }
  
  // Mostrar error en pantalla con parpadeo
  static unsigned long lastErrorBlink = 0;
  static bool errorBlinkState = false;
  if (millis() - lastErrorBlink > 300) { // Parpadeo rápido
    errorBlinkState = !errorBlinkState;
    UIController.updateErrorDisplay(errorBlinkState);
    lastErrorBlink = millis();
  }
  
  // El sistema permanece en error hasta intervención manual
  // Requiere reinicio o acción específica del usuario
}

void ProgramControllerClass::_handleEmergencyState() {
  // Estado de emergencia - Botón de emergencia presionado
  
  // Aplicar medidas de emergencia inmediatas
  static bool emergencyMeasuresApplied = false;
  if (!emergencyMeasuresApplied) {
    // Detener TODO inmediatamente
    Actuators.emergencyStop();
    
    // Abrir todas las válvulas de seguridad
    Actuators.openDrainValve();
    
    // Desbloquear puerta inmediatamente
    Actuators.unlockDoor();
    
    // Detener todos los temporizadores
    _timerRunning = false;
    
    emergencyMeasuresApplied = true;
    Utils.debug("🚨 EMERGENCIA - Sistema detenido completamente");
  }
  
  // Mostrar alerta de emergencia con sonido/visual
  static unsigned long lastEmergencyAlert = 0;
  static bool alertState = false;
  if (millis() - lastEmergencyAlert > 250) { // Alerta muy rápida
    alertState = !alertState;
    UIController.updateEmergencyAlert(alertState);
    
    lastEmergencyAlert = millis();
  }
  
  // El sistema permanece en emergencia hasta reset manual
  // Solo se puede salir mediante resetEmergency() o reinicio del sistema
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
    case NEXTION_ID_BTN_PROGRAM1:
      // Seleccionar programa 1 directamente (P22) - índice interno 0
      _currentProgram = 0; // Índice interno 0 = P22
      Storage.saveProgram(_currentProgram);
      Utils.debug("📋 Programa 1 seleccionado directamente (P22)");
      UIController.showSelectionScreen(_currentProgram); // Pasar índice 0 directamente
      break;
      
    case NEXTION_ID_BTN_PROGRAM2:
      // Seleccionar programa 2 directamente (P23) - índice interno 1
      _currentProgram = 1; // Índice interno 1 = P23
      Storage.saveProgram(_currentProgram);
      Utils.debug("📋 Programa 2 seleccionado directamente (P23)");
      UIController.showSelectionScreen(_currentProgram); // Pasar índice 1 directamente
      break;
      
    case NEXTION_ID_BTN_PROGRAM3:
      // Seleccionar programa 3 directamente (P24) - índice interno 2
      _currentProgram = 2; // Índice interno 2 = P24
      Storage.saveProgram(_currentProgram);
      Utils.debug("📋 Programa 3 seleccionado directamente (P24)");
      UIController.showSelectionScreen(_currentProgram); // Pasar índice 2 directamente
      break;
      
    // case NEXTION_ID_BTN_PROG_ANTERIOR:
    //   // Cambiar al programa anterior
    //   if (_currentProgram > 0) {
    //     _currentProgram--;
    //   } else {
    //     _currentProgram = NUM_PROGRAMAS - 1; // Circular: ir al último programa
    //   }
    //   Storage.saveProgram(_currentProgram);
    //   Utils.debug("📋 Programa seleccionado: " + String(_currentProgram + 22));
    //   UIController.showSelectionScreen(_currentProgram);
    //   break;
      
    // case NEXTION_ID_BTN_PROG_SIGUIENTE:
    //   // Cambiar al programa siguiente
    //   if (_currentProgram < NUM_PROGRAMAS - 1) {
    //     _currentProgram++;
    //   } else {
    //     _currentProgram = 0; // Circular: ir al primer programa
    //   }
    //   Storage.saveProgram(_currentProgram);
    //   Utils.debug("📋 Programa seleccionado: " + String(_currentProgram + 22));
    //   UIController.showSelectionScreen(_currentProgram);
    //   break;
      
    case NEXTION_ID_BTN_START:
      // Verificar estado de puerta para determinar acción
      if (!Sensors.isDoorClosed()) {
        // Puerta abierta - bloquear puerta
        Utils.debug("🔒 Cerrando y bloqueando puerta");
        Actuators.lockDoor();
        Hardware.nextionSetText(NEXTION_COMP_MSG, "PUERTA BLOQUEADA");
        
        // Actualizar texto del botón después de bloquear
        Hardware.nextionSetText(NEXTION_COMP_BTN_START, "INICIAR");
        
        // Breve pausa para mostrar mensaje
        delay(1000);
        Hardware.nextionSetText(NEXTION_COMP_MSG, "");
      } else {
        // Puerta cerrada - iniciar programa
        Utils.debug("▶️ Iniciando programa " + String(_currentProgram + 22));
        startProgram();
      }
      break;
      
    case NEXTION_ID_BTN_EDIT:
      // Entrar en modo de edición para el programa seleccionado
      Serial.println("=== BOTÓN EDIT PRESIONADO ===");
      Serial.println("_currentProgram = " + String(_currentProgram));
      
      // Depuración: Mostrar todos los valores almacenados
      // Storage.debugPrintAllPrograms();
      
      Utils.debug("✏️ Editando programa " + String(_currentProgram + 22));
      // Utils.debug("🔧 Llamando a startEditing(" + String(_currentProgram) + ", 0)");
      startEditing(_currentProgram, 0); // Comenzar editando la primera fase
      // Utils.debug("🔧 startEditing() completado, estado actual: " + String(_currentState));
      break;
      
    default:
      Utils.debug("⚠️ Componente no reconocido en página de selección: " + String(componentId));
      break;
  }
}

void ProgramControllerClass::_handleEditPageEvents(uint8_t componentId) {
  // Utils.debug("🔧 Delegando evento de edición a UIController - Componente: " + String(componentId));
  
  // Delegar completamente el manejo de eventos al UIController
  UIController.handleEditPageEvent(componentId);
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

/// @brief 
/// Actualiza la pantalla de edición con los valores actuales.
/// Este método se encarga de mostrar los valores actuales de los parámetros del programa en la pantalla de edición.
/// Se debe llamar después de realizar cambios en los parámetros para reflejarlos en la interfaz de usuario.
void ProgramControllerClass::_updateEditDisplay() {
  // Actualizar la pantalla de edición con los valores actuales
  Serial.println("🔄 Actualizando valores en pantalla de edición");
  
  // Solo actualizar la pantalla, no reinicializar todo
  UIController.updateEditDisplay();
}

void ProgramControllerClass::_loadProgramData() {
  // Cargar todos los datos de programa desde almacenamiento
  for (uint8_t prog = 0; prog < NUM_PROGRAMAS; prog++) {
    for (uint8_t fase = 0; fase < NUM_FASES; fase++) {
      _waterLevels[prog][fase] = Storage.loadWaterLevel(prog, fase);
      _temperatures[prog][fase] = Storage.loadTemperature(prog, fase);
      _times[prog][fase] = Storage.loadTime(prog, fase);
      _rotations[prog][fase] = Storage.loadRotation(prog, fase);
      _tipoAguaPrograma[prog][fase] = Storage.loadTipoAgua(prog, fase);
      _centrifugadoPrograma[prog][fase] = Storage.loadCentrifugado(prog, fase);
    }
  }
  
  // Cargar el estado actual del programa
  _loadCurrentProgramState();
  
  Utils.debug("Datos de programa cargados desde almacenamiento");
}
