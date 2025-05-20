// actuators.cpp
#include "actuators.h"
#include "utils.h"

// Instancia global
ActuatorsClass Actuators;

// Callbacks para las tareas temporizadas
void rotationTimerCallback() {
  Actuators.incrementMotorSeconds();
  Actuators.updateMotorDirection();
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
  
  Utils.debug("Actuadores inicializados");
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
  Utils.debug("Centrifugado iniciado");
}

void ActuatorsClass::stopCentrifuge() {
  // Detener el centrifugado
  Hardware.digitalWrite(PIN_CENTRIFUGADO, LOW);
  _centrifugeActive = false;
  Utils.debug("Centrifugado detenido");
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
  Utils.debug("Válvula de drenaje abierta");
}

void ActuatorsClass::closeDrainValve() {
  Hardware.digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  _drainValveOpen = false;
  Utils.debug("Válvula de drenaje cerrada");
}

bool ActuatorsClass::isDrainValveOpen() {
  return _drainValveOpen;
}

void ActuatorsClass::lockDoor() {
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, HIGH);
  _doorLocked = true;
  Utils.debug("Puerta bloqueada");
}

void ActuatorsClass::unlockDoor() {
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, LOW);
  _doorLocked = false;
  Utils.debug("Puerta desbloqueada");
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

void ActuatorsClass::updateTimers() {
  // Esta función se llama periódicamente desde el temporizador principal
  // a través de Utils.updateTimers()
  
  // Ya no necesitamos incrementar _motorSeconds aquí, porque ahora
  // se maneja a través del temporizador recurrente creado en startAutoRotation
  
  // En este método podríamos agregar funcionalidades adicionales
  // que necesiten actualizarse periódicamente pero que no requieran
  // un temporizador independiente
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