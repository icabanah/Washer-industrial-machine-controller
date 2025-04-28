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
  
  // El ESP32 tiene más de un puerto serial, por lo que usamos Serial2 para Nextion
  // Este pequeño retraso permite que la pantalla se inicie correctamente
  delay(500);  
  
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
  // Envía el comando a la pantalla Nextion
  NEXTION_SERIAL.print(command);
  _sendNextionEndCmd();
}

void HardwareClass::_sendNextionEndCmd() {
  // Enviar los tres bytes de finalización de comando FF FF FF
  // Estos bytes son necesarios para que la pantalla Nextion reconozca el fin de un comando
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

void HardwareClass::nextionSetPage(uint8_t pageId) {
  // Cambia a la página especificada en la pantalla Nextion
  nextionSendCommand("page " + String(pageId));
}

void HardwareClass::nextionSetText(uint8_t componentId, const String& text) {
  // Establece el texto de un componente específico en la pantalla Nextion
  // Los componentes de texto en Nextion generalmente tienen el prefijo 't'
  nextionSendCommand("t" + String(componentId) + ".txt=\"" + text + "\"");
}

void HardwareClass::nextionSetValue(uint8_t componentId, int value) {
  // Establece el valor numérico de un componente específico en la pantalla Nextion
  // Los componentes numéricos en Nextion generalmente tienen el prefijo 'x'
  nextionSendCommand("x" + String(componentId) + ".val=" + String(value));
}

bool HardwareClass::nextionCheckForEvents() {
  // Verifica si hay eventos disponibles para leer desde la pantalla Nextion
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
  // Los mensajes de Nextion terminan con 3 bytes FF FF FF
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
  // Retorna el último evento capturado desde la pantalla Nextion
  return _nextionLastEvent;
}

void HardwareClass::digitalWrite(uint8_t pin, uint8_t state) {
  // Wrapper para la función digitalWrite estándar
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin) {
  // Wrapper para la función digitalRead estándar
  return ::digitalRead(pin);
}