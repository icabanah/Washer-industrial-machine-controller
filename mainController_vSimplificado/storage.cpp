// storage.cpp
#include "storage.h"
#include "utils.h"             // Utilidades comunes
#include "debug.h"
#include <string>
#include <stdio.h>  // Para sprintf

// Instancia global
StorageClass Storage;

// Constructor para inicializar variables
StorageClass::StorageClass() : _initialized(false) {}

// Espacio de nombres para la configuración
#define STORAGE_NAMESPACE "washerCfg"

void StorageClass::init() {
  // Inicializar variables internas
  _initialized = false;
  
  // Inicializar el espacio de nombres de Preferences en modo lectura/escritura
  if (_preferences.begin(STORAGE_NAMESPACE, false)) {
    _initialized = true;
    
    // Inicializar valores por defecto si es la primera vez
    initializeDefaultValues();
    
    // Validar configuraciones al iniciar
    if (!validateSettings()) {
      resetToDefaults();
    }
  } else {
    // Si falla la inicialización, intentar una vez más
    if (_preferences.begin(STORAGE_NAMESPACE, false)) {
      _initialized = true;
      // Inicializar valores por defecto si es la primera vez
      initializeDefaultValues();
    }
  }
}

// === IMPLEMENTACIÓN API UNIFICADA ===

uint8_t StorageClass::get(ParameterType paramType, const StorageLocation& location) {
  // Construir clave unificada
  char key[32];
  _buildUnifiedKey(paramType, location, key);
  
  // Obtener valor por defecto apropiado
  uint8_t defaultValue = _getDefaultValue(paramType, location.program);
  
  return readByte(key, defaultValue);
}

void StorageClass::set(ParameterType paramType, const StorageLocation& location, uint8_t value) {
  // Construir clave unificada
  char key[32];
  _buildUnifiedKey(paramType, location, key);
  
  writeByte(key, value);
}

// Métodos de conveniencia
uint8_t StorageClass::get(ParameterType paramType, ProgramId program, uint8_t phase) {
  return get(paramType, StorageLocation(program, phase));
}

void StorageClass::set(ParameterType paramType, ProgramId program, uint8_t phase, uint8_t value) {
  set(paramType, StorageLocation(program, phase), value);
}

void StorageClass::set(ParameterType paramType, ProgramId program, uint8_t value) {
  set(paramType, StorageLocation(program, 0), value);
}

uint8_t StorageClass::readByte(const char* key, uint8_t defaultValue) {
  if (!_initialized) return defaultValue;
  return _preferences.getUChar(key, defaultValue);
}

void StorageClass::writeByte(const char* key, uint8_t value) {
  if (!_initialized) return;
  _preferences.putUChar(key, value);
  // Forzar sincronización inmediata con la flash
  _preferences.end();
  _preferences.begin(STORAGE_NAMESPACE, false);
}

uint16_t StorageClass::readWord(const char* key, uint16_t defaultValue) {
  if (!_initialized) return defaultValue;
  return _preferences.getUShort(key, defaultValue);
}

void StorageClass::writeWord(const char* key, uint16_t value) {
  if (!_initialized) return;
  _preferences.putUShort(key, value);
  // Forzar sincronización inmediata con la flash
  _preferences.end();
  _preferences.begin(STORAGE_NAMESPACE, false);
}

void StorageClass::saveProgram(uint8_t program) {
  writeByte("programa", program);
}

uint8_t StorageClass::loadProgram() {
  uint8_t program = readByte("programa", 0); 
  return (program < NUM_PROGRAMAS) ? program : 0; 
}

void StorageClass::savePhase(uint8_t phase) {
  writeByte("fase", phase);
}

uint8_t StorageClass::loadPhase() {
  uint8_t phase = readByte("fase", 0);
  return (phase < NUM_FASES) ? phase : 0;
}

void StorageClass::saveTimer(uint8_t minutes, uint8_t seconds) {
  writeByte("minutos", minutes);
  writeByte("segundos", seconds);
}

void StorageClass::loadTimer(uint8_t &minutes, uint8_t &seconds) {
  minutes = readByte("minutos", 0);
  seconds = readByte("segundos", 0);
}

const char* StorageClass::_getWaterLevelKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "nivel_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

const char* StorageClass::_getTemperatureKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "temp_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

const char* StorageClass::_getTimeKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "tiempo_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

const char* StorageClass::_getRotationKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "rotacion_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

// === FUNCIONES PARA FASE ===
const char* StorageClass::_getPhaseKey(uint8_t program, uint8_t phase) {
  static char key[20];
  snprintf(key, sizeof(key), "fase_%u_%u", program, phase);
  return key;
}

void StorageClass::savePhaseType(uint8_t program, uint8_t tanda, uint8_t phase, uint8_t phaseType) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  writeByte(_getPhaseKey(program, phase), phaseType);
}

uint8_t StorageClass::loadPhaseType(uint8_t program, uint8_t tanda, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 1; // Default: llenado
  return readByte(_getPhaseKey(program, phase), 1);
}

// === FUNCIONES PARA CENTRIFUGADO ===
const char* StorageClass::_getCentrifugadoKey(uint8_t program, uint8_t phase) {
  static char key[20];
  snprintf(key, sizeof(key), "centrif_%u_%u", program, phase);
  return key;
}

// === FUNCIONES PARA TIPO DE AGUA ===
const char* StorageClass::_getTipoAguaKey(uint8_t program, uint8_t phase) {
  static char key[20];
  snprintf(key, sizeof(key), "agua_%u_%u", program, phase);
  return key;
}

uint16_t StorageClass::loadUsageCounter() {
  return readWord("contador", 0);
}

void StorageClass::incrementUsageCounter() {
  uint16_t counter = loadUsageCounter();
  counter++;
  writeWord("contador", counter);
}

bool StorageClass::validateSettings() {
  // Verificar que los programas y fases estén dentro de los límites
  uint8_t program = readByte("programa", 0);
  uint8_t phase = readByte("fase", 0);
  
  // Validación más permisiva - solo fallar en casos extremos
  bool programValid = program < NUM_PROGRAMAS;
  bool phaseValid = phase < NUM_FASES;
  bool hasDefaults = readByte("defaults_v2", 0) == 1;
  
  // Solo es inválido si NO tiene defaults Y (programa inválido O fase inválida)
  return hasDefaults || (programValid && phaseValid);
}

void StorageClass::resetToDefaults() {
  // Resetear SOLO configuraciones básicas del sistema, NO los datos de programas
  saveProgram(0);
  savePhase(0);
  saveTimer(0, 0);
  writeWord("contador", 0);
  
  // Inicializar valores por defecto SOLO si no existen (preservar datos existentes)
  initializeDefaultValues();
  
}

bool StorageClass::loadAllProgramSettings(uint8_t program, uint8_t (&waterLevels)[NUM_FASES],
                                   uint8_t (&temperatures)[NUM_FASES],
                                   uint8_t (&times)[NUM_FASES],
                                   uint8_t (&rotations)[NUM_FASES]) {
  // Verificar que el programa sea válido
  if (program >= NUM_PROGRAMAS) {
    return false;
  }
  
  // Cargar todas las configuraciones para el programa especificado
  for (uint8_t fase = 0; fase < NUM_FASES; fase++) {
    waterLevels[fase] = loadWaterLevel(program, 0, fase);
    temperatures[fase] = loadTemperature(program, 0, fase);
    times[fase] = loadTime(program, fase);
    rotations[fase] = loadRotation(program, fase);
  }
  
  return true;
}

bool StorageClass::saveAllProgramSettings(uint8_t program, const uint8_t (&waterLevels)[NUM_FASES],
                                   const uint8_t (&temperatures)[NUM_FASES],
                                   const uint8_t (&times)[NUM_FASES],
                                   const uint8_t (&rotations)[NUM_FASES]) {
  // Verificar que el programa sea válido
  if (program >= NUM_PROGRAMAS) {
    return false;
  }
  
  // Guardar todas las configuraciones para el programa especificado
  for (uint8_t fase = 0; fase < NUM_FASES; fase++) {
    saveWaterLevel(program, 0, fase, waterLevels[fase]);
    saveTemperature(program, 0, fase, temperatures[fase]);
    saveTime(program, fase, times[fase]);
    saveRotation(program, fase, rotations[fase]);
  }
  
  return true;
}


/**
 * @brief Inicializa valores predeterminados OPTIMIZADOS para los programas P22, P23 y P24
 * P22 y P23 usan valores únicos, P24 usa matriz por fases
 */
void StorageClass::initializeDefaultValues() {
  // Verificar SOLO si ya se han inicializado los valores por defecto
  if (!readByte("defaults_v2", 0)) {
    
    // === PROGRAMA P22 - AGUA CALIENTE (Valores únicos) ===
    saveP22WaterLevel(2);        // Nivel 2 
    saveP22Temperature(30);      // 30°C (agua caliente)
    saveP22Time(50);             // 50 minutos
    saveP22Rotation(2);          // Rotación 2
    saveP22Centrifugado(1);      // Centrifugado activo
    // Debug.print("✅ P22 configurado: Nivel=2, Temp=30°C, Tiempo=50min, Rot=2, Centrif=Activo");
    
    // === PROGRAMA P23 - AGUA FRÍA (Valores únicos) ===
    saveP23WaterLevel(2);        // Nivel 2
    saveP23Temperature(20);      // 20°C (agua fría)
    saveP23Time(40);             // 40 minutos
    saveP23Rotation(2);          // Rotación 2
    saveP23Centrifugado(1);      // Centrifugado activo
    // Debug.print("✅ P23 configurado: Nivel=2, Temp=20°C, Tiempo=40min, Rot=2, Centrif=Activo");
    
    // === PROGRAMA P24 - MULTI-CICLO (Matriz por fases) ===
    // Fase 0: Prelavado intenso
    saveP24WaterLevel(0, 3);     // Nivel alto
    saveP24Temperature(0, 40);   // 40°C
    saveP24Time(0, 15);          // 15 minutos
    saveP24Rotation(0, 2);       // Rotación media
    saveP24Centrifugado(0, 0);   // Sin centrifugado en prelavado
    saveP24TipoAgua(0, 1);       // Agua caliente
    
    // Fase 1: Lavado intensivo
    saveP24WaterLevel(1, 4);     // Nivel muy alto
    saveP24Temperature(1, 60);   // 60°C
    saveP24Time(1, 30);          // 30 minutos
    saveP24Rotation(1, 3);       // Rotación rápida
    saveP24Centrifugado(1, 0);   // Sin centrifugado en lavado
    saveP24TipoAgua(1, 1);       // Agua caliente
    
    // Fase 2: Enjuague intensivo
    saveP24WaterLevel(2, 4);     // Nivel muy alto
    saveP24Temperature(2, 40);   // 40°C
    saveP24Time(2, 20);          // 20 minutos
    saveP24Rotation(2, 2);       // Rotación media
    saveP24Centrifugado(2, 0);   // Sin centrifugado en enjuague
    saveP24TipoAgua(2, 1);       // Agua caliente
    
    // Fase 3: Centrifugado intensivo
    saveP24WaterLevel(3, 1);     // Nivel bajo
    saveP24Temperature(3, 20);   // 20°C
    saveP24Time(3, 12);          // 12 minutos
    saveP24Rotation(3, 0);       // Sin rotación de lavado en centrifugado
    saveP24Centrifugado(3, 1);   // Centrifugado activo
    saveP24TipoAgua(3, 0);       // Agua fría para enjuague final
    
    
    // Marcar como inicializado (versión 2 = optimizada)
    writeByte("defaults_v2", 1);
    
  } else {
  }
}
//   Serial.println("=====================================");
// }

// ===== IMPLEMENTACIÓN DE MÉTODOS OPTIMIZADOS =====

// === PROGRAMA 22 (P22) - AGUA CALIENTE - VALORES ÚNICOS ===

void StorageClass::saveP22WaterLevel(uint8_t level) {
  writeByte("p22_nivel", level);
}

uint8_t StorageClass::loadP22WaterLevel() {
  return readByte("p22_nivel", 2); // Valor predeterminado: nivel 2
}

void StorageClass::saveP22Temperature(uint8_t temperature) {
  writeByte("p22_temp", temperature);
}

uint8_t StorageClass::loadP22Temperature() {
  return readByte("p22_temp", 30); // Valor predeterminado: 30°C
}

void StorageClass::saveP22Time(uint8_t time) {
  writeByte("p22_tiempo", time);
}

uint8_t StorageClass::loadP22Time() {
  return readByte("p22_tiempo", 50); // Valor predeterminado: 50 minutos
}

void StorageClass::saveP22Rotation(uint8_t rotation) {
  writeByte("p22_rotacion", rotation);
}

uint8_t StorageClass::loadP22Rotation() {
  return readByte("p22_rotacion", 2); // Valor predeterminado: rotación 2
}

void StorageClass::saveP22Centrifugado(uint8_t centrifugado) {
  writeByte("p22_centrif", centrifugado);
}

uint8_t StorageClass::loadP22Centrifugado() {
  return readByte("p22_centrif", 1); // Valor predeterminado: activo
}

// === PROGRAMA 23 (P23) - AGUA FRÍA - VALORES ÚNICOS ===

void StorageClass::saveP23WaterLevel(uint8_t level) {
  writeByte("p23_nivel", level);
}

uint8_t StorageClass::loadP23WaterLevel() {
  return readByte("p23_nivel", 2); // Valor predeterminado: nivel 2
}

void StorageClass::saveP23Temperature(uint8_t temperature) {
  writeByte("p23_temp", temperature);
}

uint8_t StorageClass::loadP23Temperature() {
  return readByte("p23_temp", 20); // Valor predeterminado: 20°C (agua fría)
}

void StorageClass::saveP23Time(uint8_t time) {
  writeByte("p23_tiempo", time);
}

uint8_t StorageClass::loadP23Time() {
  return readByte("p23_tiempo", 40); // Valor predeterminado: 40 minutos
}

void StorageClass::saveP23Rotation(uint8_t rotation) {
  writeByte("p23_rotacion", rotation);
}

uint8_t StorageClass::loadP23Rotation() {
  return readByte("p23_rotacion", 2); // Valor predeterminado: rotación 2
}

void StorageClass::saveP23Centrifugado(uint8_t centrifugado) {
  writeByte("p23_centrif", centrifugado);
}

uint8_t StorageClass::loadP23Centrifugado() {
  return readByte("p23_centrif", 1); // Valor predeterminado: activo
}

// === PROGRAMA 24 (P24) - MULTI-CICLO - MATRIZ POR FASES ===

void StorageClass::saveP24WaterLevel(uint8_t phase, uint8_t level) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("nivel", phase), level);
}

uint8_t StorageClass::loadP24WaterLevel(uint8_t phase) {
  if (phase >= NUM_FASES) return 2;
  return readByte(_getP24Key("nivel", phase), 2);
}

void StorageClass::saveP24Temperature(uint8_t phase, uint8_t temperature) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("temp", phase), temperature);
}

uint8_t StorageClass::loadP24Temperature(uint8_t phase) {
  if (phase >= NUM_FASES) return 25;
  return readByte(_getP24Key("temp", phase), 25);
}

void StorageClass::saveP24Time(uint8_t phase, uint8_t time) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("tiempo", phase), time);
}

uint8_t StorageClass::loadP24Time(uint8_t phase) {
  if (phase >= NUM_FASES) return 30;
  return readByte(_getP24Key("tiempo", phase), 30);
}

void StorageClass::saveP24Rotation(uint8_t phase, uint8_t rotation) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("rotacion", phase), rotation);
}

uint8_t StorageClass::loadP24Rotation(uint8_t phase) {
  if (phase >= NUM_FASES) return 2;
  return readByte(_getP24Key("rotacion", phase), 2);
}

void StorageClass::saveP24Centrifugado(uint8_t phase, uint8_t centrifugado) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("centrif", phase), centrifugado);
}

uint8_t StorageClass::loadP24Centrifugado(uint8_t phase) {
  if (phase >= NUM_FASES) return 1;
  return readByte(_getP24Key("centrif", phase), 1);
}

void StorageClass::saveP24TipoAgua(uint8_t phase, uint8_t tipoAgua) {
  if (phase >= NUM_FASES) return;
  writeByte(_getP24Key("agua", phase), tipoAgua);
}

uint8_t StorageClass::loadP24TipoAgua(uint8_t phase) {
  if (phase >= NUM_FASES) return 0;
  return readByte(_getP24Key("agua", phase), 0);
}

// === MÉTODO AUXILIAR PARA P24 ===

const char* StorageClass::_getP24Key(const char* param, uint8_t phase) {
  static char key[16];
  snprintf(key, sizeof(key), "p24_%s_%d", param, phase);
  return key;
}

// === MÉTODOS DE COMPATIBILIDAD (para transición gradual) ===

void StorageClass::saveWaterLevel(uint8_t program, uint8_t tanda, uint8_t phase, uint8_t level) {
  switch (program) {
    case 0: // P22
      saveP22WaterLevel(level); // Ignora la fase, P22 tiene un solo valor
      break;
    case 1: // P23  
      saveP23WaterLevel(level); // Ignora la fase, P23 tiene un solo valor
      break;
    case 2: // P24
      saveP24WaterLevel(phase, level);
      break;
  }
}

uint8_t StorageClass::loadWaterLevel(uint8_t program, uint8_t tanda, uint8_t phase) {
  switch (program) {
    case 0: // P22
      return loadP22WaterLevel(); // Ignora la tanda y la fase
    case 1: // P23
      return loadP23WaterLevel(); // Ignora la tanda y la fase
    case 2: // P24
      return loadP24WaterLevel(phase);
    default:
      return 2; // Valor predeterminado
  }
}

void StorageClass::saveTemperature(uint8_t program, uint8_t tanda, uint8_t phase, uint8_t temperature) {
  switch (program) {
    case 0: // P22
      saveP22Temperature(temperature);
      break;
    case 1: // P23
      saveP23Temperature(temperature);
      break;
    case 2: // P24
      saveP24Temperature(phase, temperature);
      break;
  }
}

uint8_t StorageClass::loadTemperature(uint8_t program, uint8_t tanda, uint8_t phase) {
  switch (program) {
    case 0: // P22
      return loadP22Temperature();
    case 1: // P23
      return loadP23Temperature();
    case 2: // P24
      return loadP24Temperature(phase);
    default:
      return 25;
  }
}

void StorageClass::saveTime(uint8_t program, uint8_t phase, uint8_t time) {
  switch (program) {
    case 0: // P22
      saveP22Time(time);
      break;
    case 1: // P23
      saveP23Time(time);
      break;
    case 2: // P24
      saveP24Time(phase, time);
      break;
  }
}

uint8_t StorageClass::loadTime(uint8_t program, uint8_t phase) {
  switch (program) {
    case 0: // P22
      return loadP22Time();
    case 1: // P23
      return loadP23Time();
    case 2: // P24
      return loadP24Time(phase);
    default:
      return 30;
  }
}

void StorageClass::saveRotation(uint8_t program, uint8_t phase, uint8_t rotation) {
  switch (program) {
    case 0: // P22
      saveP22Rotation(rotation);
      break;
    case 1: // P23
      saveP23Rotation(rotation);
      break;
    case 2: // P24
      saveP24Rotation(phase, rotation);
      break;
  }
}

uint8_t StorageClass::loadRotation(uint8_t program, uint8_t phase) {
  switch (program) {
    case 0: // P22
      return loadP22Rotation();
    case 1: // P23
      return loadP23Rotation();
    case 2: // P24
      return loadP24Rotation(phase);
    default:
      return 2;
  }
}

void StorageClass::saveCentrifugado(uint8_t program, uint8_t tanda, uint8_t centrifugado) {
  switch (program) {
    case 0: // P22 - solo 1 tanda
      saveP22Centrifugado(centrifugado);
      break;
    case 1: // P23 - solo 1 tanda
      saveP23Centrifugado(centrifugado);
      break;
    case 2: // P24 - 4 tandas (0, 1, 2, 3)
      saveP24Centrifugado(tanda, centrifugado);
      break;
  }
}

uint8_t StorageClass::loadCentrifugado(uint8_t program, uint8_t tanda) {
  switch (program) {
    case 0: // P22 - solo 1 tanda
      return loadP22Centrifugado();
    case 1: // P23 - solo 1 tanda
      return loadP23Centrifugado();
    case 2: // P24 - 4 tandas (0, 1, 2, 3)
      return loadP24Centrifugado(tanda);
    default:
      return 1;
  }
}

void StorageClass::saveTipoAgua(uint8_t program, uint8_t tanda, uint8_t phase, uint8_t tipoAgua) {
  switch (program) {
    case 0: // P22
      // P22 siempre usa agua caliente (1), ignorar parámetro
      break;
    case 1: // P23
      // P23 siempre usa agua fría (0), ignorar parámetro
      break;
    case 2: // P24
      saveP24TipoAgua(phase, tipoAgua);
      break;
  }
}

uint8_t StorageClass::loadTipoAgua(uint8_t program, uint8_t tanda, uint8_t phase) {
  switch (program) {
    case 0: // P22
      return 1; // Siempre agua caliente
    case 1: // P23
      return 0; // Siempre agua fría
    case 2: // P24
      return loadP24TipoAgua(phase);
    default:
      return 0;
  }
}

// === IMPLEMENTACIÓN MÉTODOS PARA API UNIFICADA ===

void StorageClass::_buildUnifiedKey(ParameterType paramType, const StorageLocation& location, char* key) {
  // Crear clave unificada: "P{program}_{paramName}_{phase}"
  const char* paramName = "";
  
  switch (paramType) {
    case ParameterType::WATER_LEVEL:  paramName = "WL"; break;
    case ParameterType::TEMPERATURE:  paramName = "TP"; break;
    case ParameterType::TIME:         paramName = "TM"; break;
    case ParameterType::ROTATION:     paramName = "RT"; break;
    case ParameterType::CENTRIFUGE:   paramName = "CF"; break;
    case ParameterType::WATER_TYPE:   paramName = "WT"; break;
    case ParameterType::PHASE_TYPE:   paramName = "PT"; break;
  }
  
  uint8_t programNum = (uint8_t)location.program + 22; // P22, P23, P24
  
  if (location.program == ProgramId::P24 && location.phase > 0) {
    // P24 con fases específicas
    sprintf(key, "P%d_%s_%d", programNum, paramName, location.phase);
  } else {
    // P22, P23 o P24 valores generales
    sprintf(key, "P%d_%s", programNum, paramName);
  }
}

uint8_t StorageClass::_getDefaultValue(ParameterType paramType, ProgramId program) {
  switch (paramType) {
    case ParameterType::WATER_LEVEL:
      return (program == ProgramId::P24) ? 2 : 3;
      
    case ParameterType::TEMPERATURE:
      switch (program) {
        case ProgramId::P22: return 65;  // Agua caliente
        case ProgramId::P23: return 25;  // Agua fría
        case ProgramId::P24: return 50;  // Templada
      }
      
    case ParameterType::TIME:
      return (program == ProgramId::P24) ? 12 : 15;
      
    case ParameterType::ROTATION:
      return 2; // Rotación media para todos
      
    case ParameterType::CENTRIFUGE:
      return 1; // Activo por defecto
      
    case ParameterType::WATER_TYPE:
      return (program == ProgramId::P22) ? 1 : 0; // Caliente/Fría
      
    case ParameterType::PHASE_TYPE:
      return 1; // Fase estándar
  }
  
  return 0; // Valor por defecto seguro
}
