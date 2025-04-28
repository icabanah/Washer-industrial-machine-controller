# Módulo de Controlador de Interfaz de Usuario (ui_controller.h / ui_controller.cpp)

## Descripción

El módulo Controlador de Interfaz de Usuario se encarga de gestionar todas las interacciones visuales con el usuario a través de la pantalla Nextion. Maneja la presentación de información en la pantalla, interpreta los eventos táctiles generados por el usuario y coordina la navegación entre las diferentes pantallas del sistema.

## Analogía: Director de Experiencia de Usuario

Este módulo funciona como un director de experiencia de usuario en una aplicación, que se encarga de crear una interfaz intuitiva y atractiva para que los usuarios interactúen con el sistema. Decide qué información mostrar, cómo presentarla visualmente y cómo responder a las interacciones del usuario, asegurando una experiencia coherente y fluida.

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
  void showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
  void showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor);
  void showErrorScreen(uint8_t errorCode = 0, const String& errorMessage = "");
  void showEmergencyScreen();
  
  // Métodos para actualizar datos en pantalla
  void updateTime(uint8_t minutos, uint8_t segundos);
  void updateTemperature(uint8_t temperatura);
  void updateWaterLevel(uint8_t nivel);
  void updateRotation(uint8_t rotacion);
  void updatePhase(uint8_t fase);
  void updateProgressBar(uint8_t progress);
  
  // Proceso de eventos de interfaz
  void processEvents();
  bool hasUserAction();
  String getUserAction();
  
  // Métodos de ayuda para interfaz
  void showMessage(const String& message, uint16_t duration = 2000);
  void playSound(uint8_t soundType);

private:
  // Variables para controlar estado de UI
  String _lastUserAction;
  bool _userActionPending;
  unsigned long _messageTimestamp;
  bool _messageActive;
  
  // Referencia a los datos del programa para visualización
  uint8_t (*_nivelAgua)[4];
  uint8_t (*_rotacionTam)[4];
  uint8_t (*_temperaturaLim)[4];
  uint8_t (*_temporizadorLim)[4];
  
  // Métodos internos para procesar componentes
  void _handleNextionEvent(const String& event);
  void _formatTimeDisplay(uint8_t minutos, uint8_t segundos, char* buffer);
  void _updateProgramInfo(uint8_t programa);
  void _updateExecutionData(uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion);
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
  
  _userActionPending = false;
  _messageActive = false;
}

void UIControllerClass::showWelcomeScreen() {
  // Cambiar a la página de bienvenida
  Hardware.nextionSetPage(NEXTION_PAGE_WELCOME);
  
  // Establecer textos de bienvenida
  Hardware.nextionSetText(1, "RH Electronics");
  Hardware.nextionSetText(2, "958970967");
  
  // Activar animación de inicio si existe (ejemplo)
  Hardware.nextionSendCommand("anim.en=1");
}

void UIControllerClass::showSelectionScreen(uint8_t programa) {
  // Cambiar a la página de selección
  Hardware.nextionSetPage(NEXTION_PAGE_SELECTION);
  
  // Si no se especifica programa, usar el actual
  if (programa == 0) {
    programa = ProgramController.getCurrentProgram();
  }
  
  // Actualizar información del programa seleccionado
  _updateProgramInfo(programa);
  
  // Resaltar el botón del programa seleccionado
  for (uint8_t i = 1; i <= 3; i++) {
    String buttonState = (i == programa) ? "1" : "0";
    Hardware.nextionSendCommand("b" + String(i) + ".val=" + buttonState);
  }
}

void UIControllerClass::showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion) {
  // Cambiar a la página de ejecución
  Hardware.nextionSetPage(NEXTION_PAGE_EXECUTION);
  
  // Mostrar información del programa
  Hardware.nextionSetText(1, "P" + String(programa + 21));
  
  // Establecer valores iniciales
  Hardware.nextionSetText(2, "Fase: " + String(fase));
  Hardware.nextionSetText(3, "00:00");  // Tiempo inicial
  
  // Actualizar indicadores
  updateWaterLevel(nivelAgua);
  updateTemperature(temperatura);
  updateRotation(rotacion);
  
  // Inicializar barra de progreso
  updateProgressBar(0);
}

void UIControllerClass::showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor) {
  // Cambiar a la página de edición
  Hardware.nextionSetPage(NEXTION_PAGE_EDIT);
  
  // Mostrar información básica
  Hardware.nextionSetText(1, "P" + String(programa + 21) + " - F" + String(fase));
  
  // Mostrar etiqueta según variable seleccionada
  String varLabel;
  switch (numeroVariable) {
    case 1: varLabel = "Nivel de agua"; break;
    case 2: varLabel = "Tiempo (min)"; break;
    case 3: varLabel = "Temperatura"; break;
    case 4: varLabel = "Velocidad"; break;
    default: varLabel = "Desconocido"; break;
  }
  
  Hardware.nextionSetText(2, varLabel);
  
  // Mostrar valor actual
  Hardware.nextionSetText(3, String(valor));
  
  // Configurar rangos para los controles
  int minVal = 0;
  int maxVal = 100;
  
  switch (numeroVariable) {
    case 1: maxVal = 4; break;  // Nivel agua: 1-4
    case 4: maxVal = 3; break;  // Velocidad: 1-3
  }
  
  // Configurar controles
  Hardware.nextionSendCommand("h0.minval=" + String(minVal));
  Hardware.nextionSendCommand("h0.maxval=" + String(maxVal));
  Hardware.nextionSendCommand("h0.val=" + String(valor));
}

void UIControllerClass::showErrorScreen(uint8_t errorCode, const String& errorMessage) {
  // Cambiar a la página de error
  Hardware.nextionSetPage(NEXTION_PAGE_ERROR);
  
  // Mostrar código de error
  Hardware.nextionSetText(1, "ERROR " + String(errorCode));
  
  // Mostrar mensaje de error si se proporciona
  if (errorMessage.length() > 0) {
    Hardware.nextionSetText(2, errorMessage);
  } else {
    // Mensaje por defecto basado en código
    switch (errorCode) {
      case 400: Hardware.nextionSetText(2, "Error de sistema"); break;
      case 401: Hardware.nextionSetText(2, "Error de temperatura"); break;
      case 402: Hardware.nextionSetText(2, "Error de presión"); break;
      default: Hardware.nextionSetText(2, "Error desconocido"); break;
    }
  }
  
  // Activar indicador de alerta
  Hardware.nextionSendCommand("alerta.en=1");
}

void UIControllerClass::showEmergencyScreen() {
  // Cambiar a la página de emergencia
  Hardware.nextionSetPage(NEXTION_PAGE_EMERGENCY);
  
  // Mostrar mensaje de emergencia
  Hardware.nextionSetText(1, "PARADA DE EMERGENCIA");
  Hardware.nextionSetText(2, "Sistema detenido por seguridad");
  
  // Activar indicador visual de emergencia
  Hardware.nextionSendCommand("alarm.en=1");
  
  // Reproducir sonido de alarma
  playSound(2);  // Código 2 para sonido de alarma
}

void UIControllerClass::updateTime(uint8_t minutos, uint8_t segundos) {
  char timeBuffer[6];
  _formatTimeDisplay(minutos, segundos, timeBuffer);
  Hardware.nextionSetText(3, timeBuffer);
  
  // Actualizar barra de progreso si es necesario
  // (suponiendo fase de 60 minutos máximo)
  uint8_t progress = (minutos * 60 + segundos) / 36;  // 0-100%
  updateProgressBar(progress);
}

void UIControllerClass::_formatTimeDisplay(uint8_t minutos, uint8_t segundos, char* buffer) {
  sprintf(buffer, "%02d:%02d", minutos, segundos);
}

void UIControllerClass::updateTemperature(uint8_t temperatura) {
  // Actualizar texto de temperatura
  Hardware.nextionSetText(4, String(temperatura) + "°C");
  
  // Actualizar medidor visual si existe
  Hardware.nextionSetValue(NEXTION_COMP_GAUGE_TEMP, temperatura);
}

void UIControllerClass::updateWaterLevel(uint8_t nivel) {
  // Actualizar texto de nivel
  Hardware.nextionSetText(5, "Nivel: " + String(nivel));
  
  // Actualizar indicador visual
  Hardware.nextionSetValue(NEXTION_COMP_GAUGE_PRESSURE, nivel * 25);  // 0-100%
}

void UIControllerClass::updateRotation(uint8_t rotacion) {
  // Actualizar texto de rotación
  Hardware.nextionSetText(6, "Vel: " + String(rotacion));
  
  // Actualizar indicador visual si existe
  Hardware.nextionSendCommand("motor.val=" + String(rotacion));
}

void UIControllerClass::updatePhase(uint8_t fase) {
  Hardware.nextionSetText(2, "Fase: " + String(fase));
}

void UIControllerClass::updateProgressBar(uint8_t progress) {
  // Actualizar barra de progreso
  Hardware.nextionSendCommand("j0.val=" + String(progress));
}

void UIControllerClass::processEvents() {
  // Verificar si hay eventos de la pantalla Nextion
  if (Hardware.nextionCheckForEvents()) {
    String event = Hardware.nextionGetLastEvent();
    _handleNextionEvent(event);
  }
  
  // Actualizar mensajes temporales
  if (_messageActive && (millis() - _messageTimestamp > 2000)) {
    _messageActive = false;
    // Ocultar mensaje
    Hardware.nextionSendCommand("msgBox.vis=0");
  }
}

void UIControllerClass::_handleNextionEvent(const String& event) {
  // Procesar evento de la pantalla Nextion
  // Los eventos típicos son del formato:
  // 65 1 2 : Botón 1 presionado en página 2
  
  if (event.length() >= 3) {
    char eventType = event[0];
    int componentId = event[1] - '0';  // Convertir char a int
    int pageId = event[2] - '0';       // Convertir char a int
    
    // Eventos de botón (código 65)
    if (eventType == 65) {
      switch (pageId) {
        case NEXTION_PAGE_SELECTION:
          if (componentId >= 1 && componentId <= 3) {
            // Selección de programa
            _lastUserAction = "PROGRAM_" + String(componentId);
            _userActionPending = true;
          } else if (componentId == NEXTION_COMP_BTN_START) {
            // Botón iniciar
            _lastUserAction = "START";
            _userActionPending = true;
          } else if (componentId == NEXTION_COMP_BTN_EDIT) {
            // Botón editar
            _lastUserAction = "EDIT";
            _userActionPending = true;
          }
          break;
          
        case NEXTION_PAGE_EXECUTION:
          if (componentId == NEXTION_COMP_BTN_STOP) {
            // Botón parar
            _lastUserAction = "STOP";
            _userActionPending = true;
          }
          break;
          
        case NEXTION_PAGE_EDIT:
          if (componentId == 7) {  // Botón guardar
            _lastUserAction = "SAVE";
            _userActionPending = true;
          } else if (componentId == 8) {  // Botón cancelar
            _lastUserAction = "CANCEL";
            _userActionPending = true;
          }
          break;
      }
    }
    // Eventos de slider o controles numéricos (código 67)
    else if (eventType == 67) {
      // Ejemplo: obtener valor de un slider
      // El formato puede variar según el componente
      _lastUserAction = "VALUE_CHANGE_" + String(componentId) + "_" + String(pageId);
      _userActionPending = true;
    }
  }
}

void UIControllerClass::_updateProgramInfo(uint8_t programa) {
  // Actualizar información mostrada para el programa seleccionado
  Hardware.nextionSetText(10, "P" + String(programa + 21));
  
  // Mostrar parámetros para la primera fase del programa
  Hardware.nextionSetText(11, "Nivel: " + String(_nivelAgua[programa - 1][0]));
  Hardware.nextionSetText(12, "Temp: " + String(_temperaturaLim[programa - 1][0]) + "°C");
  Hardware.nextionSetText(13, "Tiempo: " + String(_temporizadorLim[programa - 1][0]) + " min");
  Hardware.nextionSetText(14, "Vel: " + String(_rotacionTam[programa - 1][0]));
  
  // Si es el programa 3, mostrar información adicional
  if (programa == 3) {
    // Aquí se podría mostrar información de todas las fases
    String fasesInfo = "Fases: ";
    for (uint8_t i = 0; i < 4; i++) {
      fasesInfo += String(i + 1) + ":" + String(_temporizadorLim[programa - 1][i]) + "m ";
    }
    Hardware.nextionSetText(15, fasesInfo);
  } else {
    Hardware.nextionSetText(15, "");  // Limpiar texto si no es programa 3
  }
}

bool UIControllerClass::hasUserAction() {
  return _userActionPending;
}

String UIControllerClass::getUserAction() {
  _userActionPending = false;
  return _lastUserAction;
}

void UIControllerClass::showMessage(const String& message, uint16_t duration) {
  // Mostrar un mensaje temporal en la pantalla
  Hardware.nextionSendCommand("msgBox.txt=\"" + message + "\"");
  Hardware.nextionSendCommand("msgBox.vis=1");
  
  _messageActive = true;
  _messageTimestamp = millis();
}

void UIControllerClass::playSound(uint8_t soundType) {
  // Reproducir un sonido en la pantalla Nextion
  // Los tipos pueden ser:
  // 0: normal, 1: advertencia, 2: alarma
  Hardware.nextionSendCommand("audio.val=" + String(soundType));
}
```

## Responsabilidades

El módulo Controlador de UI tiene las siguientes responsabilidades:

1. **Presentación**: Mostrar información en la pantalla Nextion con formato adecuado.
2. **Pantallas**: Generar las diferentes pantallas del sistema (bienvenida, selección, edición, ejecución, error, emergencia).
3. **Actualización**: Proporcionar métodos para actualizar valores específicos en la pantalla.
4. **Interacción**: Procesar eventos táctiles y acciones del usuario en la interfaz.
5. **Feedback**: Proporcionar retroalimentación visual y auditiva al usuario.

## Ventajas de este Enfoque

1. **Separación de Responsabilidades**: La lógica visual está separada de la lógica de negocio.
2. **Consistencia**: El formato de presentación es consistente en toda la aplicación.
3. **Mantenibilidad**: Los cambios en la interfaz se pueden realizar en un solo lugar.
4. **Experiencia de Usuario**: Facilita la implementación de una interfaz moderna e intuitiva.
5. **Reutilización**: Los métodos de formateo y visualización pueden reutilizarse en diferentes pantallas.

## Adaptaciones para el Nuevo Hardware

El módulo ha sido completamente rediseñado para adaptarse a la pantalla Nextion:

1. **Pantalla Táctil**: Se han implementado métodos para procesar eventos táctiles, eliminando la dependencia de los botones físicos.
2. **Comunicación Serial**: La interfaz utiliza comandos seriales específicos de Nextion para controlar la pantalla.
3. **Capacidades Gráficas**: Se aprovechan las capacidades gráficas de la pantalla Nextion para mostrar indicadores visuales, barras de progreso y animaciones.
4. **Estado de Emergencia**: Se ha añadido soporte específico para la pantalla de emergencia, activada por el botón de emergencia.

Al centralizar toda la lógica de presentación en este módulo, se facilita la adaptación de la interfaz a diferentes requisitos sin afectar la lógica de negocio, y se garantiza una experiencia de usuario coherente y mejorada en toda la aplicación.
