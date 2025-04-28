// nextion_test_modular.ino
// Programa de prueba para la comunicación con la pantalla Nextion usando la estructura modular

#include <AsyncTaskLib.h>
#include "modular/config.h"
#include "modular/hardware.h"
#include "modular/ui_controller.h"

// Variables simuladas para la prueba
uint8_t NivelAgua[3][4] = {
  {2, 3, 4, 1},
  {1, 2, 3, 4},
  {4, 3, 2, 1}
};

uint8_t RotacionTam[3][4] = {
  {1, 2, 3, 1},
  {2, 2, 2, 3},
  {3, 2, 1, 1}
};

uint8_t TemperaturaLim[3][4] = {
  {30, 40, 50, 60},
  {40, 45, 50, 55},
  {60, 50, 40, 30}
};

uint8_t TemporizadorLim[3][4] = {
  {5, 10, 15, 20},
  {8, 12, 16, 20},
  {25, 20, 15, 10}
};

// Variables de estado para la demostración
uint8_t programa = 1;
uint8_t fase = 1;
int8_t minutos = 0;
int8_t segundos = 0;
uint8_t valorTemperatura = 25;
uint8_t valorNivel = 0;
boolean programaEnEjecucion = false;

// Tarea para simular el paso del tiempo
AsyncTask simuladorTiempo(1000, true, []() {
  if (programaEnEjecucion) {
    segundos++;
    if (segundos >= 60) {
      segundos = 0;
      minutos++;
    }
    
    // Simulación de cambios en la temperatura y nivel de agua
    if (segundos % 5 == 0) {
      valorTemperatura += (random(0, 2) == 0) ? 1 : -1;
      if (valorTemperatura < 20) valorTemperatura = 20;
      if (valorTemperatura > 90) valorTemperatura = 90;
      
      if (valorNivel < NivelAgua[programa-1][fase-1]) {
        valorNivel++;
      }
    }
    
    // Actualizar la información en la pantalla
    UIController.updateTime(minutos, segundos);
    UIController.updateTemperature(valorTemperatura);
    UIController.updateWaterLevel(valorNivel);
    
    // Simular cambios de fase
    if (minutos >= TemporizadorLim[programa-1][fase-1]) {
      minutos = 0;
      segundos = 0;
      fase++;
      
      if (fase > 4) {
        // Programa terminado
        fase = 1;
        programaEnEjecucion = false;
        UIController.showSelectionScreen(programa);
        UIController.showMessage("Programa completado", 3000);
      } else {
        // Nueva fase
        UIController.updatePhase(fase);
        valorNivel = 0;  // Reiniciar nivel de agua para la nueva fase
        UIController.showMessage("Iniciando fase " + String(fase), 2000);
      }
    }
  }
});

// Tarea para verificar eventos de la pantalla Nextion
AsyncTask verificarEventosNextion(100, true, []() {
  UIController.processEvents();
  
  // Procesar acciones del usuario si las hay
  if (UIController.hasUserAction()) {
    String action = UIController.getUserAction();
    Serial.print("Acción del usuario: ");
    Serial.println(action);
    
    // Procesar acciones basadas en eventos
    if (action.startsWith("PROGRAM_")) {
      programa = action.substring(8).toInt();
      UIController.showSelectionScreen(programa);
    } 
    else if (action == "START") {
      programaEnEjecucion = true;
      fase = 1;
      minutos = 0;
      segundos = 0;
      valorNivel = 0;
      UIController.showExecutionScreen(programa, fase, NivelAgua[programa-1][fase-1], valorTemperatura, RotacionTam[programa-1][fase-1]);
    } 
    else if (action == "STOP") {
      programaEnEjecucion = false;
      UIController.showSelectionScreen(programa);
    } 
    else if (action == "EDIT") {
      UIController.showEditScreen(programa, fase, 1, NivelAgua[programa-1][fase-1]);
    }
  }
});

// Tarea para verificar el botón de emergencia
AsyncTask verificarEmergencia(50, true, []() {
  Hardware.updateDebouncer();
  
  if (Hardware.isEmergencyButtonPressed()) {
    programaEnEjecucion = false;
    UIController.showEmergencyScreen();
    delay(3000);  // Esperar para mostrar la pantalla de emergencia
    UIController.showSelectionScreen(programa);
  }
});

void setup() {
  // Inicializar comunicación serial para depuración
  Serial.begin(115200);
  Serial.println("Iniciando prueba de comunicación modular con pantalla Nextion...");
  
  // Inicializar módulos
  Hardware.init();
  UIController.init();
  
  // Mostrar pantalla de bienvenida
  UIController.showWelcomeScreen();
  delay(3000); // Esperar 3 segundos
  
  // Cambiar a pantalla de selección
  UIController.showSelectionScreen(programa);
  
  // Iniciar tareas asíncronas
  simuladorTiempo.Start();
  verificarEventosNextion.Start();
  verificarEmergencia.Start();
  
  Serial.println("Sistema de prueba inicializado correctamente");
}

void loop() {
  // Actualizar tareas asíncronas
  simuladorTiempo.Update();
  verificarEventosNextion.Update();
  verificarEmergencia.Update();
}