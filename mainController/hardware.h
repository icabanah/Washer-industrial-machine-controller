// hardware.h
#ifndef HARDWARE_H
#define HARDWARE_H

#include "Arduino.h"
#include "config.h"
#include <HardwareSerial.h>

class HardwareClass {
public:
  // Inicialización general
  void init();
  
  // Funciones para manejo del botón de emergencia
  bool isEmergencyButtonPressed();
  
  // Funciones para control de Nextion
  void nextionSendCommand(const String& command); // Enviar comando a Nextion
  void nextionSetPage(uint8_t pageId); // Cambiar página en Nextion
  void nextionSetText(const String& componentId, const String& text); // Establecer texto en un componente Nextion
  void nextionSetValue(const String& componentId, int value);
  bool nextionCheckForEvents();
  String nextionGetLastEvent();
  bool isNextionInitComplete(); // Verificar si la inicialización de Nextion está completa
  void _completeNextionInit(); // Movido a público para acceso desde callback
  
  // Funciones específicas para eventos táctiles
  bool hasValidTouchEvent();
  uint8_t getTouchEventPage();
  uint8_t getTouchEventComponent();
  uint8_t getTouchEventType();
  
  // Función de prueba de conectividad
  void testNextionConnectivity();
  
  // Control de pines de salida
  void digitalWrite(uint8_t pin, uint8_t state);
  uint8_t digitalRead(uint8_t pin);
  

  /// @brief 
  /// Actualiza el estado del botón de emergencia con antirrebote.
  /// Este método lee el estado del botón de emergencia y aplica un algoritmo de antirrebote para evitar lecturas erróneas.
  /// @details
  /// Este método utiliza un algoritmo de antirrebote para asegurar que el estado del botón de emergencia se actualice correctamente.
  /// Se compara el estado actual del botón con el último estado conocido y se actualiza solo si ha pasado un tiempo suficiente desde el último cambio.
  /// @note
  /// Asegúrate de que el pin del botón de emergencia esté configurado como INPUT_PULLUP en la inicialización, si no lo tienes configurado con resistores explícitamente.
  /// @warning
  void updateDebouncer();

private:
  // Variables para control de estado del botón de emergencia
  uint8_t _emergencyButtonState;
  uint8_t _lastEmergencyButtonState;
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
  
  // Variable para inicialización de Nextion
  bool _nextionInitComplete;
  
  // Búfer para comunicación Nextion
  String _nextionLastEvent;
  char _nextionBuffer[64];
  
  // Variables para eventos táctiles
  bool _hasValidTouch;
  uint8_t _touchPage;
  uint8_t _touchComponent;
  uint8_t _touchEventType;
  
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