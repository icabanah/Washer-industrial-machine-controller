# Configuración de Pantalla Nextion para Controlador de Lavadora Industrial

## Introducción

Este documento detalla la configuración completa de la pantalla táctil Nextion para el sistema de control de lavadora industrial. La interfaz se compone de 6 pantallas principales que permiten al usuario interactuar con el sistema para seleccionar, configurar, monitorear y controlar los diferentes programas de lavado.

## Parámetros de Conexión

| Parámetro       | Valor           |
|-----------      |-------          |
| Puerto Serial   | Serial2 (ESP32) |
| Baudios         | 115200 bps      |
| Configuración   | 8N1 (8 bits, sin paridad, 1 bit de parada) |
| RX (ESP32)      | Pin 25          |
| TX (ESP32)      | Pin 27          |

## Estructura de Pantallas

El sistema utiliza 6 páginas (pantallas) para toda la interacción con el usuario:

| ID | Nombre      | Función                                    |
|----|-------------|--------------------------------------------|
| 0 | Welcome     | Pantalla de bienvenida al iniciar el sistema |
| 1 | Selection   | Selección de programa de lavado            |
| 2 | Execution   | Visualización del programa en ejecución    |
| 3 | Edit        | Edición de parámetros de los programas     |
| 4 | Error       | Visualización de errores del sistema |
| 5 | Emergency   | Pantalla de parada de emergencia |

## Detalle de Pantallas y Componentes

### Pantalla 0: Welcome (Bienvenida)

**Objetivo**: Mostrar la información inicial de la empresa mientras el sistema se prepara.

**Componentes**:

| Nombre         | Tipo        | Propiedades            | Descripción             |
|----------------|-------------|------------------------|-------------------------|
| `txtTitulo`    | Text        | Font: 2, Align: Center | Título "RH Electronics" |
| `txtContacto`  | Text        | Font: 1, Align: Center | Contacto "958970967" |
| `anim`         | Animation   | en: 1                  | Animación de inicio |
| `imgLogo`      | Picture     | -                      | Logo de la empresa (opcional) |

**Apariencia recomendada**:
- Fondo azul oscuro o gris claro profesional
- Logo centrado en la parte superior
- Texto de título centrado y grande
- Información de contacto en la parte inferior

### Pantalla 1: Selection (Selección de Programa)

**Objetivo**: Permitir al usuario seleccionar el programa de lavado y ver sus características.

**Componentes**:

| Nombre         | Tipo                  | Propiedades            | Descripción |
|----------------|-----------------------|------------------------|-------------|
| `btnPrograma1` | Dual State Button     | val: 0, Font: 1        | Botón para Programa 1 |
| `btnPrograma2` | Dual State Button     | val: 0, Font: 1        | Botón para Programa 2 |
| `btnPrograma3` | Dual State Button     | val: 0, Font: 1        | Botón para Programa 3 |
| `btnComenzar`  | Button                | Font: 1                | Botón para iniciar programa |
| `btnEditar`    | Button                | Font: 1     | Botón para editar programa |
| `txtNumPrograma` | Text | Font: 2 | Muestra "P1", "P2", etc. |
| `txtNivel`     | Text | Font: 1 | Nivel de agua del programa |
| `txtTemperatura` | Text | Font: 1 | Temperatura del programa |
| `txtTiempoFase` | Text | Font: 1 | Duración de la fase |
| `txtVelocidad` | Text | Font: 1 | Velocidad de rotación |
| `txtFasesInfo` | Text | Font: 0 | Información detallada de las fases |

**Apariencia recomendada**:
- Botones de programa alineados horizontalmente en la parte superior
- Información del programa seleccionado en panel central
- Botones de acción (Comenzar, Editar) en la parte inferior

### Pantalla 2: Execution (Ejecución)

**Objetivo**: Mostrar el progreso y estado actual del programa en ejecución.

**Componentes**:

| Nombre         | Tipo        | Propiedades | Descripción |
|----------------|-------------|-------------|-------------|
| `txtPrograma`  | Text        | Font: 2     | Muestra el programa actual |
| `txtFase`      | Text        | Font: 1     | Muestra la fase actual |
| `txtTemp`      | Text        | Font: 1     | Temperatura actual |
| `txtPresion`   | Text        | Font: 1     | Nivel de agua actual |
| `txtTiempo`    | Text        | Font: 2     | Tiempo restante (mm:ss) |
| `txtRotacion`  | Text        | Font: 1     | Velocidad de rotación actual |
| `gaugeTemp`    | Gauge       | min: 0, max: 100 | Indicador visual de temperatura |
| `gaugePresion` | Gauge       | min: 0, max: 100 | Indicador visual de nivel de agua |
| `barraProgreso` | Progress Bar | min: 0, max: 100 | Progreso general del programa |
| `btnParar`     | Button      | Font: 1, Color: RED | Botón para detener el programa |
| `motor`        | Variable    | val: 0-3    | Indicador visual del motor (opcional) |

**Apariencia recomendada**:
- Distribución en cuadrantes para fácil lectura
- Medidores gráficos para temperatura y nivel
- Tiempo restante grande y visible
- Botón de parada destacado en rojo

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

### 1. Eventos de Componentes

Todos los botones deben configurarse para enviar eventos al ser presionados. Los eventos en Nextion se configuran de la siguiente manera:

```
// Para un botón como btnPrograma1
cName=btnPrograma1
Touch Press Event:
  printh 65 01 01 FF FF FF
  
Touch Release Event:
  printh 65 01 01 FF FF FF
```

Donde:
- 65: Código para evento de botón
- 01: ID del componente
- 01: ID de la página

### 2. Código HMI para Componentes Clave

**Ejemplo para `btnPrograma1`**:
```
// Propiedades del botón Programa 1
cName=btnPrograma1
x=50
y=100
w=120
h=50
id=1
val=0
txt="Programa 1"
font=1
colors=65535,63488
```

**Ejemplo para `gaugeTemp`**:
```
// Propiedades del medidor de temperatura
cName=gaugeTemp
x=250
y=120
w=150
h=150
id=20
val=0
minval=0
maxval=100
bgcolor=65535
color=2016
```

## Consideraciones Adicionales

### 1. Optimización de Rendimiento

- Limitar el uso de imágenes de alta resolución
- Usar componentes simples cuando sea posible
- Evitar demasiados componentes en una sola pantalla

### 2. Pruebas y Validación

Antes de implementar la configuración completa:

1. Crear una pantalla de prueba con un botón y un texto
2. Verificar la comunicación con el ESP32
3. Asegurar que los eventos se envían y reciben correctamente
4. Probar la actualización de componentes desde el ESP32

### 3. Depuración

Para facilitar la depuración, considerar añadir una pantalla oculta con:
- Estado de la comunicación
- Valores en bruto de sensores
- Registro de últimos eventos

## Ejemplo de Flujo de Interacción

1. Al encender: Pantalla Welcome → Después de 3 segundos → Pantalla Selection
2. En Selection: Seleccionar programa → Presionar Comenzar → Pantalla Execution
3. En Execution: Mostrar progreso → Al finalizar → Pantalla Selection
4. Si error: Cualquier pantalla → Pantalla Error
5. Si emergencia: Cualquier pantalla → Pantalla Emergency

## Conclusión

Esta configuración proporciona una interfaz completa y funcional para el control de la lavadora industrial. El diseño prioriza la usabilidad, claridad de información y compatibilidad con el código de control implementado en el ESP32.

Al implementar esta configuración, asegúrate de mantener la coherencia en los nombres de componentes según lo definido en el archivo `config.h` para garantizar una comunicación correcta entre la pantalla Nextion y el controlador ESP32.
