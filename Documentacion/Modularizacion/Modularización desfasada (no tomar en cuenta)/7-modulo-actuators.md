# Módulo de Actuadores (actuators.h / actuators.cpp)

## Descripción

El módulo de Actuadores se encarga de controlar todos los dispositivos de salida del sistema, como válvulas, motor, calentador y sistema de bloqueo de puerta. Proporciona una capa de abstracción que simplifica la interacción con el hardware y mejora la legibilidad del código.

## Analogía: Sistema Muscular

Este módulo funciona como el sistema muscular de un organismo, que recibe órdenes del sistema nervioso central (controlador de programas) y las traduce en acciones físicas. Al igual que diferentes músculos tienen diferentes funciones (algunos levantan, otros aprietan, etc.), los distintos actuadores realizan acciones específicas, pero todos comparten la naturaleza común de responder a comandos y producir cambios en el mundo físico.

## Estructura del Módulo

El módulo de Actuadores se divide en:

- **actuators.h**: Define la interfaz pública del módulo
- **actuators.cpp**: Implementa la funcionalidad interna

### Interfaz (actuators.h)

```cpp
// actuators.h
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "config.h"
#include "hardware.h"

class ActuatorsClass {
public:
  // Inicialización
  void init();
  
  // Control de motor
  void startMotor();
  void stopMotor();
  void reverseMotor();
  bool isMotorRunning();
  
  // Control de válvulas
  void openWaterValve(bool open);
  void openDrainValve(bool open);
  void activateSteam(bool activate);
  
  // Control de seguridad
  void lockDoor(bool lock);
  
  // Control de indicadores
  void soundBuzzer(bool activate);
  
  // Control avanzado
  void setMotorRotationTime(uint8_t rotationLevel, uint8_t timeRotation, uint8_t timePause);
  
  // Estado actual
  bool isWaterValveOpen();
  bool isDrainValveOpen();
  bool isSteamActive();
  bool isDoorLocked();

private:
  // Estados actuales
  bool _motorRunning;
  bool _motorDirection;  // true = dirA, false = dirB
  bool _waterValveOpen;
  bool _drainValveOpen;
  bool _steamActive;
  bool _doorLocked;
  bool _buzzerActive;
};

// Instancia global
extern ActuatorsClass Actuators;

#endif // ACTUATORS_H
```

### Implementación (actuators.cpp)

```cpp
// actuators.cpp
#include "actuators.h"

// Definición de la instancia global
ActuatorsClass Actuators;

void ActuatorsClass::init() {
  // Inicializar todos los actuadores en estado seguro/apagado
  _motorRunning = false;
  _motorDirection = true;  // dirA por defecto
  _waterValveOpen = false;
  _drainValveOpen = false;
  _steamActive = false;
  _doorLocked = false;
  _buzzerActive = false;
  
  // Aplicar estados iniciales al hardware
  Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
  Hardware.digitalWrite(PIN_VALVULA_AGUA, LOW);
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  Hardware.digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, LOW);
  Hardware.digitalWrite(PIN_BUZZER, LOW);
}

void ActuatorsClass::startMotor() {
  if (!_motorRunning) {
    _motorRunning = true;
    
    // Por defecto, iniciar en dirección A
    Hardware.digitalWrite(PIN_MOTOR_DIR_A, HIGH);
    Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
    _motorDirection = true;
  }
}

void ActuatorsClass::stopMotor() {
  if (_motorRunning) {
    _motorRunning = false;
    Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
    Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
  }
}

void ActuatorsClass::reverseMotor() {
  if (_motorRunning) {
    if (_motorDirection) {
      // Cambiar de dirA a dirB
      Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
      Hardware.digitalWrite(PIN_MOTOR_DIR_B, HIGH);
      _motorDirection = false;
    } else {
      // Cambiar de dirB a dirA
      Hardware.digitalWrite(PIN_MOTOR_DIR_A, HIGH);
      Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
      _motorDirection = true;
    }
  } else {
    // Si el motor no está en marcha, iniciarlo en dirección alternativa
    startMotor();
    reverseMotor();
  }
}

bool ActuatorsClass::isMotorRunning() {
  return _motorRunning;
}

void ActuatorsClass::openWaterValve(bool open) {
  _waterValveOpen = open;
  Hardware.digitalWrite(PIN_VALVULA_AGUA, open ? HIGH : LOW);
}

void ActuatorsClass::openDrainValve(bool open) {
  _drainValveOpen = open;
  Hardware.digitalWrite(PIN_VALVULA_DESFOGUE, open ? HIGH : LOW);
}

void ActuatorsClass::activateSteam(bool activate) {
  _steamActive = activate;
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, activate ? HIGH : LOW);
}

void ActuatorsClass::lockDoor(bool lock) {
  _doorLocked = lock;
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, lock ? HIGH : LOW);
}

void ActuatorsClass::soundBuzzer(bool activate) {
  _buzzerActive = activate;
  Hardware.digitalWrite(PIN_BUZZER, activate ? HIGH : LOW);
}

bool ActuatorsClass::isWaterValveOpen() {
  return _waterValveOpen;
}

bool ActuatorsClass::isDrainValveOpen() {
  return _drainValveOpen;
}

bool ActuatorsClass::isSteamActive() {
  return _steamActive;
}

bool ActuatorsClass::isDoorLocked() {
  return _doorLocked;
}
```

## Responsabilidades

El módulo de Actuadores tiene las siguientes responsabilidades:

1. **Control de Motor**: Iniciar, detener y cambiar la dirección del motor del tambor.
2. **Control de Válvulas**: Manejar las válvulas de agua, desagüe y vapor.
3. **Seguridad**: Controlar el mecanismo de bloqueo de la puerta.
4. **Alertas**: Gestionar el zumbador para notificaciones audibles.
5. **Estado**: Mantener y proporcionar información sobre el estado actual de cada actuador.
6. **Abstracción**: Ocultar los detalles específicos de cómo se controla cada dispositivo.

## Ventajas de este Enfoque

1. **Seguridad**: Centraliza el control de dispositivos críticos, facilitando la implementación de protecciones.
2. **Encapsulamiento**: Mantiene todos los detalles de control de hardware en un solo lugar.
3. **Legibilidad**: Proporciona métodos con nombres significativos que describen acciones concretas.
4. **Mantenibilidad**: Facilita cambios en el control de hardware sin afectar la lógica de negocio.
5. **Coherencia**: Asegura que los actuadores se controlen de manera consistente en toda la aplicación.
6. **Depuración**: Simplifica la identificación de problemas relacionados con los actuadores.

Al separar el control de los actuadores en un módulo dedicado, se logra una clara separación entre qué se quiere hacer (lógica de negocio en el controlador de programas) y cómo se hace físicamente (detalles de implementación en el módulo de actuadores). Esto mejora significativamente la mantenibilidad y comprensibilidad del sistema como un todo.
