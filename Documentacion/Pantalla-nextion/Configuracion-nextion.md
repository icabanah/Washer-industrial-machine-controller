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

**Analogía**: La estructura de pantallas es como un "libro interactivo" donde cada página tiene un propósito específico y está diseñada para guiar al usuario a través de una historia coherente del proceso de lavado.

El sistema utiliza 9 pantallas (páginas) para toda la interacción con el usuario, cada una diseñada para soportar las características específicas de los tres programas:

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
| 8 | Maintenance | Pantalla de mantenimiento y control manual | Para técnicos especializados |

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
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `txtEstado`    | Text        | Font: 0, Align: Center | Estado actual de inicialización |

**Secuencia de Inicialización**:
- **0-25%**: "Inicializando hardware..."
- **25-50%**: "Configurando sensores..."
- **50-75%**: "Estableciendo comunicación..."
- **75-100%**: "Sistema listo"

**Comportamiento**:
- Se muestra automáticamente al encender el sistema
- Barra de progreso se incrementa mientras el sistema se inicializa
- Mensajes de estado específicos durante cada fase
- Transición automática a pantalla Selection al completarse la inicialización (ESTADO_SELECCION)
- Posible mostrar errores críticos si falla la inicialización

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
| `msgGlobal`    | Text                  | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `btnEmergencia` | Dual State Button    | Color: RED, vis: 1     | Botón de emergencia desde UI |
| `txtTiempoEst` | Text                  | Font: 1                | Tiempo estimado del programa |
| `txtCaracteristicas` | Text            | Font: 0                | Características del programa seleccionado |

**Componentes Específicos por Programa**:

Para Programa 22 (Agua Caliente):
- `iconoTemp` (Picture): Icono de termómetro/vapor
- `txtTempInfo` (Text): "Temperatura: 60°C"
- `txtTipoAgua` (Text, color: Rojo): "Agua Caliente"
- `bgBox` (Variable): Color de fondo rojo suave (#FFE6E6)
- `txtDuracion` (Text): "Duración: ~45 min"
- `txtFases` (Text): "Llenado → Calentamiento → Lavado → Drenaje → Centrifugado"

Para Programa 23 (Agua Fría):
- `iconoAgua` (Picture): Icono de gota de agua
- `txtTempInfo` (Text): "Temperatura: Ambiente"
- `txtTipoAgua` (Text, color: Azul): "Agua Fría"
- `bgBox` (Variable): Color de fondo azul suave (#E6F3FF)
- `txtDuracion` (Text): "Duración: ~35 min"
- `txtFases` (Text): "Llenado → Lavado → Drenaje → Centrifugado"

Para Programa 24 (Multitanda):
- `iconoCiclos` (Picture): Icono de múltiples ciclos
- `txtTandas` (Text): "4 Tandas"
- `txtTipoAgua` (Text, color: Variable): "Tipo de Agua Configurable"
- `btnTipoAgua` (Dual State Button): Selector Agua Fría/Caliente
- `bgBox` (Variable): Color de fondo verde suave (#E6FFE6)
- `txtDuracion` (Text): "Duración: ~2.5 horas"
- `numTandas` (Number Input): Selector de número de tandas (1-8)
- `txtConfiguracion` (Text): Muestra configuración actual de tandas

**Comportamiento**:
- Los botones de programa son mutuamente excluyentes (solo uno seleccionado)
- La interfaz se adapta para mostrar información específica del programa seleccionado
- Presionar Comenzar cambia al ESTADO_ESPERA
- Presionar Editar permite modificar parámetros del programa seleccionado
- Botón de emergencia disponible en todo momento
- Validación de selección antes de permitir continuar

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
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `btnEmergencia` | Dual State Button | Color: RED, vis: 1 | Botón de emergencia desde UI |
| `txtEstadoPuerta` | Text     | Font: 1, Color: Variable | Estado de la puerta |
| `txtResumenPrograma` | Text  | Font: 0     | Resumen del programa a ejecutar |
| `ledPuerta`    | Variable    | Color: Variable | Indicador LED de estado de puerta |

**Componentes Específicos por Programa**:
- Para P22: `txtTempObjetivo` (Text): "Temperatura objetivo: XX°C" + `txtTiempoEst` (Text): "Tiempo estimado: XX min"
- Para P23: `txtNivelObjetivo` (Text): "Nivel de agua objetivo: X" + `txtTiempoEst` (Text): "Tiempo estimado: XX min"  
- Para P24: `txtTandasConfig` (Text): "Tandas configuradas: X" + `txtTiempoTotal` (Text): "Tiempo total estimado: XX min"

**Comportamiento**:
- Verificación continua del cierre de puerta (LED verde/rojo)
- Confirmación final del usuario para iniciar el programa
- Mostrar resumen específico según programa seleccionado
- Presionar Confirmar cambia al ESTADO_EJECUCION solo si puerta está cerrada
- Botón Emergencia disponible para cancelar

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
| `btnEmergencia` | Dual State Button | color: RED | Botón para emergencia |
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `txtTiempoFase` | Text       | Font: 1     | Tiempo restante de la fase actual |
| `progresoPasos` | Progress Bar | min: 0, max: 100 | Progreso de la fase actual |

**Visualización para Programa 22 (Agua Caliente)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `gaugeTemp`    | Gauge       | min: 0, max: 100, color: RED | Indicador de temperatura prominente |
| `txtTemp`      | Text        | Font: 2, color: RED | Temperatura actual en °C |
| `txtTempObj`   | Text        | Font: 1     | Temperatura objetivo |
| `icTemp`       | Variable    | -           | Icono animado de calentamiento cuando activo |
| `txtEstadoTemp` | Text       | Font: 1     | "Calentando", "Temperatura Estable", etc. |
| `gaugePresion` | Gauge       | min: 0, max: 100 | Indicador de nivel de agua secundario |
| `txtPresion`   | Text        | Font: 1     | Nivel de agua actual |
| `alertaTemp`   | Picture     | vis: 0      | Alerta visual para temperatura crítica |
| `txtAjusteTemp` | Text       | Font: 0, vis: 0 | "Ajustando temperatura..." |

**Comportamiento P22**:
- Gauge de temperatura como elemento central y más grande
- Animación de calentamiento cuando válvula de vapor está activa
- Alerta visual si temperatura excede límites seguros
- Indicación clara durante procesos de ajuste de temperatura (drenaje parcial)

**Visualización para Programa 23 (Agua Fría)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `gaugePresion` | Gauge       | min: 0, max: 100, color: BLUE | Indicador de nivel de agua prominente |
| `txtPresion`   | Text        | Font: 2, color: BLUE | Nivel de agua actual |
| `txtPresionObj` | Text       | Font: 1     | Nivel de agua objetivo |
| `icAgua`       | Variable    | -           | Icono animado de llenado cuando activo |
| `txtEstadoAgua` | Text       | Font: 1     | "Llenando", "Nivel Correcto", etc. |
| `txtTemp`      | Text        | Font: 1     | Temperatura actual (informativo, menos prominente) |
| `alertaAgua`   | Picture     | vis: 0      | Alerta visual para nivel crítico |
| `txtFlujoAgua` | Text        | Font: 0, vis: 0 | "Llenando..." o "Drenando..." |
| `timerLlenado` | Text        | Font: 0     | Tiempo transcurrido de llenado |

**Comportamiento P23**:
- Gauge de nivel de agua como elemento central y más grande
- Animación de llenado cuando válvula de agua fría está activa
- Temperatura mostrada como información secundaria
- Alerta visual si el llenado toma más tiempo del esperado
- Indicación clara de dirección de flujo de agua

**Visualización para Programa 24 (Multitanda)**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtTanda`     | Text        | Font: 2     | "Tanda X de Y" |
| `progresoCiclos` | Progress Bar | min: 0, max: 8 | Progreso de tandas completadas |
| `txtTipoAgua`  | Text        | Font: 1     | "Agua Caliente" o "Agua Fría" |
| `gaugeTemp`    | Gauge       | min: 0, max: 100, color: variable | Indicador de temperatura (color según tipo) |
| `gaugePresion` | Gauge       | min: 0, max: 100, color: variable | Indicador de nivel de agua |
| `txtTemp`      | Text        | Font: 1     | Temperatura actual |
| `txtPresion`   | Text        | Font: 1     | Nivel de agua actual |
| `icTandas`     | Variable    | -           | Icono animado de ciclos |
| `txtTiempoTanda` | Text       | Font: 1     | Tiempo restante tanda actual |
| `txtTiempoTotal` | Text       | Font: 0     | Tiempo total restante |
| `btnCambiarAgua` | Button     | Font: 0, vis: 0 | Cambiar tipo agua (si está pausado) |
| `alertaTanda`  | Picture     | vis: 0      | Alerta de cambio de tanda |

**Comportamiento P24**:
- Visualización dual de temperatura y presión según configuración
- Colores adaptativos: rojo para agua caliente, azul para agua fría
- Progreso visual de tandas completadas vs pendientes
- Notificación visual al completar cada tanda
- Opción de cambio de tipo de agua entre tandas (si se pausa)
- Tiempo específico por tanda y tiempo total acumulado

**Comportamiento General Pantalla Execution**:
- Actualización en tiempo real de parámetros (temperatura, presión, tiempo) cada segundo
- Indicaciones visuales específicas según la fase actual y tipo de programa
- Sistema de alertas integrado para condiciones anómalas
- Presionar Pausa cambia al ESTADO_PAUSA
- Presionar Emergencia activa parada inmediata
- Adaptación automática de colores e iconos según programa activo
- Mensajes contextuales según fase (ej: "Calentando...", "Llenando...", "Centrifugando...")
- Indicadores de actividad para actuadores (válvulas, motor)
- Sistema de alertas progresivas: verde (normal), amarillo (atención), rojo (crítico)

### Pantalla 4: Pause (Pausa)

**Objetivo**: Mostrar el estado de pausa y permitir al usuario decidir si continuar o detener el programa.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtPrograma`  | Text        | Font: 2     | Programa actual pausado |
| `txtEstadoPausa` | Text      | Font: 2, Color: YELLOW | "PROGRAMA PAUSADO" |
| `txtFasePausada` | Text      | Font: 1     | Fase en la que se pausó |
| `txtTiempoTranscurrido` | Text | Font: 1   | Tiempo transcurrido del programa |
| `txtTiempoRestante` | Text   | Font: 1     | Tiempo restante estimado |
| `btnReanudar`  | Button      | Font: 1, Color: GREEN | "Reanudar Programa" |
| `btnDetener`   | Button      | Font: 1, Color: RED | "Detener Programa" |
| `btnEmergencia` | Dual State Button | Color: RED | Botón de emergencia |
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `txtMotivoPausa` | Text      | Font: 0     | Razón de la pausa (manual/automática) |
| `iconoPausa`   | Picture     | -           | Icono de pausa animado |
| `txtInstrucciones` | Text    | Font: 0     | Instrucciones para el usuario |

**Información Específica Mostrada**:
- Estado actual de sensores (temperatura, nivel de agua)
- Progreso completado hasta el momento de pausa
- Para P24: Tanda actual y tandas completadas
- Estimación de tiempo para completar si se reanuda

**Comportamiento**:
- Mostrar claramente que el programa está pausado pero no terminado
- Presionar Reanudar regresa al ESTADO_EJECUCION
- Presionar Detener cambia al ESTADO_FINALIZACION
- Timeout automático de seguridad después de 10 minutos sin acción
- Monitoreo continuo de condiciones de seguridad (puerta, emergencia)

### Pantalla 5: Completion (Finalización)

**Objetivo**: Mostrar resumen del programa completado y permitir iniciar un nuevo ciclo.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtProgramaCompletado` | Text | Font: 2, Color: GREEN | "PROGRAMA P22 COMPLETADO" |
| `iconoExito`   | Picture     | -           | Icono de check o éxito |
| `txtTiempoTotal` | Text      | Font: 1     | Tiempo total transcurrido |
| `txtResumen`   | Text        | Font: 1     | Resumen del programa ejecutado |
| `btnNuevoPrograma` | Button  | Font: 1, Color: GREEN | "Nuevo Programa" |
| `btnSalir`     | Button      | Font: 1     | "Salir" |
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `txtEstadisticas` | Text     | Font: 0     | Estadísticas del programa |
| `alertaMantenimiento` | Text | Font: 0, vis: 0 | Recordatorio de mantenimiento |

**Resumen Específico por Programa**:
- **P22**: Temperatura alcanzada, tiempo de calentamiento, fases completadas
- **P23**: Nivel de agua alcanzado, tiempo de llenado, fases completadas  
- **P24**: Tandas completadas, tipo de agua usado, tiempo total por tanda

**Comportamiento**:
- Mostrar mensaje de éxito prominente
- Resumen específico según el programa ejecutado
- Presionar Nuevo Programa regresa al ESTADO_SELECCION
- Mensaje de mantenimiento si se cumplen ciertos ciclos
- Timeout automático de 2 minutos para regresar a selección
- Posibilidad de mostrar estadísticas acumuladas

### Pantalla 6: Error (Errores)

**Objetivo**: Mostrar información clara sobre errores del sistema y guiar la resolución.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtCodigo`    | Text        | Font: 2, Color: RED | Código de error (ERR_01, ERR_02, etc.) |
| `txtMensaje`   | Text        | Font: 1     | Descripción del error |
| `txtDescripcion` | Text      | Font: 0     | Descripción detallada del problema |
| `alerta`       | Variable    | en: 1       | Indicador visual de alerta parpadeante |
| `btnAceptar`   | Button      | Font: 1     | Botón para reconocer error |
| `btnReintentar` | Button     | Font: 1, vis: 0 | Reintentar operación (si aplica) |
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `iconoError`   | Picture     | -           | Icono de error grande |
| `txtSolucion`  | Text        | Font: 0     | Sugerencia de solución |
| `txtContacto`  | Text        | Font: 0     | Información de contacto técnico |

**Códigos de Error Principales**:
- **ERR_01**: Error sensor temperatura
- **ERR_02**: Error sensor presión  
- **ERR_03**: Timeout llenado
- **ERR_04**: Puerta abierta durante operación
- **ERR_05**: Sobrecalentamiento
- **ERR_06**: Error comunicación Nextion

**Comportamiento**:
- Fondo rojo suave para indicar error crítico
- Icono de advertencia parpadeante
- Mensaje de error específico y legible
- Instrucciones claras sobre cómo proceder
- Opción de reintentar para errores recuperables
- Información de contacto para errores que requieren técnico

### Pantalla 7: Emergency (Emergencia)

**Objetivo**: Mostrar claramente que el sistema está en parada de emergencia y guiar el proceso de recuperación.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtEmergencia` | Text | Font: 3, Color: RED | "PARADA DE EMERGENCIA" |
| `txtMensajeEmerg` | Text | Font: 1 | "Sistema detenido por seguridad" |
| `alarm` | Variable | en: 1 | Indicador visual de alarma parpadeante |
| `txtInstrucciones` | Text | Font: 1 | Instrucciones para desactivar |
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Componente global de mensajes |
| `btnResetEmergencia` | Button | Font: 1, Color: YELLOW | "Reset Emergencia" |
| `txtCausaEmergencia` | Text | Font: 0 | Causa de la parada de emergencia |
| `iconoEmergencia` | Picture | - | Icono de emergencia grande parpadeante |
| `txtEstadoSistema` | Text | Font: 0 | Estado actual de componentes críticos |
| `txtTiempoEmergencia` | Text | Font: 0 | Tiempo transcurrido desde emergencia |

**Tipos de Emergencia**:
- **MANUAL**: Botón de emergencia físico presionado
- **SISTEMA**: Emergencia activada desde interfaz
- **SENSOR**: Condición crítica detectada por sensores
- **TIMEOUT**: Timeout de seguridad excedido

**Comportamiento**:
- Fondo rojo completo con marco rojo destacado
- Texto de emergencia grande y parpadeante
- Símbolos de advertencia animados
- Instrucciones paso a paso para desbloquear
- Verificación de condiciones antes de permitir reset
- Log automático del evento de emergencia
- Sonido de alarma continuo (si disponible)

### Componentes Globales Actualizados

**Analogía**: Los componentes globales son como "herramientas universales" que cada pantalla debe tener disponibles, similar a cómo un coche tiene pedales de freno en todas las versiones, sin importar el modelo.

**Elementos presentes en TODAS las pantallas**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `msgGlobal`    | Text        | x:10, y:200, w:460, h:40, vis:0, z:999 | Sistema de mensajes universal |
| `btnEmergencia` | Dual State Button | Color: RED, x:400, y:10, vis: 1 | Botón emergencia (excepto Welcome) |
| `audio`        | Variable    | val: 0-3    | Control de sonidos del sistema |
| `statusBar`    | Rectangle   | x:0, y:270, w:480, h:20 | Barra de estado del sistema |
| `txtEstadoSistema` | Text    | Font: 0, x:5, y:275 | Estado actual del sistema |
| `txtHora`      | Text        | Font: 0, x:400, y:275 | Hora actual del sistema |

**Configuración del Sistema de Mensajes Global**:
```
msgGlobal configuración:
- Background color (bco): Variable según tipo de mensaje
  - 2016 (Verde): Mensajes informativos
  - 65504 (Amarillo): Mensajes de advertencia  
  - 63488 (Rojo): Mensajes de error
- Font color (pco): 0 (Negro) para todos los tipos
- Border: 1 pixel, color negro
- Visibility: 0 por defecto, se activa según necesidad
```

**Gestión de Visibilidad por Pantalla**:
- **Welcome**: Solo msgGlobal y statusBar
- **Selection/Ready**: Todos los componentes globales
- **Execution**: Todos los componentes globales con btnEmergencia destacado
- **Pause**: Todos excepto btnEmergencia (ya hay botones específicos)
- **Completion**: Solo msgGlobal y statusBar
- **Error/Emergency**: Solo msgGlobal (otras funciones suspendidas)
- **Maintenance**: Todos los componentes globales

### Pantalla 8: Maintenance (Mantenimiento)

**Objetivo**: Proporcionar herramientas de diagnóstico y control manual para técnicos especializados.

**Analogía**: Esta pantalla funciona como el "centro de control de un técnico especializado" - similar al panel de un ingeniero que puede mover cada pieza de la maquinaria individualmente para diagnosticar problemas.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtModoManual` | Text        | Font: 2, Color: ORANGE | "MODO MANTENIMIENTO" |
| `txtAdvertencia` | Text       | Font: 1, Color: RED | "Solo para personal técnico" |
| `btnMotorCW`   | Button      | Font: 1, Color: GREEN | "Motor ⟳" |
| `btnMotorCCW`  | Button      | Font: 1, Color: GREEN | "Motor ⟲" |
| `btnMotorStop` | Button      | Font: 1, Color: RED | "Motor STOP" |
| `btnAguaFria`  | Dual State Button | Font: 1, Color: BLUE | "Válvula Agua Fría" |
| `btnAguaCaliente` | Dual State Button | Font: 1, Color: RED | "Válvula Agua Caliente" |
| `btnDesague`   | Dual State Button | Font: 1, Color: GRAY | "Válvula Desagüe" |
| `btnVapor`     | Dual State Button | Font: 1, Color: ORANGE | "Válvula Vapor" |
| `txtEstadoMotor` | Text      | Font: 1     | Estado actual del motor |
| `txtEstadoValvulas` | Text   | Font: 1     | Estado de válvulas |
| `txtSensorTemp` | Text       | Font: 1     | Lectura sensor temperatura |
| `txtSensorPresion` | Text    | Font: 1     | Lectura sensor presión |
| `btnSalir`     | Button      | Font: 1, Color: YELLOW | "Salir Mantenimiento" |
| `txtClave`     | Text        | Font: 1     | Campo para clave de acceso |

**Acceso de Seguridad**:
- Acceso mediante combinación de botones específica (ej: presionar Editar 3 veces seguidas)
- Timeout automático después de 10 minutos sin actividad
- Registro de actividades de mantenimiento en log del sistema

**Comportamiento**:
- Permite control individual de cada actuador
- Muestra en tiempo real el estado de todos los sensores
- Incluye indicadores visuales claros del estado de cada componente
- Botones de control activos solo cuando es seguro (puerta cerrada, sistema no en emergencia)

## Sistema de Mensajes y Alertas Avanzado

**Analogía**: El sistema de mensajes es como un "asistente personal inteligente" que comunica información importante al usuario en el momento preciso, usando el tono y el color adecuados según la urgencia del mensaje.

### Categorías de Mensajes

#### 1. Mensajes Informativos (Verde - #00AA00)
| Mensaje | Disparador | Duración |
|---------|------------|----------|
| "Programa iniciado correctamente" | Al comenzar cualquier programa | 3 segundos |
| "Fase completada: [nombre_fase]" | Al finalizar cada fase | 2 segundos |
| "Tanda X de Y completada" | Programa 24 - fin de tanda | 3 segundos |
| "Temperatura estabilizada" | Programa 22 - temperatura alcanzada | 2 segundos |
| "Nivel de agua correcto" | Programa 23 - nivel alcanzado | 2 segundos |

#### 2. Mensajes de Advertencia (Amarillo - #FFAA00)
| Mensaje | Disparador | Duración |
|---------|------------|----------|
| "Puerta debe permanecer cerrada" | Intento de abrir durante ejecución | 5 segundos |
| "Ajustando temperatura..." | Programa 22 - proceso de ajuste | Hasta completar |
| "Llenando tambor..." | Durante fase de llenado | Hasta completar |
| "Centrifugado en progreso" | Durante centrifugado | Hasta completar |
| "Programa pausado" | Al activar pausa | Permanente hasta reanudar |

#### 3. Mensajes de Error (Rojo - #CC0000)
| Mensaje | Disparador | Duración |
|---------|------------|----------|
| "Error de sensor de temperatura" | Falla en sensor DS18B20 | Permanente hasta reset |
| "Error de sensor de presión" | Falla en sensor HX710B | Permanente hasta reset |
| "Puerta abierta durante operación" | Sensor de puerta activado | Permanente hasta cerrar |
| "Tiempo de llenado excedido" | Timeout en llenado | Permanente hasta reset |
| "Emergencia activada" | Botón emergencia presionado | Permanente hasta reset |

### Implementación del Sistema de Mensajes

#### Componente Global de Mensajes
En todas las pantallas debe existir un componente de mensaje superpuesto:

```
Nombre: msgGlobal
Tipo: Text
Propiedades:
- x: 10, y: 200, w: 460, h: 40
- Font: 1, Align: Center
- Visible: 0 (inicialmente oculto)
- Color de fondo: Variable según tipo
- Z-order: 999 (siempre encima)
```

#### Protocolo de Activación de Mensajes

**Desde ESP32 hacia Nextion**:
```cpp
// Mensaje informativo
"msgGlobal.bco=2016" // Verde
"msgGlobal.txt=\"Programa iniciado\""
"vis msgGlobal,1"

// Mensaje de advertencia  
"msgGlobal.bco=65504" // Amarillo
"msgGlobal.txt=\"Puerta cerrada\""
"vis msgGlobal,1"

// Mensaje de error
"msgGlobal.bco=63488" // Rojo
"msgGlobal.txt=\"Error sensor\""
"vis msgGlobal,1"

// Ocultar mensaje
"vis msgGlobal,0"
```

#### Temporizadores de Mensajes

El ESP32 debe gestionar los temporizadores de mensajes usando AsyncTaskLib:

```cpp
class MessageController {
private:
  AsyncTask* hideMessageTask = nullptr;
  
public:
  void showMessage(String text, MessageType type, uint16_t duration = 0) {
    // Configurar color según tipo
    uint16_t color = (type == INFO) ? 2016 : (type == WARNING) ? 65504 : 63488;
    
    // Enviar comandos a Nextion
    Hardware.nextionSendCommand("msgGlobal.bco=" + String(color));
    Hardware.nextionSendCommand("msgGlobal.txt=\"" + text + "\"");
    Hardware.nextionSendCommand("vis msgGlobal,1");
    
    // Programar ocultación automática si es temporal
    if (duration > 0) {
      hideMessageTask = new AsyncTask(duration * 1000, false, []() {
        Hardware.nextionSendCommand("vis msgGlobal,0");
      });
      Utils.registerCallback(hideMessageTask);
    }
  }
  
  void hideMessage() {
    Hardware.nextionSendCommand("vis msgGlobal,0");
    if (hideMessageTask) {
      Utils.unregisterCallback(hideMessageTask);
      hideMessageTask = nullptr;
    }
  }
};
```

### Alertas de Emergencia y Sonidos

#### Sistema de Alertas Visuales
Para situaciones críticas (emergencias, errores graves), implementar alertas parpadeantes:

```cpp
// En Nextion, usar un timer interno para parpadeo
"tm0.en=1" // Activar timer 0 (500ms)
"tm0.tim=500"

// En evento de timer:
"vis txtEmergencia,!txtEmergencia.vis" // Invertir visibilidad
```

#### Integración con Sistema de Audio
Si la pantalla Nextion soporta audio, definir sonidos específicos:

```cpp
// Sonidos del sistema
"audio.val=0" // Sin sonido
"audio.val=1" // Click normal
"audio.val=2" // Sonido de advertencia 
"audio.val=3" // Alarma de emergencia
```

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

## Protocolo de Comunicación Bidireccional Avanzado

### Eventos desde Nextion hacia ESP32

**Analogía**: Cada evento es como un "telegrama digital" que la pantalla envía al cerebro del sistema (ESP32), con un formato específico que garantiza una comunicación sin malentendidos.

La pantalla Nextion debe enviar los siguientes eventos al ESP32:

| Evento | Formato | Descripción | Pantalla Origen |
|--------|---------|-------------|-----------------|
| Botón Programa 22 | `65 01 22` | Selección de programa 22 (Agua Caliente) | Selection |
| Botón Programa 23 | `65 01 23` | Selección de programa 23 (Agua Fría) | Selection |
| Botón Programa 24 | `65 01 24` | Selección de programa 24 (Multitanda) | Selection |
| Botón Comenzar | `65 02 00` | Iniciar programa seleccionado | Selection |
| Botón Confirmar Inicio | `65 02 01` | Confirmación final para inicio | Ready |
| Botón Pausa | `65 03 00` | Pausar programa en ejecución | Execution |
| Botón Reanudar | `65 04 00` | Reanudar programa pausado | Pause |
| Botón Detener | `65 05 00` | Detener programa (desde pausa) | Pause |
| Botón Editar | `65 06 00` | Editar parámetros del programa | Selection |
| Selección Agua P24 | `65 07 [00/01]` | Tipo de agua para P24 (00=Fría, 01=Caliente) | Selection/Edit |
| Emergencia UI | `65 08 00` | Emergencia activada desde pantalla | Cualquiera |
| Reconocer Error | `65 09 00` | Reconocimiento de error mostrado | Error |
| Resetear Emergencia | `65 10 00` | Solicitud para resetear emergencia | Emergency |
| Valor Editado | `65 11 [valor]` | Nuevo valor desde pantalla de edición | Edit |
| Cambio Manual | `65 12 [accion]` | Control manual de actuadores (para mantenimiento) | Maintenance |

### Código de Acciones para Control Manual (Valor [accion])
| Código | Acción | Descripción |
|--------|--------|-------------|
| 01 | Motor CW | Activar motor sentido horario |
| 02 | Motor CCW | Activar motor sentido antihorario |
| 03 | Motor Stop | Detener motor |
| 04 | Válvula Agua Fría ON | Abrir válvula agua fría |
| 05 | Válvula Agua Fría OFF | Cerrar válvula agua fría |
| 06 | Válvula Agua Caliente ON | Abrir válvula agua caliente |
| 07 | Válvula Agua Caliente OFF | Cerrar válvula agua caliente |
| 08 | Válvula Desagüe ON | Abrir válvula desagüe |
| 09 | Válvula Desagüe OFF | Cerrar válvula desagüe |
| 10 | Válvula Vapor ON | Abrir válvula vapor |
| 11 | Válvula Vapor OFF | Cerrar válvula vapor |

### Comandos desde ESP32 hacia Nextion

**Analogía**: Los comandos del ESP32 son como "órdenes de un director de orquesta" que coordinan todos los elementos visuales de la pantalla para crear una sinfonía informativa perfecta.

El ESP32 debe enviar los siguientes comandos a la pantalla Nextion:

#### Comandos Básicos de Control
| Comando | Descripción | Frecuencia |
|---------|-------------|------------|
| `page X` | Cambiar a página X (según estado) | Según transición de estado |
| `txtTemp.txt="XX.X"` | Actualizar valor de temperatura | Cada 1s durante ejecución |
| `txtPresion.txt="X"` | Actualizar valor de nivel de agua | Cada 1s durante ejecución |
| `txtTiempo.txt="MM:SS"` | Actualizar tiempo restante | Cada 1s durante ejecución |
| `txtFase.txt="XXXX"` | Actualizar nombre de fase actual | Al cambiar fase |
| `barraProgreso.val=XX` | Actualizar progreso general (0-100) | Cada 5s durante ejecución |

#### Comandos de Interfaz Dinámica
| Comando | Descripción | Uso |
|---------|-------------|-----|
| `vis txtMensaje,1` | Mostrar componente mensaje | Alertas y notificaciones |
| `vis txtMensaje,0` | Ocultar componente mensaje | Limpiar alertas |
| `txtMensaje.txt="XXXX"` | Actualizar texto de mensaje | Comunicación con usuario |
| `txtMensaje.pco=XXXXX` | Cambiar color de mensaje | Prioridad (verde/amarillo/rojo) |
| `vis btnEmergencia,1` | Mostrar botón emergencia | Durante ejecución |
| `vis btnEmergencia,0` | Ocultar botón emergencia | En pantallas de configuración |

#### Comandos de Actualizaciones Específicas por Programa
| Comando | Descripción | Programa |
|---------|-------------|----------|
| `gaugeTemp.val=XX` | Actualizar gauge de temperatura | P22, P24 |
| `gaugePresion.val=XX` | Actualizar gauge de presión | P23, P24 |
| `txtTanda.txt="X/Y"` | Actualizar información de tandas | P24 únicamente |
| `progresoCiclos.val=X` | Actualizar progreso de ciclos | P24 únicamente |
| `txtTipoAgua.txt="XXXX"` | Actualizar tipo de agua actual | P24 únicamente |
| `icTemp.picc=X` | Animación de calentamiento (0=off, 1=on) | P22, P24 agua caliente |
| `icAgua.picc=X` | Animación de llenado (0=off, 1=on) | P23, P24 agua fría |

#### Comandos de Sistema y Diagnóstico
| Comando | Descripción | Uso |
|---------|-------------|-----|
| `txtCodigo.txt="ERR_XX"` | Mostrar código de error | Pantalla Error |
| `txtMensajeError.txt="XXXX"` | Descripción detallada del error | Pantalla Error |
| `alerta.en=1` | Activar indicador visual de alerta | Errores y emergencias |
| `alerta.en=0` | Desactivar indicador de alerta | Al reconocer error |
| `alarm.en=1` | Activar alarma de emergencia | Pantalla Emergency |
| `alarm.en=0` | Desactivar alarma | Al resetear emergencia |
| `audio.val=X` | Control de sonidos (0=off, 1=click, 2=alarm) | Feedback auditivo |

#### Comandos para Pantalla de Mantenimiento
| Comando | Descripción | Uso |
|---------|-------------|-----|
| `txtEstadoMotor.txt="XXXX"` | Estado actual del motor | Diagnóstico |
| `txtEstadoValvulas.txt="XXXX"` | Estado de válvulas | Diagnóstico |
| `vis btnManual,1` | Mostrar controles manuales | Modo mantenimiento |
| `vis btnManual,0` | Ocultar controles manuales | Modo operación normal |

## Optimizaciones de Rendimiento para ESP32-Nextion

**Analogía**: Las optimizaciones son como "afinar un motor de alto rendimiento" - cada ajuste mejora la eficiencia y la respuesta del sistema sin comprometer la funcionalidad.

### 1. Gestión Inteligente de Actualizaciones
```cpp
// Solo actualizar componentes que han cambiado
class NextionOptimizer {
  float lastTemp = -999;
  uint8_t lastWaterLevel = 255;
  String lastPhase = "";
  
  void updateTemperature(float newTemp) {
    if (abs(newTemp - lastTemp) > 0.5) { // Solo si cambia +/- 0.5°C
      Hardware.nextionSendCommand("txtTemp.txt=\"" + String(newTemp, 1) + "\"");
      lastTemp = newTemp;
    }
  }
};
```

### 2. Buffer de Comandos y Priorización
```cpp
// Sistema de cola con prioridades para comandos Nextion
enum CommandPriority { CRITICAL, HIGH, NORMAL, LOW };

struct NextionCommand {
  String command;
  CommandPriority priority;
  unsigned long timestamp;
};

class CommandQueue {
  std::vector<NextionCommand> queue;
  
public:
  void addCommand(String cmd, CommandPriority priority) {
    queue.push_back({cmd, priority, millis()});
    std::sort(queue.begin(), queue.end(), [](const NextionCommand& a, const NextionCommand& b) {
      return a.priority < b.priority;
    });
  }
};
```

### 3. Configuración Serial Optimizada
```cpp
// En setup(), configuración optimizada para ESP32
void initializeNextion() {
  NEXTION_SERIAL.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  NEXTION_SERIAL.setRxBufferSize(512);  // Buffer más grande
  NEXTION_SERIAL.setTimeout(50);        // Timeout reducido
}
```

### 4. Monitoreo de Rendimiento
```cpp
// Métricas de rendimiento en tiempo real
struct PerformanceMetrics {
  unsigned long commandsSent = 0;
  unsigned long responsesReceived = 0;
  unsigned long averageResponseTime = 0;
  unsigned long maxResponseTime = 0;
};
```

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

## Integración con AsyncTaskLib y Recuperación de Errores

**Analogía**: AsyncTaskLib es como tener un "mayordomo digital" que se encarga de todas las tareas programadas sin que el cerebro principal (loop principal) se distraiga o se bloquee.

### Tareas Asíncronas Específicas para Nextion

```cpp
// Actualización de interfaz cada segundo durante ejecución
AsyncTask* uiUpdateTask = new AsyncTask(1000, true, []() {
  if (ProgramController.getCurrentState() == ESTADO_EJECUCION) {
    UIController.updateExecutionScreen();
  }
});

// Monitoreo de comunicación Nextion cada 100ms
AsyncTask* commsCheckTask = new AsyncTask(100, true, []() {
  Hardware.nextionCheckForEvents();
  if (millis() - lastNextionResponse > 5000) {
    UIController.handleCommunicationError();
  }
});

// Parpadeo de alertas críticas cada 500ms
AsyncTask* alertBlinkTask = new AsyncTask(500, true, []() {
  if (alertsActive) {
    UIController.toggleCriticalAlerts();
  }
});
```

### Protocolo de Recuperación de Errores

#### Detección de Pérdida de Comunicación
```cpp
void UIController::handleCommunicationError() {
  // Intentar reinicializar comunicación
  Hardware.reinitializeNextion();
  
  // Enviar comando de prueba
  Hardware.nextionSendCommand("get sleep");
  
  // Si no hay respuesta en 2 segundos, considerar falla crítica
  commsRecoveryTimer = millis();
}
```

#### Sistema de Respaldo para Visualización Crítica
- Usar LED físico como respaldo para indicar estado del sistema
- Implementar buzzer para alertas críticas cuando Nextion falla
- Log detallado en Serial para diagnóstico

## Conclusión y Estado del Proyecto

Esta configuración actualizada de la pantalla Nextion proporciona una interfaz completa y optimizada específicamente para soportar los tres programas de lavado con las siguientes mejoras implementadas:

### Características Principales Implementadas:
- **Interfaz adaptativa diferenciada** para cada programa (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda)
- **Sistema de mensajes y alertas avanzado** con categorización por prioridad y colores
- **Pantalla de mantenimiento** para técnicos especializados con controles manuales
- **Protocolo de comunicación bidireccional expandido** con 12 tipos de eventos
- **Optimizaciones de rendimiento** para ESP32 con actualización selectiva y gestión de buffer
- **Integración completa con AsyncTaskLib** para operaciones no bloqueantes
- **Sistema de recuperación de errores** para mayor robustez
- **9 pantallas especializadas** incluyendo modo mantenimiento

### Coherencia con la Máquina de Estados:
La implementación mantiene una sincronización perfecta entre el estado interno del sistema y la interfaz visual, creando una experiencia de usuario intuitiva que guía al operador a través de cada fase del proceso de lavado.

### Progreso del UI Controller: 75% (Actualizado desde 60%)
- ✅ Protocolo de comunicación bidireccional expandido
- ✅ Sistema de mensajes y alertas implementado
- ✅ Pantalla de mantenimiento agregada
- ✅ Optimizaciones de rendimiento
- ✅ Integración con AsyncTaskLib
- 🔄 Pendiente: Pruebas finales de integración
- 🔄 Pendiente: Validación con hardware real

### Recomendaciones para la Siguiente Fase:
1. **Pruebas de integración** con los módulos Sensors y Actuators completados
2. **Validación del sistema de mensajes** con casos de uso reales
3. **Optimización final** basada en métricas de rendimiento en hardware
4. **Documentación de procedimientos** para técnicos de mantenimiento

La interfaz está ahora preparada para una integración robusta con el sistema completo y proporciona todas las herramientas necesarias para una operación eficiente y segura de la lavadora industrial.

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
