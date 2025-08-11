// ui_controller.h
#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "Arduino.h"
#include "config.h"
#include "hardware.h"

// Enum para tipos de actualización del panel de edición
enum UpdateMode {
  UPDATE_FULL,     // Actualizar todo el panel derecho
  UPDATE_SINGLE,   // Actualizar solo un parámetro específico
  UPDATE_FAST,     // Actualizar parámetro principal + componente del panel
  UPDATE_INSTANT   // Solo valor principal (máxima velocidad)
};

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
  void updateEditDisplay();
  void updateParameterDisplay();
  void updateEditPanel(UpdateMode updateMode = UPDATE_FULL, uint8_t parametro = 0);
  
  void handleEditPageEvent(int componentId);
  void handleParameterIncrement();
  void handleParameterDecrement(); 
  void handleNextParameter();
  void handlePreviousParameter();
  void handleSaveParameters();
  void handleCancelEdit();
  
  // Nuevas funciones para selección directa de parámetros
  void selectParameter(uint8_t param);
  void selectPhase();
  void selectTanda();
  void selectTandaDirecta(uint8_t tanda); // Nueva función para seleccionar tanda directamente
  void selectCentrifuge(); 
  void selectWater();
  void updateTandaButtons(uint8_t tandaActiva); // Actualizar estado visual de botones de tanda
  
  // Funciones auxiliares para doble guardado
  bool _validateCurrentParameter();
  void _saveCurrentParameterToTemp();
  
  // Métodos de transición con limpieza garantizada de eventos
  void safeTransitionToSelection(uint8_t programa = 0);
  void safeTransitionToExecution(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion, uint8_t tanda = 0);
  void safeTransitionToEdit(uint8_t programa, uint8_t fase);
  void safeTransitionToError(uint8_t errorCode = 0, const String& errorMessage = "");
  
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
  // Métodos internos para actualización optimizada
  void _updateAllPanelParameters();
  void _updateSinglePanelParameter(uint8_t parametro);
  void _updateMainParameter(uint8_t parametro);
  void _updateMainParameterInstant(uint8_t parametro);
  
  // Métodos para limpieza no bloqueante de eventos
  void _processEventClearing();
  void _finishEventClearing();
  void _finishEventClearingAndShowSelection(uint8_t programa);
  void _finishEventClearingAndShowExecution(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion, uint8_t tanda);
  void _finishEventClearingAndShowEdit(uint8_t programa, uint8_t fase);
  
  // Funciones wrapper estáticas para callbacks
  static void _callbackShowSelection();
  static void _callbackShowExecution();
  static void _callbackShowEdit();
  
  // Variables para controlar estado de UI
  String _lastUserAction;
  bool _userActionPending;
  unsigned long _messageTimestamp;
  bool _messageActive;
  uint16_t _messageDuration;
  uint8_t _currentPage;  // Página actualmente mostrada en la pantalla Nextion
  
  // Variables para el sistema de limpieza de eventos
  bool _clearingEvents;
  unsigned long _clearingStartTime;
  static const uint16_t EVENT_CLEAR_TIMEOUT = 100; // ms para limpiar eventos
  int _eventClearTaskId;  // ID para temporizador de limpieza no bloqueante
  
  // Variables temporales para callbacks
  static uint8_t _tempPrograma;
  static uint8_t _tempFase;
  static uint8_t _tempNivelAgua;
  static uint8_t _tempTemperatura;
  static uint8_t _tempRotacion;
  static uint8_t _tempTanda;
  
  // === VARIABLES PARA EDICIÓN DE PARÁMETROS ===
  // Estado de edición actual
  uint8_t _programaEnEdicion;      // Programa siendo editado (0, 1, 2)
  uint8_t _faseEnEdicion;          // Fase siendo editada (1-4)
  int _parametroActual;            // Parámetro actualmente seleccionado (PARAM_NIVEL, etc.)
  int _valoresTemporales[7];       // Valores temporales: [nivel, temp, tiempo, rotacion, tanda, centrif, agua]
  bool _modoEdicionActivo;         // Indica si estamos en modo edición
  unsigned long _editTimeoutStart; // Para timeout automático de edición
  bool _parameterSaved;            // Control para doble guardado: parámetro -> programa
  
  // Referencia a los datos del programa para visualización
  uint8_t (*_nivelAgua)[4];
  uint8_t (*_rotacionTam)[4];
  uint8_t (*_temperaturaLim)[4];
  uint8_t (*_temporizadorLim)[4];
  uint8_t (*_fasesPrograma)[4];
  uint8_t (*_centrifugadoPorTanda)[4];
  uint8_t (*_tipoAguaPrograma)[4];
  
  // Métodos internos para procesar componentes
  void _handleTouchEvent();                        // Nuevo método para eventos táctiles
  void _handleSelectionPageEvent(uint8_t componentId);
  void _handleExecutionPageEvent(uint8_t componentId);
  void _formatTimeDisplay(uint8_t minutos, uint8_t segundos, char* buffer);
  void _updateProgramInfo(uint8_t programa);
  void _updateExecutionData(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  
  // Métodos internos para limpieza de eventos
  void _clearPendingEvents();
  bool _isEventClearingComplete();
  
  // === MÉTODOS INTERNOS PARA EDICIÓN DE PARÁMETROS ===
  // Carga y guardado de parámetros
  void _saveParametersToStorage(uint8_t programa, uint8_t fase);
  
  // Validación y formateo
  bool _validateAllParameters();
  void _formatParameterWithUnit(int tipoParam, int valor, char* buffer, int size);
  bool _isParameterValid(int paramType, int value);
  const char* _getParameterName(int paramType);
  uint8_t _getNextParameter(uint8_t currentParam);
  uint8_t _getPreviousParameter(uint8_t currentParam);
  void _generateProgramText(uint8_t programa, char* buffer, int size);
  
  // Gestión de timeout de edición
  void _checkEditTimeout();
  void _resetEditTimeout();
};

// Instancia global
extern UIControllerClass UIController;

#endif // UI_CONTROLLER_H