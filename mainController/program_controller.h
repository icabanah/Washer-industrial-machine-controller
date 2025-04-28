// program_controller.h
#ifndef PROGRAM_CONTROLLER_H
#define PROGRAM_CONTROLLER_H

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
  
  // Gestión de edición
  void startEditing(uint8_t program, uint8_t phase);
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
  
  // Variables para edición
  uint8_t _editingProgram;
  uint8_t _editingPhase;
  bool _isEditing;
  
  // Datos de programa
  uint8_t _waterLevels[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _temperatures[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _times[NUM_PROGRAMAS][NUM_FASES];
  uint8_t _rotations[NUM_PROGRAMAS][NUM_FASES];
  
  // Métodos internos
  void _loadProgramData();
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
  
  // Métodos de programa
  void _initializeProgram();
  void _completePhase();
  void _completeProgram();
  
  // Control de actuadores según fase
  void _configureActuatorsForPhase();
  
  // Manejo de errores
  void _triggerError(uint8_t errorCode, const String& errorMessage);
};

// Instancia global
extern ProgramControllerClass ProgramController;

#endif // PROGRAM_CONTROLLER_H