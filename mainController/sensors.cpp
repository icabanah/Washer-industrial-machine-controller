// sensors.cpp
#include "sensors.h"
#include "utils.h"
#include "hardware.h"
#include "actuators.h"

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
  
  Utils.debug("SensorsClass::init| Sensores inicializados");
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
  Utils.debug("=== DIAGNÓSTICO SENSOR TEMPERATURA ===");
  Utils.debug("Pin configurado: " + String(PIN_TEMP_SENSOR));
  Utils.debug("Sensores detectados: " + String(deviceCount));
  
  if (deviceCount == 0) {
    Utils.debug("ERROR: No se detectaron sensores DS18B20");
    Utils.debug("Verificar:");
    Utils.debug("- Conexión física del sensor al pin " + String(PIN_TEMP_SENSOR));
    Utils.debug("- Resistor pull-up de 4.7kΩ entre datos y VCC");
    Utils.debug("- Alimentación del sensor (3.3V o 5V)");
  } else {
    Utils.debug("✓ Sensores encontrados correctamente");
    
    // Mostrar información de todos los sensores detectados
    for (uint8_t i = 0; i < deviceCount; i++) {
      DeviceAddress tempAddr;
      if (_tempSensors.getAddress(tempAddr, i)) {
        String address = "Sensor " + String(i) + ": {0x";
        for (uint8_t j = 0; j < 8; j++) {
          if (tempAddr[j] < 16) address += "0";
          address += String(tempAddr[j], HEX);
          if (j < 7) address += ", 0x";
        }
        address += "}";
        Utils.debug(address);
      }
    }
    
    // Verificar si la dirección configurada funciona
    if (_tempSensors.isConnected(_tempSensorAddress)) {
      Utils.debug("✓ Dirección configurada es válida");
    } else {
      Utils.debug("⚠ La dirección configurada no responde");
      
      if (deviceCount > 0) {
        Utils.debug("Usando el primer sensor detectado...");
        
        // Obtener la dirección del primer sensor encontrado
        if (_tempSensors.getAddress(_tempSensorAddress, 0)) {
          String newAddress = "Nueva dirección: {0x";
          for (uint8_t i = 0; i < 8; i++) {
            if (_tempSensorAddress[i] < 16) newAddress += "0";
            newAddress += String(_tempSensorAddress[i], HEX);
            if (i < 7) newAddress += ", 0x";
          }
          newAddress += "}";
          Utils.debug(newAddress);
          Utils.debug("NOTA: Actualizar TEMP_SENSOR_ADDR en config.h con esta dirección");
        }
      }
    }
    
    // Establecer resolución específica para el sensor
    _tempSensors.setResolution(_tempSensorAddress, TEMP_RESOLUTION);
    Utils.debug("Resolución configurada: " + String(TEMP_RESOLUTION) + " bits");
  }
  
  Utils.debug("=== FIN DIAGNÓSTICO ===");
  
  // No iniciar ninguna lectura aquí, se hará en el ciclo de monitoreo
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
    Utils.debug("SensorsClass::_setupPressureSensor| Sensor de presión inicializado correctamente");
  } else {
    Utils.debug("SensorsClass::_setupPressureSensor| ADVERTENCIA: Sensor de presión no responde");
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
    Utils.debug("SensorsClass::_calibratePressureSensor| Sensor de presión calibrado");
  } else {
    _pressureSensorCalibrated = false;
    Utils.debug("SensorsClass::_calibratePressureSensor| Error al calibrar sensor de presión");
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
  if (!_tempConversionInProgress) {
    updateTemperature();
  }
  
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
    
    // Reiniciar flags de temperatura
    _tempConversionInProgress = false;
    _tempReadCount = 0;
    
    Utils.debug("Monitoreo de sensores detenido");
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
      float temp = _tempSensors.getTempC(_tempSensorAddress);
      _tempConversionInProgress = false;
      
      // Verificar si la lectura es válida
      if (temp != DEVICE_DISCONNECTED_C && temp >= -127.0 && temp <= 85.0) {
        _currentTemperature = temp;
        
        // Si es la primera lectura exitosa después de errores, reportarlo
        if (_tempSensorErrorCount > 0) {
          Utils.debug("✅ Sensor de temperatura recuperado");
        }
        _tempSensorErrorCount = 0;
        
        // Mostrar temperatura cada 10 lecturas (cada 5 segundos aproximadamente)
        _tempReadCount++;
        if (_tempReadCount >= 10) {
          Utils.debug("🌡️ Temperatura actual: " + String(temp) + "°C");
          _tempReadCount = 0;
        }
      } else {
        _tempSensorErrorCount++;
        Utils.debug("❌ Error de lectura temperatura: " + String(temp) + " (error #" + String(_tempSensorErrorCount) + ")");
        
        if (_tempSensorErrorCount == 5) {
          Utils.debug("⚠️ PROBLEMA: Múltiples lecturas fallidas del sensor de temperatura");
          Utils.debug("Causas posibles:");
          Utils.debug("- Sensor desconectado del pin " + String(PIN_TEMP_SENSOR));
          Utils.debug("- Falta resistor pull-up de 4.7kΩ");
          Utils.debug("- Sensor dañado");
        }
        
        if (_tempSensorErrorCount > 10) {
          Utils.debug("🔄 Reintentando inicialización del sensor de temperatura...");
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

void SensorsClass::diagnosticTemperatureSensor() {
  Utils.debug("=== DIAGNÓSTICO MANUAL DEL SENSOR DE TEMPERATURA ===");
  
  // Información básica
  Utils.debug("Pin configurado: " + String(PIN_TEMP_SENSOR));
  Utils.debug("Resolución: " + String(TEMP_RESOLUTION) + " bits");
  
  // Buscar dispositivos
  uint8_t deviceCount = _tempSensors.getDeviceCount();
  Utils.debug("Dispositivos detectados: " + String(deviceCount));
  
  if (deviceCount == 0) {
    Utils.debug("❌ NO SE DETECTARON SENSORES");
    Utils.debug("Pasos para revisar:");
    Utils.debug("1. Verificar cable de datos conectado al pin " + String(PIN_TEMP_SENSOR));
    Utils.debug("2. Verificar VCC conectado a 3.3V o 5V");
    Utils.debug("3. Verificar GND conectado a tierra");
    Utils.debug("4. Verificar resistor de 4.7kΩ entre datos y VCC");
    return;
  }
  
  // Mostrar todos los dispositivos encontrados
  for (uint8_t i = 0; i < deviceCount; i++) {
    DeviceAddress addr;
    if (_tempSensors.getAddress(addr, i)) {
      String hexAddr = "{0x";
      for (uint8_t j = 0; j < 8; j++) {
        if (addr[j] < 16) hexAddr += "0";
        hexAddr += String(addr[j], HEX);
        if (j < 7) hexAddr += ", 0x";
      }
      hexAddr += "}";
      
      Utils.debug("Dispositivo " + String(i) + ": " + hexAddr);
      
      // Probar lectura
      _tempSensors.requestTemperaturesByAddress(addr);
      delay(1000); // Esperar conversión completa
      float temp = _tempSensors.getTempC(addr);
      
      if (temp != DEVICE_DISCONNECTED_C) {
        Utils.debug("  ✅ Temperatura: " + String(temp) + "°C");
      } else {
        Utils.debug("  ❌ Error de lectura");
      }
    }
  }
  
  // Verificar sensor configurado
  Utils.debug("--- Verificando sensor configurado ---");
  String configAddr = "{0x";
  for (uint8_t i = 0; i < 8; i++) {
    if (_tempSensorAddress[i] < 16) configAddr += "0";
    configAddr += String(_tempSensorAddress[i], HEX);
    if (i < 7) configAddr += ", 0x";
  }
  configAddr += "}";
  Utils.debug("Dirección configurada: " + configAddr);
  
  if (_tempSensors.isConnected(_tempSensorAddress)) {
    Utils.debug("✅ Sensor configurado responde correctamente");
    _tempSensors.requestTemperaturesByAddress(_tempSensorAddress);
    delay(1000);
    float temp = _tempSensors.getTempC(_tempSensorAddress);
    Utils.debug("✅ Temperatura actual: " + String(temp) + "°C");
  } else {
    Utils.debug("❌ Sensor configurado NO responde");
    if (deviceCount > 0) {
      Utils.debug("💡 Sugerencia: Actualizar TEMP_SENSOR_ADDR en config.h");
      Utils.debug("💡 Usar la dirección del primer dispositivo encontrado");
    }
  }
  
  Utils.debug("=== FIN DIAGNÓSTICO ===");
}

bool SensorsClass::isDoorClosed() {
  // Verificar primero si la puerta está bloqueada electrónicamente
  if (Actuators.isDoorLocked()) {
    return true; // Si está bloqueada, considerarla cerrada
  }
  
  // Si no está bloqueada, verificar sensor físico
  // Por ahora, usamos el botón de emergencia como sensor de puerta
  // En una implementación real, esto sería un sensor magnético de puerta
  return !Hardware.isEmergencyButtonPressed();
}