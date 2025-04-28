// storage.h
#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"
#include <EEPROM.h>

class StorageClass {
public:
  // Inicialización
  void init();
  
  // Operaciones básicas de EEPROM
  uint8_t readByte(int address);
  void writeByte(int address, uint8_t value);
  uint16_t readWord(int address);
  void writeWord(int address, uint16_t value);
  
  // Gestión de programas y configuraciones
  void saveProgram(uint8_t program);
  uint8_t loadProgram();
  void savePhase(uint8_t phase);
  uint8_t loadPhase();
  void saveTimer(uint8_t minutes, uint8_t seconds);
  void loadTimer(uint8_t &minutes, uint8_t &seconds);
  
  // Configuraciones específicas para programas
  void saveWaterLevel(uint8_t program, uint8_t phase, uint8_t level);
  uint8_t loadWaterLevel(uint8_t program, uint8_t phase);
  void saveTemperature(uint8_t program, uint8_t phase, uint8_t temperature);
  uint8_t loadTemperature(uint8_t program, uint8_t phase);
  void saveTime(uint8_t program, uint8_t phase, uint8_t time);
  uint8_t loadTime(uint8_t program, uint8_t phase);
  void saveRotation(uint8_t program, uint8_t phase, uint8_t rotation);
  uint8_t loadRotation(uint8_t program, uint8_t phase);
  
  // Validación de datos
  bool validateSettings();
  void resetToDefaults();
  
  // Contador de uso
  uint16_t loadUsageCounter();
  void incrementUsageCounter();

private:
  // Variables para manejo interno
  bool _initialized;
  
  // Métodos internos
  void _commitChanges();
  int _getWaterLevelAddress(uint8_t program, uint8_t phase);
  int _getTemperatureAddress(uint8_t program, uint8_t phase);
  int _getTimeAddress(uint8_t program, uint8_t phase);
  int _getRotationAddress(uint8_t program, uint8_t phase);
};

// Instancia global
extern StorageClass Storage;

#endif // STORAGE_H