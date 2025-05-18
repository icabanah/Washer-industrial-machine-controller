# Módulo de Hardware (hardware.h / hardware.cpp)

## Descripción

El módulo de Hardware gestiona la interacción directa con los componentes físicos del sistema, como el botón de emergencia, pines de entrada/salida y la comunicación con la pantalla Nextion. Actúa como una capa de abstracción entre el hardware físico y la lógica de la aplicación.

## Analogía: Sistema Nervioso

Este módulo funciona como el sistema nervioso periférico en un cuerpo humano, que se encarga de recibir los estímulos sensoriales (botones, sensores) y enviar comandos a los músculos (actuadores). Proporciona una interfaz estándar para que el cerebro (lógica principal) interactúe con el mundo exterior sin preocuparse por los detalles específicos de cada terminación nerviosa.

## Estructura del Módulo

El módulo de Hardware se divide en:

- **hardware.h**: Define la interfaz pública del módulo
- **hardware.cpp**: Implementa la funcionalidad interna

### Interfaz (hardware.h)

```cpp
// hardware.h
#ifndef HARDWARE_H
#define HARDWARE_H

#include "config.h"
#include <HardwareSerial.h>

class HardwareClass {
public:
  // Inicialización general
  void init();
  
  // Funciones para manejo del botón de emergencia
  bool isEmergencyButtonPressed();
  
  // Funciones para control de Nextion
  void nextionSendCommand(const String& command);
  void nextionSetPage(uint8_t pageId);
  void nextionSetText(const String& componentId, const String& text);
  void nextionSetValue(const String& componentId, int value);
  bool nextionCheckForEvents();
  String nextionGetLastEvent();
  
  // Control de pines de salida
  void digitalWrite(uint8_t pin, uint8_t state);
  uint8_t digitalRead(uint8_t pin);
  
  // Función específica para antirrebote
  void updateDebouncer();

private:
  // Variables para control de estado del botón de emergencia
  uint8_t _emergencyButtonState;
  uint8_t _lastEmergencyButtonState;
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
  
  // Búfer para comunicación Nextion
  String _nextionLastEvent;
  char _nextionBuffer[64];
  
  // Métodos internos
  void _initEmergencyButton();
  void _initOutputs();
  void _initNextion();
  bool _readNextionResponse();
  void _sendNextionEndCmd();
};

// Instancia global
extern HardwareClass Hardware;

#endif // HARDWARE_H
```

### Implementación (hardware.cpp)

```cpp
// hardware.cpp
#include "hardware.h"

// Definición de la instancia global
HardwareClass Hardware;

void HardwareClass::init() {
  _initEmergencyButton();
  _initOutputs();
  _initNextion();
  
  _emergencyButtonState = LOW;
  _lastEmergencyButtonState = LOW;
  _lastDebounceTime = 0;
  _debounceDelay = 50;  // 50ms para el antirrebote
  
  _nextionLastEvent = "";
}

void HardwareClass::_initEmergencyButton() {
  // Configurar botón de emergencia con resistencia pull-up
  pinMode(PIN_BTN_EMERGENCIA, INPUT_PULLUP);
}

void HardwareClass::_initOutputs() {
  // Configurar pines de salida
  pinMode(PIN_MOTOR_DIR_A, OUTPUT);
  pinMode(PIN_MOTOR_DIR_B, OUTPUT);
  pinMode(PIN_VALVULA_AGUA, OUTPUT);
  pinMode(PIN_ELECTROV_VAPOR, OUTPUT);
  pinMode(PIN_VALVULA_DESFOGUE, OUTPUT);
  pinMode(PIN_MAGNET_PUERTA, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  // Inicializar todas las salidas en LOW
  digitalWrite(PIN_MOTOR_DIR_A, LOW);
  digitalWrite(PIN_MOTOR_DIR_B, LOW);
  digitalWrite(PIN_VALVULA_AGUA, LOW);
  digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  digitalWrite(PIN_MAGNET_PUERTA, LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void HardwareClass::_initNextion() {
  // Inicializar comunicación serial con la pantalla Nextion
  NEXTION_SERIAL.begin(NEXTION_BAUD_RATE);
  delay(500);  // Dar tiempo a que la pantalla se inicialice
  
  // Establecer el brillo a un nivel medio (valores 0-100)
  nextionSendCommand("dim=50");
  
  // Ir a la página de bienvenida
  nextionSetPage(NEXTION_PAGE_WELCOME);
}

void HardwareClass::updateDebouncer() {
  // Leer el estado actual del botón de emergencia
  uint8_t reading = ::digitalRead(PIN_BTN_EMERGENCIA);
  
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

void HardwareClass::nextionSendCommand(const String& command) {
  NEXTION_SERIAL.print(command);
  _sendNextionEndCmd();
}

void HardwareClass::_sendNextionEndCmd() {
  // Enviar los tres bytes de finalización de comando FF FF FF
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

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
    return _readNextionResponse();
  }
  return false;
}

bool HardwareClass::_readNextionResponse() {
  int index = 0;
  uint8_t endCount = 0;
  unsigned long startTime = millis();
  
  // Limpiar el búfer anterior
  memset(_nextionBuffer, 0, sizeof(_nextionBuffer));
  
  // Leer la respuesta con timeout
  while ((millis() - startTime) < NEXTION_TIMEOUT && endCount < 3) {
    if (NEXTION_SERIAL.available()) {
      uint8_t c = NEXTION_SERIAL.read();
      if (c == 0xFF) {
        endCount++;
      } else {
        endCount = 0;
        if (index < sizeof(_nextionBuffer) - 1) {
          _nextionBuffer[index++] = c;
        }
      }
    }
  }
  
  // Procesar la respuesta si es válida
  if (endCount == 3 && index > 0) {
    _nextionLastEvent = String(_nextionBuffer);
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

## Responsabilidades

El módulo de Hardware tiene las siguientes responsabilidades:

1. **Inicialización**: Configurar pines de entrada/salida y dispositivos externos.
2. **Botón de Emergencia**: Manejar la lectura y antirrebote del botón de emergencia.
3. **Comunicación Nextion**: Proporcionar una interfaz para enviar comandos y recibir eventos de la pantalla.
4. **Control de Pines**: Ofrecer métodos para controlar pines de salida.
5. **Abstracción**: Aislar la lógica de la aplicación de los detalles de hardware.

## Ventajas de este Enfoque

1. **Encapsulamiento**: Los detalles específicos del hardware están encapsulados en un solo módulo.
2. **Portabilidad**: Facilita la migración a diferentes plataformas hardware o microcontroladores.
3. **Mantenibilidad**: Centraliza el código relacionado con el hardware para un mantenimiento más sencillo.
4. **Legibilidad**: Proporciona una interfaz clara y consistente para interactuar con el hardware.
5. **Testabilidad**: Permite simular o "mockear" el hardware para pruebas unitarias.

## Adaptaciones para el Nuevo Hardware

El módulo ha sido actualizado para reflejar el cambio de hardware:

1. **ESP32-WROOM**: Se han adaptado las definiciones para ser compatibles con ESP32.
2. **Pantalla Nextion**: Se ha implementado la comunicación serial con la pantalla Nextion, incluyendo métodos para enviar comandos y recibir eventos.
3. **Botón de Emergencia**: Se ha implementado un manejo específico para el botón de emergencia con lógica de antirrebote.

Al proporcionar una capa de abstracción para el hardware, este módulo simplifica el desarrollo y mantenimiento del resto del sistema, permitiendo que la lógica de la aplicación se centre en su funcionalidad principal sin preocuparse por los detalles de bajo nivel.
