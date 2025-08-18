// sensors.cpp
#include "sensors.h"
#include "utils.h"
#include "hardware.h"
#include "actuators.h"
#include "math_utils.h"
#include "debug.h"

// Instancia global
SensorsClass Sensors;

// Callback para la tarea de monitoreo
void monitoringTimerCallback() {
  Sensors.updateSensors();
}

// NOTA: temperatureReadCallback ya no se usa con el nuevo enfoque
// Se mantiene comentado por si se necesita en el futuro
/*
// Callback para lectura de temperatura
void temperatureReadCallback() {
  // Llamamos al método público que internamente llamará al método privado
  Sensors.readTemperatureCallback();
}
*/

void SensorsClass::init() {
  _currentTemperature = 0.0;
  _currentPressureRaw = 0;
  _currentPressurePascal = 0.0;
  _currentWaterLevel = 0;
  _monitoring = false;
  _monitoringTaskId = 0;
  _tempReadTaskId = 0;
  _tempConversionInProgress = false;
  _tempConversionStartTime = 0;
  _tempReadCount = 0;
  _tempSensorErrorCount = 0;
  _pressureSensorErrorCount = 0;
  _pressureSensorCalibrated = false;
  
  _setupTemperatureSensor();
  _setupPressureSensor();
  _setupMonitoring();
  
  Debug.print("Sensores inicializados");
}

void SensorsClass::_setupTemperatureSensor() {
  // Inicializar objetos estáticos en lugar de usar asignación dinámica
  _oneWire = OneWire(PIN_TEMP_SENSOR);
  _tempSensors = DallasTemperature(&_oneWire);
  
  // Inicializar sensor
  _tempSensors.begin();
  
  // Configurar para lecturas asíncronas (no bloqueantes)
  _tempSensors.setWaitForConversion(false);
  
  // Verificar si el sensor está conectado
  uint8_t deviceCount = _tempSensors.getDeviceCount();
  Debug.print("Sensor temperatura - Pin: " + String(PIN_TEMP_SENSOR) + ", Detectados: " + String(deviceCount));
  
  if (deviceCount == 0) {
    Debug.print("ERROR: No se detectaron sensores DS18B20");
  } else {
    
    
    if (!_tempSensors.isConnected(_tempSensorAddress) && deviceCount > 0) {
      _tempSensors.getAddress(_tempSensorAddress, 0);
    }
    _tempSensors.setResolution(_tempSensorAddress, TEMP_RESOLUTION);
  }
}

/// @brief 
/// Configura el sensor de presión HX710B.
/// Este método inicializa el sensor de presión y realiza una calibración inicial.
void SensorsClass::_setupPressureSensor() {
  // Usar objeto estático en lugar de asignación dinámica
  _pressureSensor.begin(PIN_PRESION_DOUT, PIN_PRESION_SCLK);
  _pressureSensorErrorCount = 0;
  _pressureSensorCalibrated = false;
  
  // Intentar leer el sensor para verificar que funciona
  if (_pressureSensor.wait_ready_timeout(1000, 100)) {
    // Sensor está respondiendo, realizar calibración inicial
    _calibratePressureSensor();
  } else {
  }
}

/// @brief 
/// Reinicia la calibración del sensor de presión.
/// Este método restablece el offset del sensor de presión y lo recalibra.
void SensorsClass::_calibratePressureSensor() {
  // Realizar varias lecturas para establecer un offset y calibración adecuados
  if (_pressureSensor.wait_ready_timeout(1000, 100)) {
    _pressureSensor.tare(10); // 10 lecturas para establecer offset
    _pressureSensorCalibrated = true;
  } else {
    _pressureSensorCalibrated = false;
  }
}

void SensorsClass::_setupMonitoring() {
  // No iniciamos el monitoreo automáticamente, se iniciará cuando se llame a startMonitoring()
  _monitoringTaskId = 0;
}

/// @brief 
/// Actualiza los sensores de temperatura y presión.
/// Este método se encarga de leer los valores actuales de los sensores y actualizar las variables internas.
void SensorsClass::updateSensors() {
  // Para la temperatura, solo iniciamos una nueva lectura si no hay una en progreso
  updateTemperature();
  
  // La presión se puede leer directamente sin esperas
  updatePressure();
}

/// @brief 
/// Inicia el monitoreo de sensores.
/// Este método configura un temporizador que llama periódicamente a la función de actualización de sensores.
/// @details
/// Este método crea una tarea recurrente que se ejecuta cada 500 ms para actualizar los sensores de temperatura y presión.
/// Asegúrate de que el monitoreo se detenga adecuadamente con `stopMonitoring()` cuando ya no sea necesario.
void SensorsClass::startMonitoring() {
  if (!_monitoring) {
    // Crear una tarea recurrente para monitoreo cada 500ms
    _monitoringTaskId = Utils.createInterval(500, monitoringTimerCallback, true);
    
    if (_monitoringTaskId > 0) {
      _monitoring = true;
    } else {
    }
  }
}

void SensorsClass::stopMonitoring() {
  if (_monitoring && _monitoringTaskId > 0) {
    Utils.stopTask(_monitoringTaskId);
    _monitoringTaskId = 0;
    _monitoring = false;
    
    // Reiniciar flags de temperatura
    _tempConversionInProgress = false;
    _tempReadCount = 0;
    
  }
}

bool SensorsClass::isMonitoring() {
  return _monitoring;
}

void SensorsClass::updateTemperature() {
  // Verificar si hay una conversión en progreso
  if (_tempConversionInProgress) {
    // Verificar si la conversión ha terminado
    if (_tempSensors.isConversionComplete()) {
      // Leer el resultado
      // float temp = _tempSensors.getTempC(_tempSensorAddress);
      float temp = _tempSensors.getTempC(_tempSensorAddress);
      // Debug.print("Lectura de temperatura: " + String(temp) + "°C");
      _tempConversionInProgress = false;
      
      // Verificar si la lectura es válida
      if (temp != DEVICE_DISCONNECTED_C && temp >= -127.0 && temp <= 85.0) {
        _currentTemperature = temp;
        
        _tempSensorErrorCount = 0;
        
        // Mostrar temperatura cada 10 lecturas (cada 5 segundos aproximadamente)
        _tempReadCount++;
        if (_tempReadCount >= 10) {
          _tempReadCount = 0;
        }
      } else {
        _tempSensorErrorCount++;
        if (_tempSensorErrorCount > 10) {
          _setupTemperatureSensor();
          _tempSensorErrorCount = 0;
        }
      }
    }
    // Si la conversión no ha terminado, esperamos al próximo ciclo
  } else {
    // No hay conversión en progreso, iniciamos una nueva
    _tempSensors.requestTemperatures();
    _tempConversionInProgress = true;
  }
}

/// @brief 
/// Actualiza la lectura del sensor de presión.
/// Este método lee el valor actual del sensor de presión y actualiza las variables internas.
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
    
    if (_pressureSensorErrorCount > 20) {
      _setupPressureSensor();
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
    uint8_t level = (uint8_t)(1 + p);
    return (level < 1) ? 1 : ((level > 2) ? 2 : level); // Nivel 1-2 interpolado
  }
  
  if (pressure < NIVEL_PRESION_3) {
    float p = (pressure - NIVEL_PRESION_2) / (NIVEL_PRESION_3 - NIVEL_PRESION_2);
    uint8_t level = (uint8_t)(2 + p);
    return (level < 2) ? 2 : ((level > 3) ? 3 : level); // Nivel 2-3 interpolado
  }
  
  if (pressure < NIVEL_PRESION_4) {
    float p = (pressure - NIVEL_PRESION_3) / (NIVEL_PRESION_4 - NIVEL_PRESION_3);
    uint8_t level = (uint8_t)(3 + p);
    return (level < 3) ? 3 : ((level > 4) ? 4 : level); // Nivel 3-4 interpolado
  }
  
  return 4; // Máximo nivel
}

bool SensorsClass::isTemperatureReached(uint8_t targetTemp) {
  // Considerar que se ha alcanzado la temperatura si:
  // 1. Está dentro del rango definido (target ± 2°C), O
  // 2. La temperatura actual es igual o mayor al setpoint (si se sobrepasó, se considera alcanzado)
  return MathUtils::isInRange(_currentTemperature, targetTemp, TEMP_RANGE) || 
         (_currentTemperature >= targetTemp);
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

bool SensorsClass::isDoorClosed() {
  // === IMPLEMENTACIÓN PARA SISTEMA INDUSTRIAL ===
  
  // Verificar primero si la puerta está bloqueada electrónicamente
  if (Actuators.isDoorLocked()) {
    return true; // Si está bloqueada, considerarla cerrada por seguridad
  }
  
  // Para este sistema, sin sensor físico de puerta dedicado, 
  // usamos una lógica basada en el estado del sistema:
  
  // 1. Si hay emergencia activa, considerar puerta abierta para seguridad
  if (Hardware.isEmergencyButtonPressed()) {
    return false; // Emergencia = puerta debe estar abierta para evacuación
  }
  
  // 2. Durante operación normal, la puerta se considera cerrada cuando:
  //    - El sistema no está en emergencia
  //    - No hay problemas de comunicación con actuadores
  
  // Por defecto, en ausencia de sensor físico, asumir puerta cerrada
  // En una implementación futura se puede añadir:
  // - Sensor magnético en PIN adicional (ej: PIN_SENSOR_PUERTA)
  // - Sensor reed switch
  // - Fin de carrera mecánico
  
  return true; // Asumir puerta cerrada por defecto
}

// void SensorsClass::diagnosticTemperatureSensor() {
//   // === DIAGNÓSTICO MANUAL DEL SENSOR DE TEMPERATURA ===
//   Debug.print("=== DIAGNÓSTICO SENSOR TEMPERATURA ===");
  
//   // 1. Verificar conexión
//   uint8_t deviceCount = _tempSensors.getDeviceCount();
//   Debug.print("Dispositivos detectados: " + String(deviceCount));
  
//   if (deviceCount == 0) {
//     Debug.print("❌ ERROR: No se detectan sensores");
//     Debug.print("Verificar conexiones y alimentación");
//     return;
//   }
  
//   // 2. Verificar comunicación con el sensor configurado
//   if (_tempSensors.isConnected(_tempSensorAddress)) {
//     Debug.print("✅ Sensor configurado responde correctamente");
//   } else {
//     Debug.print("⚠️ El sensor configurado no responde");
//   }
  
//   // 3. Realizar lectura de prueba no bloqueante
//   _tempSensors.requestTemperatures();
//   _tempConversionInProgress = true;
//   _tempConversionStartTime = millis();
  
//   // Crear timeout para verificar conversión después de 1 segundo
//   Utils.createTimeout(1000, _callbackCompleteDiagnostic);
  
// }

// void SensorsClass::_completeTemperatureDiagnostic() {
//   if (!_tempConversionInProgress) return;
  
//   float temp = _tempSensors.getTempC(_tempSensorAddress);
  
//   if (temp != DEVICE_DISCONNECTED_C && temp >= -127.0 && temp <= 85.0) {
//     Debug.print("✅ Lectura válida: " + String(temp) + "°C");
//     _currentTemperature = temp;
//     _tempSensorErrorCount = 0;
//   } else {
//     Debug.print("❌ Lectura inválida: " + String(temp));
//     _tempSensorErrorCount++;
//   }
  
//   _tempConversionInProgress = false;
  
//   // 4. Mostrar estadísticas de errores
//   Debug.print("Errores acumulados: " + String(_tempSensorErrorCount));
//   Debug.print("Temperatura actual almacenada: " + String(_currentTemperature) + "°C");
  
//   Debug.print("=== FIN DIAGNÓSTICO ===");
// }

// Función wrapper estática para callback
void SensorsClass::_callbackCompleteDiagnostic() {
  Sensors._completeTemperatureDiagnostic();
}