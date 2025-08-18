#ifndef STORAGE_TYPES_H
#define STORAGE_TYPES_H

#include "Arduino.h"

// === TIPOS DE PARÁMETROS UNIFICADOS ===
enum class ParameterType : uint8_t {
  WATER_LEVEL = 0,
  TEMPERATURE = 1,
  TIME = 2,
  ROTATION = 3,
  CENTRIFUGE = 4,
  WATER_TYPE = 5,
  PHASE_TYPE = 6
};

// === IDENTIFICADORES DE PROGRAMA ===
enum class ProgramId : uint8_t {
  P22 = 0,  // Agua Caliente 
  P23 = 1,  // Agua Fría
  P24 = 2   // Multi-ciclo
};

// === ESTRUCTURA PARA COORDENADAS DE ALMACENAMIENTO ===
struct StorageLocation {
  ProgramId program;
  uint8_t phase;     // 0 para programas únicos (P22, P23)
  uint8_t tanda;     // Solo usado en P24 para compatibilidad
  
  StorageLocation(ProgramId prog, uint8_t ph = 0, uint8_t ta = 0) 
    : program(prog), phase(ph), tanda(ta) {}
};

#endif // STORAGE_TYPES_H