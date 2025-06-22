// actuators.h
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "Arduino.h"
#include "config.h"
#include "hardware.h"

// Definición de estados para actuadores
#define MOTOR_OFF 0
#define MOTOR_FORWARD 1
#define MOTOR_REVERSE 2

#define VALVE_CLOSED 0
#define VALVE_OPEN 1

// Estados de centrifugado
#define CENTRIFUGE_IDLE 0
#define CENTRIFUGE_BALANCING 1
#define CENTRIFUGE_RAMPING_UP 2
#define CENTRIFUGE_RUNNING 3
#define CENTRIFUGE_RAMPING_DOWN 4
#define CENTRIFUGE_ERROR 5

// Tipos de secuencia de centrifugado
#define CENTRIFUGE_SEQ_NORMAL 0    // Secuencia estándar
#define CENTRIFUGE_SEQ_DELICATE 1  // Para ropa delicada
#define CENTRIFUGE_SEQ_INTENSIVE 2 // Para ropa muy sucia
#define CENTRIFUGE_SEQ_QUICK 3     // Centrifugado rápido

// Velocidades predefinidas de centrifugado (en RPM)
#define CENTRIFUGE_SPEED_LOW 400
#define CENTRIFUGE_SPEED_MEDIUM 800
#define CENTRIFUGE_SPEED_HIGH 1200
#define CENTRIFUGE_SPEED_MAX 1400

class ActuatorsClass {
  // Declarar funciones callback como friend para acceso a miembros privados
  friend void partialDrainCompleteCallback();
  
public:
  // Constructor y destructor
  ActuatorsClass() = default;
  ~ActuatorsClass();
  
  // Inicialización
  void init();
  
  // Control del motor
  void startMotorForward();
  void startMotorReverse();
  void stopMotor();
  bool isMotorRunning();
  uint8_t getMotorState();
  void setRotationLevel(uint8_t level);
  
  // Control de centrifugado
  void startCentrifuge();
  void stopCentrifuge();
  bool isCentrifugeRunning();
  
  // Control de centrifugado avanzado
  void startCentrifugeAdvanced(uint8_t targetSpeed, uint16_t rampTime);
  void setCentrifugeSpeed(uint8_t speed); // 0-100%
  uint8_t getCentrifugeSpeed();
  void startCentrifugeSequence(uint8_t sequenceType);
  bool isBalanceCheckPassed();
  void performBalanceCheck();
  uint8_t getCentrifugeState(); // IDLE, RAMPING_UP, RUNNING, RAMPING_DOWN, BALANCING
  
  // Control de válvulas
  void openWaterValve();
  void closeWaterValve();
  bool isWaterValveOpen();
  
  void openSteamValve();
  void closeSteamValve();
  bool isSteamValveOpen();
  
  void openDrainValve();
  void closeDrainValve();
  bool isDrainValveOpen();
  
  // Control de drenaje parcial para ajuste de temperatura
  void startPartialDrain(uint8_t percentage, uint16_t duration = 0);
  void stopPartialDrain();
  bool isPartialDrainActive();
  uint8_t getPartialDrainProgress();
  
  // Control del mecanismo de puerta
  void lockDoor();
  void unlockDoor();
  bool isDoorLocked();
  
  // Control de rotación automática
  void startAutoRotation(uint8_t level);
  void stopAutoRotation();
  bool isAutoRotationActive();
  
  // Gestión de emergencia
  void emergencyStop();
  void emergencyReset();
  
  // Secuencias complejas de actuadores
  void executeTemperatureAdjustmentSequence(uint8_t targetReduction);
  void executeDetergentDistributionSequence();
  
  // Actualización de temporizadores
  void updateTimers();
  
  // Métodos públicos para callbacks
  void incrementMotorSeconds();
  void updateMotorDirection();
  void completePartialDrain();
  void handleBalanceDetectionTimeout();
  void handleBalanceCheckResult();
  void handleMotorRedistributionForward();
  void handleMotorRedistributionReverse();
  void handleMotorRedistributionComplete();
  void handleTemperatureAdjustmentFill();
  void handleDetergentDistributionPhase2();
  void handleDetergentDistributionComplete();

private:
  // Variables de estado
  uint8_t _motorState;
  bool _waterValveOpen;
  bool _steamValveOpen;
  bool _drainValveOpen;
  bool _doorLocked;
  bool _autoRotationActive;
  bool _centrifugeActive;
  
  // Variables para control de rotación
  uint32_t _motorSeconds;
  uint8_t _currentRotationLevel;
  uint8_t _rotationDirection;
  uint16_t _forwardTime;
  uint16_t _reverseTime;
  uint16_t _pauseTime;
  
  // Variables para centrifugado avanzado
  uint8_t _centrifugeState;
  uint8_t _centrifugeTargetSpeed;
  uint8_t _centrifugeCurrentSpeed;
  uint16_t _centrifugeRampTime;
  unsigned long _centrifugeStartTime;
  unsigned long _lastSpeedUpdate;
  uint8_t _centrifugeSequenceType;
  uint8_t _balanceAttempts;
  bool _balanceCheckPassed;
  
  // Variables para drenaje parcial
  bool _partialDrainActive;
  uint8_t _partialDrainPercentage;
  uint16_t _partialDrainDuration;
  unsigned long _partialDrainStartTime;
  int _partialDrainTaskId;
  
  // IDs de tareas temporizadas
  int _rotationTaskId;    // ID de la tarea de rotación
  
  // Tiempo para control manual de temporizadores
  unsigned long _lastRotationUpdate;
  
  // Métodos internos
  void _configureRotationTiming(uint8_t level);
  void _updateMotorDirection();
  
  // Métodos internos para centrifugado avanzado
  void _updateCentrifugeSpeed();
  void _executeCentrifugeSequence();
  uint16_t _calculateRPMFromSpeed(uint8_t speed);
  void _setCentrifugeMotorOnOff(uint8_t speed); // Cambiado de PWM a On/Off
  
public:
  // Método público para callback (necesario para acceso desde función estática)
  void _performBalanceDetection();
};

// Instancia global
extern ActuatorsClass Actuators;

#endif // ACTUATORS_H