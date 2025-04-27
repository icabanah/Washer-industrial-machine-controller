# Componentes Hardware del Sistema

## Diagrama de Conexiones

El sistema "automLavanderia_v1" utiliza varios componentes hardware conectados al microcontrolador Arduino. A continuación se presentan las conexiones principales:

### Pantalla LCD
- **RS**: Pin 19
- **EN**: Pin 18
- **D4**: Pin 17
- **D5**: Pin 16
- **D6**: Pin 15
- **D7**: Pin 14

### Sensores

#### Sensor de Temperatura (OneWire Dallas)
- **Pin de Datos**: Pin 25
- **Resolución**: 9 bits
- **Dirección del sensor**: 0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A

#### Sensor de Presión (HX710B)
- **DOUT**: Pin 42
- **SCLK**: Pin 40

### Entradas (Botones)
- **btnParar**: Pin 2
- **btnDisminuir**: Pin 3
- **btnAumentar**: Pin 4
- **btnEditar**: Pin 26
- **btnProgramarNivelAgua**: Pin 30
- **btnComenzar**: Pin 28

### Salidas (Actuadores)
- **MotorDirA**: Pin 12 (Control dirección A del motor)
- **MotorDirB**: Pin 11 (Control dirección B del motor)
- **ValvulAgua**: Pin 10 (Control de la válvula de entrada de agua)
- **ElectrovVapor**: Pin 9 (Control de la electroválvula de vapor para calentar)
- **ValvulDesfogue**: Pin 8 (Control de la válvula de drenaje)
- **MagnetPuerta**: Pin 7 (Control del bloqueo electromagnético de la puerta)
- **buzzer**: Pin 34 (Señal acústica)

## Descripción de Componentes

### Microcontrolador
El sistema utiliza un microcontrolador Arduino, posiblemente un Arduino Mega o similar, debido a la cantidad de pines utilizados y la complejidad del sistema.

### Pantalla LCD
La pantalla LCD de 16x2 caracteres se utiliza para mostrar:
- Información de programas
- Estado del ciclo actual
- Valores de temperatura y nivel
- Tiempo restante
- Mensajes de error

Su funcionamiento es similar al de un tablero de mandos en un vehículo, proporcionando información visual sobre el estado del sistema.

### Sensor de Temperatura
El sensor Dallas Temperature proporciona mediciones precisas de la temperatura del agua. 

**Analogía**: Funciona como un termómetro digital que constantemente envía lecturas al controlador, permitiendo mantener la temperatura dentro de los rangos deseados (similar al termostato de un horno).

### Sensor de Presión
El sensor HX710B se utiliza para medir la presión del agua dentro del tambor, lo que permite determinar el nivel de llenado.

**Analogía**: Es como la boya de un tanque de agua, pero en lugar de usar un mecanismo físico, utiliza la presión hidrostática para determinar cuánta agua hay en el tambor.

### Sistema de Botones
Los botones proporcionan la interfaz física para que el usuario interactúe con el sistema. 

**Analogía**: Funcionan como el panel de control de un electrodoméstico, donde cada botón tiene una función específica:
- Aumentar/disminuir: Para navegar entre opciones
- Editar: Para entrar en modo configuración
- Comenzar: Para iniciar el programa seleccionado
- Parar: Para interrumpir la operación

### Sistema de Motor
El motor bidireccional del tambor es controlado mediante dos pines (MotorDirA y MotorDirB) que permiten controlar la dirección de giro.

**Analogía**: Es similar al cambio de marcha en un vehículo, donde se puede seleccionar avanzar, retroceder o estar en punto muerto (ambos pines en LOW).

### Sistema de Válvulas
Las válvulas controlan el flujo de agua y vapor en el sistema:
- Válvula de agua: Controla el ingreso de agua al tambor
- Electroválvula de vapor: Regula el calentamiento del agua
- Válvula de desfogue: Permite el drenaje del agua

**Analogía**: Funcionan como los grifos de un sistema de fontanería, abriéndose y cerrándose según las necesidades del ciclo.

### Sistema de Seguridad
El bloqueo magnético de la puerta (MagnetPuerta) asegura que la puerta permanezca cerrada durante el ciclo de lavado.

**Analogía**: Opera como el seguro de una puerta de automóvil, que se activa mientras el vehículo está en movimiento para evitar aperturas accidentales.

## Parámetros de Sensores

### Niveles de Presión
El sistema define cuatro niveles de presión para determinar el nivel de agua:
- **Nivel 1**: <= 601 unidades
- **Nivel 2**: > 601 y <= 628 unidades
- **Nivel 3**: > 628 y <= 645 unidades
- **Nivel 4**: > 645 unidades

### Control de Temperatura
Para el control de temperatura, el sistema:
1. Establece un valor objetivo (valorTemperaturaLim)
2. Monitorea la temperatura actual (valorTemperatura)
3. Define un rango de tolerancia (rangoTemperatura = 2°C)
4. Activa o desactiva el calentador según sea necesario

## Interacción entre Componentes

El sistema de control supervisa constantemente el estado de los sensores y ajusta los actuadores en función de las necesidades del programa en ejecución.

**Analogía**: El sistema funciona como un termostato inteligente que no solo monitorea la temperatura, sino también otros parámetros como el nivel de agua y el tiempo, y toma decisiones basadas en estos factores para controlar los diferentes componentes.

Los componentes hardware están diseñados para trabajar en conjunto, proporcionando un sistema robusto y adaptable para el control de lavadoras industriales.
