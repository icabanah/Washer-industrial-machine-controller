# Estructura del Software

## Organización General

El software del sistema "automLavanderia_v1" está estructurado como una aplicación Arduino típica, con las funciones principales `setup()` y `loop()`, complementadas por un conjunto de funciones auxiliares organizadas por categorías funcionales.

## Analogía Arquitectónica

La estructura del software puede ser vista como un edificio con diferentes pisos y habitaciones, cada uno con una función específica:

- **Cimientos (Inicialización)**: La función `setup()` establece las bases sobre las que opera todo el sistema.
- **Planta Principal (Loop)**: La función `loop()` actúa como el vestíbulo central desde donde se accede a las diferentes áreas funcionales.
- **Habitaciones Especializadas (Subsistemas)**: Cada módulo funcional (control de motor, temperatura, etc.) es como una habitación diseñada para un propósito específico.
- **Sistema Eléctrico (AsyncTask)**: El temporizador asíncrono es como el sistema eléctrico que proporciona energía y coordinación temporal a todo el edificio.

## Librerías Utilizadas

El software utiliza las siguientes librerías externas:

1. **OneWire**: Para la comunicación con el sensor de temperatura.
2. **DallasTemperature**: Para la interpretación de los datos del sensor de temperatura.
3. **HX710B**: Para el manejo del sensor de presión.
4. **AsyncTaskLib**: Para la gestión de tareas asíncronas (temporizador).
5. **LiquidCrystal**: Para el control de la pantalla LCD.
6. **EEPROM**: Para el almacenamiento persistente de configuraciones.

## Estructuras de Datos Principales

### Matrices de Configuración

El sistema utiliza varias matrices bidimensionales para almacenar la configuración de los diferentes programas:

```c
uint8_t NivelAgua[3][4];         // Niveles de agua para 3 programas x 4 fases
uint8_t RotacionTam[3][4];       // Velocidad de rotación para 3 programas x 4 fases
uint8_t TemperaturaLim[3][4];    // Temperatura para 3 programas x 4 fases
uint8_t TemporizadorLim[3][4];   // Tiempos para 3 programas x 4 fases
uint8_t TiempoRotacion[3][2];    // Tiempos de rotación para 3 niveles de velocidad
uint8_t TiempoEntFase[3][4];     // Tiempos entre fases para 3 programas x 4 fases
```

**Analogía**: Estas matrices son como las recetas de un libro de cocina, donde cada programa es una receta diferente y cada fase es un paso en la preparación, con diferentes ingredientes (parámetros) en cada paso.

### Variables de Estado

El sistema utiliza varias variables booleanas para controlar el estado general:

```c
boolean tiempoCumplido = false;        // Indica si el tiempo de la fase actual ha terminado
boolean programaTerminado = true;      // Indica si el programa completo ha terminado
boolean programaEnPausa = false;       // Indica si el programa está en pausa
boolean editandoProgramaEjecucion = false; // Indica si se está editando un programa en ejecución
```

**Analogía**: Estas variables funcionan como interruptores que determinan el estado actual del sistema, similar a los indicadores luminosos en un panel de control.

### Variables de Proceso

Estas variables mantienen información sobre el estado actual del proceso:

```c
uint8_t programa = 1;            // Programa seleccionado (1-3)
uint8_t fase = 1;                // Fase actual (1-4)
uint8_t direccion = 1;           // Dirección de rotación del motor
int16_t segunderoTemporizador = 0; // Contador de segundos para el temporizador
int16_t segunderoMotor = 0;      // Contador de segundos para el control del motor
boolean motorON = false;         // Estado del motor
```

**Analogía**: Son como los indicadores en el tablero de un vehículo que muestran la velocidad, RPM, nivel de combustible, etc., proporcionando información sobre el estado actual de operación.

## Componentes Funcionales Principales

### 1. Inicialización del Sistema

La función `setup()` se encarga de:
- Configurar todos los pines de entrada/salida
- Inicializar los dispositivos (LCD, sensores)
- Recuperar valores almacenados en la EEPROM
- Mostrar la pantalla de inicio

### 2. Bucle Principal

La función `loop()` es el corazón del sistema, manejando:
- Detección de pulsaciones de botones
- Navegación por menús
- Inicio y control de programas
- Control de eventos y transiciones de estado

### 3. Tarea Asíncrona para Temporizador

El sistema utiliza `AsyncTask` para controlar los tiempos:

```c
AsyncTask segundosTemporizador(800, true, []() {
    // Gestión de contadores de tiempo
    // Control de sensores
    // Actualización de la pantalla
});
```

**Analogía**: Es como un reloj de cocina que funciona independientemente, avisando cuando se cumplen determinados tiempos sin detener otras actividades.

### 4. Gestores de Interfaz de Usuario

Varias funciones manejan la visualización en la pantalla LCD:
- `pintarPantallaInicio()`: Muestra el mensaje de bienvenida
- `pintarVentanaSeleccion()`: Muestra la pantalla de selección de programa
- `pintarVentanaEdicionMenu()`: Muestra la interfaz de edición
- `pintarVentanaEjecucion()`: Muestra el estado durante la ejecución
- `pintarVentanaError()`: Muestra mensajes de error

### 5. Controladores de Subsistemas

#### Control de Motor
La función `controladorDireccionMotor()` gestiona:
- Cambio de dirección del motor según temporizadores
- Pausas entre cambios de dirección
- Secuencia de rotación según el programa

**Analogía**: Funciona como una caja de cambios automática que alterna entre avance, retroceso y neutro según un patrón predefinido.

#### Control de Temperatura
La función `controladorSensorTemperatura()` gestiona:
- Lectura de la temperatura actual
- Comparación con el valor objetivo
- Activación/desactivación del sistema de calentamiento

**Analogía**: Opera como un termostato, manteniendo la temperatura dentro de un rango objetivo.

#### Control de Nivel de Agua
La función `controladorSensorPresion()` gestiona:
- Lectura del sensor de presión
- Determinación del nivel de agua
- Control de la válvula de llenado

**Analogía**: Funciona como un sistema de llenado automático que mantiene el nivel de agua adecuado.

### 6. Gestor de Programas

Funciones que controlan el ciclo de vida de un programa:
- `iniciarPrograma()`: Configura e inicia un nuevo programa
- `reiniciarPrograma()`: Prepara el sistema para una nueva fase
- `pausarPrograma()`: Pone en pausa la ejecución
- `terminarPrograma()`: Finaliza la ejecución y restablece el sistema

**Analogía**: Es como un director de orquesta que indica cuándo debe comenzar cada instrumento y cuándo debe detenerse.

### 7. Sistema de Persistencia

Funciones para guardar y recuperar configuraciones:
- `guardarValoresEEPROM()`: Almacena configuraciones en memoria no volátil
- `recuperarValoresEEPROM()`: Carga configuraciones previamente guardadas

**Analogía**: Actúa como un archivador que guarda y recupera las configuraciones preferidas del usuario.

## Flujo de Control

El flujo de control del sistema se basa en máquinas de estados, donde las transiciones están determinadas por:
1. **Entradas del usuario**: Pulsaciones de botones
2. **Condiciones de tiempo**: Temporizadores y contadores
3. **Lecturas de sensores**: Temperatura y nivel de agua

Las funciones están diseñadas para ser modulares, lo que permite una fácil modificación y expansión del sistema según las necesidades.
