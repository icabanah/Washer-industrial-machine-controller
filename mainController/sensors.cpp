// sensors.cpp
#include "sensors.h"
#include "utils.h"

// Instancia global
SensorsClass Sensors;

// Callback para la tarea de monitoreo
void monitoringTimerCallback() {
  Sensors.updateSensors();
}

// Callback para lectura de temperatura
void temperatureReadCallback() {
  // Llamamos al método público que internamente llamará al método privado
  Sensors.readTemperatureCallback();
}

void SensorsClass::init() {
  _currentTemperature = 0.0;
  _currentPressureRaw = 0;
  _currentPressurePascal = 0.0;
  _currentWaterLevel = 0;
  _monitoring = false;
  _monitoringTaskId = 0;
  _tempSensorErrorCount = 0;
  _pressureSensorErrorCount = 0;
  _pressureSensorCalibrated = false;
  
  _setupTemperatureSensor();
  _setupPressureSensor();
  _setupMonitoring();
  
  Utils.debug("Sensores inicializados");
}

void SensorsClass::_setupTemperatureSensor() {
  // Inicializar objetos estáticos en lugar de usar asignación dinámica
  _oneWire = OneWire(PIN_TEMP_SENSOR);
  _tempSensors = DallasTemperature(&_oneWire);
  
  // Inicializar sensor
  _tempSensors.begin();
  
  // Configurar para lecturas asíncronas (no bloqueantes)
  _tempSensors.setWaitForConversion(false);
  
  // Establecer resolución
  _tempSensors.setResolution(TEMP_RESOLUTION);
  
  // Verificar si el sensor está conectado
  if (_tempSensors.getDeviceCount() == 0) {
    Utils.debug("ADVERTENCIA: No se detectaron sensores de temperatura DS18B20");
  } else {
    Utils.debug("Sensor de temperatura inicializado. Sensores encontrados: " + String(_tempSensors.getDeviceCount()));
  }
  
  // Realizar primera solicitud de temperatura
  _requestTemperature();
}

void SensorsClass::_setupPressureSensor() {
  // Usar objeto estático en lugar de asignación dinámica
  _pressureSensor.begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  _pressureSensorErrorCount = 0;
  _pressureSensorCalibrated = false;
  
  // Intentar leer el sensor para verificar que funciona
  if (_pressureSensor.wait_ready_timeout(1000, 100)) {
    // Sensor está respondiendo, realizar calibración inicial
    _calibratePressureSensor();
    Utils.debug("Sensor de presión inicializado correctamente");
  } else {
    Utils.debug("ADVERTENCIA: Sensor de presión no responde");
  }
}

void SensorsClass::_calibratePressureSensor() {
  // Realizar varias lecturas para establecer un offset y calibración adecuados
  if (_pressureSensor.wait_ready_timeout(1000, 100)) {
    _pressureSensor.tare(10); // 10 lecturas para establecer offset
    _pressureSensorCalibrated = true;
    Utils.debug("Sensor de presión calibrado");
  } else {
    _pressureSensorCalibrated = false;
    Utils.debug("Error al calibrar sensor de presión");
  }
}

void SensorsClass::_requestTemperature() {
  // Iniciar la solicitud de temperatura
  _tempSensors.requestTemperatures();
  
  // Programar lectura después del tiempo de conversión
  int conversionTime = _tempSensors.millisToWaitForConversion(_tempSensors.getResolution());
  Utils.createTimeout(conversionTime, temperatureReadCallback);
}

// Método público que actúa como puente para el callback estático
void SensorsClass::readTemperatureCallback() {
  _readTemperature();
}

void SensorsClass::_readTemperature() {
  float temp = _tempSensors.getTempCByIndex(0);
  
  // Verificar si la lectura es válida
  if (temp != DEVICE_DISCONNECTED_C) {
    _currentTemperature = temp;
    _tempSensorErrorCount = 0; // Reiniciar contador de errores
  } else {
    // Si la lectura falló, incrementar contador de errores
    _tempSensorErrorCount++;
    
    // Si hay demasiados errores consecutivos, reportar problema
    if (_tempSensorErrorCount > 5) {
      Utils.debug("ERROR: Múltiples lecturas fallidas del sensor de temperatura");
    }
  }
}

void SensorsClass::_setupMonitoring() {
  // No iniciamos el monitoreo automáticamente, se iniciará cuando se llame a startMonitoring()
  _monitoringTaskId = 0;
}

// Método para actualizar todos los sensores a la vez
void SensorsClass::updateSensors() {
  updateTemperature();
  updatePressure();
}

void SensorsClass::startMonitoring() {
  if (!_monitoring) {
    // Crear una tarea recurrente para monitoreo cada 500ms
    _monitoringTaskId = Utils.createInterval(500, monitoringTimerCallback, true);
    
    if (_monitoringTaskId > 0) {
      _monitoring = true;
      Utils.debug("Monitoreo de sensores iniciado");
    } else {
      Utils.debug("Error al iniciar monitoreo de sensores");
    }
  }
}

void SensorsClass::stopMonitoring() {
  if (_monitoring && _monitoringTaskId > 0) {
    Utils.stopTask(_monitoringTaskId);
    _monitoringTaskId = 0;
    _monitoring = false;
    Utils.debug("Monitoreo de sensores detenido");
  }
}

bool SensorsClass::isMonitoring() {
  return _monitoring;
}

void SensorsClass::updateTemperature() {
  // En lugar de solicitar y leer de inmediato (bloqueante),
  // iniciamos el proceso asíncrono
  _requestTemperature();
}

void SensorsClass::updatePressure() {
  if (_pressureSensor.is_ready()) {
    // Usar el método pascal() para obtener la presión en unidades adecuadas
    _currentPressurePascal = _pressureSensor.pascal();
    _currentPressureRaw = (uint16_t)_currentPressurePascal; // Para compatibilidad
    _currentWaterLevel = _convertPressureToLevel(_currentPressurePascal);
    _pressureSensorErrorCount = 0; // Resetear contador de errores
  } else {
    // Incrementar contador de errores si el sensor no responde
    _pressureSensorErrorCount++;
    
    if (_pressureSensorErrorCount > 10) {
      Utils.debug("ERROR: Múltiples lecturas fallidas del sensor de presión");
      // Si han pasado muchos errores, intentar reiniciar el sensor
      if (_pressureSensorErrorCount > 20) {
        _setupPressureSensor();
      }
    }
  }
}

float SensorsClass::getCurrentTemperature() {
  return _currentTemperature;
}

uint16_t SensorsClass::getCurrentPressureRaw() {
  return _currentPressureRaw;
}

float SensorsClass::getCurrentPressurePascal() {
  return _currentPressurePascal;
}

uint8_t SensorsClass::getCurrentWaterLevel() {
  return _currentWaterLevel;
}

uint8_t SensorsClass::_convertPressureToLevel(float pressure) {
  // Implementación mejorada con interpolación lineal entre niveles
  if (pressure < NIVEL_PRESION_1) return 0;
  
  if (pressure < NIVEL_PRESION_2) {
    float p = (pressure - NIVEL_PRESION_1) / (NIVEL_PRESION_2 - NIVEL_PRESION_1);
    return max(1, min(2, (uint8_t)(1 + p))); // Nivel 1-2 interpolado
  }
  
  if (pressure < NIVEL_PRESION_3) {
    float p = (pressure - NIVEL_PRESION_2) / (NIVEL_PRESION_3 - NIVEL_PRESION_2);
    return max(2, min(3, (uint8_t)(2 + p))); // Nivel 2-3 interpolado
  }
  
  if (pressure < NIVEL_PRESION_4) {
    float p = (pressure - NIVEL_PRESION_3) / (NIVEL_PRESION_4 - NIVEL_PRESION_3);
    return max(3, min(4, (uint8_t)(3 + p))); // Nivel 3-4 interpolado
  }
  
  return 4; // Máximo nivel
}

bool SensorsClass::isTemperatureReached(uint8_t targetTemp) {
  // Considerar que se ha alcanzado la temperatura si está dentro del rango definido
  return Utils.isInRange(_currentTemperature, targetTemp, TEMP_RANGE);
}

bool SensorsClass::isWaterLevelReached(uint8_t targetLevel) {
  return _currentWaterLevel >= targetLevel;
}

void SensorsClass::setTemperatureResolution(uint8_t resolution) {
  _tempSensors.setResolution(resolution);
}

void SensorsClass::resetPressureCalibration() {
  _calibratePressureSensor();
}