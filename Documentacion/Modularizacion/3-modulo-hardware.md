# Módulo de Hardware (hardware.h / hardware.cpp)

## Descripción

El módulo de Hardware gestiona la interacción directa con los componentes físicos del sistema, como botones, pines de entrada/salida y LCD. Actúa como una capa de abstracción entre el hardware físico y la lógica de la aplicación.

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
#include <LiquidCrystal.h>

class HardwareClass {
public:
  // Inicialización general
  void init();
  
  // Funciones para manejo de botones
  void processButtons();
  bool isButtonPressed(uint8_t button);
  bool isButtonReleased(uint8_t button);
  
  // Funciones para control de LCD
  void lcdClear();
  void lcdPrint(uint8_t col, uint8_t row, const char* text);
  void lcdPrintNumber(uint8_t col, uint8_t row, int value, int padding = 0);
  void lcdSetCursor(uint8_t col, uint8_t row);
  void lcdBlink(bool enable = true);
  
  // Control de pines de salida
  void digitalWrite(uint8_t pin, uint8_t state);
  uint8_t digitalRead(uint8_t pin);
  
  // Acceso al objeto LCD para operaciones avanzadas
  LiquidCrystal* getLCD();

private:
  // Variables para control de estado de botones
  uint8_t _buttonStates[6];
  uint8_t _lastButtonStates[6];
  
  // Objeto LCD
  LiquidCrystal _lcd;
  
  // Métodos internos
  void _initButtons();
  void _initOutputs();
  void _debounceButton(uint8_t buttonPin, uint8_t buttonIndex);
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
  _initButtons();
  _initOutputs();
  
  // Inicializar LCD
  _lcd = LiquidCrystal(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
  _lcd.begin(LCD_COLS, LCD_ROWS);
}

void HardwareClass::_initButtons() {
  // Configurar pines de entrada
  pinMode(PIN_BTN_PARAR, INPUT);
  pinMode(PIN_BTN_DISMINUIR, INPUT);
  pinMode(PIN_BTN_AUMENTAR, INPUT);
  pinMode(PIN_BTN_EDITAR, INPUT);
  pinMode(PIN_BTN_PROGRAMAR_NIVEL_AGUA, INPUT);
  pinMode(PIN_BTN_COMENZAR, INPUT);
  
  // Inicializar estados
  for (uint8_t i = 0; i < 6; i++) {
    _buttonStates[i] = 0;
    _lastButtonStates[i] = 0;
  }
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

void HardwareClass::processButtons() {
  _debounceButton(PIN_BTN_PARAR, 0);
  _debounceButton(PIN_BTN_DISMINUIR, 1);
  _debounceButton(PIN_BTN_AUMENTAR, 2);
  _debounceButton(PIN_BTN_EDITAR, 3);
  _debounceButton(PIN_BTN_PROGRAMAR_NIVEL_AGUA, 4);
  _debounceButton(PIN_BTN_COMENZAR, 5);
}

void HardwareClass::_debounceButton(uint8_t buttonPin, uint8_t buttonIndex) {
  uint8_t reading = ::digitalRead(buttonPin);
  
  // Si el estado ha cambiado, actualizar
  if (reading != _lastButtonStates[buttonIndex]) {
    // Si el botón está activo y no estaba activo antes
    if (reading == NIVEL_ACTIVO && _lastButtonStates[buttonIndex] != NIVEL_ACTIVO) {
      _buttonStates[buttonIndex] = 1;  // Recién presionado
    } else {
      _buttonStates[buttonIndex] = 0;  // No presionado o mantenido
    }
    
    _lastButtonStates[buttonIndex] = reading;
  }
}

bool HardwareClass::isButtonPressed(uint8_t button) {
  uint8_t index;
  
  // Mapear botón a índice
  switch (button) {
    case PIN_BTN_PARAR: index = 0; break;
    case PIN_BTN_DISMINUIR: index = 1; break;
    case PIN_BTN_AUMENTAR: index = 2; break;
    case PIN_BTN_EDITAR: index = 3; break;
    case PIN_BTN_PROGRAMAR_NIVEL_AGUA: index = 4; break;
    case PIN_BTN_COMENZAR: index = 5; break;
    default: return false;
  }
  
  // Verificar si acaba de ser presionado
  if (_buttonStates[index] == 1) {
    _buttonStates[index] = 2;  // Marcar como procesado
    return true;
  }
  
  return false;
}

bool HardwareClass::isButtonReleased(uint8_t button) {
  uint8_t index;
  
  // Mapear botón a índice
  switch (button) {
    case PIN_BTN_PARAR: index = 0; break;
    case PIN_BTN_DISMINUIR: index = 1; break;
    case PIN_BTN_AUMENTAR: index = 2; break;
    case PIN_BTN_EDITAR: index = 3; break;
    case PIN_BTN_PROGRAMAR_NIVEL_AGUA: index = 4; break;
    case PIN_BTN_COMENZAR: index = 5; break;
    default: return false;
  }
  
  return (_lastButtonStates[index] != NIVEL_ACTIVO);
}

// Métodos para LCD
void HardwareClass::lcdClear() {
  _lcd.clear();
}

void HardwareClass::lcdPrint(uint8_t col, uint8_t row, const char* text) {
  _lcd.setCursor(col, row);
  _lcd.print(text);
}

void HardwareClass::lcdPrintNumber(uint8_t col, uint8_t row, int value, int padding) {
  _lcd.setCursor(col, row);
  
  // Agregar ceros si es necesario
  if (padding > 0 && value < pow(10, padding - 1)) {
    for (int i = 0; i < padding - 1; i++) {
      if (value < pow(10, i + 1)) {
        _lcd.print("0");
      }
    }
  }
  
  _lcd.print(value);
}

void HardwareClass::lcdSetCursor(uint8_t col, uint8_t row) {
  _lcd.setCursor(col, row);
}

void HardwareClass::lcdBlink(bool enable) {
  if (enable) {
    _lcd.blink();
  } else {
    _lcd.noBlink();
  }
}

void HardwareClass::digitalWrite(uint8_t pin, uint8_t state) {
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin) {
  return ::digitalRead(pin);
}

LiquidCrystal* HardwareClass::getLCD() {
  return &_lcd;
}
```

## Responsabilidades

El módulo de Hardware tiene las siguientes responsabilidades:

1. **Inicialización**: Configurar pines de entrada/salida y dispositivos externos.
2. **Entrada**: Proporcionar una interfaz limpia para leer el estado de los botones con manejo de rebotes.
3. **Salida**: Ofrecer métodos para controlar pines de salida y el LCD.
4. **Abstracción**: Aislar la lógica de la aplicación de los detalles de hardware.

## Ventajas de este Enfoque

1. **Encapsulamiento**: Los detalles específicos del hardware están encapsulados en un solo módulo.
2. **Portabilidad**: Facilita la migración a diferentes plataformas hardware o microcontroladores.
3. **Mantenibilidad**: Centraliza el código relacionado con el hardware para un mantenimiento más sencillo.
4. **Legibilidad**: Proporciona una interfaz clara y consistente para interactuar con el hardware.
5. **Testabilidad**: Permite simular o "mockear" el hardware para pruebas unitarias.

Al proporcionar una capa de abstracción para el hardware, este módulo simplifica el desarrollo y mantenimiento del resto del sistema, permitiendo que la lógica de la aplicación se centre en su funcionalidad principal sin preocuparse por los detalles de bajo nivel.
