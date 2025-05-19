# Flujo Detallado de Ejecución del Controlador de Lavadora Industrial

## Descripción Técnica

Este documento proporciona una descripción técnica detallada del flujo de ejecución del controlador modular para lavadora industrial. Se presenta la interacción entre los diferentes módulos del sistema y los pasos específicos que ocurren durante la ejecución de cualquier programa.

## Arquitectura Modular

El sistema está compuesto por los siguientes módulos principales:

1. **Hardware**: Gestiona la comunicación directa con componentes físicos (pines E/S, puerto serial, etc.)
2. **Sensors**: Administra los sensores de temperatura y presión (nivel de agua)
3. **Actuators**: Controla los actuadores (motor, válvulas, etc.)
4. **UIController**: Gestiona la interfaz de usuario en la pantalla Nextion
5. **ProgramController**: Administra la lógica central del programa y la máquina de estados
6. **Storage**: Maneja el almacenamiento persistente de configuraciones
7. **Utils**: Proporciona funciones de utilidad como temporizadores asíncronos

## Flujo de Ejecución a Nivel de Sistema

### 1. Inicialización del Sistema

```
[Arduino setup()] → [Hardware.init()] → [Utils.init()] → [Storage.init()] → [Sensors.init()] → [Actuators.init()] → [UIController.init()] → [ProgramController.init()]
```

Secuencia de eventos:

1. **Hardware.init()**: 
   - Configura los pines de entrada y salida
   - Inicializa la comunicación serial con la pantalla Nextion
   - Establece el estado inicial para el botón de emergencia

2. **Utils.init()**:
   - Inicializa el sistema de tareas temporizadas
   - Configura el temporizador principal

3. **Storage.init()**:
   - Inicia el sistema de almacenamiento persistente (ESP32 Preferences)
   - Valida la configuración almacenada
   - Carga los valores por defecto si es necesario

4. **Sensors.init()**:
   - Inicializa los sensores de temperatura (OneWire/Dallas)
   - Inicializa el sensor de presión (HX710B)
   - Establece la calibración inicial

5. **Actuators.init()**:
   - Establece los modos y estados iniciales de los actuadores
   - Configura los patrones de rotación automática

6. **UIController.init()**:
   - Configura los parámetros de la interfaz
   - Prepara los manejadores de eventos

7. **ProgramController.init()**:
   - Carga los datos de programa desde almacenamiento
   - Establece el estado inicial en ESTADO_SELECCION
   - Muestra la pantalla de bienvenida y luego la de selección

### 2. Bucle Principal (Loop)

El archivo `mainController.ino` contiene el siguiente flujo en su función `loop()`:

```cpp
void loop() {
  // Comprobar botón de emergencia primero (máxima prioridad)
  Hardware.updateDebouncer();
  if (Hardware.isEmergencyButtonPressed()) {
    ProgramController.handleEmergency();
    return;
  }
  
  // Actualizar temporizadores y tareas programadas
  Utils.updateTasks();
  
  // Leer datos de sensores
  Sensors.update();
  
  // Comprobar eventos de Nextion
  if (Hardware.nextionCheckForEvents()) {
    UIController.processNextionEvent(Hardware.nextionGetLastEvent());
  }
  
  // Actualizar controlador de programa (máquina de estados)
  ProgramController.update();
}
```

Este bucle se ejecuta continuamente y maneja:
- Verificación de emergencia con máxima prioridad
- Actualización de tareas temporizadas
- Lectura de sensores
- Procesamiento de eventos de la pantalla Nextion
- Actualización del controlador de programa

### 3. Selección de Programa

Secuencia de interacción:

1. **UIController** muestra la pantalla de selección con los programas disponibles
2. **Hardware** detecta interacción de usuario en la pantalla Nextion
3. **UIController** procesa el evento y lo comunica a **ProgramController**
4. **ProgramController** actualiza el programa seleccionado
5. **Storage** almacena la selección actual

### 4. Inicio de Programa

Cuando el usuario presiona el botón de inicio:

1. **UIController** detecta la acción y la envía a **ProgramController**
2. **ProgramController** cambia al estado ESTADO_EJECUCION
3. **ProgramController** llama a `_initializeProgram()` que:
   - Inicializa el temporizador para la fase actual
   - Prepara el programa seleccionado
4. **ProgramController** llama a `_configureActuatorsForPhase()` que:
   - Configura los actuadores según los parámetros de la fase
5. **Actuators.lockDoor()** bloquea la puerta
6. **UIController** muestra la pantalla de ejecución

### 5. Ejecución de Fase

Durante la ejecución de una fase, ocurre lo siguiente:

1. **ProgramController**:
   - Establece valores objetivo para nivel de agua y temperatura
   - Actualiza la interfaz con los parámetros actuales

2. **Actuators**:
   - Controla válvulas (agua, vapor, drenaje) según los objetivos
   - Gestiona el motor según el patrón de rotación configurado

3. **Sensors**:
   - Monitoriza continuamente temperatura y nivel de agua
   - Informa a ProgramController cuando se alcanzan condiciones objetivo

4. **UIController**:
   - Actualiza la pantalla con valores actuales
   - Muestra el progreso de la fase actual

5. **Utils**:
   - Gestiona los temporizadores para la fase actual
   - Proporciona mecanismo de tiempo sin bloqueo (evitando delay())

### 6. Secuencia de Control Detallada

Durante la ejecución de un programa, el flujo de control es el siguiente:

```
[loop()] → [Utils.updateTasks()] → [ProgramController.updateTimers()] → [_decrementTimer()] → [_checkSensorConditions()]
```

La función `_checkSensorConditions()` es especialmente importante:

```cpp
void ProgramControllerClass::_checkSensorConditions() {
  // Verificar condiciones de temperatura y nivel de agua
  uint8_t targetTemp = _temperatures[_currentProgram][_currentPhase];
  uint8_t targetLevel = _waterLevels[_currentProgram][_currentPhase];
  
  // Actualizar interfaz con valores actuales
  UIController.updateTemperature(Sensors.getCurrentTemperature());
  UIController.updateWaterLevel(Sensors.getCurrentWaterLevel());
  
  // Control de llenado de agua
  if (Sensors.getCurrentWaterLevel() < targetLevel) {
    if (!Actuators.isWaterValveOpen()) {
      Actuators.openWaterValve();
    }
  } else {
    if (Actuators.isWaterValveOpen()) {
      Actuators.closeWaterValve();
    }
  }
  
  // Control de temperatura
  if (Sensors.getCurrentTemperature() < targetTemp) {
    if (!Actuators.isSteamValveOpen()) {
      Actuators.openSteamValve();
    }
  } else {
    if (Actuators.isSteamValveOpen()) {
      Actuators.closeSteamValve();
    }
  }
  
  // Iniciar rotación cuando se alcanzan las condiciones
  uint8_t rotLevel = _rotations[_currentProgram][_currentPhase];
  if (Sensors.isWaterLevelReached(targetLevel) && 
      Sensors.isTemperatureReached(targetTemp) && 
      rotLevel > 0 && 
      !Actuators.isAutoRotationActive()) {
    Actuators.startAutoRotation(rotLevel);
    
    // Iniciar temporizador cuando se alcanzan condiciones
    if (!_timerRunning) {
      _timerRunning = true;
      Utils.debug("Condiciones alcanzadas, iniciando temporizador");
    }
  }
}
```

Este código muestra la lógica de toma de decisiones para control de actuadores y cómo el temporizador solo se inicia cuando se alcanzan las condiciones objetivo.

### 7. Cambio de Fase

Cuando se completa el tiempo de una fase:

1. **ProgramController._decrementTimer()** detecta que el tiempo llegó a cero
2. **ProgramController._completePhase()** es llamado
3. Los actuadores se detienen para la fase actual
4. Se verifica si es la última fase
5. Si no es la última fase, **ProgramController.nextPhase()** avanza a la siguiente fase
6. Se actualizan los parámetros para la nueva fase
7. **UIController** actualiza la pantalla para mostrar la nueva fase

### 8. Finalización de Programa

Cuando se completa la última fase:

1. **ProgramController._completeProgram()** es llamado
2. Los actuadores se llevan a un estado seguro:
   - **Actuators.stopAutoRotation()** detiene la rotación
   - **Actuators.closeSteamValve()** cierra válvula de vapor
   - **Actuators.closeWaterValve()** cierra válvula de agua
   - **Actuators.openDrainValve()** abre válvula de drenaje
3. **Storage.incrementUsageCounter()** incrementa el contador de uso
4. **Actuators.startBuzzer()** activa una señal acústica
5. **ProgramController.setState(ESTADO_SELECCION)** vuelve al estado de selección
6. **UIController** muestra la pantalla de selección

### 9. Gestión de Emergencias

Cuando se presiona el botón de emergencia:

1. **Hardware.isEmergencyButtonPressed()** detecta activación
2. **ProgramController.handleEmergency()** es llamado
3. **ProgramController** cambia al estado ESTADO_EMERGENCIA
4. **Actuators.emergencyStop()** detiene todos los actuadores inmediatamente:
   - Cierra válvulas de agua y vapor
   - Abre válvula de drenaje
   - Detiene el motor
5. **UIController** muestra la pantalla de emergencia

Para restablecer el sistema después de una emergencia:

1. El usuario debe presionar el botón de reinicio en la pantalla
2. **UIController** detecta esta acción y llama a **ProgramController.resetEmergency()**
3. **Actuators.emergencyReset()** restablece los actuadores
4. **ProgramController** vuelve al estado ESTADO_SELECCION

### 10. Utilización de Tareas Asíncronas

Un aspecto clave del sistema es el uso de tareas asíncronas en lugar de `delay()`:

```cpp
// Inicialización de Nextion
void HardwareClass::_initNextion() {
  // Inicializar comunicación serial con la pantalla
  NEXTION_SERIAL.begin(NEXTION_BAUD_RATE, SERIAL_8N1, NEXTION_RX_PIN, NEXTION_TX_PIN);
  
  // Programar tarea para completar inicialización
  Utils.createTimeout(500, completeNextionInitCallback);
  
  Serial.println("Iniciando pantalla Nextion...");
}
```

Este patrón de programación no bloqueante se usa en todo el sistema para:
- Control de tiempos de rotación del motor
- Monitoreo periódico de sensores
- Actualización de interfaz de usuario
- Temporización de ciclos de lavado

### 11. Diagrama de Flujo de Comunicación Entre Módulos

```
╔════════════════╗     ╔════════════════╗     ╔════════════════╗
║  UIController  ║     ║ ProgramControl ║     ║    Storage     ║
╚═══════╦════════╝     ╚══════╦═════════╝     ╚═══════╦════════╝
        ║                      ║                      ║
        ║  Eventos Usuario     ║  Cargar/Guardar      ║
        ║───────────────────▶ ║◀────────────────────▶║
        ║                      ║                      ║
        ║  Actualizar UI       ║                      ║
        ║◀──────────────────  ║                      ║
        ║                      ║                      ║
╔═══════╩════════╗      ╔══════╩═════════╗     ╔══════╩═════════╗
║    Hardware    ║      ║    Sensors     ║     ║   Actuators    ║
╚═══════╦════════╝      ╚══════╦═════════╝     ╚══════╦═════════╝
        ║                      ║                      ║
        ║  Lectura Botones     ║  Valores Sensores    ║  Comandos Control
        ║◀──────────────────▶ ║◀─────────────────────║◀──────────────────▶
        ║                      ║                      ║
        ║  Eventos Nextion     ║                      ║
        ║◀──────────────────▶ ║                      ║
        ▼                      ▼                      ▼
   [Componentes          [Sensores Físicos]     [Actuadores Físicos]
     Hardware]
```

## Transiciones de Estado

El sistema funciona como una máquina de estados con las siguientes transiciones principales:

```
┌────────────────┐
│  ESTADO_SELEC  │◄────────────────────────────┐
└───────┬────────┘                             │
        │ [Inicio]                             │
        ▼                                      │
┌────────────────┐                             │
│  ESTADO_EJECUC │◄────────┐                   │
└───────┬────────┘         │                   │
        │ [Pausa]          │ [Reanudar]        │ [Fin/Detener]
        ▼                  │                   │
┌────────────────┐         │                   │
│  ESTADO_PAUSA  ├─────────┘                   │
└───────┬────────┘                             │
        │ [Detener]                            │
        └──────────────────────────────────────┘
```

Con estados adicionales:

```
┌────────────────┐         ┌────────────────┐
│  ESTADO_EDICION│         │ ESTADO_EMERGEN │
└───────┬────────┘         └───────┬────────┘
        │                          │
        │ [Guardar/Cancelar]       │ [Reiniciar]
        ▼                          ▼
┌────────────────┐         ┌────────────────┐
│  ESTADO_SELEC  │         │  ESTADO_SELEC  │
└────────────────┘         └────────────────┘
```

## Manejo de Errores y Excepciones

El sistema implementa varias estrategias para el manejo de errores y condiciones excepcionales:

1. **Monitoreo continuo**: Los sensores son monitoreados constantemente para detectar anomalías.

2. **Sistema de reintentos**: Para operaciones críticas como la comunicación con la pantalla Nextion, se implementan mecanismos de reintento.

3. **Timeouts**: Todas las operaciones que requieren esperar una condición tienen timeouts para evitar bloqueos indefinidos.

4. **Validación de datos**: Los valores cargados desde almacenamiento son validados para asegurar que están dentro de rangos aceptables.

5. **Estados seguros**: Ante cualquier error, el sistema pasa a un estado seguro que protege el hardware y la seguridad del usuario.

6. **Gestión de errores específicos**:
   - Error de sensor de temperatura
   - Error de sensor de presión/nivel
   - Error de comunicación con Nextion
   - Error de bloqueo de puerta
   - Error de motor

## Optimización de Recursos

El diseño modular optimiza los recursos del ESP32 de la siguiente manera:

1. **Uso de memoria**: Se minimiza el uso de variables globales, y se utiliza la memoria de manera eficiente al compartir buffers entre operaciones.

2. **Eficiencia energética**: Los actuadores solo se activan cuando es necesario, reduciendo el consumo de energía.

3. **Procesamiento no bloqueante**: El uso de AsyncTaskLib permite que múltiples operaciones se ejecuten de manera concurrente sin bloquear el bucle principal.

4. **Cache de datos**: Los valores de configuración frecuentemente utilizados se mantienen en memoria RAM para reducir accesos a almacenamiento persistente.

## Depuración y Monitoreo

El sistema incluye mecanismos de depuración y monitoreo:

1. **Mensajes de depuración**: A través de la función `Utils.debug()` se envían mensajes detallados al puerto serial.

2. **Monitoreo de estado**: El estado actual de sensores y actuadores se puede revisar en tiempo real.

3. **Registro de eventos**: Las transiciones importantes se registran para análisis posterior.

4. **Panel de administración**: La pantalla Nextion puede mostrar información detallada del sistema para diagnóstico.

## Patrones de Diseño Implementados

El sistema utiliza varios patrones de diseño:

1. **Singleton**: Cada módulo tiene una única instancia global. 

2. **Observer**: ProgramController observa cambios en sensores y eventos de usuario.

3. **State**: El controlador implementa una máquina de estados completa con transiciones claras.

4. **Command**: Las acciones de los actuadores se encapsulan en comandos específicos.

5. **Facade**: Cada módulo presenta una interfaz clara y simple a otros módulos, ocultando su complejidad interna.

## Configuración y Personalización

El sistema permite configuración y personalización a varios niveles:

1. **Configuración de hardware**: A través de constantes en `config.h`, se pueden ajustar pines, velocidades de comunicación, etc.

2. **Personalización de programas**: Los parámetros de cada programa y fase pueden ser modificados.

3. **Ajuste de comportamiento**: Umbrales de detección, tiempos de respuesta y otros parámetros pueden ser afinados.

4. **Calibración de sensores**: Los sensores pueden ser calibrados para mejorar la precisión.

## Resumen de la Estructura de Datos

### Datos de Programa
```cpp
// Para cada programa y fase
uint8_t _waterLevels[NUM_PROGRAMAS][NUM_FASES];
uint8_t _temperatures[NUM_PROGRAMAS][NUM_FASES];
uint8_t _times[NUM_PROGRAMAS][NUM_FASES];
uint8_t _rotations[NUM_PROGRAMAS][NUM_FASES];
```

### Estados del Sistema
```cpp
#define ESTADO_SELECCION 0
#define ESTADO_EDICION 1
#define ESTADO_EJECUCION 2
#define ESTADO_PAUSA 3
#define ESTADO_ERROR 4
#define ESTADO_EMERGENCIA 5
```

### Variables de Estado
```cpp
uint8_t _currentState;
uint8_t _previousState;
uint8_t _currentProgram;
uint8_t _currentPhase;
uint8_t _remainingMinutes;
uint8_t _remainingSeconds;
```

## Conclusión

El controlador de lavadora industrial implementa un sistema robusto, modular y flexible que permite un control preciso de todas las fases del proceso de lavado. La arquitectura facilita el mantenimiento, extensión y personalización, mientras que el enfoque asíncrono garantiza una operación eficiente sin bloqueos.

El flujo de ejecución está cuidadosamente diseñado para garantizar un comportamiento predecible y seguro en todas las situaciones, desde la operación normal hasta la gestión de condiciones de emergencia.
