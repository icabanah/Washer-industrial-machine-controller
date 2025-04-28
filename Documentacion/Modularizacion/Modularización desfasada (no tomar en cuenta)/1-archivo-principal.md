# Archivo Principal (automLavanderia_v1.ino)

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
// automLavanderia_v1.ino
#include "config.h"
#include "hardware.h"
#include "ui_controller.h"
#include "program_controller.h"
#include "sensors.h"
#include "actuators.h"
#include "storage.h"

void setup() {
  // Inicializar hardware
  Hardware.init();
  
  // Inicializar UI
  UIController.init();
  UIController.showWelcomeScreen();
  
  // Inicializar sensores
  Sensors.init();
  
  // Cargar configuración de EEPROM
  Storage.loadFromEEPROM();
  
  // Inicializar pantalla de selección
  UIController.showSelectionScreen();
}

void loop() {
  // Verificar contador de bloqueo
  if (ProgramController.isSystemBlocked()) {
    UIController.showErrorScreen();
    while (true) {}
    return;
  }
  
  // Procesar entrada de usuario y estado actual
  Hardware.processButtons();
  ProgramController.update();
}
```

## Responsabilidades

El archivo principal tiene las siguientes responsabilidades:

1. **Inicialización**: Asegurarse de que todos los módulos se inicialicen correctamente y en el orden adecuado.
2. **Coordinación**: Llamar a los métodos apropiados de cada módulo en el momento adecuado.
3. **Gestión de Errores**: Manejar errores críticos que afecten a todo el sistema.

## Ventajas de este Enfoque

1. **Claridad**: El archivo principal muestra claramente el flujo de alto nivel del programa.
2. **Mantenibilidad**: Facilita la comprensión del flujo general sin distraer con detalles de implementación.
3. **Flexibilidad**: Permite reemplazar módulos enteros sin modificar el flujo principal.
4. **Testabilidad**: Facilita la prueba de componentes individuales al tener interfaces bien definidas.

Al delegar la implementación específica a módulos dedicados, el archivo principal se mantiene conciso y enfocado en la coordinación general del sistema.
