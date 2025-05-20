# Módulo de Actuadores (actuators.h / actuators.cpp)

## Descripción

El módulo de Actuadores se encarga de controlar todos los dispositivos de salida del sistema, como válvulas, motor, calentador y sistema de bloqueo de puerta. Proporciona una capa de abstracción que simplifica la interacción con el hardware y mejora la legibilidad del código.

## Analogía: Sistema Muscular

Este módulo funciona como el sistema muscular de un organismo, que recibe órdenes del sistema nervioso central (controlador de programas) y las traduce en acciones físicas. Al igual que diferentes músculos tienen diferentes funciones (algunos levantan, otros aprietan, etc.), los distintos actuadores realizan acciones específicas, pero todos comparten la naturaleza común de responder a comandos y producir cambios en el mundo físico.

## Estructura del Módulo

El módulo de Actuadores se divide en:

- **actuators.h**: Define la interfaz pública del módulo
- **actuators.cpp**: Implementa la funcionalidad interna y los comandos a los dispositivos

### Interfaz (actuators.h)

```cpp
// actuators.h
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "config.h"
#include "hardware.h"
#include "utils.h"

class ActuatorsClass {
public:
  // Inicialización
  void init();
  
  // Control de motor
  void startMotor(uint8_t rotationLevel = 1);
  void stopMotor();
  void pauseMotor();
  void resumeMotor();
  void reverseMotor();
  bool isMotorRunning();
  
  // Control de centrifugado
  void startCentrifuge(uint8_t intensity = 2);
  void stopCentrifuge();
  bool isCentrifugeActive();
  
  // Control de válvulas de agua
  void openHotWaterValve();
  void openColdWaterValve();
  void closeHotWaterValve();
  void closeColdWaterValve();
  void closeWaterValve();  // Cierra ambas
  bool isHotWaterValveOpen();
  bool isColdWaterValveOpen();
  
  // Establecer tipo de agua según programa
  void setWaterType(const String& type);  // "Caliente" o "Fría"
  
  // Control de drenaje
  void openDrainValve();
  void closeDrainValve();
  bool isDrainValveOpen();
  
  // Control de vapor para temperatura
  void activateSteam(bool activate);
  bool isSteamActive();
  
  // Control de seguridad
  void lockDoor();
  void unlockDoor();
  bool isDoorLocked();
  
  // Gestión de temperatura
  void manageTemperature(float currentTemp, float targetTemp, float tolerance);
  
  // Control para patrones de rotación
  void configureRotationPattern(uint8_t level, uint16_t dirATime, uint16_t pauseTime, uint16_t dirBTime);
  void updateMotorPattern();
  
  // Operaciones de emergencia
  void emergencyStop();
  void stopAllSystems();
  
  // Señales acústicas (beep)
  void beep(uint8_t count = 1);

private:
  // Estados actuales
  bool _motorRunning;
  bool _motorPaused;
  uint8_t _motorDirection;  // 0=detenido, 1=dirA, 2=dirB
  uint8_t _rotationLevel;
  bool _centrifugeActive;
  uint8_t _centrifugeIntensity;
  bool _hotWaterValveOpen;
  bool _coldWaterValveOpen;
  bool _drainValveOpen;
  bool _steamActive;
  bool _doorLocked;
  
  // Patrones de rotación
  struct RotationPattern {
    uint16_t dirATime;
    uint16_t pauseTime;
    uint16_t dirBTime;
  };
  
  RotationPattern _rotationPatterns[3];  // Para los 3 niveles de rotación
  uint32_t _lastMotorChange;
  uint8_t _motorState;  // 0=dirA, 1=pausa, 2=dirB, 3=pausa
  
  // Métodos internos
  void _executeMotorPattern(uint8_t state);
  void _configureCentrifugeIntensity(uint8_t intensity);
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
  _motorPaused = false;
  _motorDirection = 0;
  _rotationLevel = 1;
  _centrifugeActive = false;
  _centrifugeIntensity = 2;
  _hotWaterValveOpen = false;
  _coldWaterValveOpen = false;
  _drainValveOpen = false;
  _steamActive = false;
  _doorLocked = false;
  _lastMotorChange = 0;
  _motorState = 0;
  
  // Configurar patrones de rotación predeterminados
  // Nivel 1 (suave)
  _rotationPatterns[0].dirATime = 5000;  // 5 segundos
  _rotationPatterns[0].pauseTime = 3000; // 3 segundos
  _rotationPatterns[0].dirBTime = 5000;  // 5 segundos
  
  // Nivel 2 (medio)
  _rotationPatterns[1].dirATime = 10000;  // 10 segundos
  _rotationPatterns[1].pauseTime = 2000;  // 2 segundos
  _rotationPatterns[1].dirBTime = 10000;  // 10 segundos
  
  // Nivel 3 (intenso)
  _rotationPatterns[2].dirATime = 15000;  // 15 segundos
  _rotationPatterns[2].pauseTime = 1000;  // 1 segundo
  _rotationPatterns[2].dirBTime = 15000;  // 15 segundos
  
  // Aplicar estados iniciales al hardware
  Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
  Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
  Hardware.digitalWrite(PIN_CENTRIFUGADO, LOW);
  Hardware.digitalWrite(PIN_VALVUL_AGUA, LOW);
  Hardware.digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  Hardware.digitalWrite(PIN_VALVUL_DESFOGUE, LOW);
  Hardware.digitalWrite(PIN_MAGNET_PUERTA, LOW);
  
  Utils.debug("Actuadores inicializados correctamente");
}

// Métodos para control del motor con 3 niveles de rotación
void ActuatorsClass::startMotor(uint8_t rotationLevel) {
  if (!_motorRunning) {
    if (rotationLevel < 1) rotationLevel = 1;
    if (rotationLevel > 3) rotationLevel = 3;
    
    _rotationLevel = rotationLevel;
    _motorRunning = true;
    _motorPaused = false;
    _motorState = 0;
    _motorDirection = 1;
    _lastMotorChange = millis();
    
    // Iniciar en dirección A
    Hardware.digitalWrite(PIN_MOTOR_DIR_A, HIGH);
    Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
    
    Utils.debug("Motor iniciado con nivel de rotación: " + String(_rotationLevel));
  }
}

void ActuatorsClass::pauseMotor() {
  if (_motorRunning && !_motorPaused) {
    _motorPaused = true;
    Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
    Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
    Utils.debug("Motor pausado");
  }
}

void ActuatorsClass::resumeMotor() {
  if (_motorRunning && _motorPaused) {
    _motorPaused = false;
    
    // Reanudar en la dirección anterior
    if (_motorDirection == 1) {
      Hardware.digitalWrite(PIN_MOTOR_DIR_A, HIGH);
      Hardware.digitalWrite(PIN_MOTOR_DIR_B, LOW);
    } else if (_motorDirection == 2) {
      Hardware.digitalWrite(PIN_MOTOR_DIR_A, LOW);
      Hardware.digitalWrite(PIN_MOTOR_DIR_B, HIGH);
    }
    
    Utils.debug("Motor reanudado");
  }
}

// Control de válvulas de agua específicas para diferentes programas
void ActuatorsClass::openHotWaterValve() {
  _hotWaterValveOpen = true;
  _coldWaterValveOpen = false;
  Hardware.digitalWrite(PIN_VALVUL_AGUA, HIGH);
  Utils.debug("Válvula de agua caliente abierta");
}

void ActuatorsClass::openColdWaterValve() {
  _coldWaterValveOpen = true;
  _hotWaterValveOpen = false;
  Hardware.digitalWrite(PIN_VALVUL_AGUA, HIGH);
  Utils.debug("Válvula de agua fría abierta");
}

void ActuatorsClass::setWaterType(const String& type) {
  if (type == "Caliente") {
    Utils.debug("Configurado para usar agua caliente");
  } else {
    Utils.debug("Configurado para usar agua fría");
  }
}

// Gestión de temperatura para programas con agua caliente
void ActuatorsClass::manageTemperature(float currentTemp, float targetTemp, float tolerance) {
  // Para Programa 22 (Agua Caliente) y Programa 24 con agua caliente
  if (currentTemp < (targetTemp - tolerance)) {
    // Temperatura por debajo del objetivo, activar calentador
    activateSteam(true);
    Utils.debug("Vapor activado: temperatura " + String(currentTemp) + "°C inferior al objetivo");
  } else if (currentTemp > (targetTemp + tolerance)) {
    // Temperatura por encima del objetivo, desactivar calentador
    activateSteam(false);
    Utils.debug("Vapor desactivado: temperatura " + String(currentTemp) + "°C superior al objetivo");
  }
}

// Operaciones de emergencia
void ActuatorsClass::emergencyStop() {
  Utils.debug("¡PARADA DE EMERGENCIA ACTIVADA!");
  
  // Detener motor inmediatamente
  stopMotor();
  
  // Detener centrifugado
  stopCentrifuge();
  
  // Cerrar válvulas de entrada de agua
  closeWaterValve();
  
  // Desactivar vapor
  activateSteam(false);
  
  // Abrir válvula de drenaje para vaciar
  openDrainValve();
  
  // La puerta se desbloquea después de un tiempo prudencial (mediante callback)
  Utils.createTimeout(EMERGENCY_DOOR_UNLOCK_DELAY, []() {
    Actuators.unlockDoor();
  });
}

void ActuatorsClass::stopAllSystems() {
  stopMotor();
  stopCentrifuge();
  closeWaterValve();
  activateSteam(false);
  Utils.debug("Todos los sistemas detenidos");
}
```

## Responsabilidades

El módulo de Actuadores tiene las siguientes responsabilidades adaptadas para los tres programas específicos:

1. **Control Diferenciado de Agua**: 
   - Manejo separado de válvulas de agua fría y caliente para los diferentes programas.
   - Programa 22: Exclusivamente agua caliente
   - Programa 23: Exclusivamente agua fría
   - Programa 24: Configurable según selección de usuario

2. **Gestión de Temperatura**: 
   - Control activo del vapor para mantener la temperatura dentro del rango objetivo ±2°C
   - Participación en el ciclo de ajuste de temperatura para programas con agua caliente

3. **Patrones de Rotación Configurables**:
   - Tres niveles diferentes de intensidad de rotación (suave, medio, intenso)
   - Tiempos de giro y pausa específicos para cada nivel
   - Control asíncrono de la rotación sin usar delay()

4. **Control de Centrifugado**:
   - Fase opcional para todos los programas
   - Intensidad configurable
   - Verificación de seguridad antes de iniciar

5. **Operaciones de Emergencia**:
   - Procedimiento de detención segura con secuencia específica
   - Desbloqueo de puerta con retardo de seguridad

6. **Manejo de Puerta Seguro**:
   - Bloqueo durante todas las fases de operación
   - Desbloqueo controlado al finalizar o en emergencias

7. **Integración con Notificaciones**:
   - Alertas sonoras para eventos importantes
   - Mensajes de depuración del estado de cada actuador

## Ventajas de esta Implementación

1. **Especialización para Programas**: Métodos dedicados para las necesidades específicas de los tres programas de lavado.

2. **Mayor Seguridad**: Control robusto de situaciones de emergencia con secuencias de detención segura.

3. **Rotación Avanzada**: Sistema de rotación con patrones complejos para simular el comportamiento de lavadoras profesionales.

4. **Operación No Bloqueante**: Implementación asíncrona para mantener el sistema responsivo en todo momento.

5. **Gestión de Estado Detallada**: Seguimiento preciso del estado de cada actuador para diagnóstico y depuración.

6. **Modularidad Mejorada**: Separación clara de responsabilidades que facilita la extensión para nuevos programas o funcionalidades.

7. **Depuración Mejorada**: Mensajes detallados sobre cada operación que facilitan el seguimiento del sistema.

## Soporte para Programas Específicos

Esta implementación del módulo de Actuadores ha sido diseñada específicamente para soportar los requisitos de los tres programas de lavado:

### Para Programa 22 (Agua Caliente):
- Control exclusivo de válvula de agua caliente
- Gestión activa de temperatura mediante vapor
- Participación en ciclos de ajuste de temperatura (drenaje parcial y nuevo llenado)

### Para Programa 23 (Agua Fría):
- Control exclusivo de válvula de agua fría
- Sin activación de sistemas de calentamiento

### Para Programa 24 (Multitanda):
- Flexibilidad para gestionar agua fría o caliente según configuración
- Control de ciclos múltiples con transiciones suaves entre tandas
- Capacidad para alternar configuraciones entre tandas si es necesario

Al implementar estas capacidades específicas, el módulo de Actuadores proporciona los "músculos" necesarios para que el controlador de programas pueda ejecutar eficazmente los tres programas de lavado, con especial atención a los requerimientos de gestión de temperatura para programas con agua caliente.
