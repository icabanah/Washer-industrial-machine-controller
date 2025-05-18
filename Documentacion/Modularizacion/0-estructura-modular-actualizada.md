# Estructura Modular Actualizada del Controlador de Lavadora Industrial

## Introducción

Esta propuesta presenta una estructura modular actualizada para el controlador de lavadora industrial, desarrollada en Arequipa, Perú. El sistema está adaptado para funcionar con el controlador ESP32-WROOM y pantalla táctil Nextion, reemplazando la implementación original basada en Arduino y LCD. La modularización permitirá un mejor mantenimiento, mayor facilidad para implementar nuevas características y una estructura más clara. Se desarrolla utilizando el IDE de Arduino versión 2.3.6 con el core ESP32 versión 3.2.0.

## Analogía: Sistema Modular de Orquesta Modernizado

Podemos visualizar el controlador como una orquesta donde cada sección de instrumentos (módulos) tiene su propia partitura (código) pero todos siguen la batuta del director (programa principal). Con la actualización del hardware, es como si la orquesta hubiera adoptado instrumentos electrónicos modernos y un sistema de dirección digital, manteniendo la misma armonía musical pero con capacidades tecnológicas ampliadas.

## Estructura de Archivos Propuesta

La nueva estructura mantiene la organización modular pero adapta los módulos para el nuevo hardware:

```
washerController/
│
├── mainController.ino       # Archivo principal (incluye setup, loop)
├── config.h                     # Configuraciones, constantes y definiciones globales
├── hardware.cpp                 # Gestión del hardware físico (ESP32, Nextion, botón de emergencia)
├── hardware.h
├── ui_controller.cpp            # Controlador de interfaz de usuario para Nextion
├── ui_controller.h
├── program_controller.cpp       # Controlador de programas de lavado
├── program_controller.h
├── sensors.cpp                  # Gestión de sensores (temperatura, presión)
├── sensors.h
├── actuators.cpp                # Control de actuadores (motor, válvulas)
├── actuators.h
├── storage.cpp                  # Gestión de almacenamiento (EEPROM)
├── storage.h
└── utils.cpp                    # Utilidades generales
└── utils.h
```

## Principales Cambios en los Módulos

### 1. Módulo de Configuración (config.h)

**Cambios:**
- Eliminación de definiciones de pines para botones (excepto emergencia)
- Adición de configuraciones para la comunicación serial con Nextion
- Definición de componentes y páginas de la pantalla Nextion
- Adaptación de configuraciones para ESP32

### 2. Módulo de Hardware (hardware.h / hardware.cpp)

**Cambios:**
- Reemplazo del controlador LCD por interfaz serial para Nextion
- Implementación de métodos de comunicación con Nextion
- Manejo del botón de emergencia con antirrebote
- Eliminación del código para múltiples botones físicos

### 3. Módulo de UI Controller (ui_controller.h / ui_controller.cpp)

**Cambios:**
- Completa reescritura para usar la pantalla Nextion
- Implementación de pantallas táctiles para selección, edición, etc.
- Manejo de eventos táctiles de la pantalla
- Adición de retroalimentación visual y auditiva
- Nueva pantalla de emergencia

### 4. Módulo de Program Controller (program_controller.h / program_controller.cpp)

**Cambios:**
- Adición de manejo de emergencia
- Adaptación para procesar eventos táctiles en lugar de botones
- Mejora en la gestión de estados para incluir emergencia
- Métodos de seguridad mejorados

### 5. Otros Módulos

Los demás módulos (Sensors, Actuators, Storage, Utils) mantienen su funcionalidad principal, con adaptaciones menores para integrar con el ESP32 y responder adecuadamente en situaciones de emergencia. Específicamente, el módulo Utils proporciona utilidades para manejo de tiempos sin usar la función delay(), implementando alternativas no bloqueantes para cronometrado de eventos.

## Beneficios Adicionales de la Actualización

1. **Interfaz Mejorada**: La pantalla táctil Nextion permite una interfaz más intuitiva y visual que el LCD original.
2. **Capacidad de Procesamiento**: El ESP32 proporciona mayor potencia de procesamiento y memoria que Arduino.
3. **Conectividad**: Posibilidad de añadir en el futuro funcionalidades de conectividad WiFi/Bluetooth usando el ESP32.
4. **Seguridad Mejorada**: El botón de emergencia dedicado con sistema antirrebote mejora la seguridad operacional.
5. **Escalabilidad**: La arquitectura modular adaptada permite futuras expansiones de funcionalidad.

## Diagrama de Interacción de Módulos Actualizado

```
┌───────────────┐     ┌─────────────────┐     ┌───────────────┐
│  UI Controller│────>┤Hardware (ESP32 + │◄────┤Program        │
│  (Nextion)    │     │Nextion + Botón   │     │Controller     │
└───────┬───────┘     │Emergencia)       │     └───────┬───────┘
        │             └─────────┬─────────┘             │
        │                       │                       │
        │                       │                       │
┌───────▼───────┐     ┌─────────▼─────────┐     ┌───────▼───────┐
│ Sensors       │     │ Actuators         │     │ Storage       │
└───────────────┘     └───────────────────┘     └───────────────┘
                                │
                        ┌───────▼───────┐
                        │ Utils         │
                        └───────────────┘
```

Los siguientes documentos detallan cada uno de los módulos propuestos con sus adaptaciones para el nuevo hardware.
