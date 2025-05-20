// config.h
#ifndef CONFIG_H
#define CONFIG_H

// === DEFINICIÓN DE PINES ===
// Entrada de Emergencia
#define PIN_BTN_EMERGENCIA 15  // Pin para botón de emergencia con antirrebote

// Salidas (Actuadores)
#define PIN_MOTOR_DIR_A 12     // Control dirección derecha del motor
#define PIN_MOTOR_DIR_B 14     // Control dirección izquierda del motor
#define PIN_CENTRIFUGADO 27    // Control centrifugado del motor
#define PIN_VALVULA_AGUA 26    // Control de la válvula de entrada de agua
#define PIN_ELECTROV_VAPOR 33  // Control de la electroválvula de vapor para calentar
#define PIN_VALVULA_DESFOGUE 13// Control de la válvula de drenaje
#define PIN_MAGNET_PUERTA 25   // Control del bloqueo electromagnético de la puerta

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
#define NEXTION_BAUD_RATE 115200
#define NEXTION_RX_PIN 16      // ESP32 U2_RXD pin conectado al TX de Nextion
#define NEXTION_TX_PIN 17      // ESP32 U2_TXD pin conectado al RX de Nextion

// Pines para sensores
// Sensor de Presión (HX710B)
#define PIN_PRESION_DOUT 5     // Pin DOUT del sensor de presión
#define PIN_PRESION_SCLK 4     // Pin SCLK del sensor de presión

// Sensor de Temperatura (OneWire Dallas)
#define PIN_TEMP_SENSOR 23     // Pin de datos para sensor de temperatura

// === PARÁMETROS DEL SISTEMA ===
// Límites y configuraciones
#define LIMITE_BLOQUEO 10000
#define NIVEL_ACTIVO LOW

// Configuración de la pantalla Nextion
#define NEXTION_TIMEOUT 100  // Timeout para comunicación Nextion en ms
#define NEXTION_END_CMD 0xFF, 0xFF, 0xFF  // Bytes de finalización de comando

// Configuración de temperatura
#define TEMP_RESOLUTION 9
#define TEMP_RANGE 2

// Dirección del sensor de temperatura (OneWire Dallas)
#define TEMP_SENSOR_ADDR {0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A}

// Configuración de presión
#define NIVEL_PRESION_1 601
#define NIVEL_PRESION_2 628
#define NIVEL_PRESION_3 645
#define NIVEL_PRESION_4 663

// Configuración de tiempos
#define TIEMPO_BIENVENIDA 3000
#define INTERVALO_TEMPORIZADOR 800

// Configuración de tareas asíncronas
#define MAX_ASYNC_TASKS 10  // Número máximo de tareas asíncronas que se pueden gestionar

// Configuración de programas
#define NUM_PROGRAMAS 3
#define NUM_FASES 4
#define MAX_NIVEL_ROTACION 3

// === CLAVES PREFERENCES (Reemplazo de direcciones EEPROM) ===
// Las claves ahora se definen directamente en el código como strings
// Ya no se necesitan direcciones numéricas para Preferences
// Se mantienen estos comentarios como referencia de los nombres de claves
// usados en la implementación de storage.cpp:
// - "programa" - Programa actual
// - "fase" - Fase actual
// - "minutos" - Minutos del temporizador
// - "segundos" - Segundos del temporizador
// - "contador" - Contador de uso
// - "nivel_P_F" - Nivel de agua para programa P, fase F
// - "temp_P_F" - Temperatura para programa P, fase F
// - "tiempo_P_F" - Tiempo para programa P, fase F
// - "rotacion_P_F" - Rotación para programa P, fase F

// === DEFINICIÓN DE ESTADOS ===
#define ESTADO_SELECCION 0
#define ESTADO_EDICION 1
#define ESTADO_EJECUCION 2
#define ESTADO_PAUSA 3
#define ESTADO_ERROR 4
#define ESTADO_EMERGENCIA 5  // Nuevo estado para manejo de emergencias

// === COMPONENTES DE LA PANTALLA NEXTION ===
// IDs de las páginas
#define NEXTION_PAGE_WELCOME 0
#define NEXTION_PAGE_SELECTION 1
#define NEXTION_PAGE_EXECUTION 2
#define NEXTION_PAGE_EDIT 3
#define NEXTION_PAGE_ERROR 4
#define NEXTION_PAGE_EMERGENCY 5

// Componentes de la pantalla Nextion (nombres actualizados)
#define NEXTION_COMP_BTN_PROGRAM1 "btnPrograma1"
#define NEXTION_COMP_BTN_PROGRAM2 "btnPrograma2"
#define NEXTION_COMP_BTN_PROGRAM3 "btnPrograma3"
#define NEXTION_COMP_BTN_START "btnComenzar"
#define NEXTION_COMP_BTN_EDIT "btnEditar"
#define NEXTION_COMP_BTN_STOP "btnParar"
#define NEXTION_COMP_TXT_TEMP "txtTemp"
#define NEXTION_COMP_TXT_PRESSURE "txtPresion"
#define NEXTION_COMP_TXT_TIME "txtTiempo"
#define NEXTION_COMP_GAUGE_TEMP "gaugeTemp"
#define NEXTION_COMP_GAUGE_PRESSURE "gaugePresion"

#endif // CONFIG_H