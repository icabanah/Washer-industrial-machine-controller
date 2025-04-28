# Módulo de Almacenamiento (storage.h / storage.cpp)

## Descripción

El módulo de Almacenamiento gestiona la persistencia de datos del sistema, principalmente a través de la memoria EEPROM. Se encarga de guardar y recuperar las configuraciones de los programas, el estado del sistema y otros parámetros importantes que deben conservarse incluso cuando el dispositivo está apagado.

## Analogía: Sistema de Memoria a Largo Plazo

Este módulo funciona como la memoria a largo plazo de un cerebro, que almacena información importante que debe recordarse incluso después de descansar o dormir. Al igual que los recuerdos persisten en nuestro cerebro y pueden ser recuperados cuando los necesitamos, los datos almacenados en la EEPROM persisten después de apagar el sistema y pueden ser recuperados cuando se reinicia.

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
#include "program_controller.h"

// Definición de la instancia global
StorageClass Storage;

void StorageClass::init() {
  // Inicialización si es necesaria
}

void StorageClass::loadFromEEPROM() {
  // Cargar valores básicos del sistema
  uint8_t program = EEPROM.read(_addrProgram);
  uint8_t phase = EEPROM.read(_addrPhase);
  uint8_t minutes = EEPROM.read(_addrMinutes);
  uint8_t seconds = EEPROM.read(_addrSeconds);
  
  // Cargar contador de bloqueo
  byte highByte = EEPROM.read(_addrCounterHigh);
  byte lowByte = EEPROM.read(_addrCounterLow);
  uint16_t blockCounter = (highByte << 8) | lowByte;
  
  // Establecer valores en el controlador de programas
  ProgramController.selectProgram(program);
  ProgramController.setPhase(phase);
  
  // Cargar datos específicos de los programas
  _loadProgramData();
}

void StorageClass::saveToEEPROM() {
  // Guardar valores básicos del sistema
  EEPROM.write(_addrProgram, ProgramController.getCurrentProgram());
  EEPROM.write(_addrPhase, ProgramController.getCurrentPhase());
  
  // Guardar contador de bloqueo
  uint16_t blockCounter = 0; // Obtener de alguna forma
  byte lowByte = (blockCounter & 0x00FF);
  byte highByte = ((blockCounter & 0xFF00) >> 8);
  EEPROM.write(_addrCounterHigh, highByte);
  EEPROM.write(_addrCounterLow, lowByte);
  
  // Guardar datos específicos de los programas
  _saveProgramData();
}

void StorageClass::saveProgram(uint8_t program) {
  EEPROM.write(_addrProgram, program);
}

void StorageClass::savePhase(uint8_t phase) {
  EEPROM.write(_addrPhase, phase);
}

void StorageClass::saveBlockCounter(uint16_t counter) {
  byte lowByte = (counter & 0x00FF);
  byte highByte = ((counter & 0xFF00) >> 8);
  EEPROM.write(_addrCounterHigh, highByte);
  EEPROM.write(_addrCounterLow, lowByte);
}

void StorageClass::saveTimers(uint8_t minutes, uint8_t seconds) {
  EEPROM.write(_addrMinutes, minutes);
  EEPROM.write(_addrSeconds, seconds);
}

uint8_t StorageClass::getProgramNumber() {
  return EEPROM.read(_addrProgram);
}

uint8_t StorageClass::getPhaseNumber() {
  return EEPROM.read(_addrPhase);
}

uint16_t StorageClass::getBlockCounter() {
  byte highByte = EEPROM.read(_addrCounterHigh);
  byte lowByte = EEPROM.read(_addrCounterLow);
  return (highByte << 8) | lowByte;
}

bool StorageClass::isEEPROMValid() {
  // Verificar si los datos en la EEPROM son válidos
  uint8_t program = EEPROM.read(_addrProgram);
  uint8_t phase = EEPROM.read(_addrPhase);
  
  // Verificar valores básicos
  if (program < 1 || program > 3) return false;
  if (phase < 1 || phase > 4) return false;
  
  return true;
}

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

El módulo de Almacenamiento tiene las siguientes responsabilidades:

1. **Carga Inicial**: Recuperar los datos guardados en la EEPROM al iniciar el sistema.
2. **Persistencia**: Guardar los cambios en la configuración cuando se solicite.
3. **Validación**: Verificar que los datos almacenados sean válidos y coherentes.
4. **Abstracción de Direcciones**: Gestionar las direcciones EEPROM para que otros módulos no necesiten conocerlas.
5. **Gestión de Memoria**: Optimizar el uso del espacio EEPROM y evitar escrituras innecesarias.
6. **Seguridad**: Proteger los datos críticos, como el contador de bloqueo.

## Ventajas de este Enfoque

1. **Centralización**: Todo el código relacionado con la persistencia está en un solo módulo.
2. **Abstracción**: Los detalles específicos de la EEPROM están ocultos para otros módulos.
3. **Mantenibilidad**: Facilita cambiar el método de almacenamiento sin afectar al resto del sistema.
4. **Protección**: Reduce el riesgo de corrupción de datos al centralizar las operaciones de escritura.
5. **Optimización**: Permite implementar estrategias para reducir el desgaste de la EEPROM.
6. **Transparencia**: Otros módulos pueden usar datos persistentes sin preocuparse por cómo se almacenan.

Al separar la gestión de almacenamiento en un módulo dedicado, se mejora la robustez del sistema y se facilita la implementación de futuras mejoras en la persistencia de datos, como cambiar a diferentes tipos de memoria o implementar sistemas de copia de seguridad.
