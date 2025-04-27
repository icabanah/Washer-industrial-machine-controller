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
// Entradas
#define PIN_BTN_PARAR 2
#define PIN_BTN_DISMINUIR 3
#define PIN_BTN_AUMENTAR 4
#define PIN_BTN_EDITAR 26
#define PIN_BTN_PROGRAMAR_NIVEL_AGUA 30
#define PIN_BTN_COMENZAR 28

// Salidas
#define PIN_MOTOR_DIR_A 12
#define PIN_MOTOR_DIR_B 11
#define PIN_VALVULA_AGUA 10
#define PIN_ELECTROV_VAPOR 9
#define PIN_VALVULA_DESFOGUE 8
#define PIN_MAGNET_PUERTA 7
#define PIN_BUZZER 34

// Pines para LCD
#define PIN_LCD_RS 19
#define PIN_LCD_EN 18
#define PIN_LCD_D4 17
#define PIN_LCD_D5 16
#define PIN_LCD_D6 15
#define PIN_LCD_D7 14

// Pines para sensores
#define PIN_TEMP_SENSOR 25
#define PIN_PRESION_DOUT 42
#define PIN_PRESION_SCLK 40

// === PARÁMETROS DEL SISTEMA ===
// Límites y configuraciones
#define LIMITE_BLOQUEO 10000
#define NIVEL_ACTIVO LOW

// Configuración del LCD
#define LCD_COLS 16
#define LCD_ROWS 2

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

Al centralizar todas las constantes y definiciones, el módulo de configuración facilita la adaptación del sistema a diferentes entornos y requerimientos sin necesidad de modificar la lógica principal.
