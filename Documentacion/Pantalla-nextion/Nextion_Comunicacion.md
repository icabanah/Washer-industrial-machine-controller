# Comunicación con Pantalla Nextion

## Descripción General

Este documento detalla la implementación de la comunicación entre el ESP32-WROOM y la pantalla táctil Nextion para el controlador de lavadora industrial. La comunicación se basa en el protocolo serial UART y está encapsulada en una estructura modular que proporciona una interfaz clara y consistente.

## Analogía: Sistema de Conversación

Podemos pensar en la comunicación con la pantalla Nextion como una conversación entre dos personas. Una persona (el ESP32) envía mensajes (comandos) a otra persona (la pantalla Nextion), y la otra persona responde con retroalimentación (eventos). Cada mensaje debe seguir un formato específico para ser entendido, y al final de cada mensaje se incluye una señal clara de finalización (los tres bytes FF FF FF).

## Conexión Física

La conexión entre el ESP32 y la pantalla Nextion requiere cuatro cables:

1. **VCC** - Alimentación 5V para la pantalla Nextion
2. **GND** - Tierra común entre ESP32 y Nextion
3. **TX** - Pin de transmisión del ESP32 conectado al RX de Nextion
4. **RX** - Pin de recepción del ESP32 conectado al TX de Nextion

### Diagrama de Conexión

```
ESP32                  Nextion
---------------        ---------------
5V         ----------> 5V (VCC)
GND        ----------> GND
GPIO 17 TX ----------> RX
GPIO 16 RX <---------- TX
```

> Nota: Los pines GPIO utilizados pueden configurarse en config.h a través de las constantes NEXTION_TX_PIN y NEXTION_RX_PIN.

## Protocolo de Comunicación

La comunicación con la pantalla Nextion se realiza a través del puerto serie a 115200 baudios. El protocolo incluye:

### Envío de Comandos (ESP32 -> Nextion)

1. Los comandos se envían como cadenas de texto
2. Cada comando debe finalizar con tres bytes FF FF FF
3. Ejemplo: `page 0` + [FF FF FF] cambia a la página 0

### Recepción de Eventos (Nextion -> ESP32)

1. Los eventos se reciben como bytes
2. Cada evento termina con tres bytes FF FF FF
3. Formato típico: [Tipo de evento] [ID componente] [ID página] [FF FF FF]
4. Ejemplo: 65 1 0 [FF FF FF] - Botón 1 presionado en la página 0

## Estructura Modular

La comunicación con la pantalla Nextion está implementada en dos módulos principales:

### 1. Módulo Hardware (hardware.h / hardware.cpp)

Este módulo proporciona métodos de bajo nivel para la comunicación directa con la pantalla:

- `nextionSendCommand(command)` - Envía un comando a la pantalla
- `nextionSetPage(pageId)` - Cambia a una página específica
- `nextionSetText(componentId, text)` - Establece el texto de un componente
- `nextionSetValue(componentId, value)` - Establece el valor de un componente numérico
- `nextionCheckForEvents()` - Verifica si hay eventos disponibles
- `nextionGetLastEvent()` - Obtiene el último evento recibido

### 2. Módulo UI Controller (ui_controller.h / ui_controller.cpp)

Este módulo proporciona métodos de alto nivel para gestionar la interfaz de usuario:

- Métodos para mostrar diferentes pantallas (bienvenida, selección, ejecución, etc.)
- Métodos para actualizar información (tiempo, temperatura, nivel de agua, etc.)
- Procesamiento de eventos de la pantalla táctil
- Integración con la lógica de la aplicación

## Ejemplo de Uso

### Inicialización

```cpp
// En setup()
Hardware.init();
UIController.init();

// Mostrar pantalla de bienvenida
UIController.showWelcomeScreen();
```

### Mostrar Pantalla de Ejecución

```cpp
// Mostrar pantalla de ejecución con parámetros iniciales
UIController.showExecutionScreen(programa, fase, nivelAgua, temperatura, rotacion);
```

### Actualizar Datos en Tiempo Real

```cpp
// Actualizar tiempo en formato mm:ss
UIController.updateTime(minutos, segundos);

// Actualizar temperatura
UIController.updateTemperature(valorTemperatura);

// Actualizar nivel de agua
UIController.updateWaterLevel(valorNivel);
```

### Procesar Eventos de Usuario

```cpp
// En loop() o dentro de una tarea asíncrona
UIController.processEvents();

if (UIController.hasUserAction()) {
  String action = UIController.getUserAction();
  
  // Manejar la acción del usuario
  if (action == "START") {
    // Iniciar programa
  } else if (action == "STOP") {
    // Detener programa
  }
}
```

## Diseño de la Interfaz Nextion

La interfaz Nextion se organiza en varias páginas, cada una con un propósito específico:

1. **Página 0 (Bienvenida)** - Pantalla inicial con información del fabricante
2. **Página 1 (Selección)** - Selección de programa y configuración
3. **Página 2 (Ejecución)** - Visualización del programa en ejecución
4. **Página 3 (Edición)** - Edición de parámetros del programa
5. **Página 4 (Error)** - Mostrar mensajes de error
6. **Página 5 (Emergencia)** - Pantalla de parada de emergencia

## Diseño de Componentes Recomendados

Para cada página, se recomienda incluir los siguientes componentes:

### Página de Selección
- Botones para programas (b1, b2, b3)
- Botones para iniciar (b4) y editar (b5)
- Textos para mostrar información (t10-t15)

### Página de Ejecución
- Textos para programa (t1), fase (t2), tiempo (t3), etc.
- Indicadores gráficos para temperatura y nivel
- Barra de progreso (j0)
- Botón para detener (b6)

### Página de Edición
- Textos para título (t1), etiqueta (t2) y valor (t3)
- Slider o control numérico (h0)
- Botones para guardar (b7) y cancelar (b8)

## Tipos de Componentes Nextion

La pantalla Nextion ofrece varios tipos de componentes que podemos utilizar:

1. **Text (txt)** - Para mostrar texto estático o dinámico
2. **Button (btn)** - Para crear botones interactivos
3. **Progress Bar (j)** - Para mostrar progreso
4. **Gauge (z)** - Para mostrar valores con indicador visual
5. **Slider (h)** - Para ajustar valores
6. **Number (n)** - Para mostrar valores numéricos
7. **Timer (tm)** - Para control de tiempo
8. **Waveform (s)** - Para mostrar gráficos de datos

## Codificación de Eventos

Los eventos de la pantalla Nextion se codifican de la siguiente manera:

- **65**: Evento de presión (touch press)
- **66**: Evento de liberación (touch release)
- **67**: Evento de cambio de valor en un componente
- **68**: Evento de cambio de página

## Diseño del Editor Nextion

Para desarrollar la interfaz de la pantalla Nextion, se utiliza el software Nextion Editor. Pasos básicos:

1. Crear un proyecto nuevo con la resolución adecuada para tu pantalla
2. Diseñar cada página con los componentes necesarios
3. Asignar nombres e IDs a los componentes (consistentes con el código)
4. Configurar propiedades y eventos
5. Cargar el diseño a la pantalla mediante tarjeta SD o cable USB-TTL

## Buenas Prácticas

1. **Nomenclatura Consistente**: Mantener una nomenclatura coherente para componentes
2. **Manejo de Errores**: Implementar timeouts para detectar fallos de comunicación
3. **Depuración**: Utilizar mensajes de depuración para diagnosticar problemas
4. **Retroalimentación Visual**: Proporcionar siempre retroalimentación visual al usuario
5. **Optimización**: Minimizar la frecuencia de actualización para evitar sobrecarga
6. **Modularidad**: Mantener separada la lógica de UI de la lógica de negocio

## Diagrama de Flujo de Comunicación

```
┌───────────┐               ┌───────────┐
│           │   Comandos    │           │
│  ESP32    │───────────────▶ Pantalla  │
│           │               │ Nextion   │
│           │   Eventos     │           │
│           │◀───────────────│           │
└───────────┘               └───────────┘
```

## Solución de Problemas Comunes

### No hay respuesta de la pantalla

1. Verificar conexiones físicas (TX/RX cruzados)
2. Verificar velocidad de comunicación (115200 baudios)
3. Comprobar alimentación (5V estable)

### Comandos no reconocidos

1. Verificar sintaxis del comando
2. Confirmar envío de bytes de finalización (FF FF FF)
3. Verificar IDs de componentes

### Eventos no detectados

1. Implementar correctamente la función de lectura de eventos
2. Verificar que los componentes tengan habilitado el envío de eventos
3. Comprobar la lógica de procesamiento de eventos

## Conclusión

La implementación modular de la comunicación con la pantalla Nextion proporciona una interfaz fácil de usar y mantener. Al separar las responsabilidades entre los módulos Hardware y UI Controller, se logra un código más limpio y escalable que facilita futuras mejoras y adaptaciones.

## Referencias

1. [Documentación oficial de Nextion](https://nextion.tech/instruction-set/)
2. [Biblioteca AsyncTaskLib](https://github.com/tttapa/Arduino-Helpers)
3. [Documentación ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
