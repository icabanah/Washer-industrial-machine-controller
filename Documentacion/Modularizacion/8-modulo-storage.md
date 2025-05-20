# Módulo de Almacenamiento (storage.h / storage.cpp)

## Descripción

El módulo de Almacenamiento gestiona la persistencia de datos del sistema a través de ESP32 Preferences. Se encarga de guardar y recuperar las configuraciones de los tres programas específicos (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda), sus parámetros y preferencias del usuario. Este módulo es esencial para mantener la configuración incluso cuando el dispositivo está apagado y para preservar el estado entre sesiones.

## Analogía: Biblioteca de Partituras Especializada

Este módulo funciona como una biblioteca especializada que almacena diferentes partituras musicales (configuraciones de programas). Cada partitura (programa) tiene sus propias características específicas: la partitura del Programa 22 está diseñada para instrumentos de cuerda caliente, la del Programa 23 para instrumentos de viento frío, y la del Programa 24 contiene varias secciones que se pueden personalizar según el gusto del director. El bibliotecario (módulo Storage) sabe exactamente dónde guardar y encontrar cada partitura, y puede recuperarlas intactas incluso después de que la orquesta haya terminado por el día.

## Estructura del Módulo

El módulo de Almacenamiento se divide en:

- **storage.h**: Define la interfaz pública del módulo
- **storage.cpp**: Implementa la funcionalidad interna

### Interfaz (storage.h)

```cpp
// storage.h
#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"
#include <EEPROM.h>

class StorageClass {
public:
  // Inicialización
  void init();
  
  // Operaciones principales
  void loadFromEEPROM();
  void saveToEEPROM();
  
  // Operaciones específicas
  void saveProgram(uint8_t program);
  void savePhase(uint8_t phase);
  void saveBlockCounter(uint16_t counter);
  void saveTimers(uint8_t minutes, uint8_t seconds);
  
  // Acceso a datos específicos
  uint8_t getProgramNumber();
  uint8_t getPhaseNumber();
  uint16_t getBlockCounter();
  
  // Validación
  bool isEEPROMValid();

private:
  // Direcciones EEPROM
  const uint16_t _addrProgram = EEPROM_ADDR_PROGRAMA;
  const uint16_t _addrPhase = EEPROM_ADDR_FASE;
  const uint16_t _addrMinutes = EEPROM_ADDR_MINUTOS;
  const uint16_t _addrSeconds = EEPROM_ADDR_SEGUNDOS;
  const uint16_t _addrCounterHigh = EEPROM_ADDR_CONTADOR_H;
  const uint16_t _addrCounterLow = EEPROM_ADDR_CONTADOR_L;
  const uint16_t _addrBaseNiveles = EEPROM_ADDR_BASE_NIVELES;
  const uint16_t _addrBaseTemp = EEPROM_ADDR_BASE_TEMP;
  const uint16_t _addrBaseTiempos = EEPROM_ADDR_BASE_TIEMPOS;
  const uint16_t _addrBaseRotacion = EEPROM_ADDR_BASE_ROTACION;
  
  // Métodos internos
  void _loadProgramData();
  void _saveProgramData();
  uint16_t _calculateProgramAddress(uint8_t program, uint8_t phase, uint8_t dataType);
};

// Instancia global
extern StorageClass Storage;

#endif // STORAGE_H
```

### Implementación (storage.cpp)

```cpp
// storage.cpp
#include "storage.h"
#include "utils.h"

// Definición de la instancia global
StorageClass Storage;

// Constantes para las claves de ESP32 Preferences
const char* PREFS_NAMESPACE = "washerCtrl";
const char* KEY_P22_WATER_LEVEL = "p22_wl";
const char* KEY_P22_TEMP = "p22_temp";
const char* KEY_P22_WASH_TIME = "p22_time";
const char* KEY_P22_ROTATION = "p22_rot";
const char* KEY_P22_CENTRIFUGE = "p22_cent";
const char* KEY_P22_CENT_LEVEL = "p22_centlvl";
const char* KEY_P22_CENT_TIME = "p22_centtime";

const char* KEY_P23_WATER_LEVEL = "p23_wl";
const char* KEY_P23_TEMP = "p23_temp";  // Solo informativo para Programa 23
const char* KEY_P23_WASH_TIME = "p23_time";
const char* KEY_P23_ROTATION = "p23_rot";
const char* KEY_P23_CENTRIFUGE = "p23_cent";
const char* KEY_P23_CENT_LEVEL = "p23_centlvl";
const char* KEY_P23_CENT_TIME = "p23_centtime";

const char* KEY_P24_HOT_WATER = "p24_hot";
const char* KEY_P24_CYCLES = "p24_cycles";
const char* KEY_P24_WATER_LEVEL = "p24_wl";
const char* KEY_P24_TEMP = "p24_temp";
const char* KEY_P24_WASH_TIME = "p24_time";
const char* KEY_P24_ROTATION = "p24_rot";
const char* KEY_P24_CENTRIFUGE = "p24_cent";
const char* KEY_P24_CENT_LEVEL = "p24_centlvl";
const char* KEY_P24_CENT_TIME = "p24_centtime";

// Claves para estadísticas
const char* KEY_P22_USAGE = "p22_usage";
const char* KEY_P23_USAGE = "p23_usage";
const char* KEY_P24_USAGE = "p24_usage";

// Claves para tandas en Programa 24
const char* KEY_P24_CYCLE_PREFIX = "p24_c";  // Se añade número de tanda después

void StorageClass::init() {
  _preferences.begin(PREFS_NAMESPACE, false);
  
  // Verificar si es primera ejecución
  if (!hasUserSettings()) {
    resetToDefaults();
    Utils.debug("Configuración por defecto cargada (primera ejecución)");
  } else {
    loadPrograms();
    Utils.debug("Configuraciones cargadas desde almacenamiento");
  }
}

void StorageClass::loadPrograms() {
  // Cargar configuración Programa 22 (Agua Caliente)
  _program22Config.waterLevel = _preferences.getUChar(KEY_P22_WATER_LEVEL, DEFAULT_WATER_LEVEL);
  _program22Config.temperature = _preferences.getFloat(KEY_P22_TEMP, DEFAULT_HOT_TEMPERATURE);
  _program22Config.washTime = _preferences.getUShort(KEY_P22_WASH_TIME, DEFAULT_WASH_TIME);
  _program22Config.rotationLevel = _preferences.getUChar(KEY_P22_ROTATION, DEFAULT_ROTATION);
  _program22Config.centrifugeEnabled = _preferences.getBool(KEY_P22_CENTRIFUGE, true);
  _program22Config.centrifugeLevel = _preferences.getUChar(KEY_P22_CENT_LEVEL, DEFAULT_CENTRIFUGE);
  _program22Config.centrifugeTime = _preferences.getUShort(KEY_P22_CENT_TIME, DEFAULT_CENTRIFUGE_TIME);
  
  // Cargar configuración Programa 23 (Agua Fría)
  _program23Config.waterLevel = _preferences.getUChar(KEY_P23_WATER_LEVEL, DEFAULT_WATER_LEVEL);
  _program23Config.temperature = _preferences.getFloat(KEY_P23_TEMP, DEFAULT_COLD_TEMPERATURE);
  _program23Config.washTime = _preferences.getUShort(KEY_P23_WASH_TIME, DEFAULT_WASH_TIME);
  _program23Config.rotationLevel = _preferences.getUChar(KEY_P23_ROTATION, DEFAULT_ROTATION);
  _program23Config.centrifugeEnabled = _preferences.getBool(KEY_P23_CENTRIFUGE, true);
  _program23Config.centrifugeLevel = _preferences.getUChar(KEY_P23_CENT_LEVEL, DEFAULT_CENTRIFUGE);
  _program23Config.centrifugeTime = _preferences.getUShort(KEY_P23_CENT_TIME, DEFAULT_CENTRIFUGE_TIME);
  
  // Cargar configuración Programa 24 (Multitanda)
  _program24Config.useHotWater = _preferences.getBool(KEY_P24_HOT_WATER, false);
  _program24Config.numberOfCycles = _preferences.getUChar(KEY_P24_CYCLES, 4);
  
  // Configuración base para Programa 24
  _program24Config.cycleConfigs[0].waterLevel = _preferences.getUChar(KEY_P24_WATER_LEVEL, DEFAULT_WATER_LEVEL);
  _program24Config.cycleConfigs[0].temperature = _program24Config.useHotWater ?
                                               DEFAULT_HOT_TEMPERATURE : DEFAULT_COLD_TEMPERATURE;
  _program24Config.cycleConfigs[0].washTime = _preferences.getUShort(KEY_P24_WASH_TIME, DEFAULT_WASH_TIME);
  _program24Config.cycleConfigs[0].rotationLevel = _preferences.getUChar(KEY_P24_ROTATION, DEFAULT_ROTATION);
  _program24Config.cycleConfigs[0].centrifugeEnabled = _preferences.getBool(KEY_P24_CENTRIFUGE, true);
  _program24Config.cycleConfigs[0].centrifugeLevel = _preferences.getUChar(KEY_P24_CENT_LEVEL, DEFAULT_CENTRIFUGE);
  _program24Config.cycleConfigs[0].centrifugeTime = _preferences.getUShort(KEY_P24_CENT_TIME, DEFAULT_CENTRIFUGE_TIME);
  
  // Cargar configuraciones específicas de cada tanda del Programa 24
  for (int i = 1; i < _program24Config.numberOfCycles; i++) {
    String cyclePrefix = String(KEY_P24_CYCLE_PREFIX) + String(i) + "_";
    
    // Intentar cargar configuración específica de tanda o usar valores predeterminados
    _program24Config.cycleConfigs[i].waterLevel = _preferences.getUChar(
      (cyclePrefix + "wl").c_str(), 
      _program24Config.cycleConfigs[0].waterLevel
    );
    
    _program24Config.cycleConfigs[i].temperature = _preferences.getFloat(
      (cyclePrefix + "temp").c_str(), 
      _program24Config.cycleConfigs[0].temperature
    );
    
    // Cargar resto de parámetros...
  }
  
  Utils.debug("Configuraciones de programas cargadas correctamente");
}

void StorageClass::saveProgramData(uint8_t programId, ProgramConfig config) {
  if (!_validateProgramConfig(config)) {
    Utils.debug("Error: Configuración de programa inválida");
    return;
  }
  
  Utils.debug("Guardando configuración para programa " + String(programId + 22));
  
  // Seleccionar el programa y guardar
  switch (programId) {
    case 0: // Programa 22 (Agua Caliente)
      _program22Config = config;
      _preferences.putUChar(KEY_P22_WATER_LEVEL, config.waterLevel);
      _preferences.putFloat(KEY_P22_TEMP, config.temperature);
      _preferences.putUShort(KEY_P22_WASH_TIME, config.washTime);
      _preferences.putUChar(KEY_P22_ROTATION, config.rotationLevel);
      _preferences.putBool(KEY_P22_CENTRIFUGE, config.centrifugeEnabled);
      _preferences.putUChar(KEY_P22_CENT_LEVEL, config.centrifugeLevel);
      _preferences.putUShort(KEY_P22_CENT_TIME, config.centrifugeTime);
      break;
      
    case 1: // Programa 23 (Agua Fría)
      _program23Config = config;
      _preferences.putUChar(KEY_P23_WATER_LEVEL, config.waterLevel);
      _preferences.putFloat(KEY_P23_TEMP, config.temperature);
      _preferences.putUShort(KEY_P23_WASH_TIME, config.washTime);
      _preferences.putUChar(KEY_P23_ROTATION, config.rotationLevel);
      _preferences.putBool(KEY_P23_CENTRIFUGE, config.centrifugeEnabled);
      _preferences.putUChar(KEY_P23_CENT_LEVEL, config.centrifugeLevel);
      _preferences.putUShort(KEY_P23_CENT_TIME, config.centrifugeTime);
      break;
      
    case 2: // Programa 24 (Multitanda) - Configuración base
      _program24Config.cycleConfigs[0] = config;
      _preferences.putUChar(KEY_P24_WATER_LEVEL, config.waterLevel);
      _preferences.putFloat(KEY_P24_TEMP, config.temperature);
      _preferences.putUShort(KEY_P24_WASH_TIME, config.washTime);
      _preferences.putUChar(KEY_P24_ROTATION, config.rotationLevel);
      _preferences.putBool(KEY_P24_CENTRIFUGE, config.centrifugeEnabled);
      _preferences.putUChar(KEY_P24_CENT_LEVEL, config.centrifugeLevel);
      _preferences.putUShort(KEY_P24_CENT_TIME, config.centrifugeTime);
      break;
  }
}

void StorageClass::saveWaterTypeSelection(bool useHotWater) {
  _program24Config.useHotWater = useHotWater;
  _preferences.putBool(KEY_P24_HOT_WATER, useHotWater);
  
  // Actualizar la temperatura según selección
  float newTemp = useHotWater ? DEFAULT_HOT_TEMPERATURE : DEFAULT_COLD_TEMPERATURE;
  _program24Config.cycleConfigs[0].temperature = newTemp;
  _preferences.putFloat(KEY_P24_TEMP, newTemp);
  
  Utils.debug("Tipo de agua para Programa 24 actualizado: " + 
              String(useHotWater ? "Caliente" : "Fría"));
}

bool StorageClass::getWaterTypeSelection() {
  return _program24Config.useHotWater;
}

float StorageClass::getTemperature(uint8_t programId) {
  switch (programId) {
    case 0: return _program22Config.temperature;
    case 1: return _program23Config.temperature;
    case 2: return _program24Config.cycleConfigs[0].temperature;
    default: return DEFAULT_HOT_TEMPERATURE;
  }
}

bool StorageClass::getCentrifugeEnabled(uint8_t programId) {
  switch (programId) {
    case 0: return _program22Config.centrifugeEnabled;
    case 1: return _program23Config.centrifugeEnabled;
    case 2: return _program24Config.cycleConfigs[0].centrifugeEnabled;
    default: return true;
  }
}

void StorageClass::incrementProgramUsage(uint8_t programId) {
  const char* key = nullptr;
  
  switch (programId) {
    case 0: key = KEY_P22_USAGE; break;
    case 1: key = KEY_P23_USAGE; break;
    case 2: key = KEY_P24_USAGE; break;
    default: return;
  }
  
  uint32_t usageCount = _preferences.getUInt(key, 0);
  usageCount++;
  _preferences.putUInt(key, usageCount);
  
  Utils.debug("Programa " + String(programId + 22) + " ejecutado " + String(usageCount) + " veces");
}

void StorageClass::resetToDefaults() {
  _loadDefaultProgram22();
  _loadDefaultProgram23();
  _loadDefaultProgram24();
  
  Utils.debug("Todos los programas restablecidos a valores predeterminados");
}

void StorageClass::_loadDefaultProgram22() {
  // Valores predeterminados para Programa 22 (Agua Caliente)
  ProgramConfig config;
  config.waterLevel = DEFAULT_WATER_LEVEL;
  config.temperature = DEFAULT_HOT_TEMPERATURE;
  config.washTime = DEFAULT_WASH_TIME;
  config.rotationLevel = DEFAULT_ROTATION;
  config.centrifugeEnabled = true;
  config.centrifugeLevel = DEFAULT_CENTRIFUGE;
  config.centrifugeTime = DEFAULT_CENTRIFUGE_TIME;
  
  saveProgramData(0, config);
}

bool StorageClass::_isTemperatureValid(float temp, uint8_t programId) {
  // Programa 22 (Agua Caliente) - requiere temperatura alta
  if (programId == 0 && temp < MIN_HOT_TEMPERATURE) return false;
  
  // Para todos los programas
  if (temp < MIN_TEMPERATURE || temp > MAX_TEMPERATURE) return false;
  
  return true;
}
```

void StorageClass::_loadProgramData() {
  // Obtener acceso a las matrices de datos de programa
  uint8_t (*nivelAgua)[4] = ProgramController.getNivelAguaData();
  uint8_t (*rotacionTam)[4] = ProgramController.getRotacionData();
  uint8_t (*temperaturaLim)[4] = ProgramController.getTemperaturaData();
  uint8_t (*temporizadorLim)[4] = ProgramController.getTemporizadorData();
  
  // Cargar datos para cada programa y fase
  for (uint8_t i = 0; i < 3; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      nivelAgua[i][j] = EEPROM.read(_addrBaseNiveles + (i * 4) + j);
      temperaturaLim[i][j] = EEPROM.read(_addrBaseTemp + (i * 4) + j);
      temporizadorLim[i][j] = EEPROM.read(_addrBaseTiempos + (i * 4) + j);
      rotacionTam[i][j] = EEPROM.read(_addrBaseRotacion + (i * 4) + j);
    }
  }
}

void StorageClass::_saveProgramData() {
  // Obtener acceso a las matrices de datos de programa
  uint8_t (*nivelAgua)[4] = ProgramController.getNivelAguaData();
  uint8_t (*rotacionTam)[4] = ProgramController.getRotacionData();
  uint8_t (*temperaturaLim)[4] = ProgramController.getTemperaturaData();
  uint8_t (*temporizadorLim)[4] = ProgramController.getTemporizadorData();
  
  // Guardar datos para cada programa y fase
  for (uint8_t i = 0; i < 3; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      EEPROM.write(_addrBaseNiveles + (i * 4) + j, nivelAgua[i][j]);
      EEPROM.write(_addrBaseTemp + (i * 4) + j, temperaturaLim[i][j]);
      EEPROM.write(_addrBaseTiempos + (i * 4) + j, temporizadorLim[i][j]);
      EEPROM.write(_addrBaseRotacion + (i * 4) + j, rotacionTam[i][j]);
    }
  }
}

uint16_t StorageClass::_calculateProgramAddress(uint8_t program, uint8_t phase, uint8_t dataType) {
  // Calcular dirección EEPROM para un parámetro específico
  uint16_t baseAddr;
  
  switch (dataType) {
    case 0: // Nivel de agua
      baseAddr = _addrBaseNiveles;
      break;
    case 1: // Temperatura
      baseAddr = _addrBaseTemp;
      break;
    case 2: // Tiempo
      baseAddr = _addrBaseTiempos;
      break;
    case 3: // Rotación
      baseAddr = _addrBaseRotacion;
      break;
    default:
      return 0; // Error
  }
  
  return baseAddr + ((program - 1) * 4) + (phase - 1);
}
```

## Responsabilidades

El módulo de Almacenamiento tiene las siguientes responsabilidades adaptadas para los tres programas específicos:

1. **Gestión de Configuraciones Específicas**:
   - Programa 22 (Agua Caliente): Almacenar temperatura objetivo, nivel de agua, tiempos y otros parámetros
   - Programa 23 (Agua Fría): Almacenar parámetros sin control activo de temperatura
   - Programa 24 (Multitanda): Almacenar configuración de tipo de agua, número de tandas y parámetros para cada tanda

2. **Validación Especializada**:
   - Verificar rangos válidos de temperatura según el tipo de programa
   - Validar configuraciones de centrifugado
   - Asegurar coherencia en programas multitanda

3. **Persistencia Eficiente**:
   - Utilizar ESP32 Preferences en lugar de EEPROM para mayor eficiencia y menor desgaste
   - Implementar almacenamiento por claves específicas para cada parámetro

4. **Estadísticas de Uso**:
   - Contabilizar la frecuencia de uso de cada programa
   - Almacenar datos históricos para análisis

5. **Configuración Adaptable**:
   - Manejar la selección de agua caliente/fría para Programa 24
   - Ajustar automáticamente parámetros dependientes cuando cambia el tipo de agua

6. **Valores Predeterminados**:
   - Proporcionar configuraciones iniciales optimizadas para cada programa
   - Permitir restaurar a valores de fábrica

## Ventajas de esta Implementación

1. **Estructura Jerárquica Mejorada**: Organización de datos en estructuras específicas para cada programa, facilitando el mantenimiento y extensión.

2. **Persistencia Optimizada**: Uso de ESP32 Preferences en lugar de EEPROM, aprovechando la memoria flash y reduciendo el desgaste.

3. **Validación Contextual**: Reglas de validación específicas para cada programa, garantizando la integridad de los datos.

4. **Abstracción de Bajo Nivel**: Manejo interno de las claves de almacenamiento, simplificando la interfaz para otros módulos.

5. **Gestión Especializada**: Funciones dedicadas para manejar aspectos únicos del Programa 24 (multitanda).

6. **Mayor Robustez**: Verificación de validez en cada parámetro antes de guardar, previniendo configuraciones erróneas.

7. **Información Diagnóstica**: Mensajes de depuración detallados para facilitar la solución de problemas.

## Soporte para Programas Específicos

### Para Programa 22 (Agua Caliente):
- Almacenamiento de temperaturas objetivo más altas (40-80°C)
- Validación específica para asegurar temperaturas adecuadas para agua caliente
- Mantenimiento de configuraciones de vapor y calentamiento

### Para Programa 23 (Agua Fría):
- Almacenamiento de temperaturas informativas (sin control activo)
- Parámetros optimizados para ciclos de agua fría
- Independencia de subsistemas de calentamiento

### Para Programa 24 (Multitanda):
- Sistema flexible para configurar tipo de agua (caliente/fría)
- Estructura jerárquica para almacenar parámetros de múltiples tandas
- Capacidad para configurar cada tanda individualmente o usar configuración común

Al implementar estas características específicas, el módulo de Almacenamiento proporciona la base de datos necesaria para que los tres programas específicos funcionen correctamente y sean configurables según las necesidades del usuario.
