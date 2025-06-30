// actuators.cpp
#include "actuators.h"
#include "utils.h"

// Instancia global
ActuatorsClass Actuators;

// === CALLBACKS ESTÁTICOS PARA TAREAS TEMPORIZADAS ===
void rotationTimerCallback() {
  Actuators.incrementMotorSeconds();
  Actuators.updateMotorDirection();
}

void balanceDetectionCallback() {
  Actuators._performBalanceDetection();
}

void partialDrainCompleteCallback() {
  Actuators.completePartialDrain();
}

void balanceCheckTimeoutCallback() {
  Actuators.handleBalanceCheckResult();
}

void motorRedistributionForwardCallback() {
  Actuators.handleMotorRedistributionForward();
}

void motorRedistributionReverseCallback() {
  Actuators.handleMotorRedistributionReverse();
}

void motorRedistributionCompleteCallback() {
  Actuators.handleMotorRedistributionComplete();
}

void temperatureAdjustmentFillCallback() {
  Actuators.handleTemperatureAdjustmentFill();
}

void detergentDistributionPhase2Callback() {
  Actuators.handleDetergentDistributionPhase2();
}

void detergentDistributionCompleteCallback() {
  Actuators.handleDetergentDistributionComplete();
}

ActuatorsClass::~ActuatorsClass() {
  // Detener cualquier tarea activa
  if (_rotationTaskId > 0) {
    Utils.stopTask(_rotationTaskId);
    _rotationTaskId = 0;
  }
}

void ActuatorsClass::init() {
  // Inicializar estados
  _motorState = MOTOR_OFF;
  _waterValveOpen = false;
  _steamValveOpen = false;
  _drainValveOpen = false;
  _doorLocked = false;
  _autoRotationActive = false;
  _centrifugeActive = false;
  
  // Inicializar variables de rotación
  _motorSeconds = 0;
  _currentRotationLevel = 0;
  _rotationDirection = MOTOR_FORWARD;
  _forwardTime = 0;
  _reverseTime = 0;
  _pauseTime = 0;
  
  // Inicializar variables de centrifugado avanzado
  _centrifugeState = CENTRIFUGE_IDLE;
  _centrifugeTargetSpeed = 0;
  _centrifugeCurrentSpeed = 0;
  _centrifugeRampTime = 0;
  _centrifugeStartTime = 0;
  _lastSpeedUpdate = 0;
  _centrifugeSequenceType = CENTRIFUGE_SEQ_NORMAL;
  _balanceAttempts = 0;
  _balanceCheckPassed = false;
  
  // Inicializar variables de drenaje parcial
  _partialDrainActive = false;
  _partialDrainPercentage = 0;
  _partialDrainDuration = 0;
  _partialDrainStartTime = 0;
  _partialDrainTaskId = 0;
  
  // Inicializar IDs de tareas
  _rotationTaskId = 0;
  _lastRotationUpdate = 0;
  
  // Asegurar que todos los actuadores estén en estado seguro
  stopMotor();
  stopCentrifuge();
  closeWaterValve();
  closeSteamValve();
  openDrainValve(); // Por seguridad, la válvula de drenaje se abre al iniciar
  unlockDoor();
  
  Utils.debug("ActuatorsClass::init| Actuadores inicializados");
}

void ActuatorsClass::startMotorForward() {
  Hardware.digitalWrite(PIN_MOTOR_DIR_A, HIGH);
  Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
  _motorState = MOTOR_FORWARD;
  Utils.debug("Motor iniciado en dirección adelante");
}

void ActuatorsClass::startMotorReverse() {
  Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_B, HIGH);
  _motorState = MOTOR_REVERSE;
  Utils.debug("Motor iniciado en dirección reversa");
}

void ActuatorsClass::stopMotor() {
  Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
  _motorState = MOTOR_OFF;
  Utils.debug("Motor detenido");
}

bool ActuatorsClass::isMotorRunning() {
  return _motorState != MOTOR_OFF;
}

uint8_t ActuatorsClass::getMotorState() {
  return _motorState;
}

void ActuatorsClass::setRotationLevel(uint8_t level) {
  if (level > MAX_NIVEL_ROTACION) level = MAX_NIVEL_ROTACION;
  _currentRotationLevel = level;
  _configureRotationTiming(level);
}

void ActuatorsClass::_configureRotationTiming(uint8_t level) {
  // Configurar los tiempos según el nivel de rotación
  switch (level) {
    case 0: // Sin rotación
      _forwardTime = 0;
      _reverseTime = 0;
      _pauseTime = 0;
      break;
    case 1: // Rotación suave
      _forwardTime = 5;
      _reverseTime = 5;
      _pauseTime = 3;
      break;
    case 2: // Rotación media
      _forwardTime = 8;
      _reverseTime = 8;
      _pauseTime = 2;
      break;
    case 3: // Rotación intensa
      _forwardTime = 12;
      _reverseTime = 12;
      _pauseTime = 1;
      break;
  }
}

void ActuatorsClass::startCentrifuge() {
  // Iniciar el centrifugado
  Hardware.digitalWrite(PIN_CENTRIFUGADO, HIGH);
  _centrifugeActive = true;
  // Utils.debug("Centrifugado iniciado");
}


bool ActuatorsClass::isCentrifugeRunning() {
  return _centrifugeActive;
}

void ActuatorsClass::openWaterValve() {
  Hardware.digitalWrite(PIN_VALVULA_AGUA, HIGH);
  _waterValveOpen = true;
  Utils.debug("Válvula de agua abierta");
}

void ActuatorsClass::closeWaterValve() {
  Hardware.digitalWrite(PIN_VALVULA_AGUA, LOW);
  _waterValveOpen = false;
  Utils.debug("Válvula de agua cerrada");
}

bool ActuatorsClass::isWaterValveOpen() {
  return _waterValveOpen;
}

void ActuatorsClass::openSteamValve() {
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, HIGH);
  _steamValveOpen = true;
  Utils.debug("Válvula de vapor abierta");
}

void ActuatorsClass::closeSteamValve() {
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  _steamValveOpen = false;
  Utils.debug("Válvula de vapor cerrada");
}

bool ActuatorsClass::isSteamValveOpen() {
  return _steamValveOpen;
}

void ActuatorsClass::openDrainValve() {
  Hardware.digitalWrite(PIN_VALVULA_DESFOGUE, HIGH);
  _drainValveOpen = true;
  // Utils.debug("Válvula de drenaje abierta");
}

void ActuatorsClass::closeDrainValve() {
  Hardware.digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  _drainValveOpen = false;
  // Utils.debug("Válvula de drenaje cerrada");
}

bool ActuatorsClass::isDrainValveOpen() {
  return _drainValveOpen;
}

void ActuatorsClass::lockDoor() {
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, HIGH);
  _doorLocked = true;
  // Utils.debug("Puerta bloqueada");
}

void ActuatorsClass::unlockDoor() {
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, LOW);
  _doorLocked = false;
  // Utils.debug("Puerta desbloqueada");
}

bool ActuatorsClass::isDoorLocked() {
  return _doorLocked;
}



void ActuatorsClass::startAutoRotation(uint8_t level) {
  if (level > 0 && level <= MAX_NIVEL_ROTACION) {
    // Detener rotación anterior si existe
    stopAutoRotation();
    
    // Configurar nivel de rotación
    setRotationLevel(level);
    _motorSeconds = 0;
    
    // Iniciar con dirección hacia adelante
    startMotorForward();
    _autoRotationActive = true;
    
    // Crear un temporizador recurrente que actualizará la rotación del motor
    // Este se ejecutará cada segundo (1000 ms)
    _rotationTaskId = Utils.createInterval(1000, rotationTimerCallback, true);
    
    Utils.debug("Rotación automática iniciada con nivel " + String(level));
  } else {
    Utils.debug("Error: Nivel de rotación inválido (" + String(level) + ")");
  }
}

void ActuatorsClass::stopAutoRotation() {
  _autoRotationActive = false;
  
  // Detener el motor
  stopMotor();
  
  // Detener el temporizador de rotación si existe
  if (_rotationTaskId > 0) {
    Utils.stopTask(_rotationTaskId);
    _rotationTaskId = 0;
  }
  
  Utils.debug("Rotación automática detenida");
}

bool ActuatorsClass::isAutoRotationActive() {
  return _autoRotationActive;
}

uint8_t ActuatorsClass::getCurrentRotationLevel() {
  return _currentRotationLevel;
}

void ActuatorsClass::updateTimers() {
  // Esta función es llamada periódicamente desde Utils.updateTasks()
  // a través del temporizador principal
  
  // Actualizar centrifugado avanzado
  if (_centrifugeState != CENTRIFUGE_IDLE) {
    _updateCentrifugeSpeed();
  }
  
  // Verificar si hay rotación automática activa
  if (_autoRotationActive && _currentRotationLevel > 0) {
    // En lugar de actualizar directamente el motor aquí,
    // esto ahora se maneja a través del temporizador recurrente
    // creado en startAutoRotation
    
    // Lógica adicional para control de actuadores basado en tiempo
    // que no requiera un temporizador independiente
    
    // Ejemplo: Log de estado cada cierto tiempo
    // static unsigned long lastLogTime = 0;
    // if (millis() - lastLogTime > 10000) { // Cada 10 segundos
    //   lastLogTime = millis();
    //   Utils.debug("Estado motor: " + String(_motorState) + 
    //               " / Rotación: " + String(_currentRotationLevel) +
    //               " / Segundos: " + String(_motorSeconds));
    // }
  }
}

// Método público para incrementar segundos del motor
void ActuatorsClass::incrementMotorSeconds() {
  _motorSeconds++;
}

// Método público para actualizar dirección del motor
void ActuatorsClass::updateMotorDirection() {
  _updateMotorDirection();
}

void ActuatorsClass::_updateMotorDirection() {
  if (_currentRotationLevel == 0) return;
  
  uint16_t totalCycleTime = _forwardTime + _pauseTime + _reverseTime + _pauseTime;
  uint16_t cyclePosition = _motorSeconds % totalCycleTime;
  
  // Debug cada 10 segundos para verificar el ciclo
  // static unsigned long lastDebug = 0;
  // if (millis() - lastDebug > 10000) {
  //   Utils.debug("🔄 Motor - Nivel:" + String(_currentRotationLevel) + 
  //               " Pos:" + String(cyclePosition) + "/" + String(totalCycleTime) +
  //               " Estado:" + String(_motorState));
  //   lastDebug = millis();
  // }
  
  // Determinar la acción basada en la posición en el ciclo
  if (cyclePosition < _forwardTime) {
    // Giro hacia adelante
    if (_motorState != MOTOR_FORWARD) {
      Utils.debug("🔄 Cambiando a FORWARD (DIR_A=HIGH, DIR_B=LOW)");
      startMotorForward();
    }
  } else if (cyclePosition < (_forwardTime + _pauseTime)) {
    // Pausa después del giro hacia adelante
    if (_motorState != MOTOR_OFF) {
      Utils.debug("🔄 PAUSA después de FORWARD");
      stopMotor();
    }
  } else if (cyclePosition < (_forwardTime + _pauseTime + _reverseTime)) {
    // Giro hacia atrás
    if (_motorState != MOTOR_REVERSE) {
      Utils.debug("🔄 Cambiando a REVERSE (DIR_A=LOW, DIR_B=HIGH)");
      startMotorReverse();
    }
  } else {
    // Pausa después del giro hacia atrás
    if (_motorState != MOTOR_OFF) {
      Utils.debug("🔄 PAUSA después de REVERSE");
      stopMotor();
    }
  }
}

void ActuatorsClass::emergencyStop() {
  // Detener todos los actuadores y llevar el sistema a un estado seguro
  stopMotor();
  stopCentrifuge();
  closeWaterValve();
  closeSteamValve();
  openDrainValve();
  unlockDoor();
  
  Utils.debug("PARADA DE EMERGENCIA ACTIVADA");
}

void ActuatorsClass::emergencyReset() {
  // Restablecer el sistema después de una emergencia
  Utils.debug("Sistema restablecido después de emergencia");
}

// === IMPLEMENTACIÓN DE CENTRIFUGADO AVANZADO ===

/**
 * @brief Inicia el centrifugado con control avanzado
 * @param targetSpeed Velocidad objetivo (0-100%)
 * @param rampTime Tiempo de rampa en milisegundos
 */
void ActuatorsClass::startCentrifugeAdvanced(uint8_t targetSpeed, uint16_t rampTime) {
  if (targetSpeed > 100) targetSpeed = 100;
  
  // Verificar condiciones de seguridad
  if (!_doorLocked) {
    Utils.debug("⚠️ Error: No se puede iniciar centrifugado - puerta no bloqueada");
    return;
  }
  
  // Verificar que no haya agua por encima del nivel seguro
  // (Esto debería verificarse en el Program Controller)
  
  // Configurar parámetros de centrifugado
  _centrifugeTargetSpeed = targetSpeed;
  _centrifugeRampTime = rampTime;
  _centrifugeStartTime = millis();
  _lastSpeedUpdate = millis();
  _centrifugeState = CENTRIFUGE_BALANCING;
  _balanceAttempts = 0;
  _balanceCheckPassed = false;
  
  Utils.debug("🌀 Iniciando centrifugado avanzado - Velocidad objetivo: " + 
              String(targetSpeed) + "% - Rampa: " + String(rampTime) + "ms");
  
  // Iniciar verificación de balance
  performBalanceCheck();
}

/**
 * @brief Establece la velocidad del centrifugado directamente
 * @param speed Velocidad (0-100%)
 */
void ActuatorsClass::setCentrifugeSpeed(uint8_t speed) {
  if (speed > 100) speed = 100;
  
  _centrifugeCurrentSpeed = speed;
  _setCentrifugeMotorOnOff(speed);
  
  // Actualizar estado del pin de centrifugado
  if (speed > 0) {
    Hardware.digitalWrite(PIN_CENTRIFUGADO, HIGH);
    _centrifugeActive = true;
  } else {
    Hardware.digitalWrite(PIN_CENTRIFUGADO, LOW);
    _centrifugeActive = false;
  }
}

/**
 * @brief Obtiene la velocidad actual del centrifugado
 * @return Velocidad actual (0-100%)
 */
uint8_t ActuatorsClass::getCentrifugeSpeed() {
  return _centrifugeCurrentSpeed;
}

/**
 * @brief Inicia una secuencia predefinida de centrifugado
 * @param sequenceType Tipo de secuencia (NORMAL, DELICATE, INTENSIVE, QUICK)
 */
void ActuatorsClass::startCentrifugeSequence(uint8_t sequenceType) {
  _centrifugeSequenceType = sequenceType;
  
  switch (sequenceType) {
    case CENTRIFUGE_SEQ_NORMAL:
      // Secuencia normal: 80% velocidad, rampa de 30 segundos
      startCentrifugeAdvanced(80, 30000);
      break;
      
    case CENTRIFUGE_SEQ_DELICATE:
      // Secuencia delicada: 50% velocidad, rampa lenta de 45 segundos
      startCentrifugeAdvanced(50, 45000);
      break;
      
    case CENTRIFUGE_SEQ_INTENSIVE:
      // Secuencia intensiva: 100% velocidad, rampa rápida de 20 segundos
      startCentrifugeAdvanced(100, 20000);
      break;
      
    case CENTRIFUGE_SEQ_QUICK:
      // Secuencia rápida: 70% velocidad, rampa de 15 segundos
      startCentrifugeAdvanced(70, 15000);
      break;
      
    default:
      Utils.debug("⚠️ Tipo de secuencia desconocido: " + String(sequenceType));
      break;
  }
}

/**
 * @brief Verifica si el balance de carga es correcto
 * @return true si el balance es aceptable
 */
bool ActuatorsClass::isBalanceCheckPassed() {
  return _balanceCheckPassed;
}

/**
 * @brief Realiza una verificación de balance de carga
 */
void ActuatorsClass::performBalanceCheck() {
  _centrifugeState = CENTRIFUGE_BALANCING;
  _balanceAttempts++;
  
  Utils.debug("🔄 Verificando balance de carga - Intento " + String(_balanceAttempts));
  
  // Crear tarea para ejecutar la detección de balance
  Utils.createTimeout(3000, balanceDetectionCallback);
}

/**
 * @brief Obtiene el estado actual del centrifugado
 * @return Estado actual (IDLE, BALANCING, RAMPING_UP, etc.)
 */
uint8_t ActuatorsClass::getCentrifugeState() {
  return _centrifugeState;
}

/**
 * @brief Actualiza la velocidad del centrifugado según la rampa
 */
void ActuatorsClass::_updateCentrifugeSpeed() {
  if (_centrifugeState == CENTRIFUGE_IDLE || _centrifugeState == CENTRIFUGE_ERROR) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Actualizar solo cada 100ms para suavizar el control
  if (currentTime - _lastSpeedUpdate < 100) {
    return;
  }
  _lastSpeedUpdate = currentTime;
  
  switch (_centrifugeState) {
    case CENTRIFUGE_RAMPING_UP: {
      // Calcular progreso de la rampa
      unsigned long elapsed = currentTime - _centrifugeStartTime;
      if (elapsed >= _centrifugeRampTime) {
        // Rampa completada
        setCentrifugeSpeed(_centrifugeTargetSpeed);
        _centrifugeState = CENTRIFUGE_RUNNING;
        Utils.debug("✅ Centrifugado a velocidad objetivo: " + String(_centrifugeTargetSpeed) + "%");
      } else {
        // Incrementar velocidad gradualmente
        uint8_t newSpeed = map(elapsed, 0, _centrifugeRampTime, 0, _centrifugeTargetSpeed);
        setCentrifugeSpeed(newSpeed);
      }
      break;
    }
    
    case CENTRIFUGE_RAMPING_DOWN: {
      // Reducir velocidad gradualmente
      if (_centrifugeCurrentSpeed > 0) {
        uint8_t decrementSpeed = max(1, _centrifugeCurrentSpeed / 10);
        setCentrifugeSpeed(_centrifugeCurrentSpeed - decrementSpeed);
      } else {
        // Centrifugado detenido
        _centrifugeState = CENTRIFUGE_IDLE;
        Utils.debug("🛑 Centrifugado detenido completamente");
      }
      break;
    }
  }
}

/**
 * @brief Ejecuta la secuencia de centrifugado según el tipo
 */
void ActuatorsClass::_executeCentrifugeSequence() {
  // Esta función puede ser expandida para manejar secuencias más complejas
  // Por ahora, la lógica principal está en _updateCentrifugeSpeed()
  _updateCentrifugeSpeed();
}

/**
 * @brief Realiza la detección de balance de carga
 */
void ActuatorsClass::_performBalanceDetection() {
  // Algoritmo simplificado de detección de balance
  // En un sistema real, esto mediría vibraciones o corriente del motor
  
  // Girar lentamente para redistribuir la carga
  setCentrifugeSpeed(20); // 20% velocidad para detección
  
  // Simular medición (en un sistema real, se medirían sensores)
  Utils.createTimeout(2000, balanceCheckTimeoutCallback);
}

/**
 * @brief Calcula las RPM basándose en el porcentaje de velocidad
 * @param speed Velocidad en porcentaje (0-100)
 * @return RPM calculadas
 */
uint16_t ActuatorsClass::_calculateRPMFromSpeed(uint8_t speed) {
  // Mapear porcentaje a RPM (0-100% -> 0-1400 RPM)
  return map(speed, 0, 100, 0, CENTRIFUGE_SPEED_MAX);
}

/**
 * @brief Control On/Off del motor de centrifugado
 * @param speed Velocidad en porcentaje (0-100)
 */
void ActuatorsClass::_setCentrifugeMotorOnOff(uint8_t speed) {
  // Control simple On/Off basado en umbral
  // Motor encendido si velocidad > 0
  if (speed > 0) {
    Hardware.digitalWrite(PIN_CENTRIFUGADO, HIGH);
  } else {
    Hardware.digitalWrite(PIN_CENTRIFUGADO, LOW);
  }
  
  // Log de velocidad para debug
  static uint8_t lastLoggedSpeed = 255;
  if (speed != lastLoggedSpeed) {
    lastLoggedSpeed = speed;
    Utils.debug("🌀 Centrifugado: " + String(speed > 0 ? "ON" : "OFF") + 
                " (Velocidad objetivo: " + String(speed) + "%)");
  }
}

/**
 * @brief Detiene el centrifugado con rampa de desaceleración
 */
void ActuatorsClass::stopCentrifuge() {
  if (_centrifugeState != CENTRIFUGE_IDLE) {
    _centrifugeState = CENTRIFUGE_RAMPING_DOWN;
    Utils.debug("🔻 Deteniendo centrifugado con rampa de desaceleración");
  }
}


// === IMPLEMENTACIÓN DE DRENAJE PARCIAL ===

/**
 * @brief Inicia un drenaje parcial del agua
 * @param percentage Porcentaje de agua a drenar (0-100)
 * @param duration Duración en milisegundos (0 = calculado automáticamente)
 */
void ActuatorsClass::startPartialDrain(uint8_t percentage, uint16_t duration) {
  if (percentage > 100) percentage = 100;
  if (percentage == 0) return;
  
  // Detener drenaje anterior si existe
  stopPartialDrain();
  
  _partialDrainActive = true;
  _partialDrainPercentage = percentage;
  _partialDrainStartTime = millis();
  
  // Calcular duración si no se especifica
  if (duration == 0) {
    // Estimar tiempo basado en porcentaje y capacidad de drenaje
    // Asumiendo 30 segundos para drenar completamente
    duration = (30000 * percentage) / 100;
  }
  _partialDrainDuration = duration;
  
  // Abrir válvula de drenaje
  openDrainValve();
  
  Utils.debug("💧 Iniciando drenaje parcial - " + String(percentage) + 
              "% en " + String(duration) + "ms");
  
  // Programar cierre automático de válvula
  _partialDrainTaskId = Utils.createTimeout(duration, partialDrainCompleteCallback);
}

/**
 * @brief Detiene el drenaje parcial en curso
 */
void ActuatorsClass::stopPartialDrain() {
  if (_partialDrainActive) {
    closeDrainValve();
    _partialDrainActive = false;
    
    // Cancelar tarea programada
    if (_partialDrainTaskId > 0) {
      Utils.stopTask(_partialDrainTaskId);
      _partialDrainTaskId = 0;
    }
    
    Utils.debug("🛑 Drenaje parcial detenido");
  }
}

/**
 * @brief Verifica si hay un drenaje parcial activo
 * @return true si está activo
 */
bool ActuatorsClass::isPartialDrainActive() {
  return _partialDrainActive;
}

/**
 * @brief Obtiene el progreso del drenaje parcial
 * @return Porcentaje completado (0-100)
 */
uint8_t ActuatorsClass::getPartialDrainProgress() {
  if (!_partialDrainActive) return 100;
  
  unsigned long elapsed = millis() - _partialDrainStartTime;
  if (elapsed >= _partialDrainDuration) return 100;
  
  return (elapsed * 100) / _partialDrainDuration;
}

// === SECUENCIAS COMPLEJAS DE ACTUADORES ===

/**
 * @brief Ejecuta una secuencia de ajuste de temperatura mediante drenaje y llenado
 * @param targetReduction Reducción de temperatura deseada en grados
 */
void ActuatorsClass::executeTemperatureAdjustmentSequence(uint8_t targetReduction) {
  Utils.debug("🌡️ Iniciando secuencia de ajuste de temperatura - Reducción objetivo: " + 
              String(targetReduction) + "°C");
  
  // Paso 1: Drenar parcialmente (20% por cada 5°C de reducción deseada)
  uint8_t drainPercentage = min(80, (targetReduction / 5) * 20);
  startPartialDrain(drainPercentage);
  
  // Paso 2: Después del drenaje, llenar con agua fría
  Utils.createTimeout(_partialDrainDuration + 1000, temperatureAdjustmentFillCallback);
}

/**
 * @brief Ejecuta una secuencia de distribución de detergente
 */
void ActuatorsClass::executeDetergentDistributionSequence() {
  Utils.debug("🧼 Iniciando secuencia de distribución de detergente");
  
  // Secuencia típica:
  // 1. Llenar parcialmente
  // 2. Rotar suavemente
  // 3. Pausar para disolución
  // 4. Rotar en sentido contrario
  
  // Esta es una secuencia ejemplo que debe ser personalizada
  // según los requisitos específicos del sistema
  
  // Iniciar rotación suave
  startAutoRotation(1);
  
  // Después de 30 segundos, cambiar a rotación media
  Utils.createTimeout(30000, detergentDistributionPhase2Callback);
}

// === IMPLEMENTACIÓN DE MÉTODOS PARA CALLBACKS ===

void ActuatorsClass::completePartialDrain() {
  closeDrainValve();
  _partialDrainActive = false;
  Utils.debug("✅ Drenaje parcial completado");
}

void ActuatorsClass::handleBalanceDetectionTimeout() {
  // Este método se llama después del timeout de detección de balance
  // No se usa actualmente, pero se mantiene por si se necesita en el futuro
}

void ActuatorsClass::handleBalanceCheckResult() {
  // Simulación: 80% de probabilidad de éxito en cada intento
  bool balanceOk = (random(100) < 80) || (_balanceAttempts >= 3);
  
  if (balanceOk) {
    _balanceCheckPassed = true;
    _centrifugeState = CENTRIFUGE_RAMPING_UP;
    _centrifugeStartTime = millis(); // Reiniciar tiempo para rampa
    Utils.debug("✅ Balance de carga OK - Iniciando centrifugado");
  } else {
    // Balance fallido, intentar redistribuir
    setCentrifugeSpeed(0);
    
    if (_balanceAttempts < 3) {
      // Intentar redistribuir la carga
      Utils.debug("⚠️ Balance fallido - Redistribuyendo carga...");
      
      // Girar en ambas direcciones para redistribuir
      startMotorForward();
      Utils.createTimeout(2000, motorRedistributionReverseCallback);
    } else {
      // Demasiados intentos fallidos
      _centrifugeState = CENTRIFUGE_ERROR;
      Utils.debug("❌ Error: No se pudo balancear la carga después de 3 intentos");
    }
  }
}

void ActuatorsClass::handleMotorRedistributionForward() {
  startMotorForward();
}

void ActuatorsClass::handleMotorRedistributionReverse() {
  startMotorReverse();
  Utils.createTimeout(2000, motorRedistributionCompleteCallback);
}

void ActuatorsClass::handleMotorRedistributionComplete() {
  stopMotor();
  // Reintentar balance
  performBalanceCheck();
}

void ActuatorsClass::handleTemperatureAdjustmentFill() {
  openWaterValve();
  Utils.debug("🚰 Llenando con agua fría para reducir temperatura");
  // El Program Controller debe monitorear el nivel y temperatura
  // y cerrar la válvula cuando se alcance el nivel deseado
}

void ActuatorsClass::handleDetergentDistributionPhase2() {
  setRotationLevel(2);
  // Después de otros 30 segundos, detener
  Utils.createTimeout(30000, detergentDistributionCompleteCallback);
}

void ActuatorsClass::handleDetergentDistributionComplete() {
  stopAutoRotation();
  Utils.debug("✅ Secuencia de distribución completada");
}
