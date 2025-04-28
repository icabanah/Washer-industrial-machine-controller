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

class ActuatorsClass {
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
  
  // Control del mecanismo de puerta
  void lockDoor();
  void unlockDoor();
  bool isDoorLocked();
  
  // Control del zumbador
  void startBuzzer(unsigned long duration = 0);
  void stopBuzzer();
  bool isBuzzerActive();
  
  // Control de rotación automática
  void startAutoRotation(uint8_t level);
  void stopAutoRotation();
  bool isAutoRotationActive();
  
  // Gestión de emergencia
  void emergencyStop();
  void emergencyReset();
  
  // Actualización de temporizadores
  void updateTimers();
  
  // Métodos para gestión de callbacks
  void incrementMotorSeconds();
  void updateMotorDirection();

private:
  // Variables de estado
  uint8_t _motorState;
  bool _waterValveOpen;
  bool _steamValveOpen;
  bool _drainValveOpen;
  bool _doorLocked;
  bool _buzzerActive;
  bool _autoRotationActive;
  
  // Variables para control de rotación
  uint32_t _motorSeconds;
  uint8_t _currentRotationLevel;
  uint8_t _rotationDirection;
  uint16_t _forwardTime;
  uint16_t _reverseTime;
  uint16_t _pauseTime;
  
  // IDs de tareas temporizadas
  int _buzzerTaskId;      // ID de la tarea del zumbador
  int _rotationTaskId;    // ID de la tarea de rotación
  
  // Tiempo para control manual de temporizadores
  unsigned long _lastRotationUpdate;
  
  // Métodos internos
  void _configureRotationTiming(uint8_t level);
  void _updateMotorDirection();
};

// Instancia global
extern ActuatorsClass Actuators;

#endif // ACTUATORS_H