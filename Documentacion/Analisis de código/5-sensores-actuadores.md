# Sensores y Actuadores

Este documento detalla el funcionamiento e interacción de los sensores y actuadores en el sistema "automLavanderia_v1".

## Analogía del Sistema de Control

El sistema de sensores y actuadores puede compararse con el sistema nervioso humano:

- **Sensores**: Son como los sentidos (vista, tacto, oído) que recopilan información del entorno.
- **Microcontrolador**: Actúa como el cerebro, procesando la información y tomando decisiones.
- **Actuadores**: Son como los músculos, que ejecutan las acciones decididas por el cerebro.
- **Cables/Comunicación**: Funcionan como el sistema nervioso, transmitiendo señales entre los diferentes componentes.

## 1. Sensores

### 1.1. Sensor de Temperatura (Dallas OneWire)

#### Características técnicas
- **Tipo**: Sensor digital DS18B20
- **Interfaz**: OneWire (comunicación de un solo cable)
- **Resolución**: 9 bits (configurable)
- **Rango**: -55°C a +125°C
- **Dirección del dispositivo**: 0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A

#### Inicialización
```c
OneWire bus(25);  // Pin de datos
DallasTemperature thermo(&bus);
DeviceAddress sensorTemperatura = {0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A};
uint8_t resolucion = 9;

// En setup()
thermo.begin();
thermo.setResolution(resolucion);
```

#### Lectura de datos
```c
// En controladorSensorTemperatura()
thermo.requestTemperatures();
valorTemperatura = round(thermo.getTempCByIndex(0));
```

#### Interpretación y acción
El sistema compara la temperatura leída con el valor objetivo y un rango de tolerancia:
```c
// En controladorSensorTemperatura()
if ((valorTemperatura >= (valorTemperaturaLim + rangoTemperatura)) && sensarTemperatura) {
  digitalWrite(ElectrovVapor, LOW);  // Apagar calentamiento
} else if ((valorTemperatura <= (valorTemperaturaLim - rangoTemperatura)) && sensarTemperatura) {
  digitalWrite(ElectrovVapor, HIGH);  // Encender calentamiento
}
```

### 1.2. Sensor de Presión (HX710B)

#### Características técnicas
- **Tipo**: Transductor de presión HX710B
- **Interfaz**: Digital (DOUT y SCLK)
- **Unidades**: Pascal (Pa)
- **Niveles de referencia**:
  - Nivel 1: ≤ 601 Pa
  - Nivel 2: > 601 Pa y ≤ 628 Pa
  - Nivel 3: > 628 Pa y ≤ 645 Pa
  - Nivel 4: > 645 Pa

#### Inicialización
```c
const int DOUT = 42;  // Pin de datos
const int SCLK = 40;  // Pin de reloj
HX710B pressure_sensor;

// En setup()
pressure_sensor.begin(DOUT, SCLK);
```

#### Lectura de datos
```c
// En controladorSensorPresion()
if (pressure_sensor.is_ready()) {
  valorPresion = pressure_sensor.pascal();
  
  // Determinar nivel según presión
  if (valorPresion <= nivelPresion1) {
    valorNivel = 1;
  } else if (valorPresion > nivelPresion1 && valorPresion <= nivelPresion2) {
    valorNivel = 2;
  } else if (valorPresion > nivelPresion2 && valorPresion <= nivelPresion3) {
    valorNivel = 3;
  } else if (valorPresion > nivelPresion3) {
    valorNivel = 4;
  }
}
```

#### Interpretación y acción
El sistema verifica si se ha alcanzado el nivel de agua deseado:
```c
// En controladorSensorPresion()
if ((valorNivel >= valorNivelLim + 1) && sensarPresion) {
  contadorNivel++;  // Contador para evitar falsos positivos
}

if ((valorNivel >= valorNivelLim + 1) && sensarPresion && (contadorNivel >= 5)) {
  digitalWrite(ValvulAgua, LOW);  // Cerrar válvula de agua
  sensarPresion = false;
  temporizadorON = true;
  
  // Acciones adicionales según el programa
  if (programa == 1) {
    iniciarSensorTemperatura();
  } else if (programa == 2) {
    motorON = true;
    iniciarTiempoRotacion();
  }
}
```

## 2. Actuadores

### 2.1. Sistema de Motor

#### Componentes
- **MotorDirA** (Pin 12): Control dirección A
- **MotorDirB** (Pin 11): Control dirección B

#### Patrones de control
El motor puede estar en tres estados principales:
1. **Dirección A**: MotorDirA = HIGH, MotorDirB = LOW
2. **Dirección B**: MotorDirA = LOW, MotorDirB = HIGH
3. **Detenido**: MotorDirA = LOW, MotorDirB = LOW

#### Secuencia de operación
```c
// En controladorDireccionMotor()
switch (direccion) {
  case 1:  // Dirección A
    if (segunderoMotor == tiempoRotacion) {
      direccion = 2;
      pausa = true;
      segunderoMotor = 0;
      digitalWrite(MotorDirA, LOW);
      digitalWrite(MotorDirB, LOW);
    }
    break;
    
  case 2:  // Pausa
    if (segunderoMotor == tiempoPausa) {
      if (pausa == true) {
        direccion = 3;
        digitalWrite(MotorDirA, LOW);
        digitalWrite(MotorDirB, HIGH);
      } else {
        direccion = 1;
        digitalWrite(MotorDirA, HIGH);
        digitalWrite(MotorDirB, LOW);
      }
      segunderoMotor = 0;
    }
    break;
    
  case 3:  // Dirección B
    if (segunderoMotor == tiempoRotacion) {
      direccion = 2;
      pausa = false;
      segunderoMotor = 0;
      digitalWrite(MotorDirA, LOW);
      digitalWrite(MotorDirB, LOW);
    }
    break;
}
```

#### Configuración de velocidad
La velocidad no se controla mediante PWM, sino a través de tres niveles predefinidos que determinan los tiempos de rotación y pausa:
```c
uint8_t TiempoRotacion[3][2] = {{4, 2}, {10, 4}, {20, 6}};
```
Donde:
- Primera columna: Tiempo de rotación en segundos
- Segunda columna: Tiempo de pausa en segundos
- Filas: Niveles de velocidad (1-3)

### 2.2. Sistema de Válvulas

#### Válvula de Agua (ValvulAgua - Pin 10)
Controla el flujo de agua hacia el tambor:
- **HIGH**: Válvula abierta (llenado)
- **LOW**: Válvula cerrada

#### Electroválvula de Vapor (ElectrovVapor - Pin 9)
Controla el calentamiento del agua:
- **HIGH**: Calentamiento activado
- **LOW**: Calentamiento desactivado

#### Válvula de Desfogue (ValvulDesfogue - Pin 8)
Controla el drenaje del agua:
- **HIGH**: Válvula cerrada (retención de agua)
- **LOW**: Válvula abierta (drenaje)

### 2.3. Sistema de Seguridad

#### Bloqueo de Puerta (MagnetPuerta - Pin 7)
Activa un electroimán para bloquear la puerta durante la operación:
- **HIGH**: Puerta bloqueada
- **LOW**: Puerta desbloqueada

#### Buzzer (buzzer - Pin 34)
Proporciona indicaciones sonoras:
- **HIGH**: Buzzer activado
- **LOW**: Buzzer desactivado

Utilizado principalmente para indicar la finalización del programa:
```c
// En terminarPrograma()
digitalWrite(buzzer, HIGH);
delay(2000);
digitalWrite(buzzer, LOW);
```

## 3. Interacción entre Sensores y Actuadores

### 3.1. Ciclo de Control de Nivel de Agua

1. El sistema activa la válvula de agua (`ValvulAgua = HIGH`)
2. El sensor de presión monitorea continuamente el nivel
3. Cuando el nivel alcanza el valor objetivo, se cierra la válvula (`ValvulAgua = LOW`)
4. Se procede a la siguiente fase del programa

### 3.2. Ciclo de Control de Temperatura

1. Se establece la temperatura objetivo según el programa
2. Si la temperatura actual es menor que el objetivo, se activa el calentamiento (`ElectrovVapor = HIGH`)
3. El sensor monitorea constantemente la temperatura
4. Cuando la temperatura supera el objetivo, se desactiva el calentamiento (`ElectrovVapor = LOW`)
5. El sistema mantiene la temperatura dentro del rango deseado

### 3.3. Ciclo de Control del Motor

1. Se establece el patrón de rotación según el programa
2. El motor gira en dirección A durante el tiempo especificado
3. Se pausa el motor durante el tiempo de pausa
4. El motor gira en dirección B durante el tiempo especificado
5. Se repite el ciclo hasta completar la fase actual

### 3.4. Coordinación General

Los sensores y actuadores operan en conjunto bajo la coordinación del microcontrolador, que:

1. **Monitoriza** constantemente los sensores
2. **Compara** los valores leídos con los objetivos
3. **Controla** los actuadores en función de las comparaciones
4. **Gestiona** las transiciones entre fases y estados

Este enfoque basado en retroalimentación continua permite al sistema adaptarse a diferentes condiciones y mantener un funcionamiento estable y seguro.

## 4. Pseudocódigo de la Interacción

```
MIENTRAS el programa esté en ejecución:
    
    // Control de temperatura
    Leer temperatura actual
    SI temperatura > (objetivo + tolerancia) ENTONCES
        Apagar calentamiento
    SINO SI temperatura < (objetivo - tolerancia) ENTONCES
        Encender calentamiento
    FIN SI
    
    // Control de nivel de agua
    SI sensando nivel ENTONCES
        Leer presión actual
        Determinar nivel de agua
        SI nivel >= objetivo Y contador confirmación > 5 ENTONCES
            Cerrar válvula de agua
            Proceder a siguiente fase
        FIN SI
    FIN SI
    
    // Control de motor
    SI motor encendido ENTONCES
        SEGÚN dirección actual
            CASO dirección A:
                SI tiempo rotación cumplido ENTONCES
                    Parar motor
                    Cambiar a estado pausa
                FIN SI
            CASO pausa:
                SI tiempo pausa cumplido ENTONCES
                    SI pausa verdadera ENTONCES
                        Cambiar a dirección B
                    SINO
                        Cambiar a dirección A
                    FIN SI
                FIN SI
            CASO dirección B:
                SI tiempo rotación cumplido ENTONCES
                    Parar motor
                    Cambiar a estado pausa
                FIN SI
        FIN SEGÚN
    FIN SI
    
    // Control del tiempo
    SI temporizador encendido ENTONCES
        Decrementar contador de tiempo
        SI tiempo == 0 ENTONCES
            SI no es última fase ENTONCES
                Preparar siguiente fase
            SINO
                Finalizar programa
            FIN SI
        FIN SI
    FIN SI
FIN MIENTRAS
```

## 5. Diagrama de Interacción Sensor-Actuador

A continuación se representa de manera esquemática la interacción cíclica entre sensores y actuadores:

```
Sensor de Temperatura --> [Lectura] --> Microcontrolador --> [Decisión] --> Electroválvula de Vapor
      ^                                       |
      |                                       v
      +---------------------------------------+
            [Retroalimentación continua]

Sensor de Presión --> [Lectura] --> Microcontrolador --> [Decisión] --> Válvula de Agua
      ^                                  |
      |                                  v
      +----------------------------------+
          [Retroalimentación continua]

Temporizador --> [Control] --> Microcontrolador --> [Decisión] --> Motor
      ^                              |
      |                              v
      +------------------------------+
          [Secuencia programada]
```

Este sistema de control con retroalimentación permite que la lavadora industrial mantenga condiciones óptimas durante todo el ciclo de lavado, adaptándose a posibles variaciones y garantizando resultados consistentes.
