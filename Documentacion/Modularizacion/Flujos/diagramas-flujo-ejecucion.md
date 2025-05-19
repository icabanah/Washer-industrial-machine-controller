# Diagramas de Flujo del Controlador de Lavadora Industrial

Este documento proporciona representaciones visuales del flujo de ejecución del controlador de lavadora industrial mediante diagramas. Estos diagramas complementan la documentación detallada y proporcionan una vista general de la estructura y funcionamiento del sistema.

## Diagrama de Módulos del Sistema

```mermaid
graph TD
    A[Hardware] --> B[Sensores]
    A --> C[Actuadores]
    A --> D[Pantalla Nextion]
    E[Utils] --> F[Temporizadores]
    E --> G[Depuración]
    H[Storage] --> I[Preferencias ESP32]
    J[UIController] --> D
    K[ProgramController] --> B
    K --> C
    K --> J
    K --> H
    K --> E
    L[mainController.ino] --> A
    L --> E
    L --> H
    L --> B
    L --> C
    L --> J
    L --> K
    
    style L fill:#f96,stroke:#333,stroke-width:2px
    style K fill:#6af,stroke:#333,stroke-width:2px
    style J fill:#6fa,stroke:#333,stroke-width:2px
```

## Diagrama de Máquina de Estados

```mermaid
stateDiagram-v2
    [*] --> SELECCION
    
    SELECCION --> EDICION: Botón Editar
    EDICION --> SELECCION: Guardar/Cancelar
    
    SELECCION --> EJECUCION: Botón Inicio
    EJECUCION --> PAUSA: Botón Pausa
    PAUSA --> EJECUCION: Botón Reanudar
    PAUSA --> SELECCION: Botón Detener
    EJECUCION --> SELECCION: Completado/Detener
    
    state EJECUCION {
        [*] --> Fase0
        Fase0 --> Fase1: Tiempo completado
        Fase1 --> Fase2: Tiempo completado
        Fase2 --> Fase3: Tiempo completado
        Fase3 --> [*]: Tiempo completado
    }
    
    state EMERGENCIA {
        [*] --> DetenerTodo
        DetenerTodo --> AbrirDrenaje
        AbrirDrenaje --> MostrarPantalla
        MostrarPantalla --> [*]
    }
    
    EJECUCION --> EMERGENCIA: Botón Emergencia
    PAUSA --> EMERGENCIA: Botón Emergencia
    SELECCION --> EMERGENCIA: Botón Emergencia
    EDICION --> EMERGENCIA: Botón Emergencia
    
    EMERGENCIA --> SELECCION: Reinicio Manual
```

## Diagrama de Secuencia de Inicio de Programa

```mermaid
sequenceDiagram
    actor Usuario
    participant UI as UIController
    participant Program as ProgramController
    participant Actuators
    participant Sensors
    participant Storage
    
    Usuario->>UI: Seleccionar Programa
    UI->>Program: selectProgram(n)
    Program->>Storage: saveProgram(n)
    Usuario->>UI: Iniciar Programa
    UI->>Program: startProgram()
    Program->>Program: setState(ESTADO_EJECUCION)
    Program->>Program: _initializeProgram()
    Program->>Program: _configureActuatorsForPhase()
    Program->>Actuators: lockDoor()
    Program->>UI: showExecutionScreen()
    loop Ejecución de Fase
        Program->>Sensors: getCurrentTemperature()
        Program->>Sensors: getCurrentWaterLevel()
        Program->>UI: updateTemperature()
        Program->>UI: updateWaterLevel()
        Program->>Actuators: control de válvulas y motor
        Program->>Program: _decrementTimer()
        Program->>UI: updateTime()
        Program->>UI: updateProgressBar()
    end
    Program->>Program: _completePhase()
    Program->>Program: nextPhase()
```

## Diagrama de Flujo de Ejecución de Fase

```mermaid
flowchart TD
    A[Inicio de Fase] --> B{Nivel de agua alcanzado?}
    B -->|No| C[Abrir Válvula de Agua]
    B -->|Sí| D[Cerrar Válvula de Agua]
    
    C --> E{Temperatura alcanzada?}
    D --> E
    
    E -->|No| F[Abrir Válvula de Vapor]
    E -->|Sí| G[Cerrar Válvula de Vapor]
    
    F --> H{Condiciones Alcanzadas?}
    G --> H
    
    H -->|No| I[Esperar]
    H -->|Sí| J[Iniciar Rotación Automática]
    
    I --> B
    J --> K[Iniciar Temporizador de Fase]
    
    K --> L[Decrementar Temporizador]
    L --> M{Tiempo = 0?}
    M -->|No| L
    M -->|Sí| N[Finalizar Fase]
    
    N --> O{¿Última Fase?}
    O -->|No| P[Siguiente Fase]
    O -->|Sí| Q[Finalizar Programa]
```

## Diagrama de Componentes de Hardware

```mermaid
graph LR
    A[ESP32-WROOM] --- B[Botón Emergencia]
    A --- C[Sensores]
    A --- D[Actuadores]
    A --- E[Pantalla Nextion]
    
    subgraph Sensores
        C1[Sensor Temperatura]
        C2[Sensor Presión]
    end
    
    subgraph Actuadores
        D1[Motor Bidireccional]
        D2[Válvula Agua]
        D3[Válvula Vapor]
        D4[Válvula Drenaje]
        D5[Bloqueo Puerta]
    end
```

## Diagrama de Tiempos de Ejecución

```mermaid
gantt
    title Programa Normal (Estándar)
    dateFormat s
    axisFormat %M:%S
    
    section Fase 0: Prelavado
    Llenado de agua       :a1, 0, 60s
    Calentamiento         :a2, after a1, 120s
    Rotación y Lavado     :a3, after a2, 300s
    
    section Fase 1: Lavado Principal
    Llenado de agua       :b1, after a3, 60s
    Calentamiento         :b2, after b1, 180s
    Rotación y Lavado     :b3, after b2, 600s
    
    section Fase 2: Enjuague
    Drenaje               :c1, after b3, 60s
    Llenado de agua       :c2, after c1, 60s
    Rotación y Enjuague   :c3, after c2, 300s
    
    section Fase 3: Centrifugado
    Drenaje               :d1, after c3, 60s
    Centrifugado          :d2, after d1, 180s
```

## Diagrama de Secuencia para Manejo de Emergencia

```mermaid
sequenceDiagram
    actor Operador
    participant Hardware
    participant Program as ProgramController
    participant Actuators
    participant UI as UIController
    
    Operador->>Hardware: Presionar Botón Emergencia
    activate Hardware
    Hardware->>Program: handleEmergency()
    activate Program
    Program->>Program: setState(ESTADO_EMERGENCIA)
    Program->>Actuators: emergencyStop()
    activate Actuators
    Actuators->>Actuators: stopMotor()
    Actuators->>Actuators: closeWaterValve()
    Actuators->>Actuators: closeSteamValve()
    Actuators->>Actuators: openDrainValve()
    Actuators->>Actuators: unlockDoor()
    Actuators-->>Program: completado
    deactivate Actuators
    Program->>UI: showEmergencyScreen()
    activate UI
    UI-->>Program: completado
    deactivate UI
    Program-->>Hardware: completado
    deactivate Program
    Hardware-->>Operador: Sistema en estado emergencia
    deactivate Hardware
    
    Operador->>UI: Presionar Reiniciar
    activate UI
    UI->>Program: resetEmergency()
    activate Program
    Program->>Actuators: emergencyReset()
    activate Actuators
    Actuators-->>Program: completado
    deactivate Actuators
    Program->>Program: setState(ESTADO_SELECCION)
    Program-->>UI: completado
    deactivate Program
    UI->>UI: showSelectionScreen()
    UI-->>Operador: Sistema reiniciado
    deactivate UI
```

## Diagrama de Interacción de Módulos

```mermaid
graph TB
    A[Interfaz de Usuario] -->|"Eventos (touch, botones)"| B[UIController]
    B -->|"Comandos de visualización"| A
    
    B -->|"Eventos interpretados"| C[ProgramController]
    C -->|"Actualización de UI"| B
    
    C -->|"Lectura de valores"| D[Sensors]
    D -->|"Lecturas de sensores"| C
    
    C -->|"Comandos de control"| E[Actuators]
    
    C -->|"Leer/Guardar configuración"| F[Storage]
    
    G[Hardware] -->|"Señales físicas"| D
    G -->|"Control de actuadores"| E
    
    H[Utils] -->|"Temporizadores"| C
    H -->|"Temporizadores"| E
    
    subgraph "Capa de Hardware"
        G
    end
    
    subgraph "Capa de Control"
        D
        E
        F
        H
    end
    
    subgraph "Capa de Lógica"
        C
    end
    
    subgraph "Capa de Presentación"
        B
        A
    end
```

## Diagrama de Flujo de la Función Temporizadora

```mermaid
flowchart LR
    A[Inicio] --> B{Temporizador Activo?}
    B -->|No| Z[Fin]
    B -->|Sí| C[Decrementar Temporizador]
    C --> D{Tiempo Agotado?}
    D -->|No| E[Actualizar UI]
    D -->|Sí| F[Completar Fase]
    E --> Z
    F --> G{¿Última Fase?}
    G -->|No| H[Avanzar a Siguiente Fase]
    G -->|Sí| I[Completar Programa]
    H --> Z
    I --> Z
```

## Diagrama de Estructura de Datos

```mermaid
classDiagram
    class ProgramControllerClass {
        -uint8_t _currentState
        -uint8_t _previousState
        -uint8_t _currentProgram
        -uint8_t _currentPhase
        -uint8_t _remainingMinutes
        -uint8_t _remainingSeconds
        -uint8_t _waterLevels[3][4]
        -uint8_t _temperatures[3][4]
        -uint8_t _times[3][4]
        -uint8_t _rotations[3][4]
        +void init()
        +void setState(uint8_t)
        +void selectProgram(uint8_t)
        +void startProgram()
        +void pauseProgram()
        +void resumeProgram()
        +void stopProgram()
        +void nextPhase()
        +void updateTimers()
    }
    
    class StorageClass {
        -bool _initialized
        -Preferences _preferences
        +void init()
        +uint8_t readByte(const char*, uint8_t)
        +void writeByte(const char*, uint8_t)
        +void saveProgram(uint8_t)
        +uint8_t loadProgram()
        +void saveWaterLevel(uint8_t, uint8_t, uint8_t)
        +uint8_t loadWaterLevel(uint8_t, uint8_t)
    }
    
    class UtilsClass {
        -bool _mainTimerRunning
        -TimedTask _tasks[10]
        +void init()
        +int createTimeout(unsigned long, TaskCallback)
        +int createInterval(unsigned long, TaskCallback, bool)
        +void updateTasks()
        +void debug(const String&)
    }
    
    class TimedTask {
        +unsigned long interval
        +unsigned long lastExecuted
        +TaskCallback callback
        +bool recurring
        +bool active
        +int id
    }
    
    UtilsClass "1" -- "0..*" TimedTask
    ProgramControllerClass "1" -- "1" StorageClass
```

## Diagrama de Ciclo de Vida de una Operación de Lavado

```mermaid
graph TB
    subgraph "Preparación"
        A[Selección de Programa] --> B[Configuración de Parámetros]
        B --> C[Inicio de Programa]
    end
    
    subgraph "Ejecución"
        C --> D[Fase 0: Prelavado]
        D --> E[Fase 1: Lavado Principal]
        E --> F[Fase 2: Enjuague]
        F --> G[Fase 3: Centrifugado]
    end
    
    subgraph "Finalización"
        G --> H[Drenaje Final]
        H --> I[Desbloqueo de Puerta]
        I --> J[Señal Acústica]
        J --> K[Retorno a Pantalla Inicial]
    end
    
    subgraph "Eventos Excepcionales"
        L[Botón Emergencia] -.-> M[Detención Inmediata]
        N[Pausar Programa] -.-> O[Suspensión Temporal]
        O -.-> P[Reanudar Programa]
        O -.-> Q[Cancelar Programa]
        M -.-> R[Reinicio Manual]
    end
    
    D -.-> N
    E -.-> N
    F -.-> N
    G -.-> N
    
    D -.-> L
    E -.-> L
    F -.-> L
    G -.-> L
    
    P --> D
    P --> E
    P --> F
    P --> G
    
    Q --> A
    R --> A
```

## Diagrama de Conexiones Físicas del ESP32

```mermaid
graph TD
    ESP[ESP32-WROOM] --- PIN15[Pin 15 - Botón Emergencia]
    ESP --- PIN12[Pin 12 - Motor Dir A]
    ESP --- PIN14[Pin 14 - Motor Dir B]
    ESP --- PIN27[Pin 27 - Centrifugado]
    ESP --- PIN26[Pin 26 - Válvula Agua]
    ESP --- PIN33[Pin 33 - Electroválvula Vapor]
    ESP --- PIN13[Pin 13 - Válvula Desfogue]
    ESP --- PIN25[Pin 25 - Bloqueo Electromagnético]
    
    ESP --- PINS5[Pin 5 - Sensor Presión DOUT]
    ESP --- PINS4[Pin 4 - Sensor Presión SCLK]
    ESP --- PIN23[Pin 23 - Sensor Temperatura OneWire]
    
    ESP --- SERIALTX[Serial2 TX - Nextion RX]
    ESP --- SERIALRX[Serial2 RX - Nextion TX]
    
    subgraph "Control de Motor"
        PIN12
        PIN14
        PIN27
    end
    
    subgraph "Válvulas"
        PIN26
        PIN33
        PIN13
    end
    
    subgraph "Seguridad"
        PIN15
        PIN25
    end
    
    subgraph "Sensores"
        PINS5
        PINS4
        PIN23
    end
    
    subgraph "Interfaz"
        SERIALTX
        SERIALRX
    end
```

## Diagrama de Pantallas Nextion

```mermaid
flowchart TB
    A[Pantalla Bienvenida] --> B[Pantalla Selección]
    
    B -->|Botón Editar| C[Pantalla Edición]
    B -->|Botón Inicio| D[Pantalla Ejecución]
    
    C -->|Guardar| B
    C -->|Cancelar| B
    
    D -->|Pausar| E[Pantalla Pausa]
    D -->|Completar| B  
    
    E -->|Reanudar| D
    E -->|Detener| B
    
    subgraph "Emergencia"
        F[Pantalla Emergencia]
    end
    
    D -.->|Botón Emergencia| F
    E -.->|Botón Emergencia| F
    B -.->|Botón Emergencia| F
    C -.->|Botón Emergencia| F
    
    F -.->|Reiniciar| B
    
    subgraph "Error"
        G[Pantalla Error]
    end
    
    D -.->|Error detectado| G
    G -.->|Aceptar| B
```

Esta documentación visual ofrece una perspectiva integral de la estructura, interacción y flujo de ejecución del sistema de control de la lavadora industrial. Los diagramas facilitan la comprensión del diseño modular implementado y sirven como referencia para el mantenimiento y futuras ampliaciones del sistema.
