# Guía de Implementación Modular

## Introducción

Este documento proporciona una guía paso a paso para implementar la estructura modular propuesta para el controlador de lavadora industrial. La implementación se realizará de manera progresiva, asegurando que en cada paso el sistema siga siendo funcional.

## Analogía: Reconstrucción de un Edificio

La implementación de la estructura modular se puede comparar con la renovación de un edificio histórico, donde se quiere mantener la apariencia y funcionalidad externa mientras se moderniza la estructura interna. Los trabajos se realizan por secciones, asegurándose de que el edificio siga siendo habitable y funcional durante todo el proceso.

## Plan de Implementación

### Fase 1: Preparación y Estructuración Inicial

1. **Crear Estructura de Archivos**:
   - Crear todos los archivos de cabecera (.h) y de implementación (.cpp) según la estructura propuesta.
   - Organizar los archivos en el proyecto Arduino.

2. **Implementar Archivo de Configuración**:
   - Crear el archivo `config.h` con todas las definiciones y constantes.
   - Reemplazar valores literales en el código original por las constantes definidas.

3. **Preparar el Archivo Principal**:
   - Modificar `mainController.ino` para incluir todos los nuevos módulos.
   - Mantener temporalmente el código original intacto.

### Fase 2: Implementación de Módulos Básicos

1. **Implementar Módulo de Hardware**:
   - Crear la clase `HardwareClass` con métodos básicos para entrada/salida.
   - Implementar la inicialización de pines y LCD.
   - Verificar que la inicialización básica funcione correctamente.

2. **Implementar Módulo de Utilidades**:
   - Crear la clase `UtilsClass` con funciones auxiliares.
   - Implementar el manejo del temporizador asíncrono.
   - Probar las funciones básicas de utilidad.

3. **Implementar Módulo de Almacenamiento**:
   - Crear la clase `StorageClass` con métodos para lectura/escritura en EEPROM.
   - Adaptar las funciones existentes de recuperación/guardado de valores.
   - Verificar que la carga y guardado de datos funcione correctamente.

### Fase 3: Implementación de Módulos de Control Físico

1. **Implementar Módulo de Sensores**:
   - Crear la clase `SensorsClass` para gestionar sensores de temperatura y presión.
   - Trasladar la lógica actual de lectura y procesamiento de sensores.
   - Probar que las lecturas de sensores sean correctas.

2. **Implementar Módulo de Actuadores**:
   - Crear la clase `ActuatorsClass` para controlar motores, válvulas, etc.
   - Trasladar la lógica actual de control de dispositivos.
   - Verificar que el control de actuadores funcione correctamente.

### Fase 4: Implementación de Módulos de Interfaz y Control

1. **Implementar Módulo de UI Controller**:
   - Crear la clase `UIControllerClass` para manejar la interfaz de usuario.
   - Trasladar las funciones actuales de pintado de pantallas.
   - Verificar que todas las pantallas se muestren correctamente.

2. **Implementar Módulo de Program Controller**:
   - Crear la clase `ProgramControllerClass` para la lógica central.
   - Trasladar progresivamente la lógica de control de programas y estados.
   - Probar cada aspecto del control de programas.

### Fase 5: Integración y Finalización

1. **Integrar Módulos**:
   - Conectar todos los módulos a través de sus interfaces.
   - Asegurar que la comunicación entre módulos funcione correctamente.

2. **Simplificar el Archivo Principal**:
   - Reducir `mainController.ino` a un simple coordinador.
   - Mover toda la lógica específica a los módulos correspondientes.

3. **Probar el Sistema Completo**:
   - Realizar pruebas exhaustivas de todas las funcionalidades.
   - Verificar que no se haya perdido ninguna funcionalidad durante la modularización.

## Ejemplos de Código para Módulos Clave

### Ejemplo: Implementación progresiva del Módulo Hardware

#### Paso 1: Definición inicial de la interfaz
```cpp
// hardware.h (versión inicial)
#ifndef HARDWARE_H
#define HARDWARE_H

#include "config.h"
#include <LiquidCrystal.h>

class HardwareClass {
public:
  void init();
  LiquidCrystal* getLCD();
  
private:
  LiquidCrystal _lcd;
};

extern HardwareClass Hardware;

#endif
```

#### Paso 2: Implementación básica
```cpp
// hardware.cpp (versión inicial)
#include "hardware.h"

HardwareClass Hardware;

void HardwareClass::init() {
  _lcd = LiquidCrystal(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
  _lcd.begin(LCD_COLS, LCD_ROWS);
  
  // Configurar pines (mínimo necesario para probar)
  pinMode(PIN_BTN_PARAR, INPUT);
  pinMode(PIN_MOTOR_DIR_A, OUTPUT);
  digitalWrite(PIN_MOTOR_DIR_A, LOW);
}

LiquidCrystal* HardwareClass::getLCD() {
  return &_lcd;
}
```

#### Paso 3: Expansión de funcionalidad
```cpp
// Añadir progresivamente más métodos a hardware.h
void digitalWrite(uint8_t pin, uint8_t state);
uint8_t digitalRead(uint8_t pin);
void lcdPrint(uint8_t col, uint8_t row, const char* text);
```

```cpp
// Implementar los métodos en hardware.cpp
void HardwareClass::digitalWrite(uint8_t pin, uint8_t state) {
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin) {
  return ::digitalRead(pin);
}

void HardwareClass::lcdPrint(uint8_t col, uint8_t row, const char* text) {
  _lcd.setCursor(col, row);
  _lcd.print(text);
}
```

### Ejemplo: Transición del código principal

#### Versión original (fragmento)
```cpp
void setup() {
  // Definimos entradas
  pinMode(btnAumentar, INPUT);
  pinMode(btnComenzar, INPUT);
  // ...más configuración de pines...
  
  // Inicializamos el lcd
  lcd.begin(16, 2);
  pintarPantallaInicio();
  delay(3000);
  
  // Iniciamos el sensor de temperatura
  thermo.begin();
  thermo.setResolution(resolucion);
  
  // Iniciamos sensor de presion
  pressure_sensor.begin(DOUT, SCLK);
  
  // Recuperamos valores del EEPROM
  recuperarValoresEEPROM();
  
  pintarVentanaSeleccion();
}
```

#### Versión modular (fragmento)
```cpp
void setup() {
  // Inicializar hardware básico
  Hardware.init();
  
  // Inicializar UI y mostrar pantalla de bienvenida
  UIController.init();
  UIController.showWelcomeScreen();
  delay(TIEMPO_BIENVENIDA);
  
  // Inicializar sensores
  Sensors.init();
  
  // Inicializar actuadores
  Actuators.init();
  
  // Cargar configuración de EEPROM
  Storage.loadFromEEPROM();
  
  // Inicializar controlador de programas
  ProgramController.init();
  
  // Mostrar pantalla de selección
  UIController.showSelectionScreen();
}
```

## Consejos y Mejores Prácticas

1. **Implementación Progresiva**:
   - Implementar un módulo a la vez, comenzando por los más básicos.
   - Integrar cada módulo al sistema y verificar su funcionamiento antes de pasar al siguiente.

2. **Uso de Estado Temporal**:
   - Durante la transición, puede ser necesario mantener algunas variables globales temporalmente.
   - Eliminar gradualmente estas variables a medida que se completa la modularización.

3. **Pruebas Continuas**:
   - Probar cada módulo individualmente antes de integrarlo.
   - Verificar constantemente que el sistema siga funcionando correctamente.

4. **Documentación Paralela**:
   - Actualizar la documentación a medida que se implementa cada módulo.
   - Incluir comentarios detallados en el código para facilitar el mantenimiento futuro.

## Consideraciones Adicionales

1. **Optimización de Memoria**:
   - La estructura modular podría aumentar ligeramente el uso de memoria.
   - Si es necesario, optimizar el código para reducir el consumo de memoria.

2. **Compatibilidad**:
   - Asegurar que los módulos sean compatibles con la plataforma Arduino utilizada.
   - Verificar la disponibilidad de memoria para la estructura modular completa.

3. **Mantenimiento Futuro**:
   - Establecer directrices claras para el mantenimiento y expansión de los módulos.
   - Documentar la estructura y responsabilidades de cada módulo para facilitar el trabajo de futuros desarrolladores.

Al seguir este plan de implementación, se logrará una transición gradual y segura hacia una estructura modular, manteniendo la funcionalidad del sistema en todo momento mientras se mejora significativamente su organización y mantenibilidad.
