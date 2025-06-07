// ui_controller.h
#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "Arduino.h"
#include "config.h"
#include "hardware.h"

class UIControllerClass {
public:
  // Inicialización
  void init();
  
  // Métodos para mostrar diferentes pantallas
  void showWelcomeScreen();
  void showSelectionScreen(uint8_t programa = 0);
  void showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  void showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor);
  void showErrorScreen(uint8_t errorCode = 0, const String& errorMessage = "");
  void showEmergencyScreen();
  
  // Métodos de transición con limpieza garantizada de eventos
  void safeTransitionToSelection(uint8_t programa = 0);
  void safeTransitionToExecution(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  void safeTransitionToEdit(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor);
  void safeTransitionToError(uint8_t errorCode = 0, const String& errorMessage = "");
  
  // Método para limpiar eventos pendientes
  void clearPendingEvents();
  
  // Métodos para actualizar datos en pantalla
  void updateTime(uint8_t minutos, uint8_t segundos);
  void updateTemperature(uint8_t temperatura);
  void updateWaterLevel(uint8_t nivel);
  void updateRotation(uint8_t rotacion);
  void updatePhase(uint8_t fase);
  void updateProgressBar(uint8_t progress);
  
  // Proceso de eventos de interfaz
  void processEvents();
  bool hasUserAction();
  String getUserAction();
  
  // Estado de la interfaz
  bool isUIStable(); // Verifica si la UI no está limpiando eventos
  
  // Métodos de ayuda para interfaz
  void showMessage(const String& message, uint16_t duration = 2000);
  void playSound(uint8_t soundType);

private:
  // Variables para controlar estado de UI
  String _lastUserAction;
  bool _userActionPending;
  unsigned long _messageTimestamp;
  bool _messageActive;
  
  // Variables para el sistema de limpieza de eventos
  bool _clearingEvents;
  unsigned long _clearingStartTime;
  static const uint16_t EVENT_CLEAR_TIMEOUT = 100; // ms para limpiar eventos
  
  // Referencia a los datos del programa para visualización
  uint8_t (*_nivelAgua)[4];
  uint8_t (*_rotacionTam)[4];
  uint8_t (*_temperaturaLim)[4];
  uint8_t (*_temporizadorLim)[4];
  
  // Métodos internos para procesar componentes
  void _handleNextionEvent(const String& event);
  void _formatTimeDisplay(uint8_t minutos, uint8_t segundos, char* buffer);
  void _updateProgramInfo(uint8_t programa);
  void _updateExecutionData(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  
  // Métodos internos para limpieza de eventos
  void _clearPendingEvents();
  bool _isEventClearingComplete();
};

// Instancia global
extern UIControllerClass UIController;

#endif // UI_CONTROLLER_H