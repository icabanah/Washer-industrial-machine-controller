# Archivo Principal (mainController.ino)

## Descripción

El archivo principal actúa como punto de entrada y coordinador del sistema. Su función es inicializar todos los componentes y dirigir el flujo principal del programa sin implementar directamente la lógica específica de cada subsistema.

## Analogía: Director de Orquesta

Este archivo es como el director de una orquesta, que no toca ningún instrumento específico pero coordina a todos los músicos para que toquen en armonía. El director marca el tiempo, indica entradas y salidas, pero son los músicos quienes realmente producen la música.

## Estructura del Archivo

El archivo principal se reducirá significativamente en tamaño, delegando la mayoría de su funcionalidad actual a los módulos especializados. Contendrá principalmente:

- Inclusión de todos los módulos
- Función `setup()` para inicialización
- Función `loop()` para el bucle principal

## Implementación Propuesta

```cpp
// mainController.ino
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
  
  // Procesar eventos de tareas temporizadas
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
```

## Responsabilidades

El archivo principal tiene las siguientes responsabilidades:

1. **Inicialización**: Asegurarse de que todos los módulos se inicialicen correctamente y en el orden adecuado.
2. **Coordinación**: Llamar a los métodos apropiados de cada módulo en el momento adecuado.
3. **Gestión de Errores**: Manejar errores críticos que afecten a todo el sistema.
4. **Monitoreo de Emergencia**: Verificar el estado del botón de emergencia y responder adecuadamente.

## Ventajas de este Enfoque

1. **Claridad**: El archivo principal muestra claramente el flujo de alto nivel del programa.
2. **Mantenibilidad**: Facilita la comprensión del flujo general sin distraer con detalles de implementación.
3. **Flexibilidad**: Permite reemplazar módulos enteros sin modificar el flujo principal.
4. **Testabilidad**: Facilita la prueba de componentes individuales al tener interfaces bien definidas.
5. **No Bloqueo**: La implementación utiliza técnicas no bloqueantes (sin delay()) para mantener la responsividad del sistema.

## Adaptaciones para el Nuevo Hardware

El archivo principal ha sido actualizado para reflejar los cambios de hardware:

1. **ESP32-WROOM**: No requiere cambios específicos en la estructura del archivo principal, ya que los detalles de hardware están encapsulados en los módulos.
2. **Pantalla Nextion**: Se ha añadido soporte para procesar eventos de la pantalla táctil a través del módulo UIController.
3. **Botón de Emergencia**: Se ha implementado la verificación del botón de emergencia con manejo de antirrebote.

Al delegar la implementación específica a módulos dedicados, el archivo principal se mantiene conciso y enfocado en la coordinación general del sistema, independientemente de los cambios en el hardware subyacente.
