// hardware.h
#ifndef HARDWARE_H
#define HARDWARE_H

#include "config.h"
#include <HardwareSerial.h>
// #include ".../../Librerias/Hardware-serial/HardwareSerial.h"

class HardwareClass {
public:
  // Inicialización general
  void init();
  
  // Funciones para manejo del botón de emergencia
  bool isEmergencyButtonPressed();
  
  // Funciones para control de Nextion
  void nextionSendCommand(const String& command);
  void nextionSetPage(uint8_t pageId);
  void nextionSetText(uint8_t componentId, const String& text);
  void nextionSetValue(uint8_t componentId, int value);
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