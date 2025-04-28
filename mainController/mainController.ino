/*
 * automLavanderia_v1.ino
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
  Serial.begin(115200);
  Serial.println("Iniciando sistema de lavadora industrial...");
  
  // Inicializar módulos en orden de dependencia
  Hardware.init();
  Utils.init();
  Storage.init();
  Sensors.init();
  Actuators.init();
  UIController.init();
  ProgramController.init();
  
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
  
  // Actualizar controlador de programa
  ProgramController.update();
  
  // Procesar eventos de AsyncTaskLib
  Utils.updateAsyncTasks();
  
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

void showWelcomeScreen() {
  // Mostrar pantalla de bienvenida
  UIController.showWelcomeScreen();
  
  // Crear tarea asíncrona para cambiar de pantalla después del tiempo de bienvenida
  static AsyncTask welcomeDelay(TIEMPO_BIENVENIDA, false, []() {
    UIController.showSelectionScreen(ProgramController.getCurrentProgram());
  });
  
  // Registrar y activar la tarea
  Utils.registerAsyncTask(&welcomeDelay);
  welcomeDelay.Start();
}
