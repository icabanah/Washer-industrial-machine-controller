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
  Sensors._readTemperature();
}

void SensorsClass::init() {
  _currentTemperature = 0.0;
  _currentPressureRaw = 0;
  _currentWaterLevel = 0;
  _monitoring = false;
  _monitoringTaskId = 0;
  _tempSensorErrorCount = 0;
  
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
  _pressureSensor = new HX710B();
  _pressureSensor->begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  
  // Leer presión inicial
  updatePressure();
}

void SensorsClass::_requestTemperature() {
  // Iniciar la solicitud de temperatura
  _tempSensors.requestTemperatures();
  
  // Programar lectura después del tiempo de conversión
  int conversionTime = _tempSensors.millisToWaitForConversion(_tempSensors.getResolution());
  Utils.createTimeout(conversionTime, temperatureReadCallback);
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
  if (_pressureSensor->is_ready()) {
    _currentPressureRaw = _pressureSensor->read();
    _currentWaterLevel = _convertPressureToLevel(_currentPressureRaw);
  }
}

float SensorsClass::getCurrentTemperature() {
  return _currentTemperature;
}

uint16_t SensorsClass::getCurrentPressureRaw() {
  return _currentPressureRaw;
}

uint8_t SensorsClass::getCurrentWaterLevel() {
  return _currentWaterLevel;
}

uint8_t SensorsClass::_convertPressureToLevel(uint16_t pressure) {
  // Convertir la presión a nivel de agua (0-4)
  if (pressure < NIVEL_PRESION_1) return 0;
  if (pressure < NIVEL_PRESION_2) return 1;
  if (pressure < NIVEL_PRESION_3) return 2;
  if (pressure < NIVEL_PRESION_4) return 3;
  return 4;
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