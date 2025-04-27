# Programas de Lavado

Este documento describe los diferentes programas de lavado disponibles en el sistema "automLavanderia_v1" y sus características específicas.

## Analogía de los Programas

Los programas de lavado pueden compararse con recetas de cocina:

- Cada **programa** es como una receta completa (por ejemplo, una lasaña o un bizcocho).
- Cada **fase** es como un paso en la receta (por ejemplo, preparar la bechamel, hornear, etc.).
- Los **parámetros** son como los ingredientes y las instrucciones específicas (cantidades, temperaturas, tiempos).
- El **controlador** es como el chef que ejecuta la receta según lo indicado.

## Estructura de Programas

El sistema admite 3 programas de lavado diferentes, cada uno con características específicas:

### Matrices de Configuración

Los parámetros de cada programa se almacenan en matrices bidimensionales:

```c
uint8_t NivelAgua[3][4];         // Niveles de agua
uint8_t RotacionTam[3][4];       // Velocidades de rotación
uint8_t TemperaturaLim[3][4];    // Temperaturas
uint8_t TemporizadorLim[3][4];   // Tiempos
uint8_t TiempoRotacion[3][2];    // Tiempos de rotación y pausa
uint8_t TiempoEntFase[3][4];     // Tiempos entre fases
```

Donde:
- La primera dimensión [3] representa los 3 programas (índices 0, 1, 2 para programas 1, 2, 3)
- La segunda dimensión [4] representa las 4 fases posibles (índices 0, 1, 2, 3 para fases 1, 2, 3, 4)

## Programa 1: Lavado Básico

### Características
- Enfoque en control de temperatura y rotación
- Una sola fase de lavado
- Inicia motor y después controla temperatura
- Ideal para cargas pequeñas o lavados rápidos

### Secuencia de operación
1. Bloquear puerta
2. Activar válvula de agua
3. Iniciar motor con patrón de rotación establecido
4. Monitorear nivel de agua
5. Al alcanzar nivel deseado, iniciar control de temperatura
6. Mantener lavado por tiempo establecido
7. Finalizar programa

### Inicialización específica
```c
if (programa == 1) {
  motorON = true;
  iniciarTiempoRotacion();
  // Sensor temperatura se inicia cuando se alcanza nivel de agua
}
```

## Programa 2: Lavado Térmico

### Características
- Enfoque en control de temperatura
- Una sola fase de lavado
- Prioriza el calentamiento del agua
- Ideal para desinfección y prendas que requieren tratamiento térmico

### Secuencia de operación
1. Bloquear puerta
2. Activar válvula de agua
3. Iniciar sensor de temperatura
4. Monitorear nivel de agua
5. Al alcanzar nivel deseado, iniciar motor
6. Mantener temperatura y rotación por tiempo establecido
7. Finalizar programa

### Inicialización específica
```c
else if (programa == 2) {
  iniciarSensorTemperatura();
  // Motor se inicia cuando se alcanza nivel de agua
}
```

## Programa 3: Lavado Completo

### Características
- Programa más completo con 4 fases
- Control simultáneo de motor, temperatura y nivel
- Cada fase puede tener diferentes parámetros
- Tiempos de pausa entre fases

### Fases típicas
1. **Prelavado**: Nivel de agua bajo, temperatura media, rotación suave
2. **Lavado principal**: Nivel de agua medio, temperatura alta, rotación intensa
3. **Aclarado**: Nivel de agua alto, temperatura baja, rotación media
4. **Centrifugado**: Sin agua, sin calentamiento, rotación muy intensa

### Secuencia de operación
1. Bloquear puerta
2. Fase 1: Configurar parámetros según primera fase
3. Al completar tiempo de fase 1:
   - Pausar programa
   - Esperar tiempo entre fases
4. Fase 2: Reconfigurar parámetros
5. Repetir proceso hasta completar las 4 fases
6. Finalizar programa

### Inicialización específica
```c
else {  // programa == 3
  motorON = true;
  iniciarSensorTemperatura();
  iniciarTiempoRotacion();
}
```

### Transición entre fases
```c
// En controladorTemporizador():
if (segunderoEntreFase == 0) {
  segundosTemporizador.Stop();
  if (programa == 3) {
    fase++;
    reiniciarPrograma();
    if (fase > 4) {
      terminarPrograma();
    }
    else {
      iniciarTemporizador();
    }
  }
  else {
    terminarPrograma();
  }
}
```

## Parámetros Configurables

### Nivel de Agua
- Rango: 1-4 (4 niveles predefinidos)
- Control a través del sensor de presión

### Velocidad de Rotación
- Rango: 1-3 (3 velocidades predefinidas)
- Cada nivel tiene definidos dos tiempos:
  - Tiempo de rotación en cada dirección
  - Tiempo de pausa entre cambios de dirección

### Temperatura
- Rango: 0-99 (grados Celsius)
- Rango de tolerancia: ±2°C
- Control a través del sensor Dallas

### Tiempo
- Rango: 0-255 (minutos)
- Cuenta regresiva durante la ejecución
- Mostrado en formato MM:SS en el LCD

## Visualización y Edición de Programas

### Pantalla de Selección para Programa 3
```
P24 N1234 T1234  // Programa 24, Niveles, Tiempos
T12/34/67/89 R1234 // Temperaturas, Rotaciones
```

### Pantalla de Selección para Programas 1 y 2
```
P22 Niv:2 Tie:30  // Programa 22, Nivel 2, Tiempo 30
Temp:45 Rot:2 // Temperatura 45°C, Rotación nivel 2
```

### Interfaz de Edición
```
P22 Seleccione: 
Fase:1 Nivel:2  // Editando Nivel de Fase 1
```

```
P22 Seleccione:
Fase:1 Tiemp:30 // Editando Tiempo de Fase 1
```

```
P22 Seleccione:
Fase:1 Tempe:45 // Editando Temperatura de Fase 1
```

```
P22 Seleccione:
Fase:1 VRota:2 // Editando Velocidad de Rotación de Fase 1
```

## Funciones de Control de Programa

### Iniciar Programa
```c
void iniciarPrograma() {
  tiempoCumplido = false;
  programaTerminado = false;
  programaEnPausa = false;
  fase = 1;
  
  // Activar seguridad y válvulas
  digitalWrite(MagnetPuerta, HIGH);
  digitalWrite(ValvulAgua, HIGH);
  digitalWrite(ValvulDesfogue, HIGH);
  
  // Reiniciar temporizadores
  segunderoTemporizador = 0;
  segunderoMotor = 0;
  
  // Iniciar procesos según programa seleccionado
  iniciarTemporizador();
  iniciarSensorPresion();
  
  if (programa == 1) {
    motorON = true;
    iniciarTiempoRotacion();
  }
  else if (programa == 2) {
    iniciarSensorTemperatura();
  }
  else {
    motorON = true;
    iniciarSensorTemperatura();
    iniciarTiempoRotacion();
  }
  
  pintarVentanaEjecucion();
}
```

### Pausar Programa
```c
void pausarPrograma() {
  programaEnPausa = true;
  motorON = false;
  sensarTemperatura = false;
  sensarPresion = false;
  
  // Reiniciar contadores
  segunderoEntreFase = 0;
  segunderoMotor = 0;
  segunderoTemporizador = 0;
  
  // Desactivar actuadores
  digitalWrite(MotorDirA, LOW);
  digitalWrite(MotorDirB, LOW);
  digitalWrite(ValvulAgua, LOW);
  digitalWrite(ElectrovVapor, LOW);
  digitalWrite(ValvulDesfogue, LOW);
}
```

### Terminar Programa
```c
void terminarPrograma() {
  tiempoCumplido = true;
  programaTerminado = true;
  programaEnPausa = false;
  motorON = false;
  sensarPresion = false;
  sensarTemperatura = false;
  temporizadorON = false;
  fase = 1;
  
  // Reiniciar temporizadores
  segunderoTemporizador = 0;
  segunderoMotor = 0;
  segundosTemporizador.Stop();
  
  // Desactivar todos los actuadores
  digitalWrite(MotorDirA, LOW);
  digitalWrite(MotorDirB, LOW);
  digitalWrite(MagnetPuerta, LOW);
  digitalWrite(ValvulAgua, LOW);
  digitalWrite(ElectrovVapor, LOW);
  digitalWrite(ValvulDesfogue, LOW);
  
  // Aviso sonoro
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer, LOW);
  
  pintarVentanaSeleccion();
}
```

## Personalización de Programas

El sistema permite personalizar todos los parámetros de cada programa mediante:

1. Seleccionar programa con botones Aumentar/Disminuir
2. Activar modo edición con botón Editar
3. Navegar entre variables (Nivel, Temperatura, Tiempo, Rotación)
4. Modificar valores con botones Aumentar/Disminuir
5. Guardar cambios con botón Comenzar

Las configuraciones se almacenan en la EEPROM, permitiendo que persistan después de apagar el equipo.

## Interacción Especial Durante Programa

Durante la ejecución de un programa, el usuario puede:

- **Incrementar nivel de agua**: Presionando Aumentar + Editar
- **Disminuir nivel de agua**: Presionando Disminuir + Editar
- **Terminar programa**: Presionando Parar + Editar

Esta funcionalidad permite ajustes en tiempo real según las necesidades específicas.

## Seguridad y Limitaciones

- **Contador de bloqueo**: Limita el número de usos para mantenimiento
- **Bloqueo automático**: Si contador > limiteBloqueo, muestra error
- **Variables de antirebote**: Evitan activaciones falsas de botones
- **Controles de verificación**: Confirman lecturas de sensores antes de actuar

Estas medidas garantizan un funcionamiento seguro y confiable del sistema.
