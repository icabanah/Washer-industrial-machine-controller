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
  

  /// @brief 
  /// Configura el sensor de temperatura y el sensor de presión.
  /// Este método debe ser llamado al inicio del programa para configurar los sensores.
  /// @details
  /// Este método inicializa el sensor de temperatura utilizando OneWire y DallasTemperature,
  /// y el sensor de presión utilizando HX710B. Configura las resoluciones y verifica la conexión de los sensores.
  /// @note
  /// Asegúrate de que los pines de los sensores estén correctamente conectados y configurados en `config.h`.
  void startMonitoring();


  /// @brief 
  /// Detiene el monitoreo de los sensores.
  /// Este método detiene cualquier tarea de monitoreo en curso y libera los recursos asociados.
  /// @details
  /// Este método detiene la tarea de monitoreo de sensores que se ejecuta periódicamente,
  /// liberando los recursos asociados y asegurando que no se realicen más lecturas de sensores.
  /// @note
  /// Asegúrate de llamar a este método antes de apagar el sistema o cambiar la configuración de los sensores.
  /// @return
  /// No retorna ningún valor, pero detiene el monitoreo de los sensores.
  void stopMonitoring();

  /// @brief 
  /// Verifica si el monitoreo de sensores está activo.
  /// Este método comprueba si hay una tarea de monitoreo en curso.
  /// @details
  /// Este método devuelve un valor booleano que indica si el monitoreo de sensores está activo o no.
  /// Si el monitoreo está activo, significa que las lecturas de temperatura y presión se están realizando periódicamente.
  /// @note
  /// Asegúrate de que el monitoreo se haya iniciado previamente con `startMonitoring()`.
  /// @return
  /// true si el monitoreo está activo, false si no lo está.
  /// Este valor puede ser utilizado para determinar si se deben realizar lecturas de sensores o no.
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