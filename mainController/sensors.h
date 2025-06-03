// sensors.h
#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>

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
  void updateSensors(); // Método para actualizar todos los sensores
  // void readTemperatureCallback(); // Ya no se usa con el nuevo enfoque
  
  // Obtención de valores actuales
  float getCurrentTemperature();
  uint16_t getCurrentPressureRaw();
  float getCurrentPressurePascal();
  uint8_t getCurrentWaterLevel();
  
  // Verificación de objetivos
  bool isTemperatureReached(uint8_t targetTemp);  
  bool isWaterLevelReached(uint8_t targetLevel);
  
  // Control de sensores
  void setTemperatureResolution(uint8_t resolution);
  void resetPressureCalibration();
  void diagnosticTemperatureSensor(); // Nueva función para diagnóstico manual

private:
  // Variables para sensores
  OneWire _oneWire;
  DallasTemperature _tempSensors;
  DeviceAddress _tempSensorAddress = TEMP_SENSOR_ADDR;
  HX710B _pressureSensor;
  
  // Variables para almacenar lecturas
  float _currentTemperature;
  uint16_t _currentPressureRaw;  // Mantener para compatibilidad
  float _currentPressurePascal;  // Almacenar la presión en Pascales
  uint8_t _currentWaterLevel;
  bool _monitoring;
  uint8_t _tempSensorErrorCount;
  uint8_t _pressureSensorErrorCount;
  bool _pressureSensorCalibrated;

  // ID de tareas temporizadas
  int _monitoringTaskId;
  int _tempReadTaskId;  // ID para el temporizador de lectura de temperatura
  bool _tempConversionInProgress;  // Flag para evitar solicitudes concurrentes
  unsigned long _tempConversionStartTime;  // Tiempo cuando se inició la conversión
  uint8_t _tempReadCount;  // Contador para debug ocasional
  
  // Métodos internos
  void _setupTemperatureSensor();
  void _setupPressureSensor();
  void _setupMonitoring();
  // void _requestTemperature();  // Ya no se usa con el nuevo enfoque
  // void _readTemperature();      // Ya no se usa con el nuevo enfoque
  void _calibratePressureSensor();
  uint8_t _convertPressureToLevel(float pressure);
};

// Instancia global
extern SensorsClass Sensors;

#endif // SENSORS_H