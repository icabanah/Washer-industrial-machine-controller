# Módulo de Controlador de Interfaz de Usuario (ui_controller.h / ui_controller.cpp)

## Descripción

El módulo Controlador de Interfaz de Usuario se encarga de gestionar todas las interacciones visuales con el usuario. Maneja la presentación de información en el LCD, genera las diferentes pantallas del sistema y formatea los datos para su visualización.

## Analogía: Diseñador de Escenarios

Este módulo funciona como un diseñador de escenarios en una obra de teatro. Se encarga de preparar el escenario (la pantalla LCD) para cada acto (estado del sistema), colocando los elementos visuales (textos, números) en las posiciones correctas y con el formato adecuado. El diseñador no actúa en la obra ni decide cuándo cambiar de escena, pero es responsable de que todo lo visual esté correctamente configurado.

## Estructura del Módulo

El módulo de Controlador de UI se divide en:

- **ui_controller.h**: Define la interfaz pública del módulo
- **ui_controller.cpp**: Implementa la funcionalidad interna

### Interfaz (ui_controller.h)

```cpp
// ui_controller.h
#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "config.h"
#include "hardware.h"

class UIControllerClass {
public:
  // Inicialización
  void init();
  
  // Métodos para mostrar diferentes pantallas
  void showWelcomeScreen();
  void showSelectionScreen(uint8_t programa = 0);
  void showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor);
  void showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  void showErrorScreen();
  
  // Métodos para actualizar datos en pantalla
  void updateTime(uint8_t minutos, uint8_t segundos);
  void updateTemperature(uint8_t temperatura);
  void updateWaterLevel(uint8_t nivel);
  void updateRotation(uint8_t rotacion);
  void updatePhase(uint8_t fase);
  
  // Métodos para modo edición
  void enableEditMode(bool enable = true);
  void setEditCursor(uint8_t nivel);
  
  // Método para mostrar variables en ejecución
  void displayVariables(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t minutos, uint8_t segundos);

private:
  // Métodos internos para formatear/pintar elementos
  void _printProgramInfo(uint8_t programa);
  void _printPhaseInfo(uint8_t programa, uint8_t fase);
  void _printTimeFormatted(uint8_t col, uint8_t row, uint8_t minutos, uint8_t segundos);
  void _printVariableInfo(uint8_t numeroVariable, uint8_t programa, uint8_t fase, uint8_t valor);
  
  // Referencia a los datos del programa para visualización
  uint8_t (*_nivelAgua)[4];
  uint8_t (*_rotacionTam)[4];
  uint8_t (*_temperaturaLim)[4];
  uint8_t (*_temporizadorLim)[4];
};

// Instancia global
extern UIControllerClass UIController;

#endif // UI_CONTROLLER_H
```

### Implementación (ui_controller.cpp)

```cpp
// ui_controller.cpp
#include "ui_controller.h"
#include "program_controller.h"

// Definición de la instancia global
UIControllerClass UIController;

void UIControllerClass::init() {
  // Obtener referencia a los datos de programa
  _nivelAgua = ProgramController.getNivelAguaData();
  _rotacionTam = ProgramController.getRotacionData();
  _temperaturaLim = ProgramController.getTemperaturaData();
  _temporizadorLim = ProgramController.getTemporizadorData();
}

void UIControllerClass::showWelcomeScreen() {
  Hardware.lcdClear();
  Hardware.lcdPrint(1, 0, "RH Electronics");
  Hardware.lcdPrint(3, 1, "958970967");
}

void UIControllerClass::showSelectionScreen(uint8_t programa) {
  Hardware.lcdClear();
  
  // Si no se especifica programa, usar el actual
  if (programa == 0) {
    programa = ProgramController.getCurrentProgram();
  }
  
  // Mostrar número de programa
  Hardware.lcdPrint(0, 0, "P");
  Hardware.lcdPrintNumber(1, 0, programa + 21, 0);
  
  // Mostrar información según el programa
  if (programa == 3) {
    // Para programa 3, mostrar todas las fases
    Hardware.lcdPrint(4, 0, "N");
    for (uint8_t i = 0; i < 4; i++) {
      Hardware.lcdPrintNumber(i + 5, 0, _nivelAgua[programa - 1][i], 0);
    }
    
    Hardware.lcdPrint(10, 0, "T");
    for (uint8_t i = 0; i < 4; i++) {
      Hardware.lcdPrintNumber(i + 11, 0, _temporizadorLim[programa - 1][i], 0);
    }
    
    // Segunda fila
    Hardware.lcdPrint(0, 1, "T");
    Hardware.lcdPrint(2, 1, "/");
    Hardware.lcdPrint(5, 1, "/");
    Hardware.lcdPrint(8, 1, "/");
    
    for (uint8_t i = 0; i < 4; i++) {
      if (_temperaturaLim[programa - 1][i] < 10) {
        Hardware.lcdPrint(3 * i, 1, "0");
        Hardware.lcdPrintNumber(3 * i + 1, 1, _temperaturaLim[programa - 1][i], 0);
      } else {
        Hardware.lcdPrintNumber(3 * i, 1, _temperaturaLim[programa - 1][i], 0);
      }
    }
    
    Hardware.lcdPrint(11, 1, "R");
    for (uint8_t i = 0; i < 4; i++) {
      Hardware.lcdPrintNumber(i + 12, 1, _rotacionTam[programa - 1][i], 0);
    }
  } else {
    // Para programas 1 y 2, mostrar solo primera fase
    Hardware.lcdPrint(4, 0, "Niv:");
    Hardware.lcdPrintNumber(8, 0, _nivelAgua[programa - 1][0], 0);
    
    Hardware.lcdPrint(10, 0, "Tie:");
    Hardware.lcdPrintNumber(14, 0, _temporizadorLim[programa - 1][0], 0);
    
    Hardware.lcdPrint(0, 1, "Temp:");
    
    if (_temperaturaLim[programa - 1][0] < 10) {
      Hardware.lcdPrint(5, 1, "0");
      Hardware.lcdPrintNumber(6, 1, _temperaturaLim[programa - 1][0], 0);
    } else {
      Hardware.lcdPrintNumber(5, 1, _temperaturaLim[programa - 1][0], 0);
    }
    
    Hardware.lcdPrint(10, 1, "Rot:");
    Hardware.lcdPrintNumber(14, 1, _rotacionTam[programa - 1][0], 0);
  }
}

void UIControllerClass::showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor) {
  Hardware.lcdClear();
  
  // Mostrar cabecera con programa
  Hardware.lcdPrint(0, 0, "P");
  Hardware.lcdPrintNumber(1, 0, programa + 21, 0);
  Hardware.lcdPrint(4, 0, "Seleccione:");
  
  // Mostrar información según variable seleccionada
  Hardware.lcdPrint(0, 1, "Fase:");
  Hardware.lcdPrintNumber(5, 1, fase, 0);
  
  // Según la variable seleccionada, mostrar diferente información
  switch (numeroVariable) {
    case 1: // Nivel de agua
      Hardware.lcdPrint(8, 1, "Nivel:");
      Hardware.lcdPrintNumber(14, 1, valor, 0);
      break;
      
    case 2: // Tiempo
      Hardware.lcdPrint(8, 1, "Tiemp:");
      Hardware.lcdPrintNumber(14, 1, valor, 0);
      break;
      
    case 3: // Temperatura
      Hardware.lcdPrint(8, 1, "Tempe:");
      Hardware.lcdPrintNumber(14, 1, valor, 0);
      break;
      
    case 4: // Rotación
      Hardware.lcdPrint(8, 1, "VRota:");
      Hardware.lcdPrintNumber(14, 1, valor, 0);
      break;
  }
}

void UIControllerClass::showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion) {
  Hardware.lcdClear();
  
  // Primera fila - Programa, fase, nivel, temperatura
  Hardware.lcdPrint(0, 0, "P");
  Hardware.lcdPrintNumber(1, 0, programa + 21, 0);
  
  Hardware.lcdPrint(4, 0, "F:");
  Hardware.lcdPrintNumber(6, 0, fase, 0);
  
  Hardware.lcdPrint(8, 0, "N:");
  Hardware.lcdPrintNumber(10, 0, nivelAgua, 0);
  
  Hardware.lcdPrint(12, 0, "T:");
  Hardware.lcdPrintNumber(14, 0, temperatura, 0);
  
  // Segunda fila - Tiempo y rotación
  Hardware.lcdPrint(0, 1, "Ti:");
  
  // Tiempo inicial en 00:00
  _printTimeFormatted(3, 1, 0, 0);
  
  Hardware.lcdPrint(12, 1, "R:");
  Hardware.lcdPrintNumber(14, 1, rotacion, 0);
}

void UIControllerClass::showErrorScreen() {
  Hardware.lcdClear();
  Hardware.lcdPrint(6, 0, "ERROR:");
  Hardware.lcdPrint(7, 1, "400");
}

void UIControllerClass::updateTime(uint8_t minutos, uint8_t segundos) {
  _printTimeFormatted(3, 1, minutos, segundos);
}

void UIControllerClass::updateTemperature(uint8_t temperatura) {
  Hardware.lcdPrintNumber(14, 0, temperatura, 0);
}

void UIControllerClass::updateWaterLevel(uint8_t nivel) {
  Hardware.lcdPrintNumber(10, 0, nivel, 0);
}

void UIControllerClass::updateRotation(uint8_t rotacion) {
  Hardware.lcdPrintNumber(14, 1, rotacion, 0);
}

void UIControllerClass::updatePhase(uint8_t fase) {
  Hardware.lcdPrintNumber(6, 0, fase, 0);
}

void UIControllerClass::enableEditMode(bool enable) {
  if (enable) {
    Hardware.lcdBlink(true);
  } else {
    Hardware.lcdBlink(false);
  }
}

void UIControllerClass::setEditCursor(uint8_t nivel) {
  switch (nivel) {
    case 1:
      Hardware.lcdSetCursor(5, 1);
      break;
    case 2:
      Hardware.lcdSetCursor(8, 1);
      break;
    case 3:
      Hardware.lcdSetCursor(14, 1);
      break;
  }
}

void UIControllerClass::displayVariables(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t minutos, uint8_t segundos) {
  Hardware.lcdSetCursor(6, 0);
  Hardware.lcdPrintNumber(6, 0, fase, 0);
  
  Hardware.lcdSetCursor(10, 0);
  Hardware.lcdPrintNumber(10, 0, nivelAgua, 0);
  
  Hardware.lcdSetCursor(14, 0);
  Hardware.lcdPrintNumber(14, 0, temperatura, 0);
  
  _printTimeFormatted(3, 1, minutos, segundos);
}

void UIControllerClass::_printTimeFormatted(uint8_t col, uint8_t row, uint8_t minutos, uint8_t segundos) {
  if (minutos < 10) {
    Hardware.lcdPrint(col, row, "0");
    Hardware.lcdPrintNumber(col + 1, row, minutos, 0);
  } else {
    Hardware.lcdPrintNumber(col, row, minutos, 0);
  }
  
  Hardware.lcdPrint(col + 2, row, ":");
  
  if (segundos < 10) {
    Hardware.lcdPrint(col + 3, row, "0");
    Hardware.lcdPrintNumber(col + 4, row, segundos, 0);
  } else {
    Hardware.lcdPrintNumber(col + 3, row, segundos, 0);
  }
}
```

## Responsabilidades

El módulo Controlador de UI tiene las siguientes responsabilidades:

1. **Presentación**: Mostrar información en el LCD con formato adecuado.
2. **Pantallas**: Generar las diferentes pantallas del sistema (bienvenida, selección, edición, ejecución, error).
3. **Actualización**: Proporcionar métodos para actualizar valores específicos en la pantalla sin redibujarlo todo.
4. **Formateo**: Formatear datos para su correcta visualización (p. ej., añadir ceros a la izquierda en tiempo).
5. **Cursor**: Gestionar la posición y estado del cursor en modo edición.

## Ventajas de este Enfoque

1. **Separación de Responsabilidades**: La lógica visual está separada de la lógica de negocio.
2. **Consistencia**: El formato de presentación es consistente en toda la aplicación.
3. **Mantenibilidad**: Los cambios en la interfaz se pueden realizar en un solo lugar.
4. **Optimización**: Se pueden implementar optimizaciones específicas para actualizar solo las partes necesarias de la pantalla.
5. **Reutilización**: Los métodos de formateo y visualización pueden reutilizarse en diferentes pantallas.

Al centralizar toda la lógica de presentación en este módulo, se facilita la adaptación de la interfaz a diferentes requisitos sin afectar la lógica de negocio, y se garantiza una experiencia de usuario coherente en toda la aplicación.
