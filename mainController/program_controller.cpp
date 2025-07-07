// program_controller.cpp
#include "program_controller.h"

// Instancia global
ProgramControllerClass ProgramController;

void ProgramControllerClass::init() {
  // Inicializar variables de estado
  _currentState = ESTADO_SELECCION;
  _previousState = ESTADO_SELECCION;
  _currentProgram = 0; // Inicializar con programa P22 (índice 0) por defecto
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
  _pauseActuatorsStopped = false;
  _pausedPreparingPhase = false;

  // Inicializar variables de estado de fase
  _preparingPhase = false;
  _phaseStartTime = 0;

  // Inicializar variables de secuencia final
  _finalDrainMinutes = 0;
  _finalDrainSeconds = 0;
  _doorWaitMinutes = 0;
  _doorWaitSeconds = 0;

  // Inicializar variables de edición
  _editingProgram = 0; // Inicializar con programa P22 (índice 0) por defecto
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
/// Este método solo carga el programa y fase actual, los demás datos se
/// obtienen directamente de Storage.
void ProgramControllerClass::_loadCurrentProgramState() {
  // Cargar el último programa y fase utilizados
  _currentProgram = Storage.loadProgram();
  _currentPhase = Storage.loadPhase();

  Utils.debug("Estado del programa cargado: P" + String(_currentProgram + 22) +
              " F" + String(_currentPhase));
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

    case ESTADO_EJECUCION:
      _initializeProgram();
      _configureActuatorsForPhase();
      UIController.showExecutionScreen(
          _currentProgram, _currentPhase,
          _waterLevels[_currentProgram][_currentPhase],
          _temperatures[_currentProgram][_currentPhase],
          _rotations[_currentProgram][_currentPhase]);
      break;

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

    case ESTADO_ESPERA_PUERTA:
      // Inicializar espera para abrir puerta (enfriando)
      _doorWaitMinutes = TIEMPO_PUERTA_BLOQUEO / 60;
      _doorWaitSeconds = TIEMPO_PUERTA_BLOQUEO % 60;
      // Durante enfriamiento: drenaje abierto para goteo, centrifugado y
      // motores apagados
      Actuators
          .openDrainValve(); // Mantener abierto para goteo de últimas gotas
      Actuators.stopMotor(); // Motores de dirección apagados
      Actuators.stopAutoRotation(); // Sin rotación automática
      Actuators.stopCentrifuge();   // Centrifugado apagado
      Actuators.closeSteamValve();  // Cerrar vapor
      Actuators.closeWaterValve();  // Cerrar agua
      UIController.updatePhase(5);  // Mostrar "Enfriando"
      Hardware.nextionSetText(NEXTION_COMP_MSG, "Enfriando - Espere");
      break;

    case ESTADO_CENTRIFUGADO:
      // Inicializar centrifugado
      _centrifugeMinutes = TIEMPO_CENTRIFUGADO / 60;
      _centrifugeSeconds = TIEMPO_CENTRIFUGADO % 60;
      UIController.updatePhase(2); // Mostrar "Centrifugando"
      Hardware.nextionSetText(NEXTION_COMP_MSG, "Centrifugando...");
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
    Utils.debugValue("Programa seleccionado", program);
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

void ProgramControllerClass::startProgram() {
  if (_currentState == ESTADO_SELECCION) {
    // === VERIFICACIÓN INICIAL DE PUERTA SEGÚN DOCUMENTO DEL CLIENTE ===
    if (!Actuators.isDoorLocked()) {
      // Mostrar advertencia en componente de mensajes de Nextion
      Hardware.nextionSetText(NEXTION_COMP_MSG, "PUERTA ABIERTA");
      return;
    } else {
      // Limpiar mensaje si la puerta está cerrada
      Hardware.nextionSetText(NEXTION_COMP_MSG, "");
    }

    setState(ESTADO_EJECUCION);

    // Asegurar que el botón pausar muestre "PAUSAR" al iniciar
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "PAUSAR");
    Hardware.nextionSetText(NEXTION_COMP_MSG, "Programa P" +
                                                  String(_currentProgram + 22) +
                                                  " iniciado");
  }
}

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

    // Mostrar pantalla de ejecución
    UIController.showExecutionScreen(
        _currentProgram, _currentPhase,
        _waterLevels[_currentProgram][_currentPhase],
        _temperatures[_currentProgram][_currentPhase],
        _rotations[_currentProgram][_currentPhase]);

    // Cambiar texto del botón de vuelta a "PAUSAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_PAUSAR, "PAUSAR");

    // Actualizar display con tiempo restaurado
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    Hardware.nextionSetText(NEXTION_COMP_MSG, "Programa reanudado");
  }
}

void ProgramControllerClass::stopProgram() {
  if (_currentState == ESTADO_EJECUCION || _currentState == ESTADO_PAUSA ||
      _currentState == ESTADO_CENTRIFUGADO) {
    Actuators.emergencyStop(); // Detener todos los actuadores de forma segura
    setState(ESTADO_SELECCION);
    Utils.debug("Programa detenido: " + String(_currentProgram));
  }
}

uint8_t ProgramControllerClass::getCurrentPhase() { return _currentPhase; }

void ProgramControllerClass::nextPhase() {
  if (_currentPhase < NUM_FASES - 1) {
    _currentPhase++;

    // === VERIFICAR SI LA SIGUIENTE FASE ES CENTRIFUGADO Y ESTÁ DESHABILITADA
    // === Fase 2 = Centrifugado
    if (_currentPhase == 2 &&
        !_isCentrifugadoEnabled(_currentProgram, _currentPhase)) {
      // Centrifugado está deshabilitado en esta tanda, saltar al drenaje (fase
      // 3)
      Utils.debug(
          "⏭️ Saltando centrifugado (deshabilitado) - Avanzando a drenaje");
      _currentPhase++; // Avanzar a fase 3 (drenaje)
      Storage.savePhase(_currentPhase);
      _updatePhaseParameters();
      return;
    }

    Storage.savePhase(_currentPhase);
    _updatePhaseParameters();
    Utils.debugValue("Avanzado a la fase: ", _currentPhase);
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
    UIController.updatePhase(_currentPhase); // Mostrar la fase actual en la UI
    UIController.updateTime(_remainingMinutes, _remainingSeconds);
    UIController.updateProgressBar(
        0); // Inicializar barra de progreso en 0% para nueva fase

    Utils.debug("📌 Nueva fase iniciada: " + String(_currentPhase));
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

    // Solo actualizar actuadores si no se está manejando desde
    // _handleExecutionState
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

  // Nota: El progreso se actualiza en _handleExecutionState() usando
  // getProgressPercentage()
}

/// @brief
/// Verifica las condiciones de los sensores y actualiza los actuadores según la
/// fase actual. Este método se encarga de verificar si se han alcanzado las
/// condiciones de temperatura y nivel de agua
void ProgramControllerClass::_checkSensorConditions() {
  // Verificar las condiciones de temperatura y nivel de agua según la fase
  // actual
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);

  // Actualizar la interfaz con los valores actuales
  UIController.updateTemperature(Sensors.getCurrentTemperature());
  UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());

  // Si estamos preparando la fase, mostrar el estado en Nextion
  if (_preparingPhase) {
    String statusMsg = "Preparando: ";
    bool waterOk = Sensors.isWaterLevelReached(targetLevel);
    bool tempOk = Sensors.isTemperatureReached(targetTemp);

    if (!waterOk) {
      statusMsg += "Llenando... ";
    }
    if (!tempOk && (_currentProgram == 0)) { // Solo P22 requiere calentamiento
      statusMsg += "Calentando... ";
    }
    if (waterOk && tempOk) {
      statusMsg = "Iniciando ciclo...";
    }
    Hardware.nextionSetText(NEXTION_COMP_MSG, statusMsg);
  }

  // Control de llenado y temperatura por programa
  // P22 (índice 0): Agua caliente - usar vapor/electroválvula para llenado y
  // calentamiento
  if (_currentProgram == 0) {
    if (Sensors.getCurrentWaterLevel() < targetLevel ||
        Sensors.getCurrentTemperature() < targetTemp) {
      Actuators.openSteamValve(); // PIN_ELECTROV_VAPOR para agua caliente
    } else {
      Actuators.closeSteamValve();
    }
  }
  // P23 (índice 1): Agua fría, sin calentamiento
  else if (_currentProgram == 1) {
    if (Sensors.getCurrentWaterLevel() < targetLevel ||
        Sensors.getCurrentTemperature() < targetTemp) {
      Actuators.openWaterValve();
    } else {
      Actuators.closeWaterValve();
    }
  }
  // P24 (índice 2): Configurable según parámetros
  else if (_currentProgram == 2) {
    // Obtener tipo de agua para esta fase específica
    uint8_t tipoAgua = Storage.loadTipoAgua(_currentProgram, _currentPhase);

    // Control diferenciado según tipo de agua
    if (tipoAgua == 1) {
      // AGUA CALIENTE: Control completo de nivel y temperatura
      bool needsWater = Sensors.getCurrentWaterLevel() < targetLevel;
      bool needsHeat = Sensors.getCurrentTemperature() < targetTemp;

      if (needsWater) {
        // Abrir válvula de agua normal para llenar
        Actuators.openWaterValve();
      } else {
        // Nivel alcanzado, cerrar válvula de agua
        Actuators.closeWaterValve();
      }

      if (needsHeat && Sensors.getCurrentWaterLevel() >= targetLevel) {
        // Solo activar vapor cuando ya hay suficiente agua para calentamiento
        Actuators.openSteamValve();
      } else {
        // Temperatura alcanzada o sin agua suficiente, cerrar vapor
        Actuators.closeSteamValve();
      }
    } else {
      // AGUA FRÍA: Solo control de nivel (sin temperatura)
      if (Sensors.getCurrentWaterLevel() < targetLevel) {
        // Solo llenar con agua fría normal
        Actuators.openWaterValve();
      } else {
        // Nivel alcanzado, cerrar válvula
        Actuators.closeWaterValve();
      }

      // Asegurar que vapor esté cerrado (agua fría no requiere calentamiento)
      Actuators.closeSteamValve();
    }
  }

  // Verificar si se han alcanzado las condiciones para iniciar el temporizador
  // En fase 1 (lavado), las condiciones ya se cumplieron en llenado, iniciar
  // directamente
  bool conditionsReached = false;
  if (_currentPhase == 1) {
    // En lavado, asumir que condiciones ya se cumplieron en llenado
    conditionsReached = true;
  } else {
    // En otras fases, verificar condiciones normalmente
    conditionsReached = Sensors.isWaterLevelReached(targetLevel) &&
                        Sensors.isTemperatureReached(targetTemp);
  }

  if (conditionsReached && !_timerRunning) {

    // Iniciar rotación automática SOLO si estamos en fase 1 (lavado)
    if (_currentPhase == 1) { // Solo en fase de lavado
      uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
      if (rotLevel > 0 && !Actuators.isAutoRotationActive()) {
        Actuators.startAutoRotation(rotLevel);
        Hardware.nextionSetText(NEXTION_COMP_MSG,
                                "Rotacion L" + String(rotLevel) + " iniciada");
      }
    }

    // Iniciar el temporizador cuando se alcanzan las condiciones necesarias
    _timerRunning = true;
    _preparingPhase = false;

    // Reiniciar el temporizador a los valores configurados
    _remainingMinutes = _totalMinutes;
    _remainingSeconds = 0;

    // Actualizar la fase en la pantalla
    UIController.updatePhase(_currentPhase);

    // Inicializar barra de progreso desde el inicio de esta fase
    UIController.updateProgressBar(
        0); // Comenzar desde 0% cuando se cumplen las condiciones

    Hardware.nextionSetText(NEXTION_COMP_MSG,
                            "Fase " + String(_currentPhase + 1) + " iniciada");
  }
}

void ProgramControllerClass::_completePhase() {
  Utils.debug("Completando P" + String(_currentProgram + 22) + " F" +
              String(_currentPhase));

  // === LÓGICA DE TRANSICIÓN SEGÚN LA FASE ACTUAL ===

  // Fase 1 (Lavado) → Verificar si va a Centrifugado (Fase 2) o Drenaje (Fase
  // 3)
  if (_currentPhase == 1) {
    bool centrifugadoEnabled = _isCentrifugadoEnabled(_currentProgram, 2);
    if (centrifugadoEnabled) {
      // Ir directamente a centrifugado sin pasar por nextPhase()
      _currentPhase = 2; // Establecer fase 2
      Storage.savePhase(_currentPhase);

      Utils.debug("Iniciando centrifugado");

      // Configurar actuadores para centrifugado
      Actuators.closeSteamValve();
      Actuators.closeDrainValve(); // NO drenar durante centrifugado
      Actuators.closeWaterValve();
      Actuators.startCentrifuge();
      Actuators.stopAutoRotation();
      Actuators.stopMotor();

      // Cambiar al estado de centrifugado
      setState(ESTADO_CENTRIFUGADO);
      return;
    } else {
      // Saltar centrifugado, ir directamente a drenaje
      _currentPhase = 2; // Saltar centrifugado
      nextPhase();       // Va a fase 3 (drenaje)
      return;
    }
  }

  // Fase 3 (Drenaje) → Lógica según el programa
  if (_currentPhase == 3) {
    // === PARA P22/P23: IR DIRECTAMENTE A ENFRIANDO ===
    if (_currentProgram == 0 || _currentProgram == 1) {
      Utils.debug("P" + String(_currentProgram + 22) +
                  " - Drenaje completado, iniciando enfriado");

      // Configurar actuadores para drenaje y enfriado
      Actuators.closeSteamValve();
      Actuators.openDrainValve();
      Actuators.closeWaterValve();
      Actuators.stopCentrifuge();
      Actuators.stopAutoRotation();
      Actuators.stopMotor();

      // Ir directamente a enfriando (sin drenaje final)
      setState(ESTADO_ESPERA_PUERTA);
      return;
    }

    // === PARA P24: VERIFICAR SI HAY MÁS TANDAS ===
    if (_currentProgram == 2) {
      // Verificar si hay más tandas pendientes
      if (_tandaCounter < _maxTandas - 1) {
        _tandaCounter++;
        Utils.debug("P24 - Completando tanda " + String(_tandaCounter + 1) +
                    " de " + String(_maxTandas));

        // Reiniciar desde la primera fase (Llenado) para nueva tanda
        _currentPhase = 0;
        Storage.savePhase(_currentPhase);

        // Reinicializar para nueva tanda
        _updatePhaseParameters();
        _preparingPhase = true;
        _phaseStartTime = millis();

        Utils.debug("Iniciando tanda " + String(_tandaCounter + 1) +
                    " - Volviendo a Fase 0 (Llenado)");

        // Mostrar progreso de tandas en UI
        String tandaMsg =
            "Tanda " + String(_tandaCounter + 1) + "/" + String(_maxTandas);
        UIController.showMessage(tandaMsg, 3000);

        return;
      } else {
        // Última tanda de P24 - ir a enfriando
        Utils.debug("P24 - Todas las tandas completadas, iniciando enfriado");
        setState(ESTADO_ESPERA_PUERTA);
        return;
      }
    }
  }

  // Para fase 0 (Llenado) - avanzar a lavado (fase 1)
  if (_currentPhase == 0) {
    nextPhase(); // Avanza a fase 1 (lavado)
    return;
  }

  // Para fase sin manejar - esto no debería ocurrir, pero por seguridad
  if (_currentPhase < 3) {
    nextPhase();
    return;
  }
}

void ProgramControllerClass::_completeProgram() {
  Utils.debug("Programa completado");
  // Para P22/P23 ir directamente a enfriando, para P24 también ir directamente
  // a enfriando
  setState(ESTADO_ESPERA_PUERTA); // Enfriando para todos los programas
}

void ProgramControllerClass::_initializeProgram() {
  // === INICIALIZACIÓN SEGÚN DOCUMENTO DEL CLIENTE ===

  // 1. Bloquear puerta (ya verificada en startProgram)
  // Actuators.lockDoor();

  // 2. Inicializar fase al comienzo (LLENADO)
  _currentPhase = 0; // Siempre comenzar desde fase 0 (llenado)
  Storage.savePhase(_currentPhase);

  // 3. Inicializar variables del programa
  _totalMinutes = Storage.loadTime(_currentProgram, _currentPhase);
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
  _tandaCounter = 0;
  _maxTandas = (_currentProgram == 2) ? 3 : 1; // P24=3 tandas, P22/P23=1 tanda

  // 7. Actualizar UI para mostrar la fase inicial correcta (llenado)
  UIController.updatePhase(
      _currentPhase); // Asegurar que muestre fase 0 (llenado)
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
  float targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);

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
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);
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
  Utils.debug("Centrifugado P" + String(programa + 22) + " T" +
              String(tandaActual + 1) + ": " + String(enabled ? "ON" : "OFF"));

  return enabled;
}

void ProgramControllerClass::_configureActuatorsForPhase() {
  // Configurar actuadores según el flujo específico del Programa 22
  uint8_t targetLevel = Storage.loadWaterLevel(_currentProgram, _currentPhase);
  uint8_t targetTemp = Storage.loadTemperature(_currentProgram, _currentPhase);

  // ESTADO INICIAL: Al presionar "iniciar" (antes de que llegue agua y
  // temperatura)
  if (_preparingPhase) {
    // 1) PIN_VALVULA_DESFOGUE OFF - debe llenar agua
    Actuators.closeDrainValve();

    // 2) Temporizador OFF (ya manejado en _timerRunning = false)

    // 3) PIN_ELECTROV_VAPOR ON - ingresa agua caliente (solo Programa 22)
    if (_currentProgram == 0) { // P22 = agua caliente
      Utils.debug("🔥 P22: Activando vapor para agua caliente");
      if (Sensors.getCurrentWaterLevel() < targetLevel)
        Actuators.openSteamValve();
    } else if (_currentProgram == 1) { // P23 = agua fría
      Utils.debug("❄️ P23: Activando agua fría (sin vapor)");
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
    // 1) Temporizador ON (ya manejado en _timerRunning = true)

    // 2) Cerrar válvulas (ya no ingresa agua)
    Actuators.closeSteamValve();
    Actuators.closeWaterValve();

    // 3) PIN_VALVULA_DESFOGUE OFF - mantener agua
    Actuators.closeDrainValve();

    // 4) PIN_CENTRIFUGADO OFF - centrifugado solo en fase 2 específica
    if (_currentPhase != 2) {
      Actuators.stopCentrifuge();
    }

    // 5) Motores ON con permutación - activar rotación SOLO en fase 1 (lavado)
    if (_currentPhase == 1) { // Solo en fase de lavado
      uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
      if (rotLevel > 0 && !Actuators.isAutoRotationActive()) {
        Actuators.startAutoRotation(rotLevel);
        Utils.debug("🔄 Iniciando rotación con permutación nivel: " +
                    String(rotLevel));
      }
    } else {
      // En todas las demás fases, asegurar que motores estén apagados
      Actuators.stopAutoRotation();
      Actuators.stopMotor();
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
  uint8_t progress = 100 - ((remainingTotal * 100) / _totalSeconds);

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
    _editingProgram = program;
    _editingPhase = phase;
    _editingParameter = PARAM_NIVEL; // Comenzar editando el nivel
    _editingParameterValue =
        _waterLevels[program][phase]; // Cargar valor actual
    _isEditing = true;

    setState(ESTADO_EDICION);

    // Mostrar pantalla de edición
    // _updateEditDisplay();
    UIController.updateEditDisplay();
  }
}

void ProgramControllerClass::editParameter(uint8_t paramType, uint8_t value) {
  if (!_isEditing)
    return;

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
  Storage.saveWaterLevel(_editingProgram, _editingPhase,
                         _waterLevels[_editingProgram][_editingPhase]);
  Storage.saveTemperature(_editingProgram, _editingPhase,
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
  // Verificar si hay un evento táctil válido
  if (!Hardware.hasValidTouchEvent()) {
    // Utils.debug("⚠️ Evento táctil no válido recibido: " + event);
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
    return;                                  // Ignorar evento duplicado
  }

  lastPage = touchPage;
  lastComponent = touchComponent;
  lastEventTime = currentTime;

  // Debug solo para componentes importantes (botones de control)
  if (touchComponent == NEXTION_ID_BTN_PARAR ||
      touchComponent == NEXTION_ID_BTN_PAUSAR ||
      touchComponent == NEXTION_ID_BTN_START) {
    Utils.debug("🎯 Evento botón control - Página: " + String(touchPage) +
                ", Componente: " + String(touchComponent));
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

  case ESTADO_ESPERA_PUERTA:
    _handleDoorWaitState();
    break;

  case ESTADO_CENTRIFUGADO:
    _handleCentrifugeState();
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

  // 2. Verificar si estamos preparando la fase de preparación
  if (_preparingPhase) {
    // Mostrar estado de preparación en UI
    // Mostrar mensaje de preparación
    UIController.updatePhase(0); // Mostrar fase actual durante preparación

    // Verificar si se alcanzaron las condiciones necesarias
    _checkSensorConditions();

    // Si las condiciones se cumplieron, iniciar el temporizador
    if (!_preparingPhase) {
      Utils.debug("✅ Condiciones alcanzadas - iniciando temporizador de fase");
      // UIController.clearPreparationStatus();
      UIController.updatePhase(1); // Mostrar fase actual una vez que comienza
    }
    return;
  }

  // 3. Actualizar temporizadores (solo si está corriendo)
  if (_timerRunning) {
    // Actualizar display y decrementar temporizador EXACTAMENTE cada segundo
    static unsigned long lastSecondUpdate = 0;
    static unsigned long lastSensorUpdate = 0;
    unsigned long currentTime = millis();

    // Actualizar tiempo cada segundo
    if (currentTime - lastSecondUpdate >= 1000) {
      lastSecondUpdate = currentTime;
      _decrementTimer();
      UIController.updateTime(_remainingMinutes, _remainingSeconds);
      UIController.updateProgressBar(
          getProgressPercentage()); // Usar progreso de fase actual
      Actuators.updateTimers();
    }

    // Actualizar sensores solo cada 3 segundos para reducir saturación
    if (currentTime - lastSensorUpdate >= 3000) {
      lastSensorUpdate = currentTime;
      UIController.updateTemperature(Sensors.getCurrentTemperature());
      UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
      // UIController.updateRotation(Actuators.getCurrentRotationLevel());
    }

    // Verificar si la fase terminó
    if (_remainingMinutes == 0 && _remainingSeconds == 0) {
      _completePhase();
      // La UI se actualiza automáticamente en _completePhase() y las
      // transiciones de estado
    }
  }

  // 4. Control de temperatura con drenaje parcial según documento del cliente
  _handleTemperatureControl();

  // 5. Control del motor SOLO en fase 1 (lavado)
  if (_currentPhase == 1 &&
      Storage.loadRotation(_currentProgram, _currentPhase) > 0) {
    // Asegurar que el motor esté funcionando con el patrón correcto solo en
    // lavado
    if (!Actuators.isMotorRunning()) {
      uint8_t rotLevel = Storage.loadRotation(_currentProgram, _currentPhase);
      Actuators.startAutoRotation(rotLevel);
    }
  } else {
    // En todas las demás fases o sin rotación - motores apagados
    if (Actuators.isMotorRunning()) {
      Actuators.stopMotor();
      Actuators.stopAutoRotation();
      Utils.debug("⏹️ Motor detenido - Solo funciona en fase 1 (lavado)");
    }
  }
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

void ProgramControllerClass::_triggerError(uint8_t errorCode,
                                           const String &errorMessage) {
  // Activar estado de error con código y mensaje específicos
  Utils.debug("ERROR: " + errorMessage);
  setState(ESTADO_ERROR);
  UIController.showErrorScreen(errorCode, errorMessage);
}

// ===== IMPLEMENTACIÓN DE MANEJO DE EVENTOS TÁCTILES =====

void ProgramControllerClass::_handleSelectionPageEvents(uint8_t componentId) {
  Utils.debug("🔘 Evento en página de selección - Componente: " +
              String(componentId));

  switch (componentId) {
  case NEXTION_ID_BTN_PROGRAM1:
    // Seleccionar programa 1 directamente (P22) - índice interno 0
    _currentProgram = 0; // Índice interno 0 = P22
    Storage.saveProgram(_currentProgram);
    Utils.debug("📋 Programa 1 seleccionado directamente (P22)");
    UIController.showSelectionScreen(
        _currentProgram); // Pasar índice 0 directamente
    break;

  case NEXTION_ID_BTN_PROGRAM2:
    // Seleccionar programa 2 directamente (P23) - índice interno 1
    _currentProgram = 1; // Índice interno 1 = P23
    Storage.saveProgram(_currentProgram);
    Utils.debug("📋 Programa 2 seleccionado directamente (P23)");
    UIController.showSelectionScreen(
        _currentProgram); // Pasar índice 1 directamente
    break;

  case NEXTION_ID_BTN_PROGRAM3:
    // Seleccionar programa 3 directamente (P24) - índice interno 2
    _currentProgram = 2; // Índice interno 2 = P24
    Storage.saveProgram(_currentProgram);
    Utils.debug("📋 Programa 3 seleccionado directamente (P24)");
    UIController.showSelectionScreen(
        _currentProgram); // Pasar índice 2 directamente
    break;

  case NEXTION_ID_BTN_START:
    // Verificar estado de puerta para determinar acción
    if (!Sensors.isDoorClosed()) {
      // Puerta abierta - bloquear puerta
      Utils.debug("🔒 Cerrando y bloqueando puerta");
      Actuators.lockDoor();
      Hardware.nextionSetText(NEXTION_COMP_MSG, "PUERTA BLOQUEADA");

      // Actualizar texto del botón después de bloquear
      Hardware.nextionSetText(NEXTION_COMP_BTN_START, "INICIAR");
    } else {
      // Puerta cerrada - iniciar programa
      Utils.debug("▶️ Iniciando programa " + String(_currentProgram + 22));
      startProgram();
    }
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
  // Utils.debug("🔧 Delegando evento de edición a UIController - Componente: "
  // + String(componentId));

  // Delegar completamente el manejo de eventos al UIController
  UIController.handleEditPageEvent(componentId);
}

void ProgramControllerClass::_handleExecutionPageEvents(uint8_t componentId) {
  Utils.debug("⚙️ Evento en página de ejecución - Componente: " +
              String(componentId));

  switch (componentId) {
  case NEXTION_ID_BTN_PAUSAR:
    // Pausar/reanudar programa
    if (_currentState == ESTADO_EJECUCION) {
      // Solo permitir pausa si no estamos en fase de preparación
      if (_preparingPhase) {
        Utils.debug("⚠️ Pausa bloqueada durante preparación");
        Hardware.nextionSetText(NEXTION_COMP_MSG,
                                "No se puede pausar durante preparación");
      } else {
        Utils.debug("⏸️ Pausando programa");
        pauseProgram();
      }
    } else if (_currentState == ESTADO_PAUSA) {
      Utils.debug("▶️ Reanudando programa");
      resumeProgram();
    }
    break;

  case NEXTION_ID_BTN_PARAR:
    // Detener programa completamente
    if (_currentState == ESTADO_ESPERA_PUERTA) {
      // Bloquear detener durante espera de puerta (es crítico para seguridad)
      Utils.debug("⚠️ Detener bloqueado durante espera de puerta");
      Hardware.nextionSetText(NEXTION_COMP_MSG,
                              "No se puede detener durante enfriamiento");
    } else if (_currentState == ESTADO_EJECUCION ||
               _currentState == ESTADO_PAUSA ||
               _currentState == ESTADO_CENTRIFUGADO) {
      // Permitir detener en ejecución, pausa y centrifugado
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
  // Procesar eventos de interfaz de usuario
  if (UIController.hasUserAction()) {
    String action = UIController.getUserAction();
    processUserEvent(action);
  }

  // Actualizar según el estado actual
  _handleStateMachine();
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

void ProgramControllerClass::_handleDoorWaitState() {
  // Manejar el estado de espera para abrir puerta
  static unsigned long lastSecondUpdate = 0;
  static unsigned long lastSensorUpdate = 0;
  unsigned long currentTime = millis();

  // Actualizar cada segundo
  if (currentTime - lastSecondUpdate >= 1000) {
    lastSecondUpdate = currentTime;

    // Decrementar temporizador de espera
    if (_doorWaitSeconds > 0) {
      _doorWaitSeconds--;
    } else if (_doorWaitMinutes > 0) {
      _doorWaitMinutes--;
      _doorWaitSeconds = 59;
    } else {
      // Espera completada, finalizar programa
      Hardware.nextionSetText(NEXTION_COMP_MSG, "Programa terminado");
      Actuators.stopCentrifuge(); // Asegurar que centrifugado esté detenido
                                  // antes de finalizar
      _finalizeProgramSequence();
      return;
    }

    // Actualizar display con tiempo de espera
    UIController.updateTime(_doorWaitMinutes, _doorWaitSeconds);

    // Mostrar mensaje de estado
    String waitMsg = "Enfriando: " + String(_doorWaitMinutes) + ":" +
                     String(_doorWaitSeconds < 10 ? "0" : "") +
                     String(_doorWaitSeconds);
    Hardware.nextionSetText(NEXTION_COMP_MSG, waitMsg);
  }

  // Actualizar sensores cada 3 segundos para mostrar valores en tiempo real
  if (currentTime - lastSensorUpdate >= 3000) {
    lastSensorUpdate = currentTime;
    UIController.updateTemperature(Sensors.getCurrentTemperature());
    UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());

    // Asegurar que centrifugado y motores estén apagados durante espera de
    // puerta
    Actuators.stopCentrifuge();
    Actuators.stopMotor();
    Actuators.stopAutoRotation();
  }
}

void ProgramControllerClass::_handleCentrifugeState() {
  // Manejar el estado de centrifugado
  static unsigned long lastSecondUpdate = 0;
  static unsigned long lastSensorUpdate = 0;
  unsigned long currentTime = millis();

  // Actualizar cada segundo
  if (currentTime - lastSecondUpdate >= 1000) {
    lastSecondUpdate = currentTime;

    // Decrementar temporizador de centrifugado
    if (_centrifugeSeconds > 0) {
      _centrifugeSeconds--;
    } else if (_centrifugeMinutes > 0) {
      _centrifugeMinutes--;
      _centrifugeSeconds = 59;
    } else {
      // Centrifugado completado, avanzar a la fase de drenaje (fase 3)
      Utils.debug("Centrifugado completado - Avanzando a fase de drenaje");
      Actuators.stopCentrifuge();

      // Avanzar a la fase 3 (Drenaje) y continuar ejecución normal
      _currentPhase = 3;
      Storage.savePhase(_currentPhase);
      _updatePhaseParameters();

      setState(
          ESTADO_EJECUCION); // Volver a ejecución para completar el drenaje
      return;
    }

    // Actualizar display con tiempo de centrifugado
    UIController.updateTime(_centrifugeMinutes, _centrifugeSeconds);

    // Mostrar mensaje de estado
    String centrifugeMsg = "Centrifugando: " + String(_centrifugeMinutes) +
                           ":" + String(_centrifugeSeconds < 10 ? "0" : "") +
                           String(_centrifugeSeconds);
    Hardware.nextionSetText(NEXTION_COMP_MSG, centrifugeMsg);
  }

  // Actualizar sensores cada 3 segundos para mostrar valores en tiempo real
  if (currentTime - lastSensorUpdate >= 3000) {
    lastSensorUpdate = currentTime;
    UIController.updateTemperature(Sensors.getCurrentTemperature());
    UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());

    // Asegurar que motores estén apagados durante centrifugado (solo
    // centrifugado debe estar activo)
    Actuators.stopMotor();
    Actuators.stopAutoRotation();
  }
}
