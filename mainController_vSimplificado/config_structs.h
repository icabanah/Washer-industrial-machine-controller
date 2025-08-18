#ifndef CONFIG_STRUCTS_H
#define CONFIG_STRUCTS_H

#include "Arduino.h"

// === ESTRUCTURAS ORGANIZADAS PARA CONFIGURACIÓN ===

// 📌 CONFIGURACIÓN DE PINES HARDWARE
struct HardwarePins {
  // Entrada de emergencia
  static constexpr uint8_t EMERGENCY_BUTTON = 15;
  
  // Salidas - Motor
  static constexpr uint8_t MOTOR_DIR_LEFT = 12;
  static constexpr uint8_t MOTOR_DIR_RIGHT = 13;
  static constexpr uint8_t CENTRIFUGE = 14;
  
  // Salidas - Válvulas  
  static constexpr uint8_t WATER_VALVE = 27;
  static constexpr uint8_t STEAM_VALVE = 33;
  static constexpr uint8_t DRAIN_VALVE = 25;
  static constexpr uint8_t DOOR_MAGNET = 26;
  
  // Comunicación Nextion
  static constexpr uint8_t NEXTION_RX = 16;
  static constexpr uint8_t NEXTION_TX = 17;
  
  // Sensores
  static constexpr uint8_t PRESSURE_DOUT = 5;
  static constexpr uint8_t PRESSURE_SCLK = 4;
  static constexpr uint8_t TEMPERATURE = 23;
};

// 🌡️ CONFIGURACIÓN DE SENSORES
struct SensorConfig {
  // Temperatura
  static constexpr uint8_t TEMP_RESOLUTION = 9;
  static constexpr uint8_t TEMP_RANGE = 2;
  static inline const uint8_t TEMP_SENSOR_ADDR[8] = {0x28, 0xFF, 0x64, 0x1E, 0x0C, 0x31, 0x18, 0x66};
  
  // Presión (niveles de agua)
  static constexpr uint16_t PRESSURE_LEVEL_1 = 601;
  static constexpr uint16_t PRESSURE_LEVEL_2 = 628;
  static constexpr uint16_t PRESSURE_LEVEL_3 = 645;
  static constexpr uint16_t PRESSURE_LEVEL_4 = 663;
};

// ⚙️ CONFIGURACIÓN DE MOTOR
struct MotorConfig {
  // Nivel 1 - Rotación Suave
  struct Level1 {
    static constexpr uint8_t TIME_RIGHT = 5;
    static constexpr uint8_t TIME_LEFT = 5;
    static constexpr uint8_t TIME_PAUSE = 2;
  };
  
  // Nivel 2 - Rotación Media
  struct Level2 {
    static constexpr uint8_t TIME_RIGHT = 8;
    static constexpr uint8_t TIME_LEFT = 8;
    static constexpr uint8_t TIME_PAUSE = 2;
  };
  
  // Nivel 3 - Rotación Intensa
  struct Level3 {
    static constexpr uint8_t TIME_RIGHT = 10;
    static constexpr uint8_t TIME_LEFT = 10;
    static constexpr uint8_t TIME_PAUSE = 1;
  };
  
  static constexpr uint8_t MAX_ROTATION_LEVEL = 3;
  static constexpr uint16_t COMPATIBILITY_TIME_ON = 5000; // ms
  static constexpr uint16_t COMPATIBILITY_TIME_OFF = 2000; // ms
};

// 📐 LÍMITES DE PARÁMETROS
struct ParameterLimits {
  // Nivel de agua
  static constexpr uint8_t MIN_LEVEL = 1;
  static constexpr uint8_t MAX_LEVEL = 4;
  
  // Temperatura
  static constexpr uint8_t MIN_TEMPERATURE = 5;
  static constexpr uint8_t MAX_TEMPERATURE = 100;
  static constexpr uint8_t EMERGENCY_TEMPERATURE = 95;
  
  // Tiempo
  static constexpr uint8_t MIN_TIME = 1;
  static constexpr uint8_t MAX_TIME = 60;
  
  // Rotación
  static constexpr uint8_t MIN_ROTATION = 1;
  static constexpr uint8_t MAX_ROTATION = 3;
  
  // Fase
  static constexpr uint8_t MIN_PHASE = 1;
  static constexpr uint8_t MAX_PHASE = 4;
  
  // Centrifugado
  static constexpr uint8_t MIN_CENTRIFUGE = 0;
  static constexpr uint8_t MAX_CENTRIFUGE = 1;
  
  // Tipo de agua
  static constexpr uint8_t MIN_WATER_TYPE = 0;
  static constexpr uint8_t MAX_WATER_TYPE = 1;
};

// ⏱️ TIEMPOS DEL SISTEMA
struct SystemTiming {
  static constexpr uint16_t WELCOME_SCREEN = 3000;        // ms
  static constexpr uint16_t TIMER_INTERVAL = 1000;       // ms
  static constexpr uint16_t NEXTION_TIMEOUT = 5;         // ms
  static constexpr uint16_t DRAIN_TIME = 45;             // segundos
  static constexpr uint16_t DOOR_LOCK_TIME = 60;         // segundos
  static constexpr uint16_t CENTRIFUGE_TIME = 45;        // segundos
  static constexpr uint8_t NEXTION_END_CMD[3] = {0xFF, 0xFF, 0xFF};
};

// 🔧 CONFIGURACIÓN DE PROGRAMAS
struct ProgramDefaults {
  // Temperaturas por defecto [P22, P23, P24]
  static constexpr uint8_t TEMPERATURES[3] = {65, 25, 50};
  
  // Tiempos por defecto [P22, P23, P24] 
  static constexpr uint8_t TIMES[3] = {15, 15, 12};
  
  // Niveles de agua por defecto [P22, P23, P24]
  static constexpr uint8_t WATER_LEVELS[3] = {3, 3, 2};
  
  // Rotación por defecto [P22, P23, P24]
  static constexpr uint8_t ROTATIONS[3] = {2, 2, 2};
};

// 🖥️ NEXTION DISPLAY
struct NextionConfig {
  static constexpr uint32_t BAUD_RATE = 115200;
  static constexpr uint8_t TIMEOUT = 5;
  
  // Páginas
  static constexpr uint8_t PAGE_WELCOME = 0;
  static constexpr uint8_t PAGE_SELECTION = 1;
  static constexpr uint8_t PAGE_EXECUTION = 2;
  static constexpr uint8_t PAGE_EDIT = 3;
  static constexpr uint8_t PAGE_ERROR = 4;
  static constexpr uint8_t PAGE_EMERGENCY = 5;
};

#endif // CONFIG_STRUCTS_H