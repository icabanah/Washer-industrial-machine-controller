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

#include "config.h"             // Configuración de pines y parámetros del sistema
#include "hardware.h"           // Módulo de hardware para control de pines y Nextion
#include "utils.h"              // Utilidades generales para temporización y tareas
#include "storage.h"            // Módulo de almacenamiento para guardar configuraciones y programas
#include "sensors.h"            // Módulo de sensores para temperatura y presión/nivel
#include "actuators.h"          // Módulo de actuadores para control de válvulas y motor
#include "ui_controller.h"      // Controlador de interfaz de usuario para Nextion
#include "program_controller.h" // Controlador de programas para gestionar la lógica de lavado

// Declaración de funciones
void checkEmergencyButton();
void showWelcomeScreen();

void setup()
{
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

  // Prueba de conectividad con Nextion después de inicialización
  // delay(1000); // Esperar que la pantalla termine de inicializarse
  // Serial.println("Probando conectividad con pantalla Nextion...");
  // Hardware.testNextionConnectivity();

  // Registrar callbacks para el temporizador principal
  // Esto centraliza la gestión del tiempo y evita múltiples temporizadores desincronizados
  Utils.registerTimerCallback([](){
    // Verificar estados de sensores periódicamente
    if (ProgramController.getState() == ESTADO_EJECUCION) {
      Sensors.updateSensors();
    }
  });

  // Registrar callbacks para actualización de UI
  Utils.registerTimerCallback([](){
  // Actualizar UI si es necesario
  if (Hardware.isNextionInitComplete()) {
    Hardware.nextionCheckForEvents();
    if (ProgramController.getState() == ESTADO_EJECUCION) {
      // Actualizar información de UI cada segundo
      // Actualizamos directamente sin usar formatTime
      UIController.updateTime(ProgramController.getRemainingMinutes(), 
                            ProgramController.getRemainingSeconds());
    }
  } });

  // Mostrar pantalla de bienvenida
  showWelcomeScreen();

  // Iniciar monitoreo de sensores
  Sensors.startMonitoring();

  // Iniciar temporizador principal
  Utils.startMainTimer();

  Serial.println("setup| Sistema inicializado correctamente");
}

void loop()
{
  // Verificar botón de emergencia con máxima prioridad
  // checkEmergencyButton();

  // Verificar eventos táctiles de la interfaz de usuario
  if (Hardware.nextionCheckForEvents())
  {
    // El nuevo sistema procesa automáticamente los eventos táctiles
    // usando los IDs de componentes y páginas
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

void checkEmergencyButton()
{
  // Actualizar el estado del botón de emergencia
  Hardware.updateDebouncer();

  // Si se detecta presión del botón de emergencia, detener todo
  if (Hardware.isEmergencyButtonPressed())
  {
    ProgramController.handleEmergency();
  }
}

// Callback para el cambio de pantalla después de la bienvenida
void welcomeScreenCallback()
{
  uint8_t currentProgram = ProgramController.getCurrentProgram();
  Serial.println("welcomeScreenCallback| Tiempo de bienvenida finalizado, mostrando pantalla de selección para el programa " + String(currentProgram));
  UIController.showSelectionScreen(currentProgram + 1); // Convertir a 1-3 para UI
}

void showWelcomeScreen()
{
  // Mostrar pantalla de bienvenida
  UIController.showWelcomeScreen();

  // Crear temporizador para cambiar de pantalla después del tiempo de bienvenida
  Utils.createTimeout(TIEMPO_BIENVENIDA, welcomeScreenCallback);

  Serial.println("showWelcomeScreen| Pantalla de bienvenida mostrada, cambiará en " + String(TIEMPO_BIENVENIDA) + " ms");
}
