# Módulo de Configuración (config.h)

## Descripción

El módulo de configuración centraliza todas las constantes, definiciones de pines, parámetros y configuraciones globales del sistema. Actúa como un repositorio central de valores que pueden necesitar ajustes o adaptaciones.

## Analogía: Plano Arquitectónico

Este módulo es como el plano arquitectónico de un edificio, que define las dimensiones, materiales y especificaciones sin entrar en los detalles de construcción. Cualquier modificación en el diseño se refleja primero en estos planos, antes de realizar cambios en la construcción real.

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
#define PIN_BTN_EMERGENCIA 2  // Pin para botón de emergencia con antirrebote

// Salidas
#define PIN_MOTOR_DIR_A 12
#define PIN_MOTOR_DIR_B 11
#define PIN_VALVULA_AGUA 10
#define PIN_ELECTROV_VAPOR 9
#define PIN_VALVULA_DESFOGUE 8
#define PIN_MAGNET_PUERTA 7
#define PIN_BUZZER 34

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
#define NEXTION_BAUD_RATE 115200

// Pines para sensores
#define PIN_TEMP_SENSOR 25
#define PIN_PRESION_DOUT 42
#define PIN_PRESION_SCLK 40

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

// IDs de los componentes (ejemplos)
#define NEXTION_COMP_BTN_PROGRAM1 1
#define NEXTION_COMP_BTN_PROGRAM2 2
#define NEXTION_COMP_BTN_PROGRAM3 3
#define NEXTION_COMP_BTN_START 4
#define NEXTION_COMP_BTN_EDIT 5
#define NEXTION_COMP_BTN_STOP 6
#define NEXTION_COMP_TXT_TEMP 10
#define NEXTION_COMP_TXT_PRESSURE 11
#define NEXTION_COMP_TXT_TIME 12
#define NEXTION_COMP_GAUGE_TEMP 20
#define NEXTION_COMP_GAUGE_PRESSURE 21

#endif // CONFIG_H
```

## Responsabilidades

El módulo de configuración tiene las siguientes responsabilidades:

1. **Centralización**: Mantener todos los valores configurables en un solo lugar.
2. **Documentación**: Proporcionar descripciones claras para cada constante.
3. **Organización**: Agrupar lógicamente los parámetros relacionados.
4. **Abstracción**: Permitir referirse a los pines y constantes por nombres significativos.

## Ventajas de este Enfoque

1. **Mantenibilidad**: Facilita cambios en la configuración sin necesidad de buscar en todo el código.
2. **Adaptabilidad**: Permite adaptar el sistema a diferentes hardware modificando solo este archivo.
3. **Legibilidad**: Mejora la comprensión del código al usar nombres descriptivos en lugar de valores literales.
4. **Consistencia**: Evita duplicación de valores y posibles inconsistencias.

## Adaptaciones para el Nuevo Hardware

La configuración ha sido actualizada para reflejar el cambio de hardware:

1. **ESP32-WROOM**: Se han eliminado definiciones específicas de Arduino y se han adaptado para ESP32.
2. **Pantalla Nextion**: Se han añadido parámetros para comunicación serial con la pantalla Nextion.
3. **Botones**: Se ha simplificado a un único botón de emergencia con antirrebote.
4. **Nuevos Estados**: Se ha añadido un estado específico para manejar situaciones de emergencia.

Al centralizar todas las constantes y definiciones, el módulo de configuración facilita la adaptación del sistema a diferentes entornos y requerimientos sin necesidad de modificar la lógica principal.
