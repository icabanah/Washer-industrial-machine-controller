// ui_controller.h
#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "Arduino.h"
#include "config.h"
#include "hardware.h"

// === ENUM ELIMINADO: UpdateMode era sobreingeniería ===

class UIControllerClass {
public:
  // Inicialización
  void init();
  
  // Métodos para mostrar diferentes pantallas
  void showWelcomeScreen();
  void showSelectionScreen(uint8_t programa = 0);
  void showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion, uint8_t tanda = 0, bool preserveTime = false, uint8_t preservedMinutes = 0, uint8_t preservedSeconds = 0);
  void showEditScreen(uint8_t programa, uint8_t tanda = 0);
  void showErrorScreen(uint8_t errorCode = 0, const String& errorMessage = "");
  void showEmergencyScreen();
  
  void initEditMode(uint8_t programa, uint8_t fase);
  void updateDisplay(bool fullUpdate = true);
  
  void handleEditPageEvent(int componentId);
  void handleParameterIncrement();
  void handleParameterDecrement(); 
  void handleNextParameter();
  void handlePreviousParameter();
  void handleSaveParameters();
  void handleCancelEdit();
  
  // Método unificado para selección de parámetros
  void setParameter(uint8_t param, int value = -1); // -1 = solo seleccionar, >=0 = seleccionar y asignar valor
  
  // === MÉTODOS _validate* ELIMINADOS: validación inline simple ===
  
  // === MÉTODOS safeTransition* ELIMINADOS: sobreingeniería ===
  
  // Método para limpiar eventos pendientes
  void clearPendingEvents();
  
  // Métodos para actualizar datos en pantalla
  void updateTime(uint8_t minutos, uint8_t segundos);
  void updateTemperature(float temperatura);
  void updateWaterLevel(uint8_t nivel);
  void updateRotation(uint8_t rotacion);
  void updatePhase(uint8_t fase);
  void updateTanda(uint8_t programa, uint8_t tanda); // Actualizar tanda en ejecución
  void updatePauseIndicator(bool visible);         // Mostrar/ocultar indicador de pausa
  void updateEmergencyAlert(bool state);            // Actualizar alerta de emergencia
  void updateErrorDisplay(bool blinkState);         // Actualizar display de error con parpadeo
  void updateProgramInfo(uint8_t programa);         // Actualizar información del programa en selección
  void updateProgramPanel(uint8_t programa);        // Actualizar SOLO panel derecho (sin cambiar página)
  void updatePreparationStatus(unsigned long prepTime); // Mostrar estado de preparación (llenado/calentamiento)
  void clearPreparationStatus();                    // Limpiar estado de preparación
  
  // Carga de parámetros (público para ProgramController)
  void _loadParametersFromStorage(uint8_t programa, uint8_t fase);
  
  // Proceso de eventos de interfaz
  void processEvents();
  bool hasUserAction();
  String getUserAction();
  
  // Estado de la interfaz
  bool isUIStable(); // Verifica si la UI no está limpiando eventos
  
  // Métodos de ayuda para interfaz
  void showMessage(const String& message, uint16_t duration = 2000);
  void updateStartButtonText(); // Actualizar texto del botón según estado de puerta
  
  // Método para obtener la página actual (para sincronización)
  uint8_t getCurrentPage(); // Obtener página actual de Nextion
  
  // Acceso a estado de edición para ProgramController
  int getCurrentParameter() const { return _parametroActual; }

private:
  // === MÉTODOS DE ACTUALIZACIÓN CONSOLIDADOS EN updateDisplay() ===
  
  // === MÉTODOS DE EVENT CLEARING ELIMINADOS ===
  // Sistema simplificado - transiciones directas
  
  // Variables para controlar estado de UI
  String _lastUserAction;
  bool _userActionPending;
  unsigned long _messageTimestamp;
  bool _messageActive;
  uint16_t _messageDuration;
  uint8_t _currentPage;  // Página actualmente mostrada en la pantalla Nextion
  bool _clearingEvents;  // Para compatibilidad con isUIStable()
  
  // Variables temporales para datos de ejecución
  uint8_t _tempPrograma;
  uint8_t _tempFase;
  uint8_t _tempNivelAgua;
  uint8_t _tempTemperatura;
  uint8_t _tempRotacion;
  uint8_t _tempTanda;
  
  // === EVENT CLEARING VARIABLES ELIMINADAS ===
  // Sistema simplificado - transiciones directas
  
  // === VARIABLES PARA EDICIÓN DE PARÁMETROS ===
  // Estado de edición actual
  uint8_t _programaEnEdicion;      // Programa siendo editado (0, 1, 2)
  uint8_t _faseEnEdicion;          // Fase siendo editada (1-4)
  int _parametroActual;            // Parámetro actualmente seleccionado (PARAM_NIVEL, etc.)
  int _valoresTemporales[7];       // Valores temporales: [nivel, temp, tiempo, rotacion, tanda, centrif, agua]
  bool _modoEdicionActivo;         // Indica si estamos en modo edición
  unsigned long _editTimeoutStart; // Para timeout automático de edición
  bool _parameterSaved;            // Control para doble guardado: parámetro -> programa
  
  // === MATRICES ELIMINADAS ===
  // Los datos de programa se obtienen directamente desde Storage cuando se necesitan
  // Eliminando referencias duplicadas para simplificar arquitectura
  
  // Métodos internos para procesar componentes
  void _handleTouchEvent();                        // Nuevo método para eventos táctiles
  // === Métodos de manejo de eventos movidos a ui_handlers.cpp ===
  void _formatTimeDisplay(uint8_t minutos, uint8_t segundos, char* buffer);
  void _updateProgramInfo(uint8_t programa);
  void _updateExecutionData(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  
  // Métodos internos para limpieza de eventos
  void _clearPendingEvents();
  bool _isEventClearingComplete();
  
  // === MÉTODOS INTERNOS PARA EDICIÓN DE PARÁMETROS ===
  // Carga y guardado de parámetros
  void _saveParametersToStorage(uint8_t programa, uint8_t fase);
  void _saveCurrentParameterToTemp();
  
  // Validación y formateo
  void _formatParameterWithUnit(int tipoParam, int valor, char* buffer, int size);
  const char* _getParameterName(int paramType);
  uint8_t _getNextParameter(uint8_t currentParam);
  uint8_t _getPreviousParameter(uint8_t currentParam);
  void _generateProgramText(uint8_t programa, char* buffer, int size);
  
  // Validación de parámetros
  bool _validateAllParameters();
  bool _validateCurrentParameter();
  bool _isParameterValid(uint8_t paramType, int value);
  
  // Métodos de selección de parámetros
  void selectParameter(uint8_t param);
  void selectPhase(uint8_t fase);
  void selectTanda(uint8_t tanda);
  void selectCentrifuge(uint8_t centrifuge);
  void selectWater(uint8_t water);
  void selectTandaDirecta(uint8_t tanda);
  
  // Métodos de actualización de botones
  void updateTandaButtons(uint8_t tandaActiva = 0);
  void updatePhaseButtons();
  void updateParameterButtons();
  
  // Gestión de timeout de edición
  void _checkEditTimeout();
  void _resetEditTimeout();
};

// Instancia global
extern UIControllerClass UIController;

#endif // UI_CONTROLLER_H