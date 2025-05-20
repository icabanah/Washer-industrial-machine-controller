# Módulo de Configuración (config.h)

## Descripción

El módulo de configuración centraliza todas las constantes, definiciones de pines, parámetros y configuraciones globales del sistema. Actúa como un repositorio central de valores para los tres programas específicos de lavado (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda), así como para la máquina de estados y todos los subsistemas hardware.

## Analogía: Partitura Maestra de la Orquesta

Este módulo es como la partitura maestra que usa el director de una orquesta. Contiene todas las notaciones, tempos, intensidades y entradas para cada instrumento. Así como la partitura define cuándo y cómo debe intervenir cada músico, el archivo de configuración define cómo debe funcionar cada pin, sensor y actuador. Para los tres programas específicos, contiene las "claves musicales" que determinan sus características únicas: el Programa 22 (Agua Caliente) requiere notas más cálidas, el Programa 23 (Agua Fría) necesita tonos más ligeros, y el Programa 24 (Multitanda) tiene una estructura compleja que combina varios movimientos.

## Estructura del Módulo

El módulo de configuración consiste en un único archivo de cabecera (`.h`) que contiene:

- Definiciones de pines para entradas y salidas
- Constantes del sistema
- Configuraciones por defecto
- Parámetros de calibración
- Definiciones de tiempos y límites

## Implementación Propuesta

```cpp
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
#define PIN_VALVULA_DESFOGUE 13  // Control de la válvula de drenaje
#define PIN_MAGNET_PUERTA 25   // Control del bloqueo electromagnético de la puerta

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
#define NEXTION_BAUD_RATE 115200

// Pines para sensores
// Sensor de Presión (HX710B)
#define PIN_PRESION_DOUT 5
#define PIN_PRESION_SCLK 4

// Sensor de Temperatura (OneWire Dallas)
#define PIN_TEMP_SENSOR 23
#define TEMP_SENSOR_ADDR {0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A}

// === PARÁMETROS DEL SISTEMA ===
// Límites y configuraciones
#define LIMITE_BLOQUEO 10000
#define NIVEL_ACTIVO LOW

// Configuración de la pantalla Nextion
#define NEXTION_TIMEOUT 100  // Timeout para comunicación Nextion
#define NEXTION_END_CMD 0xFF, 0xFF, 0xFF  // Bytes de finalización de comando

// Configuración de temperatura
#define TEMP_RESOLUTION 9
#define TEMP_RANGE 2

// Configuración de presión
#define NIVEL_PRESION_1 601
#define NIVEL_PRESION_2 628
#define NIVEL_PRESION_3 645
#define NIVEL_PRESION_4 663

// Configuración de tiempos
#define TIEMPO_BIENVENIDA 3000
#define TIEMPO_BUZZER 2000
#define INTERVALO_TEMPORIZADOR 800

// Configuración de programas
#define NUM_PROGRAMAS 3
#define NUM_FASES 4
#define MAX_NIVEL_ROTACION 3

// === DIRECCIONES EEPROM ===
#define EEPROM_ADDR_PROGRAMA 0
#define EEPROM_ADDR_FASE 1
#define EEPROM_ADDR_MINUTOS 2
#define EEPROM_ADDR_SEGUNDOS 3
#define EEPROM_ADDR_CONTADOR_H 4
#define EEPROM_ADDR_CONTADOR_L 5
#define EEPROM_ADDR_BASE_NIVELES 6
#define EEPROM_ADDR_BASE_TEMP 18
#define EEPROM_ADDR_BASE_TIEMPOS 30
#define EEPROM_ADDR_BASE_ROTACION 42

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

// Botones en el nextion
#define NEXTION_BTN_COMENZAR "btnComenzar"
#define NEXTION_BTN_PARAR "btnParar" 
#define NEXTION_BTN_EDITAR "btnEditar"
#define NEXTION_BTN_PROGRAMAR "btnProgramar" 
#define NEXTION_BTN_DISMINUIR "btnDisminuir" 
#define NEXTION_BTN_AUMENTAR "btnAumentar"

#endif // CONFIG_H
```

## Responsabilidades

El módulo de configuración tiene las siguientes responsabilidades orientadas a los tres programas específicos:

1. **Definición de Parámetros de Programa**:
   - Valores por defecto para cada programa (22, 23, 24)
   - Umbrales específicos para agua caliente y fría
   - Constantes para configuración de tandas en Programa 24

2. **Configuración de Control de Temperatura**:
   - Rangos de temperatura para Programa 22 (agua caliente)
   - Tolerancias para el control de temperatura (±2°C)
   - Tiempos para ciclos de ajuste de temperatura

3. **Configuración de Patrones de Rotación**:
   - Definición de tres niveles de intensidad (suave, medio, intenso)
   - Tiempos específicos para cada patrón de rotación
   - Parámetros para pausa entre cambios de dirección

4. **Soporte para Máquina de Estados**:
   - Definición de los siete estados del sistema
   - Constantes para control de transiciones
   - Tiempos específicos para cada estado

5. **Configuración de Interfaz de Usuario**:
   - Definiciones para pantallas específicas de cada programa
   - Componentes para mostrar información de temperatura y nivel
   - Retroalimentación visual para cada programa

## Ventajas de esta Implementación

1. **Centralización Total**: Todas las constantes específicas para los tres programas están definidas en un solo lugar, facilitando ajustes y calibraciones.

2. **Configuración Rica en Contexto**: Los comentarios explicativos ayudan a entender la función de cada constante y su relación con los diferentes programas.

3. **Adaptabilidad Mejorada**: Permite ajustar fácilmente parámetros críticos como temperaturas, umbrales y tiempos sin modificar la lógica del programa.

4. **Organización Lógica**: Agrupación de constantes por categorías relacionadas con las necesidades de los programas específicos.

5. **Documentación Integrada**: Los comentarios sirven como documentación técnica, explicando el propósito y uso de cada constante.

6. **Flexibilidad para Calibración**: Facilita la calibración de sensores y actuadores sin necesidad de modificar el código principal.

7. **Soporte para Depuración**: Incluye definiciones para control de verbosidad de los mensajes de depuración según necesidades.

## Soporte Específico para los Tres Programas

### Para Programa 22 (Agua Caliente):
- Temperatura objetivo más alta predeterminada (60°C)
- Límite inferior de temperatura para agua caliente (40°C)
- Constantes para control activo de temperatura

### Para Programa 23 (Agua Fría):
- Temperatura informativa predeterminada (25°C)
- Optimización para operación sin control activo de temperatura

### Para Programa 24 (Multitanda):
- Parámetros para controlar número de tandas (4 por defecto)
- Constantes de tiempo para transiciones entre tandas
- Flexibilidad para configurar tipo de agua

La configuración también provee soporte para aspectos críticos de seguridad, como los tiempos de seguridad para desbloqueo de puerta en condiciones normales y de emergencia, proporcionando una operación segura para todos los programas.
