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

void partialDrainCompleteCallback() {
  Actuators.completePartialDrain();
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
  
  // Inicializar centrifugado simple
  _centrifugeActive = false;
  
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
  Hardware.digitalWrite(PIN_MOTOR_DIR_IZQ, HIGH);
  Hardware.digitalWrite(PIN_MOTOR_DIR_DER, LOW);
  _motorState = MOTOR_FORWARD;
}

void ActuatorsClass::startMotorReverse() {
  Hardware.digitalWrite(PIN_MOTOR_DIR_IZQ, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_DER, HIGH);
  _motorState = MOTOR_REVERSE;
}

void ActuatorsClass::stopMotor() {
  Hardware.digitalWrite(PIN_MOTOR_DIR_IZQ, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_DER, LOW);
  _motorState = MOTOR_OFF;
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
  // Configurar los tiempos según el nivel de rotación usando constantes de config.h
  // MOTOR_TIEMPO_ON = 5000ms (5 segundos) y MOTOR_TIEMPO_PAUSA = 2000ms (2 segundos)
  
  switch (level) {
    case 0: // Sin rotación
      _forwardTime = 0;
      _reverseTime = 0;
      _pauseTime = 0;
      break;
    case 1: // Rotación suave - usar configuración específica de config.h
      _forwardTime = MOTOR_L1_TIEMPO_DERECHA;
      _reverseTime = MOTOR_L1_TIEMPO_IZQUIERDA;
      _pauseTime = MOTOR_L1_TIEMPO_PAUSA;
      break;
    case 2: // Rotación media - usar configuración específica de config.h
      _forwardTime = MOTOR_L2_TIEMPO_DERECHA;
      _reverseTime = MOTOR_L2_TIEMPO_IZQUIERDA;
      _pauseTime = MOTOR_L2_TIEMPO_PAUSA;
      break;
    case 3: // Rotación intensa - usar configuración específica de config.h
      _forwardTime = MOTOR_L3_TIEMPO_DERECHA;
      _reverseTime = MOTOR_L3_TIEMPO_IZQUIERDA;
      _pauseTime = MOTOR_L3_TIEMPO_PAUSA;
      break;
  }
  
  // Mostrar configuración solo al inicializar (opcional)
  // if (level > 0) {
  //   String configMsg = "L" + String(level) + ": Der" + String(_forwardTime) + 
  //                      "s Izq" + String(_reverseTime) + "s Pausa" + String(_pauseTime) + "s";
  //   Hardware.nextionSetText(NEXTION_COMP_MSG, configMsg);
  // }
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
}

void ActuatorsClass::closeWaterValve() {
  Hardware.digitalWrite(PIN_VALVULA_AGUA, LOW);
  _waterValveOpen = false;
}

bool ActuatorsClass::isWaterValveOpen() {
  return _waterValveOpen;
}

void ActuatorsClass::openSteamValve() {
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, HIGH);
  _steamValveOpen = true;
}

void ActuatorsClass::closeSteamValve() {
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  _steamValveOpen = false;
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
    // Si ya está activa, solo cambiar el nivel sin reiniciar
    if (_autoRotationActive && _currentRotationLevel == level) {
      return; // Ya está funcionando con el mismo nivel
    }
    
    // Detener temporizador anterior si existe (pero mantener motor funcionando)
    if (_rotationTaskId > 0) {
      Utils.stopTask(_rotationTaskId);
      _rotationTaskId = 0;
    }
    
    // Configurar nivel de rotación
    setRotationLevel(level);
    _motorSeconds = 0;
    _autoRotationActive = true;
    
    // Solo iniciar motor si no está ya funcionando
    if (_motorState == MOTOR_OFF) {
      startMotorForward();
    }
    
    // Crear nuevo temporizador
    _rotationTaskId = Utils.createInterval(1000, rotationTimerCallback, true);
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
  
  // Centrifugado simple - no requiere actualización
  
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
  if (totalCycleTime == 0) return; // Evitar división por cero
  
  uint16_t cyclePosition = _motorSeconds % totalCycleTime;
  
  // Debug opcional (descomentars para troubleshooting)
  // static unsigned long lastDebug = 0;
  // if (millis() - lastDebug > 5000) { // Cada 5 segundos
  //   String debugMsg = "Ciclo:" + String(_motorSeconds) + "/" + String(totalCycleTime);
  //   Hardware.nextionSetText(NEXTION_COMP_MSG, debugMsg);
  //   lastDebug = millis();
  // }
  
  // Determinar la acción basada en la posición en el ciclo
  if (cyclePosition < _forwardTime) {
    // Giro hacia adelante
    if (_motorState != MOTOR_FORWARD) {
      startMotorForward();
    }
  } else if (cyclePosition < (_forwardTime + _pauseTime)) {
    // Pausa después del giro hacia adelante
    if (_motorState != MOTOR_OFF) {
      stopMotor();
    }
  } else if (cyclePosition < (_forwardTime + _pauseTime + _reverseTime)) {
    // Giro hacia atrás
    if (_motorState != MOTOR_REVERSE) {
      startMotorReverse();
    }
  } else {
    // Pausa después del giro hacia atrás
    if (_motorState != MOTOR_OFF) {
      stopMotor();
    }
  }
}

void ActuatorsClass::emergencyStop() {
  // Detener todos los actuadores y llevar el sistema a un estado seguro
  stopAutoRotation(); // IMPORTANTE: Detener temporizador de permutación
  stopMotor();
  stopCentrifuge();
  closeWaterValve();
  closeSteamValve();
  openDrainValve();
  lockDoor(); // en caso de emergencia, bloquear la puerta
  
  Utils.debug("PARADA DE EMERGENCIA ACTIVADA");
}

void ActuatorsClass::emergencyReset() {
  // Restablecer el sistema después de una emergencia
  Utils.debug("Sistema restablecido después de emergencia");
}


/**
 * @brief Detiene el centrifugado inmediatamente
 */
void ActuatorsClass::stopCentrifuge() {
  Hardware.digitalWrite(PIN_CENTRIFUGADO, LOW);
  _centrifugeActive = false;
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


// === IMPLEMENTACIÓN DE MÉTODOS PARA CALLBACKS ===

void ActuatorsClass::completePartialDrain() {
  closeDrainValve();
  _partialDrainActive = false;
  Utils.debug("✅ Drenaje parcial completado");
}


