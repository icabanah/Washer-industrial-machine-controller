// ui_controller.cpp
#include "ui_controller.h"

// Definición de la instancia global
UIControllerClass UIController;

// Definición de variables para almacenar datos de programa
// Estas variables simulan los datos que vendrían de Program Controller
uint8_t NivelAgua[3][4] = {
  {2, 3, 4, 1},  // Programa 1
  {3, 2, 4, 2},  // Programa 2
  {4, 3, 2, 1}   // Programa 3
};

uint8_t RotacionTam[3][4] = {
  {1, 2, 1, 3},  // Programa 1
  {2, 1, 3, 2},  // Programa 2
  {3, 2, 1, 2}   // Programa 3
};

uint8_t TemperaturaLim[3][4] = {
  {30, 40, 50, 40},  // Programa 1
  {35, 45, 55, 45},  // Programa 2
  {40, 50, 60, 50}   // Programa 3
};

uint8_t TemporizadorLim[3][4] = {
  {5, 10, 15, 5},   // Programa 1
  {10, 15, 20, 10}, // Programa 2
  {15, 20, 25, 15}  // Programa 3
};

void UIControllerClass::init() {
  // Obtener referencia a los datos de programa
  _nivelAgua = NivelAgua;
  _rotacionTam = RotacionTam;
  _temperaturaLim = TemperaturaLim;
  _temporizadorLim = TemporizadorLim;
  
  _userActionPending = false;
  _messageActive = false;
  
  // Inicializar variables para limpieza de eventos
  _clearingEvents = false;
  _clearingStartTime = 0;
  
  Serial.println("UI Controller inicializado con sistema de limpieza de eventos");
}

void UIControllerClass::showWelcomeScreen() {
  Serial.println("=== INICIANDO PANTALLA BIENVENIDA ===");
  Serial.print("Componente título definido como: ");
  Serial.println(NEXTION_COMP_TITULO);
  
  // Cambiar a la página de bienvenida
  Hardware.nextionSetPage(NEXTION_PAGE_WELCOME);
  delay(100); // Pausa breve para asegurar cambio de página
  
  // Establecer textos de bienvenida usando los componentes correctos de la documentación
  Serial.println("Enviando comando para título...");
  Hardware.nextionSetText(NEXTION_COMP_TITULO, "iTrebolsoft");
  
  Serial.println("Enviando comando para subtítulo...");  
  Hardware.nextionSetText(NEXTION_COMP_SUBTITULO, "Controlador de Lavadora Industrial");
  
  Serial.println("Enviando comando para contacto...");
  Hardware.nextionSetText(NEXTION_COMP_CONTACTO, "958970967");
  
  // Activar animación de inicio si existe (ejemplo)
  Hardware.nextionSendCommand("anim.en=1");
  
  Serial.println("=== PANTALLA BIENVENIDA ENVIADA ===");
}

void UIControllerClass::showSelectionScreen(uint8_t programa) {
  // Cambiar a la página de selección
  Hardware.nextionSetPage(NEXTION_PAGE_SELECTION);
  
  // Actualizar información del programa seleccionado
  _updateProgramInfo(programa);
  
  // Resaltar el botón del programa seleccionado
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM1, (programa == 1) ? 1 : 0);
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM2, (programa == 2) ? 1 : 0);
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM3, (programa == 3) ? 1 : 0);
  
  Serial.println("Mostrando pantalla de selección de programa");
}

void UIControllerClass::showExecutionScreen(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion) {
  // Cambiar a la página de ejecución
  Hardware.nextionSetPage(NEXTION_PAGE_EXECUTION);
  
  // Mostrar información del programa usando los componentes correctos de la documentación
  Hardware.nextionSetText(NEXTION_COMP_PROG_EJECUCION, "P" + String(programa + 21));
  
  // Establecer valores iniciales
  Hardware.nextionSetText(NEXTION_COMP_FASE_EJECUCION, "Fase: " + String(fase));
  Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, "00:00");  // Tiempo inicial
  
  // Actualizar indicadores usando los componentes existentes que funcionan correctamente
  updateWaterLevel(nivelAgua);
  updateTemperature(temperatura);
  updateRotation(rotacion);
  
  // Inicializar barra de progreso
  updateProgressBar(0);
  
  Serial.println("Mostrando pantalla de ejecución de programa");
}

void UIControllerClass::showEditScreen(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor) {
  // Cambiar a la página de edición
  Hardware.nextionSetPage(NEXTION_PAGE_EDIT);
  
  // Mostrar información básica
  Hardware.nextionSetText("txtProgramaFase", "P" + String(programa + 21) + " - F" + String(fase));
  
  // Mostrar etiqueta según variable seleccionada
  String varLabel;
  switch (numeroVariable) {
    case 1: varLabel = "Nivel de agua"; break;
    case 2: varLabel = "Tiempo (min)"; break;
    case 3: varLabel = "Temperatura"; break;
    case 4: varLabel = "Velocidad"; break;
    default: varLabel = "Desconocido"; break;
  }
  
  Hardware.nextionSetText("txtVariable", varLabel);
  
  // Mostrar valor actual
  Hardware.nextionSetText("txtValor", String(valor));
  
  // Configurar rangos para los controles
  int minVal = 0;
  int maxVal = 100;
  
  switch (numeroVariable) {
    case 1: maxVal = 4; break;  // Nivel agua: 1-4
    case 4: maxVal = 3; break;  // Velocidad: 1-3
  }
  
  // Configurar controles
  Hardware.nextionSendCommand("sliderValor.minval=" + String(minVal));
  Hardware.nextionSendCommand("sliderValor.maxval=" + String(maxVal));
  Hardware.nextionSendCommand("sliderValor.val=" + String(valor));
  
  Serial.println("Mostrando pantalla de edición");
}

void UIControllerClass::showErrorScreen(uint8_t errorCode, const String& errorMessage) {
  // Cambiar a la página de error
  Hardware.nextionSetPage(NEXTION_PAGE_ERROR);
  
  // Mostrar código de error
  Hardware.nextionSetText("txtCodigo", "ERROR " + String(errorCode));
  
  // Mostrar mensaje de error si se proporciona
  if (errorMessage.length() > 0) {
    Hardware.nextionSetText("txtMensaje", errorMessage);
  } else {
    // Mensaje por defecto basado en código
    switch (errorCode) {
      case 400: Hardware.nextionSetText("txtMensaje", "Error de sistema"); break;
      case 401: Hardware.nextionSetText("txtMensaje", "Error de temperatura"); break;
      case 402: Hardware.nextionSetText("txtMensaje", "Error de presión"); break;
      default: Hardware.nextionSetText("txtMensaje", "Error desconocido"); break;
    }
  }
  
  // Activar indicador de alerta
  Hardware.nextionSendCommand("alerta.en=1");
  
  Serial.println("Mostrando pantalla de error");
}

void UIControllerClass::showEmergencyScreen() {
  // Cambiar a la página de emergencia
  Hardware.nextionSetPage(NEXTION_PAGE_EMERGENCY);
  
  // Mostrar mensaje de emergencia
  Hardware.nextionSetText("txtEmergencia", "PARADA DE EMERGENCIA");
  Hardware.nextionSetText("txtMensajeEmerg", "Sistema detenido por seguridad");
  
  // Activar indicador visual de emergencia
  Hardware.nextionSendCommand("alarm.en=1");
  
  // Reproducir sonido de alarma
  playSound(2);  // Código 2 para sonido de alarma
  
  Serial.println("EMERGENCIA: Sistema detenido");
}

void UIControllerClass::updateTime(uint8_t minutos, uint8_t segundos) {
  char timeBuffer[6];
  _formatTimeDisplay(minutos, segundos, timeBuffer);
  Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, timeBuffer);
  
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
  Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, String(temperatura) + "°C");
  
  // Actualizar medidor visual si existe
  Hardware.nextionSetValue(NEXTION_COMP_GAUGE_TEMP_EJECUCION, temperatura);
}

void UIControllerClass::updateWaterLevel(uint8_t nivel) {
  // Actualizar texto de nivel
  Hardware.nextionSetText(NEXTION_COMP_VAL_NIVEL, "Nivel: " + String(nivel));
  
  // Actualizar indicador visual
  Hardware.nextionSetValue(NEXTION_COMP_VAL_ROTACION, nivel * 25);  // 0-100%
}

void UIControllerClass::updateRotation(uint8_t rotacion) {
  // Actualizar texto de rotación
  Hardware.nextionSetText("txtRotacion", "Vel: " + String(rotacion));
  
  // Actualizar indicador visual si existe
  Hardware.nextionSendCommand("motor.val=" + String(rotacion));
}

void UIControllerClass::updatePhase(uint8_t fase) {
  Hardware.nextionSetText(NEXTION_COMP_FASE_EJECUCION, "Fase: " + String(fase));
}

void UIControllerClass::updateProgressBar(uint8_t progress) {
  // Actualizar barra de progreso usando el componente correcto de la documentación
  Hardware.nextionSendCommand(String(NEXTION_COMP_BARRA_PROGRESO) + ".val=" + String(progress));
}

void UIControllerClass::processEvents() {
  // Verificar si estamos en proceso de limpieza de eventos
  if (_clearingEvents) {
    // Durante la limpieza, descartar todos los eventos sin procesarlos
    while (Hardware.nextionCheckForEvents()) {
      Hardware.nextionGetLastEvent(); // Descartar evento
    }
    
    // Verificar si la limpieza ha terminado
    _isEventClearingComplete();
    return; // No procesar eventos normales durante la limpieza
  }
  
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
    uint8_t eventType = event[0];
    String componentName;
    int pageId = event[2];       // Convertir byte a int
    
    Serial.print("Evento procesado: Tipo=");
    Serial.print(eventType);
    Serial.print(" Página=");
    Serial.println(pageId);
    
    // Eventos de botón (código 65)
    if (eventType == 65) {
      // Obtener nombre del componente basado en el ID del evento
      switch (pageId) {
        case NEXTION_PAGE_SELECTION:
          // Determinar qué botón fue presionado
          if (event[1] == NEXTION_COMP_BTN_PROGRAM1[0]) {
            _lastUserAction = "PROGRAM_1";
            _userActionPending = true;
          } else if (event[1] == NEXTION_COMP_BTN_PROGRAM2[0]) {
            _lastUserAction = "PROGRAM_2";
            _userActionPending = true;
          } else if (event[1] == NEXTION_COMP_BTN_PROGRAM3[0]) {
            _lastUserAction = "PROGRAM_3";
            _userActionPending = true;
          } else if (event[1] == NEXTION_COMP_BTN_START[0]) {
            // Botón iniciar
            _lastUserAction = "START";
            _userActionPending = true;
          } else if (event[1] == NEXTION_COMP_BTN_EDIT[0]) {
            // Botón editar
            _lastUserAction = "EDIT";
            _userActionPending = true;
          }
          break;
          
        case NEXTION_PAGE_EXECUTION:
          if (event[1] == NEXTION_COMP_BTN_STOP[0]) {
            // Botón parar
            _lastUserAction = "STOP";
            _userActionPending = true;
          }
          break;
          
        case NEXTION_PAGE_EDIT:
          if (event[1] == 7) {  // Botón guardar - mantener ID numérico hasta actualización
            _lastUserAction = "SAVE";
            _userActionPending = true;
          } else if (event[1] == 8) {  // Botón cancelar - mantener ID numérico hasta actualización
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
      _lastUserAction = "VALUE_CHANGE_" + String(event[1]) + "_" + String(pageId);
      _userActionPending = true;
    }
  }
}

void UIControllerClass::_updateProgramInfo(uint8_t programa) {
  // Actualizar información mostrada para el programa seleccionado
  Hardware.nextionSetText(NEXTION_COMP_PROGRAMA_SEL, "P" + String(programa + 21));
  
  // Mostrar valores para la primera fase del programa usando los nuevos componentes
  // Los valores se muestran en t21, t22, t23, t24 (las etiquetas t11-t14 son fijas en Nextion)
  Hardware.nextionSetText(NEXTION_COMP_VAL_NIVEL, String(_nivelAgua[programa - 1][0]));
  Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, String(_temperaturaLim[programa - 1][0]) + "°C");
  Hardware.nextionSetText(NEXTION_COMP_VAL_TIEMPO, String(_temporizadorLim[programa - 1][0]) + " min");
  
  // Convertir valor numérico de rotación a texto descriptivo
  String rotacionTexto;
  switch (_rotacionTam[programa - 1][0]) {
    case 1: rotacionTexto = "Lento"; break;
    case 2: rotacionTexto = "Medio"; break;
    case 3: rotacionTexto = "Rápido"; break;
    default: rotacionTexto = "Desconocido"; break;
  }
  Hardware.nextionSetText(NEXTION_COMP_VAL_ROTACION, rotacionTexto);
  
  // Si es el programa 3 (P24), mostrar información adicional de múltiples fases
  if (programa == 3) {
    // Aquí se podría mostrar información de todas las fases en t15
    String fasesInfo = "Fases: ";
    for (uint8_t i = 0; i < 4; i++) {
      fasesInfo += String(i + 1) + ":" + String(_temporizadorLim[programa - 1][i]) + "m ";
    }
    Hardware.nextionSetText(NEXTION_COMP_INFO_FASES, fasesInfo);
  } else {
    Hardware.nextionSetText(NEXTION_COMP_INFO_FASES, "");  // Limpiar texto si no es programa 3
  }
  
  Serial.println("Información del programa P" + String(programa + 21) + " actualizada en pantalla");
}

bool UIControllerClass::hasUserAction() {
  return _userActionPending;
}

String UIControllerClass::getUserAction() {
  _userActionPending = false;
  return _lastUserAction;
}

bool UIControllerClass::isUIStable() {
  // La UI es estable cuando no está limpiando eventos y no hay mensajes activos críticos
  return !_clearingEvents && _isEventClearingComplete();
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

// ===== MÉTODOS DE LIMPIEZA DE EVENTOS =====
//
// ESTRATEGIA DE LIMPIEZA DE EVENTOS:
// 
// Problema: Las pantallas Nextion pueden acumular eventos táctiles en su buffer interno
// mientras el ESP32 está procesando otras tareas. Esto puede causar comportamientos 
// inesperados cuando el usuario toca botones rápidamente o durante transiciones.
//
// Solución: Antes de mostrar una nueva pantalla crítica, ejecutamos una secuencia de 
// limpieza que:
// 1. Cambia temporalmente a una página neutra (WELCOME)
// 2. Procesa y descarta todos los eventos pendientes durante un período de tiempo
// 3. Luego cambia a la pantalla objetivo con garantía de estado limpio
//
// Uso recomendado:
// - Para transiciones normales: usar métodos show...() directos
// - Para transiciones críticas: usar métodos safeTransitionTo...()
// - Para emergencias: usar safeTransitionToError() (con timeout más corto)
//
// EJEMPLO DE USO DESDE PROGRAM CONTROLLER:
// 
// // Transición normal (rápida)
// UIController.showSelectionScreen(programa);
//
// // Transición crítica (con limpieza garantizada)
// UIController.safeTransitionToExecution(programa, fase, nivel, temp, rot);
//
// // Verificar estabilidad antes de procesar eventos críticos
// if (UIController.isUIStable()) {
//   // Proceder con lógica crítica
// }
//
// ===================================================================

void UIControllerClass::clearPendingEvents() {
  // Limpiar eventos locales pendientes
  _userActionPending = false;
  _lastUserAction = "";
  
  // Procesar y descartar eventos pendientes en el hardware
  while (Hardware.nextionCheckForEvents()) {
    Hardware.nextionGetLastEvent(); // Descartar evento
  }
  
  Serial.println("Eventos de UI limpiados");
}

void UIControllerClass::_clearPendingEvents() {
  // Método interno para iniciar proceso de limpieza
  _clearingEvents = true;
  _clearingStartTime = millis();
  
  // Cambiar temporalmente a una página neutra para limpiar eventos
  Hardware.nextionSetPage(NEXTION_PAGE_WELCOME);
  
  // Limpiar eventos locales
  clearPendingEvents();
  
  Serial.println("Iniciando limpieza profunda de eventos...");
}

bool UIControllerClass::_isEventClearingComplete() {
  // Verificar si el proceso de limpieza ha terminado
  if (!_clearingEvents) {
    return true; // No estamos limpiando
  }
  
  // Verificar timeout
  if (millis() - _clearingStartTime >= EVENT_CLEAR_TIMEOUT) {
    _clearingEvents = false;
    Serial.println("Limpieza de eventos completada");
    return true;
  }
  
  return false;
}

// ===== MÉTODOS DE TRANSICIÓN SEGURA =====

void UIControllerClass::safeTransitionToSelection(uint8_t programa) {
  // Iniciar limpieza de eventos
  _clearPendingEvents();
  
  // Procesar eventos durante el período de limpieza
  unsigned long startTime = millis();
  while (millis() - startTime < EVENT_CLEAR_TIMEOUT) {
    if (Hardware.nextionCheckForEvents()) {
      Hardware.nextionGetLastEvent(); // Descartar evento
    }
    delay(1); // Breve pausa para permitir que lleguen eventos
  }
  
  // Ahora mostrar la pantalla objetivo con eventos limpios
  showSelectionScreen(programa);
  
  Serial.println("Transición segura a pantalla de selección completada");
}

void UIControllerClass::safeTransitionToExecution(uint8_t programa, uint8_t fase, uint8_t nivelAgua, uint8_t temperatura, uint8_t rotacion) {
  // Iniciar limpieza de eventos
  _clearPendingEvents();
  
  // Procesar eventos durante el período de limpieza
  unsigned long startTime = millis();
  while (millis() - startTime < EVENT_CLEAR_TIMEOUT) {
    if (Hardware.nextionCheckForEvents()) {
      Hardware.nextionGetLastEvent(); // Descartar evento
    }
    delay(1); // Breve pausa para permitir que lleguen eventos
  }
  
  // Ahora mostrar la pantalla objetivo con eventos limpios
  showExecutionScreen(programa, fase, nivelAgua, temperatura, rotacion);
  
  Serial.println("Transición segura a pantalla de ejecución completada");
}

void UIControllerClass::safeTransitionToEdit(uint8_t programa, uint8_t fase, uint8_t numeroVariable, uint8_t valor) {
  // Iniciar limpieza de eventos
  _clearPendingEvents();
  
  // Procesar eventos durante el período de limpieza
  unsigned long startTime = millis();
  while (millis() - startTime < EVENT_CLEAR_TIMEOUT) {
    if (Hardware.nextionCheckForEvents()) {
      Hardware.nextionGetLastEvent(); // Descartar evento
    }
    delay(1); // Breve pausa para permitir que lleguen eventos
  }
  
  // Ahora mostrar la pantalla objetivo con eventos limpios
  showEditScreen(programa, fase, numeroVariable, valor);
  
  Serial.println("Transición segura a pantalla de edición completada");
}

void UIControllerClass::safeTransitionToError(uint8_t errorCode, const String& errorMessage) {
  // Para errores, la limpieza debe ser inmediata y prioritaria
  _clearPendingEvents();
  
  // Breve limpieza de eventos críticos
  unsigned long startTime = millis();
  while (millis() - startTime < 50) { // Timeout más corto para errores
    if (Hardware.nextionCheckForEvents()) {
      Hardware.nextionGetLastEvent(); // Descartar evento
    }
    delay(1);
  }
  
  // Mostrar pantalla de error inmediatamente
  showErrorScreen(errorCode, errorMessage);
  
  Serial.println("Transición segura a pantalla de error completada");
}