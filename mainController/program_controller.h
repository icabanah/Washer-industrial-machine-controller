// program_controller.h
#ifndef PROGRAM_CONTROLLER_H
#define PROGRAM_CONTROLLER_H

#include "Arduino.h"
#include "config.h"
#include "utils.h"
#include "storage.h"
#include "sensors.h"
#include "actuators.h"
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
  void startProgram();
  void pauseProgram();
  void resumeProgram();
  void stopProgram();
  
  // Control de fase
  void setPhase(uint8_t phase);
  uint8_t getCurrentPhase();
  void nextPhase();
  bool isLastPhase();
  
  // Gestión de temporizadores
  void updateTimers();
  uint8_t getRemainingMinutes();
  uint8_t getRemainingSeconds();
  uint8_t getTotalMinutes();
  uint8_t getTotalSeconds();
  uint8_t getProgressPercentage();
  uint8_t getTotalProgramProgressPercentage();
  
  // Gestión de edición
  void startEditing(uint8_t program, uint8_t phase);
  void endEditing(); // Terminar edición y volver a selección
  void editParameter(uint8_t paramType, uint8_t value);
  void saveEditing();
  void cancelEditing();
  
  // Manejo de eventos de usuario
  void processUserEvent(const String& event);
  
  // Manejo de emergencias
  void handleEmergency();
  void resetEmergency();
  
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
  uint8_t _totalSeconds;
  bool _timerRunning;
  
  // Variables para pausa
  uint8_t _pausedMinutes;
  uint8_t _pausedSeconds;
  bool _pauseActuatorsStopped;
  bool _pausedPreparingPhase;
  
  // Variables de estado de fase
  bool _preparingPhase;
  unsigned long _phaseStartTime;
  
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
  
  // Variables para edición
  uint8_t _editingProgram;
  uint8_t _editingPhase;
  uint8_t _editingParameter; // Parámetro actualmente siendo editado
  uint8_t _editingParameterValue; // Valor del parámetro siendo editado
  bool _isEditing;
  
  // Datos de programa
  uint8_t _waterLevels[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _temperatures[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _times[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _rotations[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _tipoAguaPrograma[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _centrifugadoPrograma[NUM_PROGRAMAS][NUM_FASES]; // [programa][tanda] - P22/P23: [prog][0], P24: [prog][0,1,2]
  
  void _loadProgramData();
  void _loadCurrentProgramState();
  void _updatePhaseParameters();
  void _checkSensorConditions();
  void _decrementTimer();
  void _handleStateMachine();
  void _handleSelectionState();
  void _handleEditingState();
  void _handleExecutionState();
  void _handlePauseState();
  void _handleErrorState();
  void _handleEmergencyState();
  void _handleFinalDrainState();
  void _handleDoorWaitState();
  void _handleCentrifugeState();
  
  // Métodos para manejar eventos táctiles por página
  void _handleSelectionPageEvents(uint8_t componentId);
  void _handleEditPageEvents(uint8_t componentId);
  void _handleExecutionPageEvents(uint8_t componentId);
  
  // Métodos auxiliares para edición de parámetros
  void _decreaseCurrentParameter();
  void _increaseCurrentParameter();
  void _selectPreviousParameter();
  void _selectNextParameter();
  void _updateEditDisplay();
  
  // Métodos de programa
  void _initializeProgram();
  void _completePhase();
  void _completeProgram();
  void _configureProgramType();
  
  // Métodos auxiliares para progreso total del programa
  uint16_t _getTotalProgramDuration(uint8_t programa);
  uint16_t _getElapsedProgramTime(uint8_t programa);
  
  // Control de actuadores según tanda
  void _configureActuatorsForPhase();
  void _handleTemperatureControl();
  bool _isCentrifugadoEnabled(uint8_t programa, uint8_t tanda);
  
  // Secuencias especiales del programa
  void _startDoorLockTimer();
  void _finalizeProgramSequence();
  
  // Manejo de errores
  void _triggerError(uint8_t errorCode, const String& errorMessage);
};

// Instancia global
extern ProgramControllerClass ProgramController;

#endif // PROGRAM_CONTROLLER_H