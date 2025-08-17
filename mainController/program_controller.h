// program_controller.h
#ifndef PROGRAM_CONTROLLER_H
#define PROGRAM_CONTROLLER_H

#include "Arduino.h"
#include "config.h"
#include "utils.h"
#include "storage.h"
#include "sensors.h"
#include "actuators.h"
#include "hardware.h"
#include "ui_controller.h"


class ProgramControllerClass {
public:
  // Inicialización
  void init();
  
  // Gestión de estados
  void setState(uint8_t newState);
  uint8_t getState();
  
  // Control de programa
  void selectProgram(uint8_t program);
  uint8_t getCurrentProgram();
  void pauseProgram();
  void resumeProgram();
  void stopProgram();
  
  // Control de fase
  uint8_t getCurrentPhase();
  
  // Control de tanda (para edición)
  uint8_t getCurrentEditingTanda();
  void setEditingTanda(uint8_t tanda); // Establecer tanda en edición directamente
  
  // Gestión de temporizadores
  void updateTimers();
  
  // Gestión de edición
  void endEditing();
  
  // Manejo de eventos de usuario
  void processUserEvent(const String& event);
  
  // Manejo de emergencias
  void handleEmergency(bool triggeredByButton = true);
  void resetEmergency();
  void forceResetEmergency(); // Reset forzado desde interfaz (para emergencias por software)
  
  // Actualización periódica (debe llamarse en cada ciclo)
  void update();

private:
  // Variables de estado del programa
  uint8_t _currentState;
  uint8_t _previousState;
  uint8_t _currentProgram;
  uint8_t _currentPhase;
  
  // Variables de temporizador
  uint8_t _remainingMinutes;
  uint8_t _remainingSeconds;
  uint8_t _totalMinutes;
  uint16_t _totalSeconds;
  bool _timerRunning;
  
  // Variables para pausa
  uint8_t _pausedMinutes;
  uint8_t _pausedSeconds;
  bool _pauseActuatorsStopped;
  bool _pausedPreparingPhase;
  
  // Variables de estado de fase
  bool _preparingPhase;
  unsigned long _phaseStartTime;
  uint8_t _currentPhaseState; // Estado actual de la máquina de fases
  
  // Variables para secuencia final
  uint8_t _finalDrainMinutes;
  uint8_t _finalDrainSeconds;
  uint8_t _doorWaitMinutes;
  uint8_t _doorWaitSeconds;
  uint8_t _centrifugeMinutes;
  uint8_t _centrifugeSeconds;
  
  // Variables para sistema de tandas (Programa 24)
  uint8_t _tandaCounter;
  uint8_t _maxTandas;
  
  // Variables para temporizadores no bloqueantes
  int _pauseBlinkTaskId;
  int _errorBlinkTaskId;
  int _emergencyBlinkTaskId;
  int _emergencyDoorUnlockTaskId;
  bool _blinkState;
  
  // Variables para control de emergencia
  bool _emergencyTriggeredByButton; // true=botón físico, false=software
  
  // Variables para edición
  uint8_t _editingProgram;
  uint8_t _editingPhase;
  uint8_t _editingTanda; // Tanda actualmente siendo editada (solo para P24)
  uint8_t _editingParameter; // Parámetro actualmente siendo editado
  uint8_t _editingParameterValue; // Valor del parámetro siendo editado
  bool _isEditing;
  
  // Datos de programa
  uint8_t _waterLevels[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _temperatures[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _times[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _rotations[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _tipoAguaPrograma[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _centrifugadoPorTanda[NUM_PROGRAMAS][NUM_FASES]; // [programa][tanda] - P22/P23: 1 tanda, P24: 4 tandas
  
  void _loadProgramData(); // 
  void _loadCurrentProgramState();
  void _updatePhaseParameters();
  
  // Métodos de control de fase (uso interno)
  void nextPhase();
  bool isLastPhase();
  
  // Métodos de temporizadores (uso interno)
  uint8_t getProgressPercentage();
  uint8_t getTotalProgramProgressPercentage();
  void _checkSensorConditions();
  void _checkCriticalSafety(); // Verificaciones de seguridad crítica (emergencia automática)
  void _controlActuatorsForPhase(); // Control separado de actuadores
  void _decrementTimer();
  void _handleStateMachine();
  void _handleSelectionState();
  void _handleEditingState();
  void _handleExecutionState();
  void _handlePauseState();
  void _handleErrorState();
  void _handleEmergencyState();
  void _handlePhaseStateMachine(); // Máquina de estados de fases
  void _initializePhaseState();
  
  // Métodos para parpadeos no bloqueantes
  void _togglePauseBlink();
  void _toggleErrorBlink();
  void _toggleEmergencyBlink();
  void _stopAllBlinkTasks(); // Inicializar parámetros para nueva fase
  
  // Métodos para manejar eventos táctiles por página
  void _handleSelectionPageEvents(uint8_t componentId);
  void _handleEditPageEvents(uint8_t componentId);
  void _handleExecutionPageEvents(uint8_t componentId);
  void _handleEmergencyPageEvents(uint8_t componentId);
  
  // Flujo simplificado (nueva implementación)
  bool validateConditions(); // Todas las validaciones en un solo método
  void executeStart(); // Ejecución directa sin capas intermedias
  
  // Funciones de validación optimizadas (obsoletas - mantener temporalmente)
  bool _validateStartConditions(const String& context = "");
  
  // Métodos de gestión de edición (uso interno)
  void startEditing(uint8_t program, uint8_t phase);
  void editParameter(uint8_t paramType, uint8_t value);
  void saveEditing();
  void cancelEditing();
  void _updateEditScreenForProgram(); // Actualizar pantalla de edición según programa
  void _loadEditingParametersForCurrentTanda(); // Cargar parámetros de la tanda actual
  void _handleTandaSelection(); // Manejar selección de tanda en P24
  void _incrementTanda(); // Incrementar tanda con botón +
  void _decrementTanda(); // Decrementar tanda con botón -
  void _updateTandaDisplay(); // Actualizar display de tanda
  
  
  // Métodos de programa
  void _initializeProgram();
  void _completeProgram();
  void _configureProgramType();
  
  
  // Control de actuadores según tanda
  void _configureActuatorsForPhase();
  void _handleTemperatureControl();
  bool _isCentrifugadoEnabled(uint8_t programa, uint8_t tanda);
  
  // Secuencias especiales del programa
  void _finalizeProgramSequence();
  void _finalizeProgramWithDrainOpen();
  
  // Funciones auxiliares rápidas para UI
  void _updateProgramButtons();
  
  // Manejo de errores
  void _triggerError(uint8_t errorCode, const String& errorMessage);
};

// Instancia global
extern ProgramControllerClass ProgramController;

#endif // PROGRAM_CONTROLLER_H