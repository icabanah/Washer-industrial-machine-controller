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
#include "config.h"
#include "hardware.h"
#include "ui_controller.h"
#include "program_controller.h"
#include "sensors.h"
#include "actuators.h"
#include "storage.h"
#include "utils.h"

void setup() {
  // Inicializar hardware
  Hardware.init();
  
  // Inicializar UI
  UIController.init();
  UIController.showWelcomeScreen();
  delay(TIEMPO_BIENVENIDA);
  
  // Inicializar actuadores
  Actuators.init();
  
  // Inicializar sensores
  Sensors.init();
  
  // Cargar configuración de EEPROM
  Storage.loadFromEEPROM();
  
  // Inicializar controlador de programas
  ProgramController.init();
  
  // Inicializar utilidades
  Utils.init();
  
  // Mostrar pantalla de selección
  UIController.showSelectionScreen();
}

void loop() {
  // Verificar botón de emergencia
  Hardware.updateDebouncer();
  if (Hardware.isEmergencyButtonPressed()) {
    ProgramController.handleEmergency();
    return;
  }
  
  // Verificar contador de bloqueo
  if (ProgramController.isSystemBlocked()) {
    UIController.showErrorScreen(400, "Sistema bloqueado");
    while (true) {}  // Detener ejecución
    return;
  }
  
  // Procesar eventos de la pantalla táctil
  UIController.processEvents();
  
  // Si hay acciones del usuario, procesarlas
  if (UIController.hasUserAction()) {
    ProgramController.handleUserAction(UIController.getUserAction());
  }
  
  // Actualizar estado general del sistema
  ProgramController.update();
  
  // Actualizar temporizador si está activo
  Utils.updateTimer();
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

## Adaptaciones para el Nuevo Hardware

El archivo principal ha sido actualizado para reflejar los cambios de hardware:

1. **ESP32-WROOM**: No requiere cambios específicos en la estructura del archivo principal, ya que los detalles de hardware están encapsulados en los módulos.
2. **Pantalla Nextion**: Se ha añadido soporte para procesar eventos de la pantalla táctil a través del módulo UIController.
3. **Botón de Emergencia**: Se ha implementado la verificación del botón de emergencia con manejo de antirrebote.

Al delegar la implementación específica a módulos dedicados, el archivo principal se mantiene conciso y enfocado en la coordinación general del sistema, independientemente de los cambios en el hardware subyacente.
