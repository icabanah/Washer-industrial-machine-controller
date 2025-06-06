/*
 * mainController.ino
 * 
 * Controlador para lavadora industrial
 * Arquitectura modular para ESP32-WROOM con pantalla táctil Nextion
 * 
 * Creado: Abril 2025
 * Autor: itrebolsoft
 * 
 * Descripción:
 * Sistema modular para control de lavadora industrial que gestiona:
 * - Interfaz de usuario mediante pantalla táctil Nextion
 * - Control de motor bidireccional
 * - Sensores de temperatura y presión/nivel
 * - Válvulas de agua, vapor y drenaje
 * - Programas de lavado configurables
 */

#include "config.h"
#include "hardware.h"
#include "utils.h"
#include "storage.h"
#include "sensors.h"
#include "actuators.h"
#include "ui_controller.h"
#include "program_controller.h"

// Declaración de funciones
void checkEmergencyButton();
void showWelcomeScreen();

void setup() {
  // Inicializar puerto serial para depuración
  Serial.begin(9600);
  Serial.println("Iniciando sistema de lavadora industrial...");
  
  // Inicializar módulos en orden de dependencia
  Hardware.init();
  Utils.init();
  Storage.init();
  Sensors.init();
  Actuators.init();
  UIController.init();
  ProgramController.init();
  
  // Registrar callbacks para el temporizador principal
  // Esto centraliza la gestión del tiempo y evita múltiples temporizadores desincronizados
  Utils.registerTimerCallback([]() {
    // Verificar estados de sensores periódicamente
    if (ProgramController.getState() == ESTADO_EJECUCION) {
      Sensors.updateSensors();
    }
  });
  
  // Registrar callbacks para actualización de UI
  Utils.registerTimerCallback([]() {
    // Actualizar UI si es necesario
    if (Hardware.isNextionInitComplete()) {
      Hardware.nextionCheckForEvents();
      if (ProgramController.getState() == ESTADO_EJECUCION) {
        // Actualizar información de UI cada segundo
        // Actualizamos directamente sin usar formatTime
        UIController.updateTime(ProgramController.getRemainingMinutes(), 
                              ProgramController.getRemainingSeconds());
      }
    }
  });
  
  // Mostrar pantalla de bienvenida
  showWelcomeScreen();
  
  // Iniciar monitoreo de sensores
  Sensors.startMonitoring();
  
  // Iniciar temporizador principal
  Utils.startMainTimer();
  
  Serial.println("Sistema inicializado correctamente");
}

void loop() {
  // Verificar botón de emergencia con máxima prioridad
  checkEmergencyButton();
  
  // Verificar eventos de la interfaz de usuario
  if (Hardware.nextionCheckForEvents()) {
    String event = Hardware.nextionGetLastEvent();
    ProgramController.processUserEvent(event);
  }
  
  // Actualizar controlador de programa
  ProgramController.update();
  
  // Procesar eventos de tareas temporizadas
  // Esto actualiza todas las tareas de tiempo de todos los módulos integrados
  Utils.updateTasks();
  
  // Pequeña pausa para evitar saturación del CPU
  yield();
}

void checkEmergencyButton() {
  // Actualizar el estado del botón de emergencia
  Hardware.updateDebouncer();
  
  // Si se detecta presión del botón de emergencia, detener todo
  if (Hardware.isEmergencyButtonPressed()) {
    ProgramController.handleEmergency();
  }
}

// Callback para el cambio de pantalla después de la bienvenida
void welcomeScreenCallback() {
  UIController.showSelectionScreen(ProgramController.getCurrentProgram());
}

void showWelcomeScreen() {
  // Mostrar pantalla de bienvenida
  UIController.showWelcomeScreen();
  
  // Crear temporizador para cambiar de pantalla después del tiempo de bienvenida
  Utils.createTimeout(TIEMPO_BIENVENIDA, welcomeScreenCallback);
  
  Serial.println("Pantalla de bienvenida mostrada, cambiará en " + String(TIEMPO_BIENVENIDA) + " ms");
}
