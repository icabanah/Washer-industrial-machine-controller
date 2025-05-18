# Módulo de Sensores (sensors.h / sensors.cpp)

## Descripción

El módulo de Sensores se encarga de gestionar la lectura, interpretación y monitorización de los sensores físicos del sistema, específicamente el sensor de temperatura y el sensor de presión. Proporciona una capa de abstracción que traduce las lecturas de hardware en información útil para el sistema de control.

## Analogía: Sistema Sensorial

Este módulo funciona como el sistema sensorial de un organismo vivo, que continuamente recoge información del entorno (temperatura, presión, etc.) y la procesa para proporcionar datos significativos al sistema nervioso central (controlador de programas). No toma decisiones sobre cómo responder a estos estímulos, pero es crucial para percibir el estado del entorno e informar al cerebro para que pueda tomar decisiones adecuadas.

## Estructura del Módulo

El módulo de Sensores se divide en:

- **sensors.h**: Define la interfaz pública del módulo
- **sensors.cpp**: Implementa la funcionalidad interna

### Interfaz (sensors.h)

```cpp
// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>

class SensorsClass {
public:
  // Inicialización
  void init();
  
  // Iniciar monitoreo periódico de sensores
  void startMonitoring();
  
  // Control del sensor de temperatura
  void startTemperatureMonitoring(uint8_t targetTemperature);
  void stopTemperatureMonitoring();
  uint8_t getCurrentTemperature();
  bool isTemperatureReached();
  void updateTemperature();
  
  // Control del sensor de presión/nivel
  void startPressureMonitoring(uint8_t targetLevel);
  void stopPressureMonitoring();
  uint8_t getCurrentWaterLevel();
  uint16_t getCurrentPressure();
  bool isWaterLevelReached();
  void updatePressure();
  
  // Métodos de configuración
  void setTemperatureRange(uint8_t range);
  void setPressureLevelThresholds(uint16_t level1, uint16_t level2, uint16_t level3, uint16_t level4);

private:
  // Objetos para sensores
  OneWire _oneWire;
  DallasTemperature _temperatureSensor;
  HX710B _pressureSensor;
  
  // Variables para sensor de temperatura
  DeviceAddress _temperatureAddress;
  uint8_t _temperatureResolution;
  uint8_t _currentTemperature;
  uint8_t _targetTemperature;
  uint8_t _temperatureRange;
  bool _monitoringTemperature;
  
  // Variables para sensor de presión
  uint16_t _currentPressure;
  uint8_t _currentWaterLevel;
  uint8_t _targetWaterLevel;
  uint8_t _levelCounter;
  bool _monitoringPressure;
  
  // Umbrales de presión para niveles de agua
  uint16_t _pressureLevel1;
  uint16_t _pressureLevel2;
  uint16_t _pressureLevel3;
  uint16_t _pressureLevel4;
  
  // Métodos internos
  uint8_t _calculateWaterLevel(uint16_t pressure);
};

// Instancia global
extern SensorsClass Sensors;

#endif // SENSORS_H
```

### Implementación (sensors.cpp)

```cpp
// sensors.cpp
#include "sensors.h"
#include "config.h"
#include "actuators.h"
#include "utils.h"

// Definición de la instancia global
SensorsClass Sensors;

void SensorsClass::init() {
  // Inicializar sensor de temperatura
  _oneWire = OneWire(PIN_TEMP_SENSOR);
  _temperatureSensor = DallasTemperature(&_oneWire);
  _temperatureSensor.begin();
  
  // Configurar dirección del sensor de temperatura
  uint8_t addr[8] = TEMP_SENSOR_ADDR;
  memcpy(_temperatureAddress, addr, 8);
  
  _temperatureResolution = TEMP_RESOLUTION;
  _temperatureSensor.setResolution(_temperatureResolution);
  _temperatureRange = TEMP_RANGE;
  _currentTemperature = 0;
  _targetTemperature = 0;
  _monitoringTemperature = false;
  
  // Inicializar sensor de presión
  _pressureSensor.begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  _currentPressure = 0;
  _currentWaterLevel = 0;
  _targetWaterLevel = 0;
  _levelCounter = 0;
  _monitoringPressure = false;
  
  // Configurar umbrales de presión para niveles de agua
  _pressureLevel1 = NIVEL_PRESION_1;
  _pressureLevel2 = NIVEL_PRESION_2;
  _pressureLevel3 = NIVEL_PRESION_3;
  _pressureLevel4 = NIVEL_PRESION_4;
}

void SensorsClass::startMonitoring() {
  // Iniciar monitoreo periódico de sensores usando AsyncTask
  Utils.createPeriodicTask(1000, [this]() {
    this->updateTemperature();
    this->updatePressure();
  });
}

void SensorsClass::startTemperatureMonitoring(uint8_t targetTemperature) {
  _targetTemperature = targetTemperature;
  _monitoringTemperature = true;
}

void SensorsClass::stopTemperatureMonitoring() {
  _monitoringTemperature = false;
}

uint8_t SensorsClass::getCurrentTemperature() {
  return _currentTemperature;
}

bool SensorsClass::isTemperatureReached() {
  return (_currentTemperature >= _targetTemperature);
}

void SensorsClass::updateTemperature() {
  // Leer temperatura actual
  _temperatureSensor.requestTemperatures();
  _currentTemperature = round(_temperatureSensor.getTempCByIndex(0));
  
  // Si estamos monitoreando, controlar la válvula de vapor
  if (_monitoringTemperature) {
    if (_currentTemperature >= (_targetTemperature + _temperatureRange)) {
      // Temperatura por encima del objetivo + rango, apagar vapor
      Actuators.activateSteam(false);
    } else if (_currentTemperature <= (_targetTemperature - _temperatureRange)) {
      // Temperatura por debajo del objetivo - rango, encender vapor
      Actuators.activateSteam(true);
    }
  }
}

void SensorsClass::startPressureMonitoring(uint8_t targetLevel) {
  _targetWaterLevel = targetLevel;
  _levelCounter = 0;
  _monitoringPressure = true;
}

void SensorsClass::stopPressureMonitoring() {
  _monitoringPressure = false;
}

uint8_t SensorsClass::getCurrentWaterLevel() {
  return _currentWaterLevel;
}

uint16_t SensorsClass::getCurrentPressure() {
  return _currentPressure;
}

bool SensorsClass::isWaterLevelReached() {
  return (_currentWaterLevel >= _targetWaterLevel && _levelCounter >= 5);
}

void SensorsClass::updatePressure() {
  if (_pressureSensor.is_ready()) {
    _currentPressure = _pressureSensor.pascal();
    _currentWaterLevel = _calculateWaterLevel(_currentPressure);
    
    // Si estamos monitoreando, controlar la válvula de agua
    if (_monitoringPressure) {
      if (_currentWaterLevel >= _targetWaterLevel + 1) {
        _levelCounter++;  // Evitar falsos positivos
      } else {
        _levelCounter = 0;
      }
      
      if (_currentWaterLevel >= _targetWaterLevel + 1 && _levelCounter >= 5) {
        // Nivel alcanzado, cerrar válvula de agua
        Actuators.openWaterValve(false);
        _monitoringPressure = false;
      }
    }
  }
}

uint8_t SensorsClass::_calculateWaterLevel(uint16_t pressure) {
  if (pressure <= _pressureLevel1) {
    return 1;
  } else if (pressure > _pressureLevel1 && pressure <= _pressureLevel2) {
    return 2;
  } else if (pressure > _pressureLevel2 && pressure <= _pressureLevel3) {
    return 3;
  } else if (pressure > _pressureLevel3) {
    return 4;
  }
  return 0;  // En caso de error o lectura inválida
}

void SensorsClass::setTemperatureRange(uint8_t range) {
  _temperatureRange = range;
}

void SensorsClass::setPressureLevelThresholds(uint16_t level1, uint16_t level2, uint16_t level3, uint16_t level4) {
  _pressureLevel1 = level1;
  _pressureLevel2 = level2;
  _pressureLevel3 = level3;
  _pressureLevel4 = level4;
}
```

## Responsabilidades

El módulo de Sensores tiene las siguientes responsabilidades:

1. **Inicialización de Sensores**: Configurar los sensores físicos (temperatura y presión) para su funcionamiento.
2. **Lectura de Datos**: Obtener lecturas periódicas de los sensores.
3. **Interpretación**: Traducir valores brutos (ej. pascales) a valores significativos (niveles de agua, temperatura en grados).
4. **Monitorización**: Vigilar continuamente si los valores alcanzan los objetivos establecidos.
5. **Control Simple**: Activar o desactivar actuadores básicos según las lecturas (ej. apagar la válvula de vapor si la temperatura es excesiva).
6. **Configuración**: Permitir ajustar los parámetros de los sensores (rangos, umbrales, etc.).

## Ventajas de este Enfoque

1. **Abstracción**: Oculta los detalles específicos de la comunicación con los sensores.
2. **Modularidad**: Permite cambiar el hardware de los sensores sin afectar al resto del sistema.
3. **Robustez**: Implementa mecanismos para evitar falsos positivos (ej. contador de nivel).
4. **Flexibilidad**: Permite configurar parámetros como umbrales y rangos según las necesidades.
5. **Encapsulamiento**: Mantiene todas las variables relacionadas con los sensores en un solo lugar.
6. **Mantenibilidad**: Facilita la depuración y ajuste de la lógica de los sensores sin afectar otras partes.

Al separar la gestión de sensores en un módulo dedicado, se logra una clara separación de responsabilidades que facilita el mantenimiento y extensión del sistema. Este módulo proporciona una interfaz clara y consistente para acceder a los datos de los sensores, independientemente de los detalles específicos de implementación o del hardware utilizado.
