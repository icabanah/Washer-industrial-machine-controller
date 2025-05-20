# Configuración de Pantalla Nextion para Controlador de Lavadora Industrial

## Introducción

Este documento detalla la configuración completa de la pantalla táctil Nextion para el sistema de control de lavadora industrial con soporte para los tres programas específicos (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda). La interfaz está diseñada para interactuar con la máquina de estados del controlador, proporcionando una experiencia de usuario intuitiva y una visualización clara del proceso de lavado en cada una de sus fases.

## Analogía: Panel de Control Sinfonía Orquestal

La pantalla Nextion funciona como un panel de control avanzado que permite al "director de orquesta" (el usuario) seleccionar qué "pieza musical" (programa de lavado) desea interpretar. Para el Programa 22 (Agua Caliente), la interfaz destaca los indicadores de temperatura con colores cálidos; para el Programa 23 (Agua Fría), enfatiza los indicadores de nivel de agua con tonos frescos; y para el Programa 24 (Multitanda), muestra una visualización de progreso de múltiples movimientos con información sobre la tanda actual. Cada sección de la interfaz está cuidadosamente diseñada para reflejar la naturaleza específica de cada programa, permitiendo al usuario "dirigir la orquesta" de manera eficiente.

## Integración con la Máquina de Estados

La pantalla Nextion interactúa directamente con la máquina de estados del controlador, reflejando y permitiendo transiciones entre los siete estados principales del sistema:

| Estado del Sistema | Pantalla Nextion | Interacción Usuario |
|--------------------|------------------|---------------------|
| ESTADO_INICIO      | Welcome          | Solo visualización (sistema inicializándose) |
| ESTADO_SELECCION   | Selection        | Selección entre programas 22, 23 y 24 |
| ESTADO_ESPERA      | Ready            | Confirmación para iniciar programa |
| ESTADO_EJECUCION   | Execution        | Monitoreo del progreso de ejecución |
| ESTADO_PAUSA       | Pause            | Decisión de continuar o detener |
| ESTADO_FINALIZACION| Completion       | Visualización de resumen y volver a selección |
| ESTADO_ERROR       | Error            | Confirmación de error |
| ESTADO_EMERGENCIA  | Emergency        | Información de parada de emergencia |

Cada transición de estado en el controlador se refleja en un cambio correspondiente en la pantalla Nextion, manteniendo una sincronización constante entre el estado interno del sistema y lo que se muestra al usuario.

## Parámetros de Conexión

| Parámetro       | Valor           |
|-----------      |-------          |
| Puerto Serial   | Serial2 (ESP32) |
| Baudios         | 115200 bps      |
| Configuración   | 8N1 (8 bits, sin paridad, 1 bit de parada) |
| RX (ESP32)      | Pin 16          |
| TX (ESP32)      | Pin 17          |

## Estructura de Pantallas para los Tres Programas

El sistema utiliza 8 pantallas (páginas) para toda la interacción con el usuario, cada una diseñada para soportar las características específicas de los tres programas:

| ID | Nombre      | Función                                    | Soporte de Programas |
|----|-------------|--------------------------------------------|----------------------|
| 0 | Welcome     | Pantalla de bienvenida al iniciar el sistema | Universal |
| 1 | Selection   | Selección entre los programas 22, 23 y 24  | Muestra características diferenciadoras |
| 2 | Ready       | Verificación de seguridad y confirmación de inicio | Universal |
| 3 | Execution   | Monitoreo de programa con visualización específica | Adapta según el programa en ejecución |
| 4 | Pause       | Pantalla de programa en pausa | Universal |
| 5 | Completion  | Resumen al finalizar el programa | Adapta según programa finalizado |
| 6 | Error       | Visualización de errores del sistema | Universal |
| 7 | Emergency   | Pantalla de parada de emergencia | Universal |

## Detalle de Pantallas y Componentes Específicos para los Tres Programas

### Pantalla 0: Welcome (Bienvenida)

**Objetivo**: Mostrar la información inicial de la empresa mientras el sistema se prepara.

**Componentes**:

| Nombre         | Tipo        | Propiedades            | Descripción             |
|----------------|-------------|------------------------|-------------------------|
| `txtTitulo`    | Text        | Font: 2, Align: Center | Título "RH Electronics" |
| `txtSubtitulo` | Text        | Font: 1, Align: Center | "Sistema de Lavadora Industrial" |
| `txtVersion`   | Text        | Font: 0, Align: Center | "v3.0 - Soporte para Programas 22, 23 y 24" |
| `txtCarga`     | Text        | Font: 0, Align: Center | "Inicializando..." |
| `prgCarga`     | Progress Bar| min: 0, max: 100       | Barra de progreso de inicialización |

**Comportamiento**:
- Se muestra automáticamente al encender el sistema
- Barra de progreso se incrementa mientras el sistema se inicializa
- Transición automática a pantalla Selection al completarse la inicialización (ESTADO_SELECCION)

### Pantalla 1: Selection (Selección de Programa)

**Objetivo**: Permitir al usuario seleccionar entre los programas 22, 23 y 24, destacando sus características específicas.

**Componentes Comunes**:

| Nombre         | Tipo                  | Propiedades            | Descripción |
|----------------|-----------------------|------------------------|-------------|
| `btnPrograma22` | Dual State Button     | val: 0, Font: 1        | "P22: Agua Caliente" |
| `btnPrograma23` | Dual State Button     | val: 0, Font: 1        | "P23: Agua Fría" |
| `btnPrograma24` | Dual State Button     | val: 0, Font: 1        | "P24: Multitanda" |
| `btnComenzar`  | Button                | Font: 1                | Botón para iniciar programa |
| `btnEditar`    | Button                | Font: 1                | Botón para editar parámetros |
| `txtProgSelec` | Text                  | Font: 2                | Muestra programa seleccionado |

**Componentes Específicos por Programa**:

Para Programa 22 (Agua Caliente):
- `iconoTemp` (Picture): Icono de termómetro/vapor
- `txtTempInfo` (Text): "Temperatura: 60°C"
- `txtTipoAgua` (Text, color: Rojo): "Agua Caliente"
- `bgBox` (Variable): Color de fondo rojo suave

Para Programa 23 (Agua Fría):
- `iconoAgua` (Picture): Icono de gota de agua
- `txtTempInfo` (Text): "Temperatura: Ambiente"
- `txtTipoAgua` (Text, color: Azul): "Agua Fría"
- `bgBox` (Variable): Color de fondo azul suave

Para Programa 24 (Multitanda):
- `iconoCiclos` (Picture): Icono de múltiples ciclos
- `txtTandas` (Text): "4 Tandas"
- `txtTipoAgua` (Text, color: Variable): "Tipo de Agua Configurable"
- `btnTipoAgua` (Dual State Button): Selector Agua Fría/Caliente
- `bgBox` (Variable): Color de fondo verde suave

**Comportamiento**:
- Los botones de programa son mutuamente excluyentes (solo uno seleccionado)
- La interfaz se adapta para mostrar información específica del programa seleccionado
- Presionar Comenzar cambia al ESTADO_ESPERA

### Pantalla 2: Ready (Preparación)

**Objetivo**: Verificar condiciones de seguridad y obtener confirmación final antes de iniciar el programa.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtPrograma`  | Text        | Font: 2     | Muestra programa seleccionado |
| `txtMensaje`   | Text        | Font: 1     | "¿Confirma iniciar este programa?" |
| `txtVerificar` | Text        | Font: 1     | "Verifique que la puerta esté cerrada" |
| `btnConfirmar` | Button      | Font: 1     | "Iniciar Ahora" |
| `btnRegresar`  | Button      | Font: 1     | "Regresar" |
| `imgDoor`      | Picture     | -           | Icono de puerta |

**Componentes Específicos por Programa**:
- Para P22: Indicación de temperatura objetivo y tiempo estimado
- Para P23: Indicación de nivel de agua objetivo y tiempo estimado
- Para P24: Indicación de número de tandas y tiempo total estimado

**Comportamiento**:
- Verificación del cierre de puerta antes de permitir continuar
- Confirmación final del usuario para iniciar el programa
- Presionar Confirmar cambia al ESTADO_EJECUCION

### Pantalla 3: Execution (Ejecución)

**Objetivo**: Mostrar el progreso detallado del programa en ejecución con visualización adaptada a cada programa específico.

**Componentes Comunes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtPrograma`  | Text        | Font: 2     | Muestra "P22", "P23" o "P24" |
| `txtFase`      | Text        | Font: 1     | Fase actual (Llenado/Lavado/Drenaje/Centrifugado) |
| `txtTiempo`    | Text        | Font: 2     | Tiempo restante (mm:ss) |
| `barraProgreso` | Progress Bar | min: 0, max: 100 | Progreso general del programa |
| `btnPausa`     | Button      | Font: 1     | Botón para pausar programa |
| `btnEmergencia` | Dual State Button | color: RED | Botón para emergencia (simulación) |

**Visualización para Programa 22 (Agua Caliente)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `gaugeTemp`    | Gauge       | min: 0, max: 100, color: RED | Indicador de temperatura prominente |
| `txtTemp`      | Text        | Font: 2, color: RED | Temperatura actual en °C |
| `txtTempObj`   | Text        | Font: 1     | Temperatura objetivo |
| `icTemp`       | Variable    | -           | Icono animado de calentamiento cuando activo |
| `txtEstadoTemp` | Text       | Font: 1     | "Calentando", "Temperatura Estable", etc. |
| `gaugePresion` | Gauge       | min: 0, max: 100 | Indicador de nivel de agua secundario |

**Visualización para Programa 23 (Agua Fría)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `gaugePresion` | Gauge       | min: 0, max: 100, color: BLUE | Indicador de nivel de agua prominente |
| `txtPresion`   | Text        | Font: 2, color: BLUE | Nivel de agua actual |
| `txtPresionObj` | Text       | Font: 1     | Nivel de agua objetivo |
| `icAgua`       | Variable    | -           | Icono animado de llenado cuando activo |
| `txtEstadoAgua` | Text       | Font: 1     | "Llenando", "Nivel Correcto", etc. |
| `txtTemp`      | Text        | Font: 1     | Temperatura actual (informativo, menos prominente) |

**Visualización para Programa 24 (Multitanda)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtTanda`     | Text        | Font: 2     | "Tanda X de 4" |
| `progresoCiclos` | Progress Bar | min: 0, max: 4 | Progreso de tandas completadas |
| `txtTipoAgua`  | Text        | Font: 1     | "Agua Caliente" o "Agua Fría" |
| `gaugeTemp`    | Gauge       | min: 0, max: 100, color: variable | Indicador de temperatura (color según tipo) |
| `gaugePresion` | Gauge       | min: 0, max: 100, color: variable | Indicador de nivel de agua |
| `txtTemp`      | Text        | Font: 1     | Temperatura actual |
| `txtPresion`   | Text        | Font: 1     | Nivel de agua actual |
| `icTandas`     | Variable    | -           | Icono animado de ciclos |

**Comportamiento**:
- Actualización en tiempo real de parámetros (temperatura, presión, tiempo)
- Indicaciones visuales específicas según la fase actual y tipo de programa
- Presionar Pausa cambia al ESTADO_PAUSA

### Pantalla 3: Edit (Edición)

**Objetivo**: Permitir modificar los parámetros de cada programa y fase.

**Componentes**:

| Nombre            | Tipo        | Propiedades | Descripción |
|----------------   |-------------|-------------|-------------|
| `txtProgramaFase` | Text        | Font: 2     | "P1 - F2" (Programa y Fase) |
| `txtVariable`    | Text        | Font: 1     | Variable en edición |
| `txtValor`       | Text        | Font: 2     | Valor actual |
| `sliderValor`    | Slider      | minval: 0, maxval: 100 | Slider para ajustar el valor |
| `btnGuardar`     | Button      | Font: 1, ID: 7 | Guardar cambios |
| `btnCancelar`    | Button      | Font: 1, ID: 8 | Cancelar edición |
| `btnMenos`       | Button      | - | Decrementar valor (opcional) |
| `btnMas`         | Button      | - | Incrementar valor (opcional) |

**Apariencia recomendada**:
- Título claro indicando programa y fase
- Slider grande para ajuste preciso
- Botones de guardar/cancelar claramente distinguibles
- Valores numéricos grandes y legibles

### Pantalla 4: Error (Errores)

**Objetivo**: Mostrar información clara sobre errores del sistema.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtCodigo`    | Text        | Font: 2, Color: RED | Código de error    |
| `txtMensaje`   | Text        | Font: 1     | Descripción del error      |
| `alerta`       | Variable    | en: 1       | Indicador visual de alerta |
| `btnAceptar`   | Button      | Font: 1     | Botón para reconocer error |

**Apariencia recomendada**:
- Fondo que indique error (rojo suave o amarillo)
- Icono de advertencia grande
- Mensaje de error claramente visible
- Instrucciones sobre cómo proceder

### Pantalla 5: Emergency (Emergencia)

**Objetivo**: Mostrar claramente que el sistema está en parada de emergencia.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtEmergencia` | Text | Font: 3, Color: RED | "PARADA DE EMERGENCIA" |
| `txtMensajeEmerg` | Text | Font: 1 | "Sistema detenido por seguridad" |
| `alarm` | Variable | en: 1 | Indicador visual de alarma |
| `txtInstrucciones` | Text | Font: 1 | Instrucciones para desactivar |

**Apariencia recomendada**:
- Fondo rojo o con marco rojo destacado
- Texto de emergencia grande y visible
- Símbolos de advertencia parpadeantes
- Instrucciones claras sobre cómo desbloquear

### Componentes Globales

**Elementos presentes en múltiples pantallas**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `msgBox`       | Variable    | vis: 0      | Cuadro para mensajes temporales |
| `audio`        | Variable    | val: 0-2    | Control de sonidos |

## Guía de Diseño Visual

### 1. Paleta de Colores Recomendada

- **Fondo principal**: Gris claro (#F0F0F0) o azul muy claro (#F0F8FF)
- **Elementos activos**: Azul (#0066CC)
- **Elementos inactivos**: Gris (#CCCCCC)
- **Alertas/Errores**: Rojo (#CC0000)
- **Texto principal**: Negro (#000000)
- **Texto secundario**: Gris oscuro (#666666)

### 2. Tipografía

- **Font 0**: Pequeña (16px) para información secundaria
- **Font 1**: Media (24px) para información general
- **Font 2**: Grande (32px) para valores importantes
- **Font 3**: Extra grande (48px) para emergencias y títulos principales

### 3. Indicadores Visuales

- Usar colores para indicar estados:
  - Verde: Operación normal
  - Amarillo: Advertencia o atención requerida
  - Rojo: Error o emergencia

- Utilizar iconos junto a texto para mejor comprensión:
  - Termómetro para temperatura
  - Gota para nivel de agua
  - Reloj para tiempo
  - Motor para velocidad

## Manejo de Eventos y Comunicación

## Integración Específica con los Tres Programas

### Programa 22 (Agua Caliente)

**Elementos Destacados**:
- Indicador de temperatura grande y prominente (rojo)
- Visualización constante del estado de calentamiento
- Alertas visuales cuando se ajusta la temperatura

**Interacciones Específicas**:
- Indicación visual cuando se activa la válvula de vapor
- Indicación visual durante el proceso de ajuste de temperatura (drenaje parcial/rellenado)
- Información detallada sobre temperatura objetivo y actual

**Ejemplo de Código para ESP32**:
```cpp
// Actualizar pantalla para Programa 22 (Agua Caliente)
void UIController::updateProgram22Screen(float currentTemp, float targetTemp, bool heatingActive) {
  // Actualizar valores de temperatura
  Hardware.nextionSendCommand("txtTemp.txt=\"" + String(currentTemp, 1) + "\"");
  Hardware.nextionSendCommand("txtTempObj.txt=\"" + String(targetTemp, 1) + "\"");
  Hardware.nextionSendCommand("gaugeTemp.val=" + String(int(currentTemp)));
  
  // Indicar estado de calentamiento
  if (heatingActive) {
    Hardware.nextionSendCommand("txtEstadoTemp.txt=\"Calentando\"");
    Hardware.nextionSendCommand("icTemp.picc=1"); // Icono activo
  } else {
    Hardware.nextionSendCommand("txtEstadoTemp.txt=\"Temperatura Estable\"");
    Hardware.nextionSendCommand("icTemp.picc=0"); // Icono inactivo
  }
}
```

### Programa 23 (Agua Fría)

**Elementos Destacados**:
- Indicador de nivel de agua grande y prominente (azul)
- Temperatura mostrada solo como información secundaria
- Indicación clara de fase actual

**Interacciones Específicas**:
- Indicación visual al abrir/cerrar válvula de agua
- Indicación visual durante el llenado/drenaje
- Indicación secundaria de temperatura (informativa)

**Ejemplo de Código para ESP32**:
```cpp
// Actualizar pantalla para Programa 23 (Agua Fría)
void UIController::updateProgram23Screen(uint8_t waterLevel, float currentTemp, bool fillingActive) {
  // Actualizar valores de nivel y temperatura
  Hardware.nextionSendCommand("txtPresion.txt=\"" + String(waterLevel) + "\"");
  Hardware.nextionSendCommand("txtTemp.txt=\"" + String(currentTemp, 1) + "\"");
  Hardware.nextionSendCommand("gaugePresion.val=" + String(waterLevel * 25)); // Convertir nivel 0-4 a 0-100
  
  // Indicar estado de llenado
  if (fillingActive) {
    Hardware.nextionSendCommand("txtEstadoAgua.txt=\"Llenando\"");
    Hardware.nextionSendCommand("icAgua.picc=1"); // Icono activo
  } else {
    Hardware.nextionSendCommand("txtEstadoAgua.txt=\"Nivel Correcto\"");
    Hardware.nextionSendCommand("icAgua.picc=0"); // Icono inactivo
  }
}
```

### Programa 24 (Multitanda)

**Elementos Destacados**:
- Visualización clara de la tanda actual y total
- Indicación del tipo de agua seleccionado
- Adaptación dinámica según configuración

**Interacciones Específicas**:
- Transición visual entre tandas
- Actualización de barra de progreso de tandas
- Visualización de tiempo total y tiempo de tanda actual

**Ejemplo de Código para ESP32**:
```cpp
// Actualizar pantalla para Programa 24 (Multitanda)
void UIController::updateProgram24Screen(uint8_t currentCycle, uint8_t totalCycles, 
                                         bool isHotWater, float temp, uint8_t waterLevel) {
  // Actualizar información de tanda
  Hardware.nextionSendCommand("txtTanda.txt=\"" + String(currentCycle) + "/" + String(totalCycles) + "\"");
  Hardware.nextionSendCommand("progresoCiclos.val=" + String(currentCycle));
  
  // Actualizar tipo de agua y sensores
  if (isHotWater) {
    Hardware.nextionSendCommand("txtTipoAgua.txt=\"Agua Caliente\"");
    Hardware.nextionSendCommand("txtTipoAgua.pco=63488"); // Color rojo
    Hardware.nextionSendCommand("gaugeTemp.pco=63488"); // Color rojo para temperatura
  } else {
    Hardware.nextionSendCommand("txtTipoAgua.txt=\"Agua Fría\"");
    Hardware.nextionSendCommand("txtTipoAgua.pco=31999"); // Color azul
    Hardware.nextionSendCommand("gaugeTemp.pco=31999"); // Color azul para temperatura
  }
  
  // Actualizar valores de sensores
  Hardware.nextionSendCommand("txtTemp.txt=\"" + String(temp, 1) + "\"");
  Hardware.nextionSendCommand("txtPresion.txt=\"" + String(waterLevel) + "\"");
  Hardware.nextionSendCommand("gaugeTemp.val=" + String(int(temp)));
  Hardware.nextionSendCommand("gaugePresion.val=" + String(waterLevel * 25));
}
```

## Protocolo de Comunicación Bidireccional

### Eventos desde Nextion hacia ESP32

La pantalla Nextion debe enviar los siguientes eventos al ESP32:

| Evento | Formato | Descripción |
|--------|---------|-------------|
| Botón Programa 22 | `65 01 22` | Selección de programa 22 (Agua Caliente) |
| Botón Programa 23 | `65 01 23` | Selección de programa 23 (Agua Fría) |
| Botón Programa 24 | `65 01 24` | Selección de programa 24 (Multitanda) |
| Botón Comenzar | `65 02 00` | Iniciar programa seleccionado |
| Botón Pausa | `65 03 00` | Pausar programa en ejecución |
| Botón Reanudar | `65 04 00` | Reanudar programa pausado |
| Botón Detener | `65 05 00` | Detener programa (desde pausa) |
| Botón Editar | `65 06 00` | Editar parámetros del programa |
| Selección Agua P24 | `65 07 [00/01]` | Tipo de agua para P24 (00=Fría, 01=Caliente) |

### Comandos desde ESP32 hacia Nextion

El ESP32 debe enviar los siguientes comandos a la pantalla Nextion:

| Comando | Descripción |
|---------|-------------|
| `page X` | Cambiar a página X (según estado) |
| `txtTemp.txt="XX.X"` | Actualizar valor de temperatura |
| `txtPresion.txt="X"` | Actualizar valor de nivel de agua |
| `txtTiempo.txt="MM:SS"` | Actualizar tiempo restante |
| `txtFase.txt="XXXX"` | Actualizar nombre de fase actual |
| `barraProgreso.val=XX` | Actualizar progreso general (0-100) |
| `txtMensaje.txt="XXXX"` | Mostrar mensaje temporal |
| `vis txtMensaje,1` | Mostrar componente |
| `vis txtMensaje,0` | Ocultar componente |

## Consideraciones de Implementación

### 1. Optimización de Rendimiento

- **Actualización Selectiva**: Solo actualizar componentes que han cambiado
- **Priorización**: Priorizar actualizaciones críticas (temperatura para P22, nivel para P23)
- **Frecuencia**: Temperatura y nivel cada segundo, otros componentes según necesidad

### 2. Pruebas por Programa

**Programa 22 (Agua Caliente)**:
1. Verificar que la visualización de temperatura sea fácilmente visible
2. Comprobar que las alertas de ajuste de temperatura sean claras
3. Probar la transición entre fases con sus indicadores correspondientes

**Programa 23 (Agua Fría)**:
1. Verificar que la visualización de nivel de agua sea fácilmente visible
2. Comprobar que la temperatura se muestre como información secundaria
3. Probar las indicaciones de llenado y drenaje

**Programa 24 (Multitanda)**:
1. Verificar la visualización correcta de tandas y progreso
2. Comprobar la adaptación según tipo de agua seleccionado
3. Probar transiciones entre tandas

### 3. Integración con la Máquina de Estados

Cada transición de estado del sistema debe reflejarse en la interfaz:

- **ESTADO_INICIO → ESTADO_SELECCION**: 
  ```cpp
  // En ProgramController al cambiar a ESTADO_SELECCION
  UIController::showSelectionScreen();
  ```

- **ESTADO_EJECUCION (con emergencia)**:
  ```cpp
  // En ProgramController::handleEmergency()
  UIController::showEmergencyScreen();
  ```

## Conclusión

Esta configuración de Nextion proporciona una interfaz completa y adaptada específicamente para soportar los tres programas de lavado. El diseño diferenciado para cada programa (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda) mejora la experiencia del usuario al destacar visualmente los aspectos más relevantes de cada programa, a la vez que mantiene una coherencia visual y funcional con la máquina de estados implementada en el controlador.

La implementación de pantallas específicas para cada estado del sistema asegura que el usuario siempre tenga la información adecuada y las opciones de control correspondientes a la fase actual del proceso de lavado, creando así una experiencia más intuitiva y eficiente.

## Sincronización con Programas Específicos y Máquina de Estados

Para garantizar un funcionamiento cohesivo entre la pantalla Nextion y el controlador ESP32, se ha implementado un sistema de sincronización bidireccional que refleja el estado actual del sistema y se adapta a las características específicas de cada programa.

### Mapeo de Estados a Pantallas

Cada estado de la máquina de estados tiene su correspondiente pantalla en Nextion:

```
ESTADO_INICIO       → página 0 (Welcome)
ESTADO_SELECCION    → página 1 (Selection)
ESTADO_ESPERA       → página 2 (Ready)
ESTADO_EJECUCION    → página 3 (Execution)
ESTADO_PAUSA        → página 4 (Pause)
ESTADO_FINALIZACION → página 5 (Completion)
ESTADO_ERROR        → página 6 (Error)
ESTADO_EMERGENCIA   → página 7 (Emergency)
```

### Implementación de la Comunicación Bidireccional

El siguiente fragmento de código muestra cómo el UIController maneja los eventos recibidos desde la pantalla Nextion y los comunica al ProgramController:

```cpp
void UIControllerClass::processNextionEvent(uint8_t *event) {
  // Procesar eventos de la pantalla Nextion
  if (event[0] == 65) {  // Código para evento de botón
    switch (event[1]) {
      case 1:  // Selección de programa
        uint8_t progId = event[2];
        ProgramController.selectProgram(progId);
        break;
        
      case 2:  // Iniciar programa
        ProgramController.setState(ESTADO_ESPERA);
        break;
        
      case 3:  // Pausar programa
        ProgramController.setState(ESTADO_PAUSA);
        break;
        
      case 4:  // Reanudar programa
        ProgramController.setState(ESTADO_EJECUCION);
        break;
        
      case 5:  // Detener programa
        ProgramController.setState(ESTADO_FINALIZACION);
        break;
        
      case 6:  // Editar programa
        showEditScreen(ProgramController.getCurrentProgram());
        break;
        
      case 7:  // Seleccionar tipo de agua (P24)
        bool useHotWater = (event[2] == 1);
        Storage.saveWaterTypeSelection(useHotWater);
        updateProgram24Info(useHotWater);
        break;
    }
  }
}
```

### Adaptación Dinámica para los Tres Programas

El UIController adapta la interfaz según el programa activo mediante una estructura condicional:

```cpp
void UIControllerClass::updateExecutionScreen() {
  uint8_t currentProgram = ProgramController.getCurrentProgram();
  
  // Actualizar componentes comunes
  Hardware.nextionSendCommand("txtPrograma.txt=\"P" + String(currentProgram + 22) + "\"");
  Hardware.nextionSendCommand("txtFase.txt=\"" + getFaseText(ProgramController.getCurrentPhase()) + "\"");
  Hardware.nextionSendCommand("txtTiempo.txt=\"" + formatTime(ProgramController.getRemainingTime()) + "\"");
  Hardware.nextionSendCommand("barraProgreso.val=" + String(ProgramController.getProgress()));
  
  // Actualizar componentes específicos según programa
  switch (currentProgram) {
    case PROGRAMA_22:  // Agua Caliente
      updateProgram22Screen(
        Sensors.getCurrentTemperature(),
        ProgramController.getTargetTemperature(),
        ProgramController.isHeatingActive()
      );
      break;
      
    case PROGRAMA_23:  // Agua Fría
      updateProgram23Screen(
        Sensors.getCurrentWaterLevel(),
        Sensors.getCurrentTemperature(),
        ProgramController.isFillingActive()
      );
      break;
      
    case PROGRAMA_24:  // Multitanda
      updateProgram24Screen(
        ProgramController.getCurrentCycle(),
        ProgramController.getNumberOfCycles(),
        Storage.getWaterTypeSelection(),
        Sensors.getCurrentTemperature(),
        Sensors.getCurrentWaterLevel()
      );
      break;
  }
}
```

Esta implementación garantiza que la interfaz se adapte dinámicamente a las necesidades específicas de cada programa, ofreciendo al usuario la información más relevante según el contexto actual.
