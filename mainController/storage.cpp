// storage.cpp
#include "storage.h"
#include "utils.h"             // Utilidades comunes
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
    
    // Validar configuraciones al iniciar
    if (!validateSettings()) {
      resetToDefaults();
    }
  } else {
    // Si falla la inicialización, intentar una vez más
    if (_preferences.begin(STORAGE_NAMESPACE, false)) {
      _initialized = true;
    }
  }
}

uint8_t StorageClass::readByte(const char* key, uint8_t defaultValue) {
  if (!_initialized) return defaultValue;
  return _preferences.getUChar(key, defaultValue);
}

void StorageClass::writeByte(const char* key, uint8_t value) {
  if (!_initialized) return;
  _preferences.putUChar(key, value);
}

uint16_t StorageClass::readWord(const char* key, uint16_t defaultValue) {
  if (!_initialized) return defaultValue;
  return _preferences.getUShort(key, defaultValue);
}

void StorageClass::writeWord(const char* key, uint16_t value) {
  if (!_initialized) return;
  _preferences.putUShort(key, value);
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

void StorageClass::saveWaterLevel(uint8_t program, uint8_t phase, uint8_t level) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  writeByte(_getWaterLevelKey(program, phase), level);
}

uint8_t StorageClass::loadWaterLevel(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  return readByte(_getWaterLevelKey(program, phase), 0);
}

const char* StorageClass::_getTemperatureKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "temp_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

void StorageClass::saveTemperature(uint8_t program, uint8_t phase, uint8_t temperature) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  writeByte(_getTemperatureKey(program, phase), temperature);
}

uint8_t StorageClass::loadTemperature(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  return readByte(_getTemperatureKey(program, phase), 0);
}

const char* StorageClass::_getTimeKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "tiempo_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

void StorageClass::saveTime(uint8_t program, uint8_t phase, uint8_t time) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  writeByte(_getTimeKey(program, phase), time);
}

uint8_t StorageClass::loadTime(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  return readByte(_getTimeKey(program, phase), 0);
}

const char* StorageClass::_getRotationKey(uint8_t program, uint8_t phase) {
  static char key[20]; // Buffer más grande para mayor seguridad
  snprintf(key, sizeof(key), "rotacion_%u_%u", program, phase); // Uso de snprintf más seguro
  return key;
}

void StorageClass::saveRotation(uint8_t program, uint8_t phase, uint8_t rotation) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  writeByte(_getRotationKey(program, phase), rotation);
}

uint8_t StorageClass::loadRotation(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  return readByte(_getRotationKey(program, phase), 0);
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
  
  return (program < NUM_PROGRAMAS && phase < NUM_FASES);
}

void StorageClass::resetToDefaults() {
  // Programa por defecto (cambiado a 1 para mostrar el primer programa)
  saveProgram(0);
  savePhase(0);
  saveTimer(0, 0);
  
  // Reiniciar el contador de uso
  writeWord("contador", 0);
  
  // Valores por defecto para el programa 1
  uint8_t defaultWaterLevels[3][4] = {
    {2, 3, 2, 1}, // Programa 1
    {3, 3, 2, 1}, // Programa 2
    {2, 2, 2, 1}  // Programa 3
  };
  
  uint8_t defaultTemperatures[3][4] = {
    {40, 60, 40, 30}, // Programa 1
    {45, 70, 45, 30}, // Programa 2
    {30, 50, 40, 30}  // Programa 3
  };
  
  uint8_t defaultTimes[3][4] = {
    {5, 10, 5, 3}, // Programa 1
    {6, 12, 6, 3}, // Programa 2
    {4, 8, 4, 3}   // Programa 3
  };
  
  uint8_t defaultRotations[3][4] = {
    {1, 2, 1, 3}, // Programa 1
    {2, 3, 2, 3}, // Programa 2
    {1, 2, 1, 2}  // Programa 3
  };
  
  // Guardar valores por defecto
  for (uint8_t p = 0; p < NUM_PROGRAMAS; p++) {
    for (uint8_t f = 0; f < NUM_FASES; f++) {
      saveWaterLevel(p, f, defaultWaterLevels[p][f]);
      saveTemperature(p, f, defaultTemperatures[p][f]);
      saveTime(p, f, defaultTimes[p][f]);
      saveRotation(p, f, defaultRotations[p][f]);
    }
  }
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
    waterLevels[fase] = loadWaterLevel(program, fase);
    temperatures[fase] = loadTemperature(program, fase);
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
    saveWaterLevel(program, fase, waterLevels[fase]);
    saveTemperature(program, fase, temperatures[fase]);
    saveTime(program, fase, times[fase]);
    saveRotation(program, fase, rotations[fase]);
  }
  
  return true;
}


/**
 * @brief Inicializa valores predeterminados para los programas P22, P23 y P24
 * Solo se ejecuta si no existen valores guardados previamente
 */
void StorageClass::initializeDefaultValues() {
  // Verificar si ya existen valores guardados
  if (loadWaterLevel(0, 0) == 0 && loadTemperature(0, 0) == 0) {
    Utils.debug("🔧 Inicializando valores predeterminados de programas...");
    
    // === PROGRAMA P22 (índice 0) - Lavado Normal ===
    // Fase 0: Prelavado
    saveWaterLevel(0, 0, 2);      // Nivel medio
    saveTemperature(0, 0, 30);    // 30°C
    saveTime(0, 0, 10);           // 10 minutos
    saveRotation(0, 0, 1);        // Rotación lenta
    
    // Fase 1: Lavado principal
    saveWaterLevel(0, 1, 3);      // Nivel alto
    saveTemperature(0, 1, 40);    // 40°C
    saveTime(0, 1, 20);           // 20 minutos
    saveRotation(0, 1, 2);        // Rotación media
    
    // Fase 2: Enjuague
    saveWaterLevel(0, 2, 3);      // Nivel alto
    saveTemperature(0, 2, 30);    // 30°C
    saveTime(0, 2, 15);           // 15 minutos
    saveRotation(0, 2, 2);        // Rotación media
    
    // Fase 3: Centrifugado
    saveWaterLevel(0, 3, 1);      // Nivel bajo
    saveTemperature(0, 3, 20);    // 20°C
    saveTime(0, 3, 8);            // 8 minutos
    saveRotation(0, 3, 3);        // Rotación rápida
    
    // === PROGRAMA P23 (índice 1) - Lavado Delicado ===
    // Fase 0: Prelavado suave
    saveWaterLevel(1, 0, 3);      // Nivel alto
    saveTemperature(1, 0, 20);    // 20°C
    saveTime(1, 0, 8);            // 8 minutos
    saveRotation(1, 0, 1);        // Rotación lenta
    
    // Fase 1: Lavado delicado
    saveWaterLevel(1, 1, 3);      // Nivel alto
    saveTemperature(1, 1, 30);    // 30°C
    saveTime(1, 1, 15);           // 15 minutos
    saveRotation(1, 1, 1);        // Rotación lenta
    
    // Fase 2: Enjuague suave
    saveWaterLevel(1, 2, 3);      // Nivel alto
    saveTemperature(1, 2, 20);    // 20°C
    saveTime(1, 2, 12);           // 12 minutos
    saveRotation(1, 2, 1);        // Rotación lenta
    
    // Fase 3: Centrifugado suave
    saveWaterLevel(1, 3, 2);      // Nivel medio
    saveTemperature(1, 3, 20);    // 20°C
    saveTime(1, 3, 5);            // 5 minutos
    saveRotation(1, 3, 2);        // Rotación media
    
    // === PROGRAMA P24 (índice 2) - Lavado Intensivo ===
    // Fase 0: Prelavado intenso
    saveWaterLevel(2, 0, 3);      // Nivel alto
    saveTemperature(2, 0, 40);    // 40°C
    saveTime(2, 0, 15);           // 15 minutos
    saveRotation(2, 0, 2);        // Rotación media
    
    // Fase 1: Lavado intensivo
    saveWaterLevel(2, 1, 4);      // Nivel muy alto
    saveTemperature(2, 1, 60);    // 60°C
    saveTime(2, 1, 30);           // 30 minutos
    saveRotation(2, 1, 3);        // Rotación rápida
    
    // Fase 2: Enjuague intensivo
    saveWaterLevel(2, 2, 4);      // Nivel muy alto
    saveTemperature(2, 2, 40);    // 40°C
    saveTime(2, 2, 20);           // 20 minutos
    saveRotation(2, 2, 2);        // Rotación media
    
    // Fase 3: Centrifugado intensivo
    saveWaterLevel(2, 3, 1);      // Nivel bajo
    saveTemperature(2, 3, 20);    // 20°C
    saveTime(2, 3, 12);           // 12 minutos
    saveRotation(2, 3, 3);        // Rotación rápida
    
    Utils.debug("✅ Valores predeterminados inicializados correctamente");
  }
}