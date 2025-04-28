#include <AsyncTaskLib.h>
#include "src/config.h"
#include "src/hardware.h"

// Tarea asíncrona para verificar eventos de la pantalla Nextion
AsyncTask checkNextionEvents(100, true, []() {
  // Verificar eventos cada 100ms
  if (Hardware.nextionCheckForEvents()) {
    String event = Hardware.nextionGetLastEvent();
    Serial.print("Evento Nextion recibido: ");
    Serial.println(event);
  }
});

// Tarea asíncrona para actualizar información en la pantalla cada segundo
AsyncTask updateDisplayInfo(1000, true, []() {
  static int counter = 0;
  counter++;
  
  // Enviar información de prueba a la pantalla
  Hardware.nextionSetText(1, "Cuenta: " + String(counter));
  
  // Simular un valor de temperatura que cambia
  int tempValue = 20 + (counter % 10);
  Hardware.nextionSetText(2, "Temp: " + String(tempValue) + "°C");
});

void setup() {
  // Inicializar comunicación serial para depuración
  Serial.begin(115200);
  Serial.println("Iniciando prueba de comunicación con pantalla Nextion...");
  
  // Inicializar el módulo de hardware
  Hardware.init();
  
  // Enviar un mensaje de prueba a la pantalla
  Hardware.nextionSendCommand("cls");  // Limpiar pantalla
  Hardware.nextionSetText(0, "Prueba de Comunicacion");
  
  // Iniciar las tareas asíncronas
  checkNextionEvents.Start();
  updateDisplayInfo.Start();
  
  Serial.println("Sistema inicializado correctamente");
}

void loop() {
  // Actualizar tareas asíncronas
  checkNextionEvents.Update();
  updateDisplayInfo.Update();
  
  // Actualizar el debouncer del botón de emergencia
  Hardware.updateDebouncer();
  
  // Verificar el botón de emergencia
  if (Hardware.isEmergencyButtonPressed()) {
    Serial.println("¡BOTÓN DE EMERGENCIA PRESIONADO!");
    Hardware.nextionSetPage(NEXTION_PAGE_EMERGENCY);
    Hardware.nextionSetText(1, "EMERGENCIA ACTIVADA");
    delay(3000);  // Pausa para mostrar el mensaje
  }
}