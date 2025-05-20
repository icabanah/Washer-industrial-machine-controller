// storage.h
#ifndef STORAGE_H
#define STORAGE_H

#include "Arduino.h"
#include "config.h"
#include <Preferences.h>

class StorageClass {
public:
  // Constructor
  StorageClass();
  
  // Inicialización
  void init();
  
  // Operaciones básicas de almacenamiento
  uint8_t readByte(const char* key, uint8_t defaultValue = 0);
  void writeByte(const char* key, uint8_t value);
  uint16_t readWord(const char* key, uint16_t defaultValue = 0);
  void writeWord(const char* key, uint16_t value);
  
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
  
  // Métodos adicionales para integración con módulos
  bool loadAllProgramSettings(uint8_t program, uint8_t (&waterLevels)[NUM_FASES], 
                             uint8_t (&temperatures)[NUM_FASES], 
                             uint8_t (&times)[NUM_FASES], 
                             uint8_t (&rotations)[NUM_FASES]);
  
  bool saveAllProgramSettings(uint8_t program, const uint8_t (&waterLevels)[NUM_FASES], 
                             const uint8_t (&temperatures)[NUM_FASES], 
                             const uint8_t (&times)[NUM_FASES], 
                             const uint8_t (&rotations)[NUM_FASES]);
  
  // Validación de datos
  bool validateSettings();
  void resetToDefaults();
  
  // Contador de uso
  uint16_t loadUsageCounter();
  void incrementUsageCounter();

private:
  // Variables para manejo interno
  bool _initialized;
  Preferences _preferences;
  
  // Métodos internos
  const char* _getWaterLevelKey(uint8_t program, uint8_t phase);
  const char* _getTemperatureKey(uint8_t program, uint8_t phase);
  const char* _getTimeKey(uint8_t program, uint8_t phase);
  const char* _getRotationKey(uint8_t program, uint8_t phase);
};

// Instancia global
extern StorageClass Storage;

#endif // STORAGE_H