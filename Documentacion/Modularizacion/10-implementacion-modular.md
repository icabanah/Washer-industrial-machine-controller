# Guía de Implementación Modular

## Introducción

Este documento proporciona una guía paso a paso para implementar la estructura modular propuesta para el controlador de lavadora industrial. La implementación se realizará de manera progresiva para un proyecto desarrollado en Arequipa, Perú, usando el ESP32-WROOM y la pantalla táctil Nextion como hardware principal. Se utilizará el IDE de Arduino versión 2.3.6 con el core ESP32 versión 3.2.0.

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
#include <HardwareSerial.h>

class HardwareClass {
public:
  void init();
  void nextionSendCommand(const String& command);
  void updateDebouncer();
  bool isEmergencyButtonPressed();
  
private:
  uint8_t _emergencyButtonState;
  uint8_t _lastEmergencyButtonState;
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
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
  // Configurar botón de emergencia con resistencia pull-up
  pinMode(PIN_BTN_EMERGENCIA, INPUT_PULLUP);
  
  // Inicializar variables de antirrebote
  _emergencyButtonState = LOW;
  _lastEmergencyButtonState = LOW;
  _lastDebounceTime = 0;
  _debounceDelay = 50;  // 50ms para el antirrebote
  
  // Inicializar comunicación serial con la pantalla Nextion
  NEXTION_SERIAL.begin(NEXTION_BAUD_RATE);
  delay(500);  // Dar tiempo a que la pantalla se inicialice
}

void HardwareClass::nextionSendCommand(const String& command) {
  NEXTION_SERIAL.print(command);
  
  // Enviar los tres bytes de finalización de comando FF FF FF
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

void HardwareClass::updateDebouncer() {
  // Leer el estado actual del botón de emergencia
  uint8_t reading = digitalRead(PIN_BTN_EMERGENCIA);
  
  // Si el estado ha cambiado, resetear el temporizador de antirrebote
  if (reading != _lastEmergencyButtonState) {
    _lastDebounceTime = millis();
  }
  
  // Si ha pasado suficiente tiempo desde el último cambio, actualizar el estado
  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    // Si el estado ha cambiado:
    if (reading != _emergencyButtonState) {
      _emergencyButtonState = reading;
    }
  }
  
  // Guardar la lectura para la próxima vez
  _lastEmergencyButtonState = reading;
}

bool HardwareClass::isEmergencyButtonPressed() {
  // El botón está conectado con pull-up, por lo que se activa cuando está en LOW
  return (_emergencyButtonState == LOW);
}
```

#### Paso 3: Expansión de funcionalidad
```cpp
// Añadir progresivamente más métodos a hardware.h
void nextionSetPage(uint8_t pageId);
void nextionSetText(const String& componentId, const String& text);
void nextionSetValue(const String& componentId, int value);
bool nextionCheckForEvents();
String nextionGetLastEvent();
void digitalWrite(uint8_t pin, uint8_t state);
uint8_t digitalRead(uint8_t pin);
```

```cpp
// Implementar los métodos en hardware.cpp
void HardwareClass::nextionSetPage(uint8_t pageId) {
  nextionSendCommand("page " + String(pageId));
}

void HardwareClass::nextionSetText(const String& componentId, const String& text) {
  nextionSendCommand(componentId + ".txt=\"" + text + "\"");
}

void HardwareClass::nextionSetValue(const String& componentId, int value) {
  nextionSendCommand(componentId + ".val=" + String(value));
}

bool HardwareClass::nextionCheckForEvents() {
  if (NEXTION_SERIAL.available()) {
    // Procesar respuesta y guardar el evento
    // ...
    return true;
  }
  return false;
}

String HardwareClass::nextionGetLastEvent() {
  return _nextionLastEvent;
}

void HardwareClass::digitalWrite(uint8_t pin, uint8_t state) {
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin) {
  return ::digitalRead(pin);
}
```

### Ejemplo: Transición del código principal

#### Versión original (fragmento)
```cpp
void setup() {
  // Definimos entradas y salidas
  pinMode(btnEmergencia, INPUT_PULLUP);
  pinMode(MotorDirA, OUTPUT);
  pinMode(MotorDirB, OUTPUT);
  // ...más configuración de pines...
  
  // Iniciamos el puerto serial para la pantalla Nextion
  Serial2.begin(115200);
  
  // Iniciamos el sensor de temperatura
  sensors.begin();
  sensors.setResolution(resolucion);
  
  // Iniciamos sensor de presion
  pressure_sensor.begin(DOUT, SCLK);
  
  // Recuperamos valores del EEPROM
  recuperarValoresEEPROM();
  
  mostrarPantallaInicio();
  delay(3000);
  mostrarPantallaSeleccion();
}
```

#### Versión modular (fragmento)
```cpp
void setup() {
  // Inicializar puerto serial para depuración
  Serial.begin(115200);
  Serial.println("Iniciando sistema de lavadora industrial...");
  
  // Inicializar módulos en orden de dependencia
  Hardware.init();
  Utils.init();
  Storage.init();
  Sensors.init();
  Actuators.init();
  UIController.init();
  ProgramController.init();
  
  // Mostrar pantalla de bienvenida
  showWelcomeScreen();
  
  // Iniciar monitoreo de sensores
  Sensors.startMonitoring();
  
  // Iniciar temporizador principal
  Utils.startMainTimer();
  
  Serial.println("Sistema inicializado correctamente");
}

void loop() {
  // Verificar botón de emergencia con máxima prioridad
  checkEmergencyButton();
  
  // Actualizar controlador de programa
  ProgramController.update();
  
  // Procesar eventos de tareas temporizadas
  Utils.updateTasks();
  
  // Pequeña pausa para evitar saturación del CPU
  yield();
}

void checkEmergencyButton() {
  // Actualizar el estado del botón de emergencia
  Hardware.updateDebouncer();
  
  // Si se detecta presión del botón de emergencia, detener todo
  if (Hardware.isEmergencyButtonPressed()) {
    ProgramController.handleEmergency();
  }
}

void welcomeScreenCallback() {
  UIController.showSelectionScreen(ProgramController.getCurrentProgram());
}

void showWelcomeScreen() {
  // Mostrar pantalla de bienvenida
  UIController.showWelcomeScreen();
  
  // Crear temporizador para cambiar de pantalla después del tiempo de bienvenida
  Utils.createTimeout(TIEMPO_BIENVENIDA, welcomeScreenCallback);
  
  Serial.println("Pantalla de bienvenida mostrada, cambiará en " + String(TIEMPO_BIENVENIDA) + " ms");
}
```

## Consejos y Mejores Prácticas

1. **Implementación Progresiva**:
   - Implementar un módulo a la vez, comenzando por los más básicos.
   - Integrar cada módulo al sistema y verificar su funcionamiento antes de pasar al siguiente.

2. **Utilización de Técnicas No Bloqueantes**:
   - Eliminar todas las instancias de `delay()` y reemplazarlas por temporizadores asíncronos proporcionados por el módulo Utils.
   - Adoptar un enfoque basado en eventos para mejorar la responsividad del sistema.

3. **Pruebas Continuas**:
   - Probar cada módulo individualmente antes de integrarlo.
   - Verificar constantemente que el sistema siga funcionando correctamente.

4. **Documentación Paralela**:
   - Actualizar la documentación a medida que se implementa cada módulo.
   - Incluir comentarios detallados en el código para facilitar el mantenimiento futuro.

5. **Adaptar la Interfaz al Hardware Nextion**:
   - Utilizar nombres de componentes en lugar de índices numéricos para mejorar la legibilidad.
   - Implementar un sistema robusto para procesar eventos táctiles de la pantalla.

## Consideraciones Adicionales

1. **Optimización de Memoria**:
   - El ESP32-WROOM proporciona significativamente más memoria que Arduino, pero sigue siendo importante optimizar el código.
   - Monitorear el uso de memoria con herramientas como ESP-IDF si se observa comportamiento inesperado.

2. **Compatibilidad**:
   - Asegurar que los módulos sean compatibles con el ESP32 y el IDE de Arduino 2.3.6 con core ESP32 versión 3.2.0.
   - Considerar las diferencias entre Arduino y ESP32 al adaptar el código, especialmente en aspectos como la gestión de eventos y múltiples núcleos.

3. **Mantenimiento Futuro**:
   - Establecer directrices claras para el mantenimiento y expansión de los módulos.
   - Documentar la estructura y responsabilidades de cada módulo para facilitar el trabajo de futuros desarrolladores.

4. **Aprovechamiento de Características del ESP32**:
   - Considerar el uso de tareas separadas (FreeRTOS) para operaciones intensivas.
   - Implementar gestión de energía adecuada para prolongar la vida útil en caso de alimentación por batería.

Al seguir este plan de implementación, se logrará una transición gradual y segura hacia una estructura modular adaptada específicamente para el ESP32-WROOM y la pantalla Nextion, manteniendo la funcionalidad del sistema en todo momento mientras se mejora significativamente su organización y mantenibilidad.
