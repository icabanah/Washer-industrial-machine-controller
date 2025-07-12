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
  void initializeDefaultValues(); // Inicializar valores predeterminados de programas
  
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
  
  // === CONFIGURACIONES OPTIMIZADAS POR PROGRAMA ===
  
  // Programa 22 (P22) - Agua Caliente - Valores únicos
  void saveP22WaterLevel(uint8_t level);
  uint8_t loadP22WaterLevel();
  void saveP22Temperature(uint8_t temperature);
  uint8_t loadP22Temperature();
  void saveP22Time(uint8_t time);
  uint8_t loadP22Time();
  void saveP22Rotation(uint8_t rotation);
  uint8_t loadP22Rotation();
  void saveP22Centrifugado(uint8_t centrifugado);
  uint8_t loadP22Centrifugado();
  
  // Programa 23 (P23) - Agua Fría - Valores únicos
  void saveP23WaterLevel(uint8_t level);
  uint8_t loadP23WaterLevel();
  void saveP23Temperature(uint8_t temperature);
  uint8_t loadP23Temperature();
  void saveP23Time(uint8_t time);
  uint8_t loadP23Time();
  void saveP23Rotation(uint8_t rotation);
  uint8_t loadP23Rotation();
  void saveP23Centrifugado(uint8_t centrifugado);
  uint8_t loadP23Centrifugado();
  
  // Programa 24 (P24) - Multi-ciclo - Matriz por fases
  void saveP24WaterLevel(uint8_t phase, uint8_t level);
  uint8_t loadP24WaterLevel(uint8_t phase);
  void saveP24Temperature(uint8_t phase, uint8_t temperature);
  uint8_t loadP24Temperature(uint8_t phase);
  void saveP24Time(uint8_t phase, uint8_t time);
  uint8_t loadP24Time(uint8_t phase);
  void saveP24Rotation(uint8_t phase, uint8_t rotation);
  uint8_t loadP24Rotation(uint8_t phase);
  void saveP24Centrifugado(uint8_t phase, uint8_t centrifugado);
  uint8_t loadP24Centrifugado(uint8_t phase);
  void saveP24TipoAgua(uint8_t phase, uint8_t tipoAgua);
  uint8_t loadP24TipoAgua(uint8_t phase);
  
  // === MÉTODOS DE COMPATIBILIDAD (para transición gradual) ===
  void saveWaterLevel(uint8_t program, uint8_t phase, uint8_t level);
  uint8_t loadWaterLevel(uint8_t program, uint8_t tanda, uint8_t phase);
  void saveTemperature(uint8_t program, uint8_t phase, uint8_t temperature);
  uint8_t loadTemperature(uint8_t program, uint8_t tanda, uint8_t phase);
  void saveTime(uint8_t program, uint8_t phase, uint8_t time);
  uint8_t loadTime(uint8_t program, uint8_t phase);
  void saveRotation(uint8_t program, uint8_t phase, uint8_t rotation);
  uint8_t loadRotation(uint8_t program, uint8_t phase);
  void saveCentrifugado(uint8_t program, uint8_t tanda, uint8_t centrifugado);
  uint8_t loadCentrifugado(uint8_t program, uint8_t tanda);
  void saveTipoAgua(uint8_t program, uint8_t phase, uint8_t tipoAgua);
  uint8_t loadTipoAgua(uint8_t program, uint8_t phase);
  void savePhaseType(uint8_t program, uint8_t phase, uint8_t phaseType);
  uint8_t loadPhaseType(uint8_t program, uint8_t phase);
  
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
  
  // // Depuración
  // void debugPrintAllPrograms();

private:
  // Variables para manejo interno
  bool _initialized;
  Preferences _preferences;
  
  // Métodos internos optimizados
  const char* _getP24Key(const char* param, uint8_t phase);
  
  // Métodos de compatibilidad (deprecated)
  const char* _getWaterLevelKey(uint8_t program, uint8_t phase);
  const char* _getTemperatureKey(uint8_t program, uint8_t phase);
  const char* _getTimeKey(uint8_t program, uint8_t phase);
  const char* _getRotationKey(uint8_t program, uint8_t phase);
  const char* _getPhaseKey(uint8_t program, uint8_t phase);
  const char* _getCentrifugadoKey(uint8_t program, uint8_t phase);
  const char* _getTipoAguaKey(uint8_t program, uint8_t phase);
};

// Instancia global
extern StorageClass Storage;

#endif // STORAGE_H