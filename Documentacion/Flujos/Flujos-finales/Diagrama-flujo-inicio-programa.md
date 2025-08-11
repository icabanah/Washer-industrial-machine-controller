# Diagrama de Flujo - Proceso de Inicio de Programa

## 📊 Vista General del Flujo Actual

```mermaid
graph TD
    A[Usuario presiona BOTÓN INICIAR] --> B{Hardware.nextionCheckForEvents()}
    
    B -->|Sin eventos| C[Fin - Sin procesamiento]
    B -->|Evento detectado| D[ProgramController.processUserEvent()]
    
    D --> E{Hardware.hasValidTouchEvent()}
    E -->|No válido| F[FALLO 1: Evento no válido]
    E -->|Válido| G[Extraer datos del evento]
    
    G --> H[touchPage, touchComponent, touchType]
    H --> I{touchType == 1?}
    I -->|No| J[FALLO 2: Tipo incorrecto]
    I -->|Sí| K{touchPage == NEXTION_PAGE_SELECTION?}
    
    K -->|No| L[FALLO 3: Página incorrecta]
    K -->|Sí| M[_handleSelectionPageEvents()]
    
    M --> N{componentId == NEXTION_ID_BTN_START?}
    N -->|No| O[Otros botones P22/P23/P24]
    N -->|Sí| P{Sensors.isDoorClosed()}
    
    P -->|Abierta| Q[Actuators.lockDoor()]
    Q --> R[Actualizar botón a INICIAR]
    
    P -->|Cerrada| S[startProgram()]
    
    S --> T{_currentState == ESTADO_SELECCION?}
    T -->|No| U[FALLO 4: Estado incorrecto]
    T -->|Sí| V[Configurar P24 tanda]
    
    V --> W[setState(ESTADO_EJECUCION)]
    W --> X[_initializeProgram()]
    X --> Y[_configureActuatorsForPhase()]
    Y --> Z[UIController.showExecutionScreen()]
    Z --> AA[_timerRunning = false, _preparingPhase = true]
    AA --> BB[PROGRAMA INICIADO]
    
    style A fill:#e1f5fe
    style BB fill:#c8e6c9
    style F fill:#ffcdd2
    style J fill:#ffcdd2
    style L fill:#ffcdd2
    style U fill:#ffcdd2
```

## 🔄 Flujo Detallado por Módulos

### 1️⃣ **Captura del Evento (mainController.ino)**
```
loop() 
├── Hardware.nextionCheckForEvents() [CADA CICLO]
├── Hardware.nextionGetLastEvent()
└── ProgramController.processUserEvent(event)
```

### 2️⃣ **Procesamiento del Evento (program_controller.cpp)**
```
processUserEvent()
├── Hardware.hasValidTouchEvent() [VALIDACIÓN 1]
├── Hardware.getTouchEventPage() [EXTRACCIÓN]
├── Hardware.getTouchEventComponent()
├── Hardware.getTouchEventType()
├── touchType == 1 [VALIDACIÓN 2]
└── switch(touchPage) [ROUTING]
```

### 3️⃣ **Manejo de Página de Selección**
```
_handleSelectionPageEvents()
├── componentId == NEXTION_ID_BTN_START [VALIDACIÓN 3]
├── Sensors.isDoorClosed() [VERIFICACIÓN PUERTA]
├── Actuators.lockDoor() [SI ABIERTA]
└── startProgram() [SI CERRADA]
```

### 4️⃣ **Inicio del Programa**
```
startProgram()
├── _currentState == ESTADO_SELECCION [VALIDACIÓN 4]
├── _tandaCounter = 0 [P24]
├── setState(ESTADO_EJECUCION) [CAMBIO ESTADO]
├── _initializeProgram() [CONFIGURACIÓN]
├── _configureActuatorsForPhase() [ACTUADORES]
├── UIController.showExecutionScreen() [UI]
└── _preparingPhase = true [MODO ESPERA]
```

## ⚠️ **REDUNDANCIAS IDENTIFICADAS**

### 🔴 **Redundancia 1: Verificación de Puerta DOBLE**
```
❌ ACTUAL:
_handleSelectionPageEvents() → isDoorClosed() → startProgram()
startProgram() → [ANTIGUA VERIFICACIÓN ELIMINADA] ✅

✅ CORREGIDO: Solo en _handleSelectionPageEvents()
```

### 🔴 **Redundancia 2: Debug Múltiple**
```
❌ ACTUAL:
processUserEvent() → Debug botón START
_handleSelectionPageEvents() → Debug botón START
startProgram() → Debug inicio

⚠️ PROPUESTA: Consolidar debug en un solo punto
```

### 🔴 **Redundancia 3: Validación de Estado**
```
❌ INNECESARIO:
startProgram() verifica _currentState == ESTADO_SELECCION

✅ OPTIMIZACIÓN:
Solo se llama desde página SELECTION → Estado siempre es correcto
```

### 🔴 **Redundancia 4: Configuración de UI**
```
❌ ACTUAL:
setState(ESTADO_EJECUCION) → Configura UI
startProgram() → Configura botones adicionales

⚠️ OPTIMIZACIÓN: Consolidar configuración UI en setState()
```

## 🚨 **PUNTOS CRÍTICOS DE FALLO**

### **Fallo 1:** `Hardware.hasValidTouchEvent() = false`
- **Causa:** Problema en comunicación Nextion
- **Solución:** Verificar cable serie, baudios, protocolo

### **Fallo 2:** `touchType != 1`
- **Causa:** Evento de liberación (touchType=0) en lugar de presión
- **Solución:** Verificar configuración Nextion

### **Fallo 3:** `touchPage != NEXTION_PAGE_SELECTION`
- **Causa:** Usuario no está en página correcta
- **Solución:** Forzar navegación a página de selección

### **Fallo 4:** `_currentState != ESTADO_SELECCION`
- **Causa:** Sistema en estado incorrecto (EJECUCION/PAUSA/ERROR)
- **Solución:** Reset del sistema o manejo de transiciones

## 📈 **OPTIMIZACIONES PROPUESTAS**

### **Optimización 1: Consolidar Validaciones**
```cpp
// En lugar de múltiples verificaciones, una función:
bool validateStartConditions() {
    return Hardware.hasValidTouchEvent() && 
           _currentState == ESTADO_SELECCION &&
           touchComponent == NEXTION_ID_BTN_START;
}
```

### **Optimización 2: Simplificar Flujo**
```cpp
// Flujo directo sin redundancias:
pressStartButton() → validateConditions() → executeStart()
```

### **Optimización 3: Debug Unificado**
```cpp
// Un solo punto de debug con contexto completo:
debugStartProcess(step, status, details)
```

## 🎯 **RESULTADO ESPERADO**

Después del diagnóstico, el flujo debe ser:
1. **Evento capturado** → Debug: "Evento START detectado"
2. **Validaciones pasadas** → Debug: "Validaciones OK"
3. **Programa iniciado** → Debug: "Estado: EJECUCION, Preparando: true"
4. **Pantalla actualizada** → Debug: "UI configurada"
5. **Sistema esperando condiciones** → Debug: "Esperando nivel/temperatura"

---
*Generado por Claude Code - Análisis del flujo de inicio del programa industrial*