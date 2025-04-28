// sensors.cpp
#include "sensors.h"
#include "utils.h"

// Instancia global
SensorsClass Sensors;

void SensorsClass::init() {
  _currentTemperature = 0.0;
  _currentPressureRaw = 0;
  _currentWaterLevel = 0;
  _monitoring = false;
  
  _setupTemperatureSensor();
  _setupPressureSensor();
  _setupMonitoring();
  
  Utils.debug("Sensores inicializados");
}

void SensorsClass::_setupTemperatureSensor() {
  _oneWire = new OneWire(PIN_TEMP_SENSOR);
  _tempSensors = new DallasTemperature(_oneWire);
  _tempSensors->begin();
  _tempSensors->setResolution(TEMP_RESOLUTION);
  
  // Leer temperatura inicial
  updateTemperature();
}

void SensorsClass::_setupPressureSensor() {
  _pressureSensor = new HX710B();
  _pressureSensor->begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  
  // Leer presión inicial
  updatePressure();
}

void SensorsClass::_setupMonitoring() {
  // Configurar temporizador asincrónico para lectura de sensores
  _sensorTimer = new AsyncTask(500, true, []() {
    Sensors.updateTemperature();
    Sensors.updatePressure();
  });
}

void SensorsClass::startMonitoring() {
  if (!_monitoring) {
    _sensorTimer->Start();
    _monitoring = true;
    Utils.debug("Monitoreo de sensores iniciado");
  }
}

void SensorsClass::stopMonitoring() {
  if (_monitoring) {
    _sensorTimer->Stop();
    _monitoring = false;
    Utils.debug("Monitoreo de sensores detenido");
  }
}

bool SensorsClass::isMonitoring() {
  return _monitoring;
}

void SensorsClass::updateTemperature() {
  _tempSensors->requestTemperatures();
  float temp = _tempSensors->getTempCByIndex(0);
  
  // Solo actualizar si es una lectura válida (DS18B20 devuelve -127 en caso de error)
  if (temp > -100.0) {
    _currentTemperature = temp;
  }
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
  _tempSensors->setResolution(resolution);
}
