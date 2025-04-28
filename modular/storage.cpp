// storage.cpp
#include "storage.h"

// Instancia global
StorageClass Storage;

void StorageClass::init() {
  EEPROM.begin(512); // Inicializar EEPROM con 512 bytes
  _initialized = true;
  
  // Validar configuraciones al iniciar
  if (!validateSettings()) {
    resetToDefaults();
  }
}

uint8_t StorageClass::readByte(int address) {
  if (!_initialized) return 0;
  return EEPROM.read(address);
}

void StorageClass::writeByte(int address, uint8_t value) {
  if (!_initialized) return;
  EEPROM.write(address, value);
  _commitChanges();
}

uint16_t StorageClass::readWord(int address) {
  if (!_initialized) return 0;
  uint16_t value = EEPROM.read(address);
  value |= (EEPROM.read(address + 1) << 8);
  return value;
}

void StorageClass::writeWord(int address, uint16_t value) {
  if (!_initialized) return;
  EEPROM.write(address, value & 0xFF);
  EEPROM.write(address + 1, (value >> 8) & 0xFF);
  _commitChanges();
}

void StorageClass::_commitChanges() {
  EEPROM.commit();
}

void StorageClass::saveProgram(uint8_t program) {
  writeByte(EEPROM_ADDR_PROGRAMA, program);
}

uint8_t StorageClass::loadProgram() {
  uint8_t program = readByte(EEPROM_ADDR_PROGRAMA);
  return (program < NUM_PROGRAMAS) ? program : 0;
}

void StorageClass::savePhase(uint8_t phase) {
  writeByte(EEPROM_ADDR_FASE, phase);
}

uint8_t StorageClass::loadPhase() {
  uint8_t phase = readByte(EEPROM_ADDR_FASE);
  return (phase < NUM_FASES) ? phase : 0;
}

void StorageClass::saveTimer(uint8_t minutes, uint8_t seconds) {
  writeByte(EEPROM_ADDR_MINUTOS, minutes);
  writeByte(EEPROM_ADDR_SEGUNDOS, seconds);
}

void StorageClass::loadTimer(uint8_t &minutes, uint8_t &seconds) {
  minutes = readByte(EEPROM_ADDR_MINUTOS);
  seconds = readByte(EEPROM_ADDR_SEGUNDOS);
}

int StorageClass::_getWaterLevelAddress(uint8_t program, uint8_t phase) {
  return EEPROM_ADDR_BASE_NIVELES + (program * NUM_FASES) + phase;
}

void StorageClass::saveWaterLevel(uint8_t program, uint8_t phase, uint8_t level) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  int address = _getWaterLevelAddress(program, phase);
  writeByte(address, level);
}

uint8_t StorageClass::loadWaterLevel(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  int address = _getWaterLevelAddress(program, phase);
  return readByte(address);
}

int StorageClass::_getTemperatureAddress(uint8_t program, uint8_t phase) {
  return EEPROM_ADDR_BASE_TEMP + (program * NUM_FASES) + phase;
}

void StorageClass::saveTemperature(uint8_t program, uint8_t phase, uint8_t temperature) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  int address = _getTemperatureAddress(program, phase);
  writeByte(address, temperature);
}

uint8_t StorageClass::loadTemperature(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  int address = _getTemperatureAddress(program, phase);
  return readByte(address);
}

int StorageClass::_getTimeAddress(uint8_t program, uint8_t phase) {
  return EEPROM_ADDR_BASE_TIEMPOS + (program * NUM_FASES) + phase;
}

void StorageClass::saveTime(uint8_t program, uint8_t phase, uint8_t time) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  int address = _getTimeAddress(program, phase);
  writeByte(address, time);
}

uint8_t StorageClass::loadTime(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  int address = _getTimeAddress(program, phase);
  return readByte(address);
}

int StorageClass::_getRotationAddress(uint8_t program, uint8_t phase) {
  return EEPROM_ADDR_BASE_ROTACION + (program * NUM_FASES) + phase;
}

void StorageClass::saveRotation(uint8_t program, uint8_t phase, uint8_t rotation) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return;
  int address = _getRotationAddress(program, phase);
  writeByte(address, rotation);
}

uint8_t StorageClass::loadRotation(uint8_t program, uint8_t phase) {
  if (program >= NUM_PROGRAMAS || phase >= NUM_FASES) return 0;
  int address = _getRotationAddress(program, phase);
  return readByte(address);
}

uint16_t StorageClass::loadUsageCounter() {
  return readWord(EEPROM_ADDR_CONTADOR_L);
}

void StorageClass::incrementUsageCounter() {
  uint16_t counter = loadUsageCounter();
  counter++;
  writeWord(EEPROM_ADDR_CONTADOR_L, counter);
}

bool StorageClass::validateSettings() {
  // Verificar que los programas y fases estén dentro de los límites
  uint8_t program = readByte(EEPROM_ADDR_PROGRAMA);
  uint8_t phase = readByte(EEPROM_ADDR_FASE);
  
  return (program < NUM_PROGRAMAS && phase < NUM_FASES);
}

void StorageClass::resetToDefaults() {
  // Programa por defecto
  saveProgram(0);
  savePhase(0);
  saveTimer(0, 0);
  
  // Valores por defecto para el programa 1
  uint8_t defaultWaterLevels[3][4] = {
    {2, 3, 2, 1}, // Programa 0
    {3, 3, 2, 1}, // Programa 1
    {2, 2, 2, 1}  // Programa 2
  };
  
  uint8_t defaultTemperatures[3][4] = {
    {40, 60, 40, 30}, // Programa 0
    {45, 70, 45, 30}, // Programa 1
    {30, 50, 40, 30}  // Programa 2
  };
  
  uint8_t defaultTimes[3][4] = {
    {5, 10, 5, 3}, // Programa 0
    {6, 12, 6, 3}, // Programa 1
    {4, 8, 4, 3}   // Programa 2
  };
  
  uint8_t defaultRotations[3][4] = {
    {1, 2, 1, 3}, // Programa 0
    {2, 3, 2, 3}, // Programa 1
    {1, 2, 1, 2}  // Programa 2
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
