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
  void showEditScreen(uint8_t programa, uint8_t fase);
  void showErrorScreen(uint8_t errorCode = 0, const String& errorMessage = "");
  void showEmergencyScreen();
  
  // === NUEVOS MÉTODOS PARA PÁGINA DE EDICIÓN ===
  // Inicialización y gestión de edición de parámetros
  void initEditMode(uint8_t programa, uint8_t fase);
  void updateEditDisplay();
  void updateParameterDisplay();
  void updateRightPanel();
  
  // Manejo de eventos de edición
  void handleEditPageEvent(int componentId);
  void handleParameterIncrement();
  void handleParameterDecrement(); 
  void handleNextParameter();
  void handlePreviousParameter();
  void handleSaveParameters();
  void handleCancelEdit();
  
  // Métodos de transición con limpieza garantizada de eventos
  void safeTransitionToSelection(uint8_t programa = 0);
  void safeTransitionToExecution(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  void safeTransitionToEdit(uint8_t programa, uint8_t fase);
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
  
  // === VARIABLES PARA EDICIÓN DE PARÁMETROS ===
  // Estado de edición actual
  uint8_t _programaEnEdicion;      // Programa siendo editado (1, 2, 3)
  uint8_t _faseEnEdicion;          // Fase siendo editada (1-4)
  int _parametroActual;            // Parámetro actualmente seleccionado (PARAM_NIVEL, etc.)
  int _valoresTemporales[4];       // Valores temporales: [nivel, temp, tiempo, rotacion]
  bool _modoEdicionActivo;         // Indica si estamos en modo edición
  unsigned long _editTimeoutStart; // Para timeout automático de edición
  
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
  
  // === MÉTODOS INTERNOS PARA EDICIÓN DE PARÁMETROS ===
  // Carga y guardado de parámetros
  void _loadParametersFromStorage(uint8_t programa, uint8_t fase);
  void _saveParametersToStorage(uint8_t programa, uint8_t fase);
  
  // Validación y formateo
  bool _validateAllParameters();
  void _formatParameterWithUnit(int tipoParam, int valor, char* buffer, int size);
  
  // Gestión de timeout de edición
  void _checkEditTimeout();
  void _resetEditTimeout();
};

// Instancia global
extern UIControllerClass UIController;

#endif // UI_CONTROLLER_H