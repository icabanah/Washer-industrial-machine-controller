# Documentación del Flujo de Ejecución - Controlador de Lavadora Industrial

## Introducción

Esta carpeta contiene documentación detallada sobre el flujo de ejecución del controlador de lavadora industrial basado en ESP32. Se han estructurado los documentos en tres partes complementarias que abarcan diferentes aspectos del sistema.

## Contenido

### 1. [Flujo de Ejecución de Programas](./flujo-ejecucion-programas.md)

Este documento describe en detalle el flujo de ejecución de cada programa disponible en el controlador. Incluye:

- Estructura general de programas y fases
- Parámetros de control para cada programa
- Flujo general de ejecución
- Detalle por programa y fase
- Controladores y flujo de señales
- Control y gestión de emergencias

**Enfoque principal**: Parámetros específicos de cada programa, secuencia de operaciones y configuraciones.

### 2. [Flujo de Ejecución Técnico](./flujo-ejecucion-tecnico.md)

Este documento ofrece una descripción técnica más profunda de la implementación, enfocándose en la interacción entre módulos y los aspectos de programación. Incluye:

- Arquitectura modular
- Flujo de ejecución a nivel de sistema
- Secuencias detalladas de operación
- Manejo de errores y excepciones
- Optimización de recursos
- Patrones de diseño implementados

**Enfoque principal**: Implementación técnica, interacción entre módulos y aspectos de programación.

### 3. [Diagramas de Flujo de Ejecución](./diagramas-flujo-ejecucion.md)

Este documento complementa los anteriores con representaciones visuales que facilitan la comprensión del sistema. Incluye diagramas de:

- Módulos del sistema
- Máquina de estados
- Secuencia de inicio de programa
- Flujo de ejecución de fase
- Componentes de hardware
- Tiempos de ejecución
- Secuencia para manejo de emergencia
- Interacción de módulos
- Estructura de datos

**Enfoque principal**: Representación visual de la estructura y comportamiento del sistema.

## Uso de esta Documentación

Esta documentación puede ser utilizada por diferentes roles del proyecto:

1. **Desarrolladores**: Consultar el flujo técnico y los diagramas para entender la implementación y realizar modificaciones.
2. **Técnicos de Mantenimiento**: Revisar el flujo de ejecución de programas y los diagramas para diagnosticar problemas.
3. **Usuarios Avanzados**: Entender el flujo de programas para personalizar parámetros y optimizar ciclos de lavado.

Los tres documentos están diseñados para complementarse entre sí, proporcionando diferentes perspectivas del mismo sistema.

## Mantenimiento de la Documentación

Esta documentación debe actualizarse cuando se realicen cambios significativos en:

1. Parámetros de programas o fases
2. Estructura de la máquina de estados
3. Interacción entre módulos
4. Secuencias de operación

La documentación actualizada ayuda a mantener la coherencia del sistema a lo largo de su ciclo de vida.
