# Documentación del Controlador para Lavadoras Industriales

## Índice General

### 1. [Descripción General](1-descripcion-general.md)
- Introducción al sistema
- Analogía conceptual del controlador
- Componentes principales
- Capacidades del sistema
- Paradigma de funcionamiento

### 2. [Componentes Hardware](2-componentes-hardware.md)
- Diagrama de conexiones
- Descripción de componentes
- Pantalla LCD
- Sensores
- Entradas y salidas
- Parámetros de sensores
- Interacción entre componentes

### 3. [Estructura del Software](3-estructura-software.md)
- Organización general
- Analogía arquitectónica
- Librerías utilizadas
- Estructuras de datos principales
- Componentes funcionales principales
- Flujo de control

### 4. [Flujo de Ejecución](4-flujo-ejecucion.md)
- Inicialización del sistema
- Bucle principal
- Modo de selección de programa
- Modo de edición de programa
- Ejecución de programa
- Control del motor
- Control de temperatura
- Control de nivel de agua
- Transiciones entre fases
- Finalización del programa

### 5. [Sensores y Actuadores](5-sensores-actuadores.md)
- Analogía del sistema de control
- Sensores (temperatura, presión)
- Actuadores (motor, válvulas, seguridad)
- Interacción entre sensores y actuadores
- Pseudocódigo de la interacción
- Diagrama de interacción

### 6. [Programas de Lavado](6-programas-lavado.md)
- Analogía de los programas
- Estructura de programas
- Programa 1: Lavado Básico
- Programa 2: Lavado Térmico
- Programa 3: Lavado Completo
- Parámetros configurables
- Visualización y edición de programas
- Funciones de control de programa
- Personalización de programas
- Interacción especial durante programa
- Seguridad y limitaciones

## Diagramas Incluidos

- **Diagrama de Flujo del Sistema**: Representación visual de las secuencias de operación del controlador.
- **Diagrama de Componentes**: Muestra la interconexión entre los diferentes elementos hardware y software.
- **Diagrama de Estados**: Ilustra los diferentes estados del sistema y las transiciones entre ellos.

## Información Técnica

- **Microcontrolador**: Arduino (posiblemente Arduino Mega)
- **Sensores**: Dallas Temperature (OneWire), HX710B (presión)
- **Actuadores**: Motor bidireccional, válvulas, electroimán para puerta
- **Interfaz**: LCD 16x2, 6 botones de control
- **Almacenamiento**: EEPROM para configuraciones

## Autor

iTrebolSoft - RH Electronics (958970967)

## Versión

automLavanderia_v1 - Documentación generada: Abril 2025
