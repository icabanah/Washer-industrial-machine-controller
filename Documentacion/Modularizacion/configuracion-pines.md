# Configuración de Pines para Controlador de Lavadora Industrial

## Introducción

Este documento detalla la configuración de pines del ESP32-WROOM utilizado en el controlador de lavadora industrial. El sistema utiliza diversos pines para interactuar con componentes físicos como sensores, actuadores, pantalla Nextion y entradas de usuario. La correcta asignación y configuración de estos pines es fundamental para el funcionamiento del sistema.

## Resumen de Asignación de Pines

| Categoría    | Cantidad de Pines | Descripción                            |
|-----------   |-------------------|---------------------                   |
| Entradas     | 1                 | Botón de emergencia                    |
| Actuadores   | 7                 | Control de motor, válvulas y seguridad |
| Sensores     | 4                 | Temperatura y presión                  |
| Comunicación | 2                 | Interfaz con pantalla Nextion          |
| **Total**    | **14**            | Pines utilizados en el proyecto        |

## Detalle de Pines por Categoría

### Entradas de Usuario

| Nombre                | Pin | Tipo          | Configuración | Descripción |
|----------------       |-----|---------------|---------------|-------------|
| `PIN_BTN_EMERGENCIA`  | 15  | INPUT_PULLUP  | Antirrebote en software | Botón de parada de emergencia |

**Notas sobre Entradas:**
- El botón de emergencia utiliza resistencia pull-up interna
- Se implementa antirrebote por software con un tiempo de 50ms
- Se activa en nivel BAJO (LOW) debido a la configuración pull-up

### Actuadores (Salidas)

| Nombre                | Pin | Tipo      | Estado Inicial | Descripción |
|----------------       |-----|-----------|----------------|-------------|
| `PIN_MOTOR_DIR_A`     | 12  | OUTPUT    | LOW            | Control dirección derecha del motor |
| `PIN_MOTOR_DIR_B`     | 14  | OUTPUT    | LOW            | Control dirección izquierda del motor |
| `PIN_CENTRIFUGADO`    | 27  | OUTPUT    | LOW            | Control de centrifugado del motor |
| `PIN_VALVULA_AGUA`    | 26  | OUTPUT    | LOW            | Válvula de entrada de agua |
| `PIN_ELECTROV_VAPOR`  | 33  | OUTPUT    | LOW            | Electroválvula de vapor (calentamiento) |
| `PIN_VALVULA_DESFOGUE` | 13 | OUTPUT    | LOW            | Válvula de drenaje |
| `PIN_MAGNET_PUERTA`   | 25  | OUTPUT    | LOW            | Bloqueo electromagnético de puerta |

**Notas sobre Actuadores:**
- Todos los actuadores se inicializan en estado LOW (desactivados) por seguridad
- La lógica de control asegura que direcciones contrarias del motor no se activen simultáneamente

### Sensores

#### Sensor de Presión (HX710B)

| Nombre             | Pin | Tipo      | Descripción |
|----------------    |-----|-----------|-------------|
| `PIN_PRESION_DOUT` | 5   | INPUT     | Pin de datos del sensor HX710B |
| `PIN_PRESION_SCLK` | 4   | OUTPUT    | Pin de reloj del sensor HX710B |

**Configuración HX710B:**
- Protocolo de comunicación: Dos hilos (DOUT y SCLK)
- Rangos de calibración definidos en config.h:
  - `NIVEL_PRESION_1`: 601
  - `NIVEL_PRESION_2`: 628
  - `NIVEL_PRESION_3`: 645
  - `NIVEL_PRESION_4`: 663

#### Sensor de Temperatura (OneWire Dallas)

| Nombre             | Pin | Tipo          | Descripción |
|----------------    |-----|---------------|-------------|
| `PIN_TEMP_SENSOR`  | 23  | INPUT/OUTPUT  | Pin de datos para sensor DS18B20 |

**Configuración DS18B20:**
- Protocolo: OneWire
- Resolución: 9 bits (configurable)
- Dirección del sensor: `0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A`
- Rango de temperatura: Definido por `TEMP_RANGE` (±2°C por defecto)

### Comunicación con Pantalla Nextion

| Nombre             | Pin | Función ESP32 | Conexión Nextion | Descripción |
|----------------    |-----|---------------|-----------------|-------------|
| `NEXTION_RX_PIN`   | 25  | TX2 (U2_TXD)    | RX              | Transmisión de datos a Nextion |
| `NEXTION_TX_PIN`   | 27  | RX2 (U2_RXD)    | TX              | Recepción de datos desde Nextion |

**Configuración de Comunicación:**
- Puerto: Serial2
- Baudios: 115200
- Configuración: 8N1 (8 bits de datos, sin paridad, 1 bit de parada)
- Tiempo de espera (timeout): 100ms

## Diagrama de Conexiones

```
                         +---------------+
                         |   ESP32-WROOM |
                         +-------+-------+
                                 |
     +------------+              |
     | Emergencia |----[D15]-----|
     +------------+              |
                                 |                 +-------------+
     +------------+              |                 |   Pantalla  |
     |  Motor     |-+--[D12]-----|                 |   Nextion   |
     |  Direccion | |            |                 |             |
     |            |-+--[D14]-----|                 |             |
     +------------+              |                 |             |
                                 |                 |             |
     +------------+              |                 |             |
     |Centrifugado|----[D27]-----|                 |             |
     +------------+              |-----[TX2]-------|RX           |
                                 |                 |             |
     +------------+              |-----[RX2]-------|TX           |
     |  Válvula   |----[D26]-----|                 |             |
     |    Agua    |              |                 +-------------+
     +------------+              |
                                 |
     +------------+              |                 +-------------+
     |   Vapor    |----[D33]-----|                 |   Sensor    |
     | Calentador |              |                 | Temperatura |
     +------------+              |                 |  (DS18B20)  |
                                 |-----[D23]-------|Data         |
     +------------+              |                 +-------------+
     |  Válvula   |----[D13]-----|
     |  Drenaje   |              |
     +------------+              |                 +-------------+
                                 |                 |   Sensor    |
     +------------+              |                 |  Presión    |
     |   Bloqueo  |----[D25]-----|                 |  (HX710B)   |
     |   Puerta   |              |-----[D5]--------|DOUT         |
     +------------+              |-----[D4]--------|SCLK         |
                                 |                 +-------------+
                         +-------+-------+
                         |               |
                         +---------------+
```

## Consideraciones para Instalación y Conexión

### Recomendaciones Generales

1. **Separación de Circuitos de Potencia y Control**:
   - Utilizar optoacopladores o relés para aislar circuitos de potencia
   - Mantener separadas las líneas de alimentación de señales digitales

2. **Protección de Entradas**:
   - Incluir resistencias limitadoras en las entradas (especialmente botón de emergencia)
   - Considerar añadir diodos de protección contra sobretensiones
   
3. **Actuadores y Cargas Inductivas**:
   - Usar diodos volantes (flyback) para proteger contra picos de voltaje inverso
   - Dimensionar correctamente los relés según las cargas a controlar
   
4. **Cableado**:
   - Utilizar cables apantallados para sensores y comunicación serial
   - Separar físicamente el cableado de potencia del cableado de señal
   - Etiquetar claramente todos los cables para facilitar mantenimiento
   
5. **Alimentación**:
   - Proporcionar una fuente de alimentación estable y adecuada para el ESP32
   - Considerar añadir capacitores de filtrado cerca del microcontrolador

### Notas sobre Sensores

1. **Sensor de Temperatura (DS18B20)**:
   - Requiere resistencia pull-up de 4.7kΩ entre el pin de datos y VCC
   - Se recomienda cable apantallado para distancias mayores a 1 metro
   - La librería DallasTemperature gestiona la comunicación OneWire

2. **Sensor de Presión (HX710B)**:
   - Sensible a ruido eléctrico, usar cableado apantallado
   - Requiere calibración inicial para establecer offsets
   - La lectura se convierte a nivel de agua mediante la función `_convertPressureToLevel()`

## Pruebas y Verificación

Se recomienda realizar las siguientes pruebas para verificar la correcta configuración de pines:

1. **Prueba de Continuidad**:
   - Verificar continuidad entre los pines del ESP32 y los conectores de los dispositivos

2. **Prueba de Aislamiento**:
   - Comprobar que no hay cortocircuitos entre líneas de señal

3. **Prueba de Control Individual**:
   - Verificar cada actuador de forma independiente usando programas de prueba

4. **Prueba de Sensores**:
   - Confirmar lecturas correctas de los sensores de temperatura y presión

5. **Prueba de Comunicación**:
   - Verificar la comunicación bidireccional con la pantalla Nextion

## Código de Configuración

El siguiente fragmento de código muestra la configuración de pines en el archivo `config.h`:

```cpp
// === DEFINICIÓN DE PINES ===
// Entrada de Emergencia
#define PIN_BTN_EMERGENCIA 15  // Pin para botón de emergencia con antirrebote

// Salidas
#define PIN_MOTOR_DIR_A 12
#define PIN_MOTOR_DIR_B 14
#define PIN_CENTRIFUGADO 27
#define PIN_VALVULA_AGUA 26
#define PIN_ELECTROV_VAPOR 33
#define PIN_VALVULA_DESFOGUE 13
#define PIN_MAGNET_PUERTA 25

// Comunicación Serial para Nextion
#define NEXTION_SERIAL Serial2
#define NEXTION_BAUD_RATE 115200
#define NEXTION_RX_PIN 25  // ESP32 U2_RXD pin conectado al TX de Nextion
#define NEXTION_TX_PIN 27  // ESP32 U2_TXD pin conectado al RX de Nextion

// Pines para sensores
#define PIN_TEMP_SENSOR 23
#define PIN_PRESION_DOUT 5
#define PIN_PRESION_SCLK 4
```

## Conclusión

La correcta configuración y conexión de los pines del ESP32 es fundamental para el funcionamiento del controlador de lavadora industrial. Este documento proporciona una referencia completa para la asignación de pines, su configuración y las consideraciones para su implementación física.

Se recomienda revisar periódicamente las conexiones físicas y verificar que concuerdan con esta documentación, especialmente después de realizar mantenimiento o modificaciones al sistema.
