// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"
#include "config.h"
#include "../Librerias/OneWire/OneWire.h"
// #include "../Librerias/DallasTemperature/DallasTemperature.h"
#include <DallasTemperature.h>
// #include "../Librerias/hx710B_pressure_sensor-main/HX710B.h"

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
  // void updatePressure();
  void updateSensors(); // Método para actualizar todos los sensores
  void readTemperatureCallback(); // Método público para el callback de temperatura
  
  // Obtención de valores actuales
  float getCurrentTemperature();
  // uint16_t getCurrentPressureRaw();
  float getCurrentPressurePascal();
  uint8_t getCurrentWaterLevel();
  
  // Verificación de objetivos
  bool isTemperatureReached(uint8_t targetTemp);  
  bool isWaterLevelReached(uint8_t targetLevel);
  
  // Control de sensores
  void setTemperatureResolution(uint8_t resolution);
  // void resetPressureCalibration();

private:
  // Variables para sensores
  OneWire _oneWire;
  DallasTemperature _tempSensors;
  // HX710B _pressureSensor;
  
  // Variables para almacenar lecturas
  float _currentTemperature;
  // uint16_t _currentPressureRaw;  // Mantener para compatibilidad
  // float _currentPressurePascal;  // Almacenar la presión en Pascales
  uint8_t _currentWaterLevel;
  bool _monitoring;
  uint8_t _tempSensorErrorCount;
  uint8_t _pressureSensorErrorCount;
  bool _pressureSensorCalibrated;

  // ID de tarea temporizada
  int _monitoringTaskId;
  
  // Métodos internos
  void _setupTemperatureSensor();
  // void _setupPressureSensor();
  void _setupMonitoring();
  void _requestTemperature();
  void _readTemperature();
  // void _calibratePressureSensor();
  // uint8_t _convertPressureToLevel(float pressure);
};

// Instancia global
extern SensorsClass Sensors;

#endif // SENSORS_H