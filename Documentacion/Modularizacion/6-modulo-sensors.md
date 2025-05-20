# Módulo de Sensores (sensors.h / sensors.cpp)

## Descripción

El módulo de Sensores se encarga de gestionar la lectura, interpretación y monitorización de los sensores físicos del sistema, específicamente el sensor de temperatura (OneWire Dallas) y el sensor de presión (HX710B). Este módulo es crucial para los tres programas específicos de lavado (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda), ya que proporciona información esencial sobre las condiciones de temperatura y nivel de agua, permitiendo el control preciso de cada fase del proceso de lavado.

## Analogía: Sistema Sensorial Especializado

Este módulo funciona como el sistema sensorial de un organismo vivo avanzado, capaz de distinguir diferentes "sabores" de información del entorno. Para los programas que utilizan agua caliente (22 y posiblemente 24), actúa como un termómetro de alta precisión que continuamente monitorea si la temperatura está dentro del rango ideal. Para todos los programas, funciona como un barómetro que detecta con exactitud el nivel de agua en el tambor. Al igual que los sentidos humanos, no toma decisiones sobre cómo responder a estos estímulos, pero proporciona información crucial para que el "cerebro" (ProgramController) pueda tomar decisiones adecuadas según el programa seleccionado.

## Estructura del Módulo

El módulo de Sensores se divide en:

- **sensors.h**: Define la interfaz pública del módulo
- **sensors.cpp**: Implementa la funcionalidad interna y algoritmos de monitoreo

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
  
  // Actualización general (llamada desde loop)
  void update();
  
  // Iniciar monitoreo periódico de sensores
  void startMonitoring();
  
  // Control del sensor de temperatura
  void startTemperatureMonitoring(float targetTemperature);
  void stopTemperatureMonitoring();
  float getCurrentTemperature();
  bool isTemperatureReached();
  bool isTemperatureStable();
  bool isTemperatureLow();  // Detecta temperatura bajo el umbral objetivo-2°C
  
  // Control del sensor de presión/nivel
  void startPressureMonitoring(uint8_t targetLevel);
  void stopPressureMonitoring();
  uint8_t getCurrentWaterLevel();
  uint16_t getCurrentPressure();
  bool isWaterLevelReached();
  
  // Métodos de configuración
  void setTemperatureThresholds(float target, float tolerance);
  void setPressureLevelThresholds(uint16_t level1, uint16_t level2, uint16_t level3, uint16_t level4);
  
  // Utilidades
  bool readSensorData();  // Lee todos los sensores, devuelve true si la lectura fue exitosa

private:
  // Objetos para sensores
  OneWire _oneWire;
  DallasTemperature _temperatureSensor;
  HX710B _pressureSensor;
  
  // Variables para sensor de temperatura
  DeviceAddress _temperatureAddress;
  uint8_t _temperatureResolution;
  float _currentTemperature;
  float _targetTemperature;
  float _temperatureTolerance;
  uint8_t _temperatureReadErrorCount;
  bool _monitoringTemperature;
  
  // Variables para sensor de presión
  uint16_t _currentPressure;
  uint16_t _previousPressure;
  uint8_t _currentWaterLevel;
  uint8_t _targetWaterLevel;
  uint8_t _levelStableCounter;
  bool _monitoringPressure;
  
  // Umbrales de presión para niveles de agua
  uint16_t _pressureLevel1;
  uint16_t _pressureLevel2;
  uint16_t _pressureLevel3;
  uint16_t _pressureLevel4;
  
  // Método para filtrar lecturas
  bool _isReadingValid(float newTemp, uint16_t newPressure);
  uint8_t _calculateWaterLevel(uint16_t pressure);
};

// Instancia global
extern SensorsClass Sensors;

#endif // SENSORS_H
```

### Implementación (sensors.cpp)

A continuación se muestra un fragmento de la implementación que destaca las funciones clave para la gestión de temperatura en programas con agua caliente:

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
  _temperatureSensor.setResolution(_temperatureAddress, _temperatureResolution);
  _currentTemperature = 0.0;
  _targetTemperature = DEFAULT_TEMPERATURE;
  _temperatureTolerance = DEFAULT_TEMP_TOLERANCE;
  _temperatureReadErrorCount = 0;
  _monitoringTemperature = false;
  
  // Inicializar sensor de presión
  _pressureSensor.begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  _currentPressure = 0;
  _previousPressure = 0;
  _currentWaterLevel = 0;
  _targetWaterLevel = 0;
  _levelStableCounter = 0;
  _monitoringPressure = false;
  
  // Configurar umbrales de presión para niveles de agua
  _pressureLevel1 = NIVEL_PRESION_1;
  _pressureLevel2 = NIVEL_PRESION_2;
  _pressureLevel3 = NIVEL_PRESION_3;
  _pressureLevel4 = NIVEL_PRESION_4;
  
  Utils.debug("Sensores inicializados correctamente");
}

void SensorsClass::update() {
  if (!readSensorData()) {
    Utils.debug("Error al leer datos de sensores");
    return;
  }
  
  // Actualizamos las variables internas
  _previousPressure = _currentPressure;
  
  // Verificar nivel de agua alcanzado
  if (_monitoringPressure && isWaterLevelReached()) {
    Utils.debug("Nivel de agua alcanzado: " + String(_currentWaterLevel) + " (objetivo: " + String(_targetWaterLevel) + ")");
  }
  
  // Verificar temperatura para programas con agua caliente
  if (_monitoringTemperature) {
    if (isTemperatureLow()) {
      Utils.debug("Temperatura baja detectada: " + String(_currentTemperature) + 
                  "°C (objetivo: " + String(_targetTemperature) + "°C)");
    }
  }
}

void SensorsClass::startMonitoring() {
  // Iniciar monitoreo periódico de sensores usando AsyncTask
  Utils.createPeriodicTask(SENSOR_UPDATE_INTERVAL, [this]() {
    this->update();
  });
  Utils.debug("Monitoreo periódico de sensores iniciado");
}

bool SensorsClass::readSensorData() {
  bool success = true;
  
  // Leer temperatura
  _temperatureSensor.requestTemperaturesByAddress(_temperatureAddress);
  float newTemp = _temperatureSensor.getTempC(_temperatureAddress);
  
  // Verificar lectura de temperatura válida
  if (newTemp == DEVICE_DISCONNECTED_C || newTemp < -10.0 || newTemp > 100.0) {
    _temperatureReadErrorCount++;
    Utils.debug("Error en lectura de temperatura: " + String(newTemp));
    if (_temperatureReadErrorCount > MAX_TEMP_READ_ERRORS) {
      success = false;
    }
  } else {
    _currentTemperature = newTemp;
    _temperatureReadErrorCount = 0;
  }
  
  // Leer presión si el sensor está listo
  if (_pressureSensor.is_ready()) {
    uint16_t newPressure = _pressureSensor.pascal();
    
    // Verificar lectura de presión válida
    if (_isReadingValid(newTemp, newPressure)) {
      _currentPressure = newPressure;
      _currentWaterLevel = _calculateWaterLevel(newPressure);
    } else {
      success = false;
    }
  } else {
    Utils.debug("Sensor de presión no está listo");
    success = false;
  }
  
  return success;
}

void SensorsClass::startTemperatureMonitoring(float targetTemperature) {
  _targetTemperature = targetTemperature;
  _monitoringTemperature = true;
  Utils.debug("Iniciando monitoreo de temperatura con objetivo: " + String(_targetTemperature) + "°C");
}

bool SensorsClass::isTemperatureLow() {
  // Función clave para programas con agua caliente (22 y posiblemente 24)
  // Detecta si la temperatura está por debajo del umbral objetivo-tolerancia
  return (_currentTemperature < (_targetTemperature - _temperatureTolerance));
}

bool SensorsClass::isTemperatureStable() {
  // Verifica si la temperatura está dentro del rango de tolerancia
  return (abs(_currentTemperature - _targetTemperature) <= _temperatureTolerance);
}

void SensorsClass::startPressureMonitoring(uint8_t targetLevel) {
  _targetWaterLevel = targetLevel;
  _levelStableCounter = 0;
  _monitoringPressure = true;
  Utils.debug("Iniciando monitoreo de nivel de agua con objetivo: " + String(_targetWaterLevel));
}

bool SensorsClass::isWaterLevelReached() {
  if (_currentWaterLevel >= _targetWaterLevel) {
    _levelStableCounter++;
    // Requerir lecturas estables consecutivas para confirmar nivel
    return (_levelStableCounter >= LEVEL_STABILITY_THRESHOLD);
  } else {
    _levelStableCounter = 0;
    return false;
  }
}

uint8_t SensorsClass::_calculateWaterLevel(uint16_t pressure) {
  // Algoritmo mejorado para calcular nivel de agua basado en umbrales
  if (pressure < _pressureLevel1) {
    return 0; // Sin agua o nivel muy bajo
  } else if (pressure < _pressureLevel2) {
    return 1; // Nivel bajo
  } else if (pressure < _pressureLevel3) {
    return 2; // Nivel medio-bajo
  } else if (pressure < _pressureLevel4) {
    return 3; // Nivel medio-alto
  } else {
    return 4; // Nivel alto
  }
}
```

## Responsabilidades

El módulo de Sensores tiene las siguientes responsabilidades adaptadas para los tres programas específicos:

1. **Inicialización y Configuración**: Configurar los sensores de temperatura (OneWire Dallas) y presión (HX710B) con los parámetros adecuados.

2. **Monitoreo de Temperatura**: 
   - Para Programa 22 (Agua Caliente): Monitoreo constante para garantizar que la temperatura se mantenga dentro del rango objetivo ±2°C.
   - Para Programa 23 (Agua Fría): Monitoreo informativo sin control activo.
   - Para Programa 24 (Multitanda): Monitoreo activo o informativo según configuración de tipo de agua.

3. **Detección de Temperatura Baja**: Detectar cuando la temperatura cae por debajo del umbral (objetivo-2°C), crucial para iniciar el proceso de ajuste de temperatura en programas con agua caliente.

4. **Monitoreo de Nivel de Agua**: Monitorear el nivel de agua en el tambor para todos los programas, asociando mediciones de presión con niveles de agua (1-4).

5. **Validación de Lecturas**: Verificar que las lecturas de sensores sean válidas y estables antes de reportarlas, aumentando la fiabilidad del sistema.

6. **Filtrado de Ruido**: Implementar mecanismos para filtrar lecturas erráticas o ruidosas, especialmente importantes para el control preciso de temperatura en agua caliente.

7. **Interfaz Hacia Controladores**: Proporcionar una API clara para que ProgramController pueda verificar condiciones de temperatura y nivel de agua según el programa activo.

## Ventajas de esta Implementación

1. **Precisión Mejorada**: El uso de variables de tipo float para temperatura permite un control más preciso para programas con agua caliente.

2. **Detección Robusta**: Los algoritmos de verificación de estabilidad (contadores de lecturas consecutivas) evitan falsos positivos en la detección de niveles.

3. **Gestión de Errores**: Implementación de contadores de errores y validación de lecturas para detectar y manejar fallos en los sensores.

4. **Independencia de Hardware**: Abstracción que permite cambiar los sensores físicos sin afectar la lógica de los programas de lavado.

5. **Adaptabilidad**: Interfaces específicas (`isTemperatureLow()`, `isWaterLevelReached()`) que simplifican la integración con la lógica de los tres programas.

6. **Monitoreo No Bloqueante**: Implementa lecturas periódicas mediante temporizadores asíncronos, manteniendo el sistema responsivo.

7. **Depuración Mejorada**: Mensajes de depuración detallados para monitorear el funcionamiento de los sensores en tiempo real.

## Soporte para Programas Específicos

Esta implementación del módulo de Sensores ha sido diseñada específicamente para soportar los requisitos de los tres programas de lavado:

### Para Programa 22 (Agua Caliente):
- Monitoreo activo de temperatura con tolerancia de ±2°C
- Detección inmediata cuando la temperatura cae por debajo del umbral
- Mayor precisión en lecturas de temperatura para el control de agua caliente

### Para Programa 23 (Agua Fría):
- Monitoreo de temperatura solo con fines informativos
- Enfoque en precisión para el monitoreo de nivel de agua

### Para Programa 24 (Multitanda):
- Flexibilidad para alternar entre monitoreo activo (agua caliente) e informativo (agua fría)
- Soporte para múltiples ciclos con verificación consistente del llenado al inicio de cada tanda

Al implementar estas mejoras, el módulo de Sensores proporciona la base sensorial robusta necesaria para que los tres programas específicos funcionen correctamente y de manera confiable en el entorno industrial.
