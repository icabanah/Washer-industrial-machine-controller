// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// === DEFINICIÓN DE PINES ===
// Entrada de Emergencia
#define PIN_BTN_EMERGENCIA 15 // Pin para botón de emergencia con antirrebote

// Salidas (Actuadores)
#define PIN_MOTOR_DIR_A 12      // Control dirección derecha del motor
#define PIN_MOTOR_DIR_B 14      // Control dirección izquierda del motor
#define PIN_CENTRIFUGADO 27     // Control centrifugado del motor
#define PIN_VALVULA_AGUA 26     // Control de la válvula de entrada de agua
#define PIN_ELECTROV_VAPOR 33   // Control de la electroválvula de vapor para calentar
#define PIN_VALVULA_DESFOGUE 13 // Control de la válvula de drenaje
#define PIN_MAGNET_PUERTA 25    // Control del bloqueo electromagnético de la puerta

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
// #define NEXTION_BAUD_RATE 115200
#define NEXTION_BAUD_RATE 9600
#define NEXTION_RX_PIN 16 // ESP32 U2_RXD pin conectado al TX de Nextion
#define NEXTION_TX_PIN 17 // ESP32 U2_TXD pin conectado al RX de Nextion
#define NEXTION_RX_PIN 16 // ESP32 U2_RXD pin conectado al TX de Nextion
#define NEXTION_TX_PIN 17 // ESP32 U2_TXD pin conectado al RX de Nextion

// Pines para sensores
// Sensor de Presión (HX710B)
#define PIN_PRESION_DOUT 5 // Pin DOUT del sensor de presión
#define PIN_PRESION_SCLK 4 // Pin SCLK del sensor de presión

// Sensor de Temperatura (OneWire Dallas)
#define PIN_TEMP_SENSOR 23 // Pin de datos para sensor de temperatura

// === PARÁMETROS DEL SISTEMA ===
// Límites y configuraciones
#define LIMITE_BLOQUEO 10000
#define NIVEL_ACTIVO LOW

// Configuración de la pantalla Nextion
#define NEXTION_TIMEOUT 100              // Timeout para comunicación Nextion en ms
#define NEXTION_END_CMD 0xFF, 0xFF, 0xFF // Bytes de finalización de comando

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
#define MAX_ASYNC_TASKS 10 // Número máximo de tareas asíncronas que se pueden gestionar

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
#define ESTADO_EMERGENCIA 5 // Nuevo estado para manejo de emergencias

// === COMPONENTES DE LA PANTALLA NEXTION ===
// IDs de las páginas
#define NEXTION_PAGE_WELCOME 0
#define NEXTION_PAGE_SELECTION 1
#define NEXTION_PAGE_EXECUTION 2
#define NEXTION_PAGE_EDIT 3
#define NEXTION_PAGE_ERROR 4
#define NEXTION_PAGE_EMERGENCY 5

// === COMPONENTES PÁGINA 0 - BIENVENIDA ===
#define NEXTION_COMP_TITULO "lbl_titulo"       // Título principal "RH Electronics"
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
#define NEXTION_COMP_PROGRAMA_SEL "val_prog" // Texto del programa seleccionado (ej: "P22")

// Valores de los parámetros del programa
#define NEXTION_COMP_SEL_NIVEL "val_nivel"    // Valor del nivel de agua
#define NEXTION_COMP_SEL_TEMP "val_temp"      // Valor de temperatura
#define NEXTION_COMP_SEL_TIEMPO "val_tiempo"  // Valor del tiempo
#define NEXTION_COMP_SEL_ROTACION "val_rotac" // Valor de rotación
#define NEXTION_COMP_SEL_FASE "val_fase"      // Valor de fase
#define NEXTION_COMP_MSG_TEXT "msg_sel"       // Texto del mensaje temporal

// === ALIASES PARA COMPATIBILIDAD ===
// Estos aliases permiten usar diferentes nombres para los mismos componentes
#define NEXTION_COMP_VAL_NIVEL NEXTION_COMP_SEL_NIVEL       // Alias para nivel
#define NEXTION_COMP_VAL_TEMP NEXTION_COMP_SEL_TEMP         // Alias para temperatura
#define NEXTION_COMP_VAL_TIEMPO NEXTION_COMP_SEL_TIEMPO     // Alias para tiempo
#define NEXTION_COMP_VAL_ROTACION NEXTION_COMP_SEL_ROTACION // Alias para rotación

// === COMPONENTES ADICIONALES PARA INFORMACIÓN ===
#define NEXTION_COMP_INFO_FASES "lbl_fases" // Información adicional de fases (P24)

// Ids numéricos
#define NEXTION_ID_BTN_PROGRAM1 1 // Botón "P22" para programa 1
#define NEXTION_ID_BTN_PROGRAM2 2 // Botón "P23" para programa 2
#define NEXTION_ID_BTN_PROGRAM3 3 // Botón "P24" para programa 3
#define NEXTION_ID_BTN_START 9    // Botón "Comenzar"
#define NEXTION_ID_BTN_EDIT 10    // Botón "Editar"

// === IDs ADICIONALES PARA NAVEGACIÓN ===
#define NEXTION_ID_BTN_PROG_ANTERIOR 12              // Botón "Anterior" en selección de programa
#define NEXTION_ID_BTN_PROG_SIGUIENTE 13             // Botón "Siguiente" en selección de programa
#define NEXTION_ID_BTN_COMENZAR NEXTION_ID_BTN_START // Alias para compatibilidad
#define NEXTION_ID_BTN_EDITAR NEXTION_ID_BTN_EDIT    // Alias para compatibilidad

// === COMPONENTES PÁGINA 2 - EJECUCIÓN ===
#define NEXTION_COMP_PROG_EJECUCION "progr_ejec"         // Programa en ejecución
#define NEXTION_COMP_FASE_EJECUCION "fase_ejec"          // Fase actual en ejecución
#define NEXTION_COMP_TIEMPO_EJECUCION "tiempo_ejec"      // Tiempo transcurrido en ejecución
#define NEXTION_COMP_TEMP_EJECUCION "temp_ejec"          // Temperatura transcurrido en ejecución
#define NEXTION_COMP_NIVEL_EJECUCION "nivel_ejec"        // Nivel transcurrido en ejecución
#define NEXTION_COMP_BARRA_PROGRESO "barra_prog"         // Barra de progreso del
#define NEXTION_COMP_GAUGE_TEMP_EJECUCION "gauge_temp"   // Gauge de temperatura
#define NEXTION_COMP_BARRA_NIVEL_EJECUCION "barra_nivel" // Gauge de presión (nivel de agua)
#define NEXTION_COMP_GAUGE_VEL_EJECUCION "gauge_vel"     // Gauge de presión (nivel de agua)

// Botones de control
#define NEXTION_COMP_BTN_PARAR "btnParar"
#define NEXTION_COMP_BTN_PAUSAR "btnPausar"

// === ALIASES PARA BOTONES DE CONTROL ===
#define NEXTION_COMP_BTN_STOP NEXTION_COMP_BTN_PARAR // Alias para botón parar

// Información de estado
#define NEXTION_ID_BTN_PAUSAR 11 // Botón "Pausar"
#define NEXTION_ID_BTN_PARAR 8   // Botón "Parar"

// === COMPONENTES PÁGINA 3 - EDICIÓN DE PARÁMETROS ===
// Etiquetas de parámetros principales
#define NEXTION_COMP_PROG_EDICION "progr_edic"    // Visualización del programa actual en edición
#define NEXTION_COMP_FASE_EDICION "fase_edic"     // Visualización del valor de fase de programa en edición
#define NEXTION_COMP_TIEMPO_EDICION "tiempo_edic" // Visualización del valor de temporizador de programa en edición
#define NEXTION_COMP_TEMP_EDICION "temp_edic"     // Visualización del valor de temperatura de programa en edición
#define NEXTION_COMP_NIVEL_EDICION "nivel_edic"   // Visualización del valor de nivel de programa en edición

// Valores de parámetros en panel derecho (para mostrar cambios en tiempo real)
#define NEXTION_COMP_VAL_NIVEL_EDIT "val_nivel"   // Valor actual del nivel en panel derecho
#define NEXTION_COMP_VAL_TEMP_EDIT "val_temp"     // Valor actual de temperatura en panel derecho
#define NEXTION_COMP_VAL_TIEMPO_EDIT "val_tiempo" // Valor actual del tiempo en panel derecho
#define NEXTION_COMP_VAL_ROTAC_EDIT "val_rotac"   // Valor actual de rotación en panel derecho
#define NEXTION_COMP_VAL_FASE_EDIT "val_fase"     // Valor actual de fase en panel derecho

// Etiquetas de parámetros
#define NEXTION_COMP_BTN_SIGUIENTE "btnSiguiente"     // Botón "Siguiente" para editar parámetro
#define NEXTION_COMP_BTN_ANTERIOR "btnAnterior"       // Botón "Anterior" para editar parámetro
#define NEXTION_COMP_BTN_MAS "btnMas"                 // Botón "Mas" para editar parámetro
#define NEXTION_COMP_BTN_MENOS "btnMenos"             // Botón "Menos" para editar parámetro
#define NEXTION_COMP_BTN_GUARDAR "btnGuardar"         // Botón "Guardar" para guardar parámetro
#define NEXTION_COMP_BTN_CANCELAR "btnCancelar"       // Botón "Cancelar" para cancelar la edición de parámetro
#define NEXTION_COMP_PARAM_EDITAR "param"             // Texto "param" para editar un parámetro en específico
#define NEXTION_COMP_PARAM_VALOR_EDITAR "param_value" // Texto "param" para editar parámetro

#define NEXTION_ID_BTN_PARAM_MENOS 17     // ID de Botón "-" para disminuir parámetro
#define NEXTION_ID_BTN_PARAM_MAS 16       // ID de Botón "+" para aumentar parámetro
#define NEXTION_ID_BTN_PARAM_ANTERIOR 18  // ID de Botón anterior para parámetro
#define NEXTION_ID_BTN_PARAM_SIGUIENTE 15 // ID de Botón siguiente parámetro
#define NEXTION_ID_BTN_GUARDAR 3          // ID de Botón "Guardar"
#define NEXTION_ID_BTN_CANCELAR 4         // ID de Botón "Cancelar"

// === PARÁMETROS EDITABLES ===
#define PARAM_NIVEL 0
#define PARAM_TEMPERATURA 1
#define PARAM_TIEMPO 2
#define PARAM_ROTACION 3

// === LÍMITES DE PARÁMETROS ===
#define MIN_NIVEL 1
#define MAX_NIVEL 4
#define MIN_TEMPERATURA 20
#define MAX_TEMPERATURA 90
#define MIN_TIEMPO 1
#define MAX_TIEMPO 60
#define MIN_ROTACION 1
#define MAX_ROTACION 3

// === INCREMENTOS DE EDICIÓN ===
#define INCREMENT_NIVEL 1    // Incremento para nivel de agua
#define INCREMENT_TEMP 5     // Incremento para temperatura (°C)
#define INCREMENT_TIEMPO 1   // Incremento para tiempo (minutos)
#define INCREMENT_ROTACION 1 // Incremento para rotación

// === TEXTOS DE PARÁMETROS PARA DISPLAY ===
#define TEXT_PARAM_NIVEL "Nivel"
#define TEXT_PARAM_TEMPERATURA "Temperatura"
#define TEXT_PARAM_TIEMPO "Tiempo"
#define TEXT_PARAM_ROTACION "Rotacion"

// === CONFIGURACIÓN DE EDICIÓN ===
#define EDIT_TIMEOUT_MS 30000    // Timeout para salir automáticamente de edición (30 segundos)
#define PARAM_BLINK_INTERVAL 500 // Intervalo de parpadeo para parámetro en edición (ms)

// ===== DECLARACIONES DE FUNCIONES DE CONFIGURACIÓN =====

// Funciones de validación de parámetros
int validarNivel(int valor);
int validarTemperatura(int valor);
int validarTiempo(int valor);
int validarRotacion(int valor);

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
void formatearParametroConUnidad(int tipoParam, int valor, char *buffer, int tamanioBuffer);

#endif // CONFIG_H