# Flujo Detallado de Ejecución del Controlador de Lavadora Industrial

## Descripción Técnica

Este documento proporciona una descripción técnica detallada del flujo de ejecución del controlador modular para lavadora industrial. Se presenta la interacción entre los diferentes módulos del sistema y los pasos específicos que ocurren durante la ejecución de los programas 22, 23 y 24.

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

1. **UIController** muestra la pantalla de selección con los programas disponibles (22, 23 y 24)
2. **Hardware** detecta interacción de usuario en la pantalla Nextion
3. **UIController** procesa el evento y lo comunica a **ProgramController**
4. **ProgramController** actualiza el programa seleccionado
5. **Storage** almacena la selección actual

#### Configuración específica según programa:

- **Programa 22 (Agua Caliente)**:
  - Se cargan parámetros predeterminados para agua caliente
  - La interfaz muestra opciones de configuración estándar (nivel, tiempo, rotación, centrifugado)

- **Programa 23 (Agua Fría)**:
  - Se cargan parámetros predeterminados para agua fría
  - La interfaz muestra opciones de configuración estándar (nivel, tiempo, rotación, centrifugado)

- **Programa 24 (Configurable)**:
  - Se cargan parámetros básicos
  - La interfaz solicita selección adicional del tipo de agua (caliente/fría)
  - Según la selección, se configura el modo de gestión de temperatura
  - La interfaz muestra configuración para número de tandas

### 4. Inicio de Programa

Cuando el usuario presiona el botón de inicio:

1. **UIController** detecta la acción y la envía a **ProgramController**
2. **ProgramController** cambia al estado ESTADO_EJECUCION
3. **ProgramController** llama a `_initializeProgram()` que:
   - Inicializa variables del programa específico
   - Prepara contadores para tandas (en caso del Programa 24)
4. **ProgramController** configura el tipo de agua y comportamiento según el programa:
   ```cpp
   // Configuración específica según programa
   switch(_currentProgram) {
     case 22: // Programa agua caliente
       _useHotWater = true;
       _enableTemperatureControl = true;
       _numberOfCycles = 1;
       break;
     case 23: // Programa agua fría
       _useHotWater = false;
       _enableTemperatureControl = false;
       _numberOfCycles = 1;
       break;
     case 24: // Programa configurable
       _useHotWater = Storage.getWaterTypeSelection();
       _enableTemperatureControl = _useHotWater;
       _numberOfCycles = 4; // 4 tandas por defecto
       _currentCycle = 0;
       break;
   }
   ```
5. **Actuators.lockDoor()** bloquea la puerta
6. **UIController** muestra la pantalla de ejecución

### 5. Ejecución de Programa según Tipo

#### 5.1 Programa 22 (Agua Caliente)

Secuencia de fases:

1. **Fase de Llenado**:
   ```cpp
   void ProgramControllerClass::_executeFilling() {
     // Abrir válvula de agua caliente
     if (_useHotWater) {
       Actuators.openHotWaterValve();
     }
     
     // Monitorear nivel y temperatura
     if (Sensors.getCurrentWaterLevel() >= _targetWaterLevel) {
       Actuators.closeHotWaterValve();
       _moveToNextPhase();
     } else if (_enableTemperatureControl) {
       // Control activo de temperatura
       if (Sensors.getCurrentTemperature() < (_targetTemperature - 2)) {
         // Temperatura muy baja, drenar parcialmente y agregar agua caliente
         _manageLowTemperature();
       }
     }
   }
   ```

2. **Fase de Lavado**:
   ```cpp
   void ProgramControllerClass::_executeWashing() {
     // Iniciar rotación del motor si no está activa
     if (!Actuators.isMotorRunning()) {
       Actuators.startMotor(_rotationLevel);
     }
     
     // Gestión activa de temperatura
     if (_enableTemperatureControl) {
       if (Sensors.getCurrentTemperature() < (_targetTemperature - 2)) {
         _manageLowTemperature();
       }
     }
     
     // Verificar tiempo
     if (_phaseTimer <= 0) {
       _moveToNextPhase();
     }
   }
   
   void ProgramControllerClass::_manageLowTemperature() {
     // Drenar parcialmente
     Actuators.openDrainValve();
     Utils.createTimeout(TIEMPO_DRENAJE_PARCIAL, [this]() {
       Actuators.closeDrainValve();
       
       // Agregar agua caliente nueva
       Actuators.openHotWaterValve();
       Utils.createTimeout(TIEMPO_LLENADO_PARCIAL, [this]() {
         Actuators.closeHotWaterValve();
       });
     });
   }
   ```

3. **Fase de Drenaje**:
   ```cpp
   void ProgramControllerClass::_executeDraining() {
     // Abrir válvula de drenaje
     Actuators.openDrainValve();
     
     // Verificar si el drenaje está completo
     if (Sensors.getCurrentWaterLevel() <= 0) {
       Actuators.closeDrainValve();
       _moveToNextPhase();
     }
   }
   ```

4. **Fase de Centrifugado (opcional)**:
   ```cpp
   void ProgramControllerClass::_executeCentrifuge() {
     // Activar modo centrifugado
     if (!Actuators.isCentrifugeActive()) {
       Actuators.startCentrifuge(_centrifugeLevel);
     }
     
     // Verificar tiempo
     if (_phaseTimer <= 0) {
       Actuators.stopCentrifuge();
       _completeProgram();
     }
   }
   ```

#### 5.2 Programa 23 (Agua Fría)

Secuencia similar al Programa 22, pero con estas diferencias clave:

1. **Fase de Llenado**:
   ```cpp
   void ProgramControllerClass::_executeFilling() {
     // Abrir válvula de agua fría
     if (!_useHotWater) {
       Actuators.openColdWaterValve();
     }
     
     // Monitorear nivel (sin control de temperatura)
     if (Sensors.getCurrentWaterLevel() >= _targetWaterLevel) {
       Actuators.closeColdWaterValve();
       _moveToNextPhase();
     }
     
     // Actualizar info de temperatura solo para mostrarla (informativo)
     if (!_enableTemperatureControl) {
       UIController.updateTemperature(Sensors.getCurrentTemperature());
     }
   }
   ```

2. **Fase de Lavado**:
   - Sin gestión activa de temperatura, solo monitoreo informativo
   - El resto del proceso (motor y temporizador) es idéntico

3. **Fase de Drenaje**:
   - Proceso idéntico al Programa 22

4. **Fase de Centrifugado (opcional)**:
   - Proceso idéntico al Programa 22

#### 5.3 Programa 24 (Configurable con Múltiples Tandas)

Este programa combina características de los Programas 22 y 23, añadiendo control de múltiples tandas:

```cpp
void ProgramControllerClass::_moveToNextPhase() {
  _currentPhase++;
  
  // Verificar fin de ciclo (3 fases por ciclo: llenado, lavado, drenaje)
  if (_currentPhase > 2) { // 0: Llenado, 1: Lavado, 2: Drenaje
    _currentPhase = 0;
    
    // Si es Programa 24, verificar si hay más tandas
    if (_currentProgram == 24) {
      _currentCycle++;
      UIController.updateCycleDisplay(_currentCycle);
      
      // Verificar si hemos terminado todas las tandas
      if (_currentCycle >= _numberOfCycles) {
        // Verificar si el centrifugado está habilitado
        if (_centrifugeEnabled) {
          _currentPhase = 3; // Fase de centrifugado
          _executePhase();
        } else {
          _completeProgram();
        }
        return;
      }
    } else {
      // Para programas 22 y 23, verificar si hay fase de centrifugado
      if (_centrifugeEnabled) {
        _currentPhase = 3; // Fase de centrifugado
      } else {
        _completeProgram();
        return;
      }
    }
  }
  
  // Configurar e iniciar la nueva fase
  _setupPhase();
  _executePhase();
}
```

La ejecución de las fases individuales (llenado, lavado, drenaje) del Programa 24 depende de la configuración de tipo de agua:
- Si se configuró agua caliente, se comporta como el Programa 22 con gestión activa de temperatura
- Si se configuró agua fría, se comporta como el Programa 23 sin gestión de temperatura

### 6. Gestión de Temperatura en Detalle

Para los programas que utilizan agua caliente (Programa 22 y Programa 24 con agua caliente seleccionada), la gestión de temperatura es un proceso clave:

```cpp
void ProgramControllerClass::_monitorAndControlTemperature() {
  // Solo aplica a programas con control de temperatura activo
  if (!_enableTemperatureControl) return;
  
  float currentTemp = Sensors.getCurrentTemperature();
  float targetTemp = _targetTemperature;
  
  UIController.updateTemperature(currentTemp);
  
  // Verificar si la temperatura está por debajo del límite (target - 2°C)
  if (currentTemp < (targetTemp - 2.0)) {
    Utils.debug("Temperatura baja detectada: " + String(currentTemp) + "°C (objetivo: " + String(targetTemp) + "°C)");
    
    // Si no estamos ya en proceso de ajuste
    if (!_temperatureAdjustmentInProgress) {
      _temperatureAdjustmentInProgress = true;
      
      // Notificar en UI
      UIController.showMessage("Ajustando temperatura...");
      
      // Detener motor temporalmente si está en fase de lavado
      if (_currentPhase == 1) {
        Actuators.pauseMotor();
      }
      
      // Iniciar secuencia de drenaje parcial
      Actuators.openDrainValve();
      Utils.createTimeout(TIEMPO_DRENAJE_PARCIAL, [this]() {
        // Cerrar válvula de drenaje
        Actuators.closeDrainValve();
        
        // Abrir válvula de agua caliente para reponer con agua más caliente
        Actuators.openHotWaterValve();
        Utils.createTimeout(TIEMPO_LLENADO_PARCIAL, [this]() {
          // Cerrar válvula de agua
          Actuators.closeHotWaterValve();
          
          // Restaurar motor si estaba en fase de lavado
          if (_currentPhase == 1) {
            Actuators.resumeMotor();
          }
          
          // Fin de ajuste
          _temperatureAdjustmentInProgress = false;
          UIController.clearMessage();
        });
      });
    }
  }
}
```

Este proceso garantiza que:
1. La temperatura se mantiene dentro de un rango determinado (no más de 2°C por debajo del objetivo)
2. Cuando la temperatura baja demasiado, se drena parcialmente y se repone con agua caliente nueva
3. Se realiza una notificación en la interfaz de usuario durante este proceso
4. El motor se pausa temporalmente si el ajuste ocurre durante la fase de lavado

### 7. Control de Ciclos Múltiples (Programa 24)

El Programa 24 implementa una gestión especializada para manejar las 4 tandas:

```cpp
void ProgramControllerClass::_manageCycles() {
  // Solo aplica al Programa 24
  if (_currentProgram != 24) return;
  
  // Actualizar información en pantalla
  UIController.updateCycleInfo(_currentCycle, _numberOfCycles);
  
  // Verificar si hemos completado todas las tandas
  if (_currentCycle >= _numberOfCycles) {
    Utils.debug("Todas las tandas completadas");
    
    // Verificar si centrifugado está habilitado
    if (_centrifugeEnabled) {
      Utils.debug("Iniciando fase de centrifugado final");
      _currentPhase = 3;
      _setupPhase();
      _executePhase();
    } else {
      _completeProgram();
    }
  }
}
```

En cada tanda, el programa ejecuta:
1. Fase de llenado
2. Fase de lavado
3. Fase de drenaje

Al completar las 4 tandas, el programa verifica si el centrifugado está habilitado y procede en consecuencia.

### 8. Finalización de Programa

Cuando un programa se completa:

```cpp
void ProgramControllerClass::_completeProgram() {
  Utils.debug("Programa completado");
  
  // Detener todos los actuadores
  Actuators.stopAllSystems();
  
  // Actualizar estadísticas
  Storage.incrementProgramUsage(_currentProgram);
  
  // Desbloquear puerta
  Actuators.unlockDoor();
  
  // Notificar fin de programa
  UIController.showProgramComplete();
  Actuators.beep(3);
  
  // Volver a estado de selección
  _setState(ESTADO_SELECCION);
  
  // Mostrar pantalla de selección después de un tiempo
  Utils.createTimeout(5000, [this]() {
    UIController.showSelectionScreen();
  });
}
```

### 9. Gestión de Emergencias

Cuando se detecta una emergencia:

```cpp
void ProgramControllerClass::handleEmergency() {
  Utils.debug("¡EMERGENCIA DETECTADA!");
  
  // Cambiar a estado de emergencia
  _setState(ESTADO_EMERGENCIA);
  
  // Detener todos los sistemas
  Actuators.emergencyStop();
  
  // Mostrar pantalla de emergencia
  UIController.showEmergencyScreen();
  
  // Activar alarma
  Actuators.beep(5);
}

void ActuatorsClass::emergencyStop() {
  // Detener motor inmediatamente
  stopMotor();
  
  // Cerrar válvulas de entrada de agua
  closeWaterValve();
  closeSteamValve();
  
  // Abrir válvula de drenaje para vaciar
  openDrainValve();
  
  // Desbloquear puerta después de un tiempo prudencial
  Utils.createTimeout(EMERGENCY_DOOR_UNLOCK_DELAY, [this]() {
    unlockDoor();
  });
}
```

### 10. Utilización de Tareas Asíncronas

Un aspecto clave del sistema es el uso de tareas asíncronas en lugar de `delay()`:

```cpp
// Gestión de temperatura sin bloquear el sistema
void ProgramControllerClass::_manageLowTemperature() {
  // Notificar inicio del proceso
  _temperatureAdjustmentInProgress = true;
  UIController.showMessage("Ajustando temperatura...");
  
  // Paso 1: Drenar parcialmente
  Actuators.openDrainValve();
  Utils.createTimeout(TIEMPO_DRENAJE_PARCIAL, [this]() {
    // Paso 2: Cerrar drenaje y abrir entrada agua caliente
    Actuators.closeDrainValve();
    Actuators.openHotWaterValve();
    
    // Programar cierre de agua después del tiempo de llenado
    Utils.createTimeout(TIEMPO_LLENADO_PARCIAL, [this]() {
      Actuators.closeHotWaterValve();
      _temperatureAdjustmentInProgress = false;
      UIController.clearMessage();
    });
  });
}
```

Este patrón permite que el sistema:
1. Continúe monitoreando botones y sensores durante operaciones que toman tiempo
2. Pueda responder inmediatamente a una emergencia en cualquier momento
3. Realice múltiples operaciones en "paralelo"
4. Mantenga la interfaz de usuario actualizada de forma continua

### 11. Comunicación y Estructura de Datos

#### Configuración de Programa

Cada programa (22, 23, 24) tiene su propia configuración almacenada:

```cpp
// Estructura para almacenar configuración específica de programa
struct ProgramConfig {
  uint8_t waterLevel;
  uint8_t temperature;
  uint8_t washTime;
  uint8_t rotationLevel;
  bool centrifugeEnabled;
  uint8_t centrifugeLevel;
  uint8_t centrifugeTime;
  
  // Solo para Programa 24
  bool useHotWater;
  uint8_t numberOfCycles;
};

// Acceso y almacenamiento de configuración
void StorageClass::saveProgramConfig(uint8_t programId, ProgramConfig config) {
  String prefix = "prog" + String(programId) + "_";
  
  _preferences.putUChar(String(prefix + "wl").c_str(), config.waterLevel);
  _preferences.putUChar(String(prefix + "temp").c_str(), config.temperature);
  // ...otros parámetros...
  
  if (programId == 24) {
    _preferences.putBool(String(prefix + "hot").c_str(), config.useHotWater);
    _preferences.putUChar(String(prefix + "cycles").c_str(), config.numberOfCycles);
  }
}
```

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

5. **Strategy**: El comportamiento del sistema varía según el programa seleccionado.

## Resumen de Estados del Sistema

El sistema opera bajo una máquina de estados con los siguientes estados principales:

```cpp
#define ESTADO_SELECCION 0
#define ESTADO_EDICION 1
#define ESTADO_EJECUCION 2
#define ESTADO_PAUSA 3
#define ESTADO_ERROR 4
#define ESTADO_EMERGENCIA 5
```

La lógica de cambio de estados está centralizada en el método `_setState()`:

```cpp
void ProgramControllerClass::_setState(uint8_t newState) {
  _previousState = _currentState;
  _currentState = newState;
  
  Utils.debug("Cambio de estado: " + String(_previousState) + " -> " + String(_currentState));
  
  // Acciones específicas según el nuevo estado
  switch(newState) {
    case ESTADO_SELECCION:
      _resetVariables();
      break;
    case ESTADO_EJECUCION:
      Actuators.lockDoor();
      break;
    case ESTADO_EMERGENCIA:
      // La lógica principal está en handleEmergency()
      break;
    // Otros casos...
  }
}
```

## Conclusión

El controlador de lavadora industrial implementa un sistema robusto, modular y flexible que permite un control preciso de tres programas especializados:

1. **Programa 22**: Lavado con agua caliente y gestión activa de temperatura
2. **Programa 23**: Lavado con agua fría sin gestión de temperatura
3. **Programa 24**: Lavado configurable con múltiples tandas

La arquitectura facilita el mantenimiento, extensión y personalización, mientras que el enfoque asíncrono garantiza una operación eficiente sin bloqueos. El flujo de ejecución está cuidadosamente diseñado para garantizar un comportamiento predecible y seguro en todas las situaciones, desde la operación normal hasta la gestión de condiciones de emergencia.
