// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include "../Librerias/OneWire/OneWire.h"
#include "../Librerias/DallasTemperature/DallasTemperature.h"
#include "../Librerias/hx710B_pressure_sensor-main/HX710B.h"
#include "../Librerias/Arduino-AsyncTask/src/AsyncTaskLib.h"

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