// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"
#include "config_structs.h"  // Nuevas estructuras organizadas

// === DEFINICIÓN DE PINES ===
// Usando HardwarePins struct de config_structs.h
#define PIN_BTN_EMERGENCIA HardwarePins::EMERGENCY_BUTTON

// Salidas (Actuadores)
#define PIN_MOTOR_DIR_IZQ HardwarePins::MOTOR_DIR_LEFT
#define PIN_MOTOR_DIR_DER HardwarePins::MOTOR_DIR_RIGHT
#define PIN_CENTRIFUGADO HardwarePins::CENTRIFUGE
#define PIN_VALVULA_AGUA HardwarePins::WATER_VALVE
#define PIN_ELECTROV_VAPOR HardwarePins::STEAM_VALVE
#define PIN_VALVULA_DESFOGUE HardwarePins::DRAIN_VALVE
#define PIN_MAGNET_PUERTA HardwarePins::DOOR_MAGNET

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
#define NEXTION_BAUD_RATE NextionConfig::BAUD_RATE
#define NEXTION_RX_PIN HardwarePins::NEXTION_RX
#define NEXTION_TX_PIN HardwarePins::NEXTION_TX

// Pines para sensores
#define PIN_PRESION_DOUT HardwarePins::PRESSURE_DOUT
#define PIN_PRESION_SCLK HardwarePins::PRESSURE_SCLK
#define PIN_TEMP_SENSOR HardwarePins::TEMPERATURE

// Sensor de Puerta (Opcional - para implementación futura)
// #define PIN_SENSOR_PUERTA 22 // Pin para sensor magnético/reed switch de
// puerta #define PUERTA_CERRADA_LEVEL LOW // Nivel lógico cuando puerta está
// cerrada

// === PARÁMETROS DEL SISTEMA ===
// Límites y configuraciones
#define LIMITE_BLOQUEO 10000
#define NIVEL_ACTIVO LOW

// Configuración de la pantalla Nextion usando NextionConfig struct
#define NEXTION_TIMEOUT NextionConfig::TIMEOUT
#define NEXTION_END_CMD SystemTiming::NEXTION_END_CMD[0], SystemTiming::NEXTION_END_CMD[1], SystemTiming::NEXTION_END_CMD[2]

// Configuración de sensores usando SensorConfig struct
#define TEMP_RESOLUTION SensorConfig::TEMP_RESOLUTION
#define TEMP_RANGE SensorConfig::TEMP_RANGE
#define TEMP_SENSOR_ADDR SensorConfig::TEMP_SENSOR_ADDR

// Configuración de presión
#define NIVEL_PRESION_1 SensorConfig::PRESSURE_LEVEL_1
#define NIVEL_PRESION_2 SensorConfig::PRESSURE_LEVEL_2
#define NIVEL_PRESION_3 SensorConfig::PRESSURE_LEVEL_3
#define NIVEL_PRESION_4 SensorConfig::PRESSURE_LEVEL_4

// Configuración de tiempos usando SystemTiming struct
#define TIEMPO_BIENVENIDA SystemTiming::WELCOME_SCREEN
#define INTERVALO_TEMPORIZADOR SystemTiming::TIMER_INTERVAL

// === CONFIGURACIÓN DE MOTOR BIDIRECCIONAL (PERMUTACIÓN EN LAVADO) ===
//
// Tiempos en segundos para cada nivel de rotación
// Secuencia: DERECHA → PAUSA → IZQUIERDA → PAUSA → [repetir]
//
// TABLA DE CONFIGURACIÓN ACTUAL:
// +-------+----------+------------+---------+----------------+
// | Nivel | Derecha  | Izquierda  |  Pausa  | Ciclo Total    |
// +-------+----------+------------+---------+----------------+
// |   1   |    5s    |     5s     |    2s   |   14s (suave)  |
// |   2   |    8s    |     8s     |    2s   |   20s (media)  |
// |   3   |   10s    |    10s     |    1s   |   22s (intensa)|
// +-------+----------+------------+---------+----------------+

// Configuración de motor usando MotorConfig struct
// Nivel 1 - Rotación Suave
#define MOTOR_L1_TIEMPO_DERECHA MotorConfig::Level1::TIME_RIGHT
#define MOTOR_L1_TIEMPO_IZQUIERDA MotorConfig::Level1::TIME_LEFT
#define MOTOR_L1_TIEMPO_PAUSA MotorConfig::Level1::TIME_PAUSE

// Nivel 2 - Rotación Media
#define MOTOR_L2_TIEMPO_DERECHA MotorConfig::Level2::TIME_RIGHT
#define MOTOR_L2_TIEMPO_IZQUIERDA MotorConfig::Level2::TIME_LEFT
#define MOTOR_L2_TIEMPO_PAUSA MotorConfig::Level2::TIME_PAUSE

// Nivel 3 - Rotación Intensa
#define MOTOR_L3_TIEMPO_DERECHA MotorConfig::Level3::TIME_RIGHT
#define MOTOR_L3_TIEMPO_IZQUIERDA MotorConfig::Level3::TIME_LEFT
#define MOTOR_L3_TIEMPO_PAUSA MotorConfig::Level3::TIME_PAUSE

// Tiempos de compatibilidad
#define MOTOR_TIEMPO_ON MotorConfig::COMPATIBILITY_TIME_ON
#define MOTOR_TIEMPO_PAUSA MotorConfig::COMPATIBILITY_TIME_OFF

// Configuración de temporizadores especiales usando SystemTiming struct
#define TIEMPO_DRENAJE SystemTiming::DRAIN_TIME
#define TIEMPO_PUERTA_BLOQUEO SystemTiming::DOOR_LOCK_TIME
#define TIEMPO_CENTRIFUGADO SystemTiming::CENTRIFUGE_TIME

// Configuración de tareas asíncronas
#define MAX_ASYNC_TASKS                                                        \
  10 // Número máximo de tareas asíncronas que se pueden gestionar

// Configuración de programas
#define NUM_PROGRAMAS 3
#define NUM_FASES 4 // 0=Llenado, 1=Lavado, 2=Centrifugado(opcional), 3=Drenaje
#define MAX_NIVEL_ROTACION MotorConfig::MAX_ROTATION_LEVEL

// P22/P23: 1 tanda × 4 fases, P24: 4 tandas × 4 fases

// === DEFINICIÓN DE ESTADOS ===
#define ESTADO_SELECCION 0
#define ESTADO_EDICION 1
#define ESTADO_EJECUCION 2
#define ESTADO_PAUSA 3
#define ESTADO_ERROR 4
#define ESTADO_EMERGENCIA 5
#define ESTADO_IDLE 6
#define ESTADO_DRENAJE_FINAL 7
#define ESTADO_ESPERA_PUERTA 8
#define ESTADO_CENTRIFUGADO 9

// === ESTADOS DE FASES (Máquina de estados para fases) ===
#define FASE_LLENANDO 0     // Llenando agua y calentando
#define FASE_LAVADO 1       // Lavado con rotación de motor
#define FASE_CENTRIFUGA 2   // Centrifugado
#define FASE_DRENAJE 3      // Drenaje de agua
#define FASE_ENFRIAMIENTO 4 // Enfriamiento final

// === CÓDIGOS DE ERROR ===
#define ERROR_NINGUNO 0
#define ERROR_PUERTA 1
#define ERROR_TEMPERATURA 2
#define ERROR_NIVEL 3
#define ERROR_SENSOR 4
#define ERROR_TIMEOUT 5
#define ERROR_MOTOR 6
#define ERROR_COMUNICACION 7 // Nuevo estado para manejo de emergencias

// === COMPONENTES DE LA PANTALLA NEXTION ===
// IDs de las páginas usando NextionConfig struct
#define NEXTION_PAGE_WELCOME NextionConfig::PAGE_WELCOME
#define NEXTION_PAGE_SELECTION NextionConfig::PAGE_SELECTION
#define NEXTION_PAGE_EXECUTION NextionConfig::PAGE_EXECUTION
#define NEXTION_PAGE_EDIT NextionConfig::PAGE_EDIT
#define NEXTION_PAGE_ERROR NextionConfig::PAGE_ERROR
#define NEXTION_PAGE_EMERGENCY NextionConfig::PAGE_EMERGENCY

// Valores de los parámetros de programa seteado
#define NEXTION_COMP_SET_PROG "progr_sel"    // Programa en seleccion
#define NEXTION_COMP_SET_NIVEL "val_nivel"   // Valor del nivel de agua
#define NEXTION_COMP_SET_TEMP "val_temp"     // Valor de temperatura
#define NEXTION_COMP_SET_TIEMPO "val_tiempo" // Valor del tiempo
#define NEXTION_COMP_SET_ROTACION                                              \
  "val_rotac" // Nivel de rotación (suave, media o intensa)
#define NEXTION_COMP_SET_FASE                                                  \
  "val_fase" // Fase de programa (llenado, lavado, Drenaje, Centrifugado
             // (opcional))
#define NEXTION_COMP_SET_CENTRIF                                               \
  "val_centrif" // Centrifugado activo (1) o inactivo (0)
#define NEXTION_COMP_SET_AGUA "val_agua" // tipo de agua (caliente o fria)
#define NEXTION_COMP_MSG "mensaje"       // Texto del mensaje temporal

// === COMPONENTES PÁGINA 0 - BIENVENIDA ===
#define NEXTION_COMP_TITULO "lbl_titulo" // Título principal "RH Electronics"
#define NEXTION_COMP_SUBTITULO "lbl_subtitulo" // Subtítulo del sistema
#define NEXTION_COMP_CONTACTO "lbl_contacto"   // Información de contacto

// === COMPONENTES PÁGINA 1 - SELECCIÓN DE PROGRAMA ===
// Programa seleccionado
#define NEXTION_COMP_BTN_PROGRAM1 "btnPrograma1"
#define NEXTION_COMP_BTN_PROGRAM2 "btnPrograma2"
#define NEXTION_COMP_BTN_PROGRAM3 "btnPrograma3"

// Botones de control
#define NEXTION_COMP_BTN_START "btnComenzar"
#define NEXTION_COMP_BTN_EDIT "btnEditar"
#define NEXTION_COMP_PROGRAMA_SEL                                              \
  "val_prog" // Texto del programa seleccionado (ej: "P22")

// Ids numéricos
#define NEXTION_ID_BTN_PROGRAM1 1 // Botón "P22" para programa 1
#define NEXTION_ID_BTN_PROGRAM2 2 // Botón "P23" para programa 2
#define NEXTION_ID_BTN_PROGRAM3 3 // Botón "P24" para programa 3
#define NEXTION_ID_BTN_EDIT 21    // Botón "Editar"
#define NEXTION_ID_BTN_START 22   // Botón "Comenzar"

// === COMPONENTES PÁGINA 2 - EJECUCIÓN ===
#define NEXTION_COMP_PROG_EJECUCION "progr_ejec" // Programa en ejecución
#define NEXTION_COMP_FASE_EJECUCION "fase_ejec"  // Fase actual en ejecución
#define NEXTION_COMP_TIEMPO_EJECUCION                                          \
  "tiempo_ejec" // Tiempo transcurrido en ejecución
#define NEXTION_COMP_TANDA_EJECUCION "tanda_ejec"   // Tanda en ejecución
#define NEXTION_COMP_TEMP_EJECUCION "temp_ejec"     // Temperatura en ejecución
#define NEXTION_COMP_NIVEL_EJECUCION "nivel_ejec"   // Nivel en ejecución
#define NEXTION_COMP_VELOCIDAD_EJECUCION "vel_ejec" // Velocidad de motor
#define NEXTION_COMP_BARRA_TEMP_EJECUCION                                      \
  "barra_temp" // Gauge de presión (nivel de agua)
#define NEXTION_COMP_BARRA_NIVEL_EJECUCION                                     \
  "barra_nivel" // Gauge de presión (nivel de agua)
#define NEXTION_COMP_BARRA_VELOC_EJECUCION                                     \
  "barra_veloc" // Gauge de presión (nivel de agua)

#define NEXTION_COMP_CENTRIF_EJECUCION "centrif_ejec"      // Velocidad de motor
#define NEXTION_COMP_AGUA_EJECUCION "agua_ejec"            // Velocidad de motor
#define NEXTION_COMP_TIEMPO_TOTAL_EJECUCION "tiempo_total" // Velocidad de motor

// Botones de control
#define NEXTION_COMP_BTN_PARAR "btnParar"
#define NEXTION_COMP_BTN_PAUSAR "btnPausar"

// Información de estado
#define NEXTION_ID_BTN_PAUSAR 21 // Botón "Pausar"
#define NEXTION_ID_BTN_PARAR 22  // Botón "Parar"

// === COMPONENTES PÁGINA 5 - EMERGENCIA ===

// === COMPONENTES PÁGINA 3 - EDICIÓN DE PARÁMETROS ===

// Componentes de edición
#define NEXTION_COMP_PARAM_EDITAR "param" // Texto del parámetro en edición
#define NEXTION_COMP_PARAM_VALOR_EDITAR                                        \
  "param_value" // Valor del parámetro en edición

#define NEXTION_ID_BTN_PARAM_MENOS 7 // ID de Botón "-" para disminuir parámetro
#define NEXTION_ID_BTN_PARAM_MAS 6   // ID de Botón "+" para aumentar parámetro
#define NEXTION_ID_BTN_PARAM_ANTERIOR 8  // ID de Botón anterior para parámetro
#define NEXTION_ID_BTN_PARAM_SIGUIENTE 5 // ID de Botón siguiente parámetro
#define NEXTION_ID_BTN_GUARDAR 3         // ID de Botón "Guardar"
#define NEXTION_ID_BTN_CANCELAR 4        // ID de Botón "Cancelar"
#define NEXTION_ID_BTN_TANDA1 26         // ID de Botón "tanda1"
#define NEXTION_ID_BTN_TANDA2 27         // ID de Botón "tanda2"
#define NEXTION_ID_BTN_TANDA3 28         // ID de Botón "tanda3"
#define NEXTION_ID_BTN_TANDA4 29         // ID de Botón "tanda4"

// Nombres de componentes para botones de tanda
#define NEXTION_COMP_BTN_TANDA1 "tanda1" // Componente botón tanda 1
#define NEXTION_COMP_BTN_TANDA2 "tanda2" // Componente botón tanda 2
#define NEXTION_COMP_BTN_TANDA3 "tanda3" // Componente botón tanda 3
#define NEXTION_COMP_BTN_TANDA4 "tanda4" // Componente botón tanda 4

#define NEXTION_ID_BTN_REINICIAR 3 // ID de Botón "Reiniciar ERROR"

// Valores de parámetros en panel derecho (para mostrar cambios en tiempo real)
#define NEXTION_ID_PARAM_NIVEL_EDIT                                            \
  18 // Valor actual del nivel en panel derecho
#define NEXTION_ID_PARAM_TEMP_EDIT                                             \
  19 // Valor actual de temperatura en panel derecho
#define NEXTION_ID_PARAM_TIEMPO_EDIT                                           \
  20 // Valor actual del tiempo en panel derecho
#define NEXTION_ID_PARAM_ROTAC_EDIT                                            \
  21 // Valor actual de rotación en panel derecho
#define NEXTION_ID_PARAM_FASE_EDIT 22 // Valor actual de fase en panel derecho
#define NEXTION_ID_PARAM_CENTRIF_EDIT                                          \
  23 // Valor actual de centrifugado en panel derecho
#define NEXTION_ID_PARAM_AGUA_EDIT                                             \
  24 // Valor actual de tipo de agua en panel derecho

// === PARÁMETROS EDITABLES ===
#define PARAM_NIVEL 0
#define PARAM_TEMPERATURA 1
#define PARAM_TIEMPO 2
#define PARAM_ROTACION 3
#define PARAM_FASE 4
#define PARAM_CENTRIF 5
#define PARAM_AGUA 6

// === LÍMITES DE PARÁMETROS ===
// Usando ParameterLimits struct de config_structs.h
#define MIN_NIVEL ParameterLimits::MIN_LEVEL
#define MAX_NIVEL ParameterLimits::MAX_LEVEL
#define MIN_TEMPERATURA ParameterLimits::MIN_TEMPERATURE
#define MAX_TEMPERATURA ParameterLimits::MAX_TEMPERATURE
#define TEMPERATURA_EMERGENCIA ParameterLimits::EMERGENCY_TEMPERATURE
#define MIN_TIEMPO ParameterLimits::MIN_TIME
#define MAX_TIEMPO ParameterLimits::MAX_TIME
#define MIN_ROTACION ParameterLimits::MIN_ROTATION
#define MAX_ROTACION ParameterLimits::MAX_ROTATION
#define MIN_FASE ParameterLimits::MIN_PHASE
#define MAX_FASE ParameterLimits::MAX_PHASE
#define MIN_CENTRIF ParameterLimits::MIN_CENTRIFUGE
#define MAX_CENTRIF ParameterLimits::MAX_CENTRIFUGE
#define MIN_AGUA ParameterLimits::MIN_WATER_TYPE
#define MAX_AGUA ParameterLimits::MAX_WATER_TYPE

// === INCREMENTOS DE EDICIÓN ===
#define INCREMENT_NIVEL 1    // Incremento para nivel de agua
#define INCREMENT_TEMP 1     // Incremento para temperatura (°C)
#define INCREMENT_TIEMPO 1   // Incremento para tiempo (minutos)
#define INCREMENT_ROTACION 1 // Incremento para rotación
#define INCREMENT_FASE 1     // Incremento para fase
#define INCREMENT_CENTRIF 1  // Incremento para centrifugado
#define INCREMENT_AGUA 1     // Incremento para tipo de agua

// === TEXTOS DE PARÁMETROS ===
#define TEXT_PARAM_NIVEL "Nivel"
#define TEXT_PARAM_TEMPERATURA "Temperatura"
#define TEXT_PARAM_TIEMPO "Tiempo"
#define TEXT_PARAM_ROTACION "Rotacion"
#define TEXT_PARAM_FASE "Fase"
#define TEXT_PARAM_CENTRIF "Centrifugado"
#define TEXT_PARAM_AGUA "Tipo Agua"

// === CONFIGURACIÓN DE EDICIÓN ===
#define EDIT_TIMEOUT_MS                                                        \
  30000 // Timeout para salir automáticamente de edición (30 segundos)
#define PARAM_BLINK_INTERVAL                                                   \
  500 // Intervalo de parpadeo para parámetro en edición (ms)

// ===== DECLARACIONES DE FUNCIONES DE CONFIGURACIÓN =====

// Funciones de validación de parámetros
int validarParametro(int tipoParam, int valor);

// Funciones de incremento/decremento
int incrementarParametro(int tipoParam, int valorActual);
int decrementarParametro(int tipoParam, int valorActual);

// Funciones de información de parámetros
const char *obtenerTextoParametro(int tipoParam);
int obtenerValorMinimo(int tipoParam);
int obtenerValorMaximo(int tipoParam);
int obtenerIncrementoParametro(int tipoParam);

// Funciones de navegación entre parámetros
int obtenerSiguienteParametro(int tipoActual);
int obtenerAnteriorParametro(int tipoActual);

// Funciones de utilidad para programas
void generarTextoPrograma(int numeroPrograma, char *buffer, int tamanioBuffer);
bool esParametroValido(int tipoParam, int valor);

// Funciones de formateo para display
void formatearParametroConUnidad(int tipoParam, int valor, char *buffer,
                                 int tamanioBuffer);

#endif // CONFIG_H