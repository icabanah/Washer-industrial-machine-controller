// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>
#include <AsyncTaskLib.h>

class SensorsClass {
public:
  // Inicialización
  void init();
  
  // Control de monitorización
  void startMonitoring();
  void stopMonitoring();
  bool isMonitoring();
  
  // Lectura de sensores
  void updateTemperature();
  void updatePressure();
  
  // Obtención de valores actuales
  float getCurrentTemperature();
  uint16_t getCurrentPressureRaw();
  uint8_t getCurrentWaterLevel();
  
  // Verificación de objetivos
  bool isTemperatureReached(uint8_t targetTemp);
  bool isWaterLevelReached(uint8_t targetLevel);
  
  // Control de sensores
  void setTemperatureResolution(uint8_t resolution);

private:
  // Variables para sensores
  OneWire* _oneWire;
  DallasTemperature* _tempSensors;
  HX710B* _pressureSensor;
  AsyncTask* _sensorTimer;
  
  // Variables para almacenar lecturas
  float _currentTemperature;
  uint16_t _currentPressureRaw;
  uint8_t _currentWaterLevel;
  bool _monitoring;
  
  // Métodos internos
  void _setupTemperatureSensor();
  void _setupPressureSensor();
  void _setupMonitoring();
  uint8_t _convertPressureToLevel(uint16_t pressure);
};

// Instancia global
extern SensorsClass Sensors;

#endif // SENSORS_H