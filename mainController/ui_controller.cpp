// ui_controller.cpp
#include "ui_controller.h"
#include "Arduino.h"
#include "actuators.h"
#include "program_controller.h"
#include "sensors.h"
#include "storage.h"
#include <stdio.h>

// Definición de la instancia global
UIControllerClass UIController;

// === CONFIGURACIÓN SEGÚN DOCUMENTO DEL CLIENTE ===
// Programa 22 (P22): Agua Caliente - 3 fases + centrifugado
// Programa 23 (P23): Agua Fría - 3 fases + centrifugado
// Programa 24 (P24): Multi-ciclo configurable - 3 fases, sin centrifugado

uint8_t NivelAgua[3][4] = {
    {3, 4, 2, 1}, // P22:
    {3, 4, 2, 1}, // P23:
    {2, 3, 2, 0}  // P24:
};

uint8_t RotacionTam[3][4] = {
    {0, 2, 0, 3}, // P22: Sin rotación en llenado/drenaje, media en lavado,
                  // rápida en centrifugado
    {0, 2, 0, 3}, // P23: Igual que P22
    {0, 2, 0, 0}  // P24: Sin centrifugado
};

uint8_t TemperaturaLim[3][4] = {
    {60, 65, 40,
     25}, // P22: Caliente para llenado/lavado, templado para drenaje
    {25, 25, 25, 25}, // P23: Temperatura ambiente (agua fría)
    {45, 50, 30, 25}  // P24: Configurable (por defecto tibio)
};

uint8_t TemporizadorLim[3][4] = {
    {8, 15, 5,
     4}, // P22: Llenado 8min, Lavado 15min, Drenaje 5min, Centrifugado 4min
    {8, 15, 5, 4}, // P23: Mismos tiempos que P22
    {6, 12, 4, 0}  // P24: Ciclo más corto, sin centrifugado
};

// Fases según documento: 1=Llenado, 2=Lavado, 3=Centrifugado, 4=Drenaje
uint8_t FasesPrograma[3][4] = {
    {1, 2, 3, 4}, // P22: Secuencia completa con centrifugado
    {1, 2, 3, 4}, // P23: Secuencia completa con centrifugado
    {1, 2, 3, 0}  // P24: Solo 3 fases, sin centrifugado
};

// Centrifugado opcional en todos los programas (según configuración)
uint8_t CentrifugadoPrograma[3][4] = {
    {0, 0, 0,
     1}, // P22: Centrifugado configurable (por defecto habilitado al final)
    {0, 0, 0,
     1}, // P23: Centrifugado configurable (por defecto habilitado al final)
    {0, 0, 0, 0} // P24: Centrifugado configurable (por defecto deshabilitado)
};

// Tipo de agua según especificaciones del cliente
uint8_t TipoAguaPrograma[3][4] = {
    {1, 1, 1, 0}, // P22: Agua caliente en todas las fases activas
    {0, 0, 0, 0}, // P23: Agua fría en todas las fases
    {1, 1, 0, 0}  // P24: Configurable (defecto: caliente para llenado/lavado)
};

void UIControllerClass::init() {
  // Obtener referencia a los datos de programa
  _nivelAgua = NivelAgua;
  _rotacionTam = RotacionTam;
  _temperaturaLim = TemperaturaLim;
  _temporizadorLim = TemporizadorLim;
  _fasesPrograma = FasesPrograma;
  _centrifugadoPorTanda = CentrifugadoPrograma;
  _tipoAguaPrograma = TipoAguaPrograma;

  _userActionPending = false;
  _messageActive = false;
  _currentPage = NEXTION_PAGE_WELCOME; // Inicializar página actual

  // Inicializar variables para limpieza de eventos
  _clearingEvents = false;
  _clearingStartTime = 0;

  // === INICIALIZAR VARIABLES DE EDICIÓN ===
  _programaEnEdicion = 0;
  _faseEnEdicion = 0;
  _parametroActual = PARAM_NIVEL;
  _modoEdicionActivo = false;
  _editTimeoutStart = 0;
  _parameterSaved = false;

  // Inicializar valores temporales (expandido para 7 parámetros)
  for (int i = 0; i < 7; i++) {
    _valoresTemporales[i] = 0; // Array para: [nivel, temp, tiempo, rotacion,
                               // tanda, centrifugado, tipoAgua]
  }

  Serial.println(
      "UI Controller inicializado con sistema de limpieza de eventos");
}

/// @brief
/// Muestra la pantalla de bienvenida.
/// Esta pantalla se muestra al iniciar el sistema y presenta información básica
/// sobre el controlador.
void UIControllerClass::showWelcomeScreen() {
  // Cambiar a la página de bienvenida
  Hardware.nextionSetPage(NEXTION_PAGE_WELCOME);
  _currentPage = NEXTION_PAGE_WELCOME; // Actualizar página actual
  // delay(100);                          // Pausa breve para asegurar cambio de
  // página

  // Establecer textos de bienvenida usando los componentes correctos de la
  // documentación Serial.println("Enviando comando para título...");
  Hardware.nextionSetText(NEXTION_COMP_TITULO, "iTrebolsoft");

  // Serial.println("Enviando comando para subtítulo...");
  Hardware.nextionSetText(NEXTION_COMP_SUBTITULO,
                          "Controlador de Lavadora Industrial");

  // Serial.println("Enviando comando para contacto...");
  Hardware.nextionSetText(NEXTION_COMP_CONTACTO, "958970967");
}

/// @brief
/// Muestra la pantalla de selección de programa.
/// Esta pantalla permite al usuario seleccionar entre los programas
/// disponibles.
/// @note
/// Asegúrate de que los componentes de la pantalla Nextion estén correctamente
/// configurados con los IDs especificados.
/// @warning
/// Este método asume que los programas están numerados del 0 al 2.
/// Si se intenta seleccionar un programa fuera de este rango, no se realizará
/// ninguna acción.
/// @param programa
/// El número del programa a mostrar (0, 1 o 2).
/// Si se pasa 0, se mostrará la pantalla de selección sin resaltar ningún
/// programa.
void UIControllerClass::showSelectionScreen(uint8_t programa) {
  Utils.debug("Programa seleccionado: " + String(programa + 22));

  // Cambiar de página SOLO si no estamos ya en la página de selección
  if (_currentPage != NEXTION_PAGE_SELECTION) {
    Hardware.nextionSetPage(NEXTION_PAGE_SELECTION);
    _currentPage = NEXTION_PAGE_SELECTION;
    Utils.debug("Cambiando a página de selección");
  } else {
    Utils.debug("Ya en página de selección - solo actualizando componentes");
  }

  // Actualizar información del programa seleccionado (solo componentes
  // individuales)
  _updateProgramInfo(programa);

  // Actualizar texto del botón START según estado de puerta
  updateStartButtonText();

  // Resaltar el botón del programa seleccionado (solo componentes individuales)
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM1, (programa == 0) ? 1 : 0);
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM2, (programa == 1) ? 1 : 0);
  Hardware.nextionSetValue(NEXTION_COMP_BTN_PROGRAM3, (programa == 2) ? 1 : 0);
}

/// @brief
/// Muestra la pantalla de ejecución del programa.
/// Esta pantalla muestra el estado actual del programa en ejecución, incluyendo
/// fase, tiempo, nivel de agua, temperatura y rotación.
/// @details
/// La pantalla de ejecución se actualiza con los valores actuales del programa
/// y muestra un temporizador que indica el tiempo transcurrido en la fase
/// actual. También se actualizan los indicadores de nivel de agua, temperatura
/// y rotación.
/// @param programa
/// El número del programa en ejecución (0, 1 o 2).
/// @param fase
/// La fase actual del programa (0 a 3).
/// @param nivelAgua
/// El nivel de agua actual (0 a 4).
/// @param temperatura
/// La temperatura actual en grados Celsius (0 a 100).
/// @param rotacion
/// La rotación actual del tambor en RPM (0 a 300).
void UIControllerClass::showExecutionScreen(uint8_t programa, uint8_t fase,
                                            uint8_t nivelAgua,
                                            uint8_t temperatura,
                                            uint8_t rotacion, uint8_t tanda,
                                            bool preserveTime,
                                            uint8_t preservedMinutes,
                                            uint8_t preservedSeconds) {
  // Cambiar a la página de ejecución
  Hardware.nextionSetPage(NEXTION_PAGE_EXECUTION);
  _currentPage = NEXTION_PAGE_EXECUTION; // Actualizar página actual

  // Mostrar información del programa usando los componentes correctos de la
  // documentación
  Hardware.nextionSetText(NEXTION_COMP_PROG_EJECUCION,
                          "P" + String(programa + 22));

  // Usar updatePhase para mostrar nombre descriptivo de la fase
  updatePhase(fase); // indica fase actual
  
  // Actualizar tanda en ejecución (solo para P24, P22/P23 siempre muestran 1)
  if (programa == 2) { // P24
    Hardware.nextionSetText(NEXTION_COMP_TANDA_EJECUCION, String(tanda + 1)); // Mostrar 1-4
  } else { // P22/P23
    Hardware.nextionSetText(NEXTION_COMP_TANDA_EJECUCION, "1"); // Siempre tanda 1
  }

  // NUNCA resetear tiempo a "00:00" en página de ejecución
  // El tiempo se maneja externamente según el contexto

  // Actualizar indicadores usando los componentes existentes que funcionan
  // correctamente
  updateWaterLevel(
      Sensors.getCurrentWaterLevel()); // Usar nivel real del sensor
  updateTemperature(
      Sensors.getCurrentTemperature()); // Usar temperatura real del sensor
  updateRotation(
      Actuators.getCurrentRotationLevel()); // Usar rotación real del actuator

  Serial.println("Mostrando pantalla de ejecución de programa");
}

/// @brief
/// Muestra la pantalla de edición del programa.
/// Esta pantalla permite al usuario editar los parámetros del programa
/// seleccionado.
/// @param programa
/// El número del programa a editar (0, 1 o 2).
/// @param fase
/// La fase del programa a editar (0 a 3).
/// @note
/// Asegúrate de que los componentes de la pantalla Nextion estén correctamente
/// configurados con los IDs especificados.
void UIControllerClass::showEditScreen(uint8_t programa, uint8_t fase) {
  // Debug prints eliminados para mejor rendimiento

  // Inicializar modo de edición
  initEditMode(programa, fase);

  // Cambiar a la página de edición
  Hardware.nextionSetPage(NEXTION_PAGE_EDIT);
  _currentPage = NEXTION_PAGE_EDIT; // Actualizar página actual

  // ELIMINADO: _updateProgramInfo(programa) para evitar doble actualización
  // Los mismos componentes se actualizan en updateEditDisplay() ->
  // updateEditPanel(UPDATE_FULL)

  // Actualizar toda la pantalla con los valores iniciales
  updateEditDisplay();

  // Ejecutar diagnóstico del estado de edición
  // diagnosticarEstadoEdicion();

  // Debug prints eliminados para mejor rendimiento
}

void UIControllerClass::showErrorScreen(uint8_t errorCode,
                                        const String &errorMessage) {
  // Cambiar a la página de error
  Hardware.nextionSetPage(NEXTION_PAGE_ERROR);
  _currentPage = NEXTION_PAGE_ERROR; // Actualizar página actual

  // Mostrar código de error
  Hardware.nextionSetText("txtCodigo", "ERROR " + String(errorCode));

  // Mostrar mensaje de error si se proporciona
  if (errorMessage.length() > 0) {
    Hardware.nextionSetText("txtMensaje", errorMessage);
  } else {
    // Mensaje por defecto basado en código
    switch (errorCode) {
    case 400:
      Hardware.nextionSetText("txtMensaje", "Error de sistema");
      break;
    case 401:
      Hardware.nextionSetText("txtMensaje", "Error de temperatura");
      break;
    case 402:
      Hardware.nextionSetText("txtMensaje", "Error de presión");
      break;
    default:
      Hardware.nextionSetText("txtMensaje", "Error desconocido");
      break;
    }
  }

  // Activar indicador de alerta
  Hardware.nextionSendCommand("alerta.en=1");

  Serial.println("Mostrando pantalla de error");
}

void UIControllerClass::showEmergencyScreen() {
  // Cambiar a la página de emergencia
  Hardware.nextionSetPage(NEXTION_PAGE_EMERGENCY);
  _currentPage = NEXTION_PAGE_EMERGENCY; // Actualizar página actual

  // Mostrar mensaje de emergencia
  Hardware.nextionSetText("txtEmergencia", "PARADA DE EMERGENCIA");
  Hardware.nextionSetText("txtMensajeEmerg", "Sistema detenido por seguridad");

  // Activar indicador visual de emergencia
  // Hardware.nextionSendCommand("alarm.en=1");

  // Reproducir sonido de alarma
  // playSound(2);  // Código 2 para sonido de alarma

  Serial.println("EMERGENCIA: Sistema detenido");
}

/// @brief
/// Actualiza el tiempo transcurrido en la pantalla de ejecución.
/// Este método formatea el tiempo en minutos y segundos y lo muestra en el
/// componente de tiempo de ejecución.
/// @param minutos
/// @param segundos
void UIControllerClass::updateTime(uint8_t minutos, uint8_t segundos) {
  char timeBuffer[6];
  _formatTimeDisplay(minutos, segundos, timeBuffer);
  Hardware.nextionSetText(NEXTION_COMP_TIEMPO_EJECUCION, timeBuffer);
}

void UIControllerClass::_formatTimeDisplay(uint8_t minutos, uint8_t segundos,
                                           char *buffer) {
  sprintf(buffer, "%02d:%02d", minutos, segundos);
}

void UIControllerClass::updateTemperature(float temperatura) {
  // Actualizar texto de temperatura con 1 decimal
  Hardware.nextionSetText(NEXTION_COMP_TEMP_EJECUCION,
                          String(temperatura, 1) + "°C");

  // Actualizar barra de temperatura (mapear 0-100°C al rango 0-100)
  uint8_t barValue = (uint8_t)constrain(temperatura, 0, 100);
  Hardware.nextionSetValue(NEXTION_COMP_BARRA_TEMP_EJECUCION, barValue);
}

void UIControllerClass::updateWaterLevel(uint8_t nivel) {
  // Actualizar texto de nivel en pantalla de ejecución
  Hardware.nextionSetText(NEXTION_COMP_NIVEL_EJECUCION, String(nivel));

  // Actualizar gauge vertical de nivel (barra vertical)
  // Mapear nivel del sensor (0-4) al rango del gauge en Nextion (0-100)
  // Esto hace que cada nivel del sensor sea muy visible en la barra
  uint8_t barValue = nivel * 25; // 0->0, 1->25, 2->50, 3->75, 4->100
  Hardware.nextionSetValue(NEXTION_COMP_BARRA_NIVEL_EJECUCION, barValue);
}

void UIControllerClass::updateRotation(uint8_t rotacion) {
  // Actualizar texto de velocidad de rotación en pantalla de ejecución
  Hardware.nextionSetText(NEXTION_COMP_VELOCIDAD_EJECUCION, String(rotacion));

  // Actualizar barra de velocidad
  // Mapear nivel de rotación (0-4) al rango de la barra en Nextion (0-100)
  uint8_t barValue = rotacion * 25; // 0->0, 1->25, 2->50, 3->75, 4->100
  Hardware.nextionSetValue(NEXTION_COMP_BARRA_VELOC_EJECUCION, barValue);
}

void UIControllerClass::updatePhase(uint8_t fase) {
  if (_currentPage != NEXTION_PAGE_EXECUTION)
    return;
    
  // Mostrar nombre descriptivo de la fase según el diseño del cliente
  String faseTexto;
  switch (fase) {
  case 0:
    faseTexto = "Llenado";
    break;
  case 1:
    faseTexto = "Lavado";
    break;
  case 2:
    faseTexto = "Centrifugado";
    break;
  case 3:
    faseTexto = "Drenaje";
    break;
  case 4:
    faseTexto = "Enfriando";
    break;
  default:
    faseTexto = "No especificado";
    break;
  }

  Hardware.nextionSetText(NEXTION_COMP_FASE_EJECUCION, faseTexto);
}

void UIControllerClass::processEvents() {
  // Verificar si estamos en proceso de limpieza de eventos
  if (_clearingEvents) {
    // Durante la limpieza, descartar todos los eventos sin procesarlos
    while (Hardware.nextionCheckForEvents()) {
      // Los eventos se procesan y descartan automáticamente en Hardware
    }

    // Verificar si la limpieza ha terminado
    _isEventClearingComplete();
    return; // No procesar eventos normales durante la limpieza
  }

  // === VERIFICAR TIMEOUT DE EDICIÓN ===
  if (_modoEdicionActivo) {
    _checkEditTimeout();
  }

  // === MODO ULTRA-RÁPIDO PARA PÁGINA DE EDICIÓN ===
  if (_currentPage == NEXTION_PAGE_EDIT) {
    // POLLING AGRESIVO - leer múltiples eventos por ciclo para máxima respuesta
    for (int i = 0; i < 3;
         i++) { // Hasta 3 eventos por llamada a processEvents()
      if (Hardware.nextionCheckForEvents()) {
        _handleTouchEvent();
      }
    }
    return; // Salir inmediatamente para máxima velocidad en edición
  }

  // === MONITOREO PERIÓDICO DEL ESTADO DE PUERTA (SOLO OTRAS PÁGINAS) ===
  static unsigned long lastButtonUpdate = 0;
  if (_currentPage == NEXTION_PAGE_SELECTION &&
      millis() - lastButtonUpdate > 50) {
    updateStartButtonText();
    lastButtonUpdate = millis();
  }

  // Verificar si hay eventos de la pantalla Nextion
  if (Hardware.nextionCheckForEvents()) {
    // Procesar eventos táctiles directamente
    _handleTouchEvent();
  }

  // Actualizar mensajes temporales
  if (_messageActive && (millis() - _messageTimestamp > _messageDuration)) {
    _messageActive = false;
    // Limpiar mensaje
    Hardware.nextionSetText(NEXTION_COMP_MSG, "");
  }
}

/**
 * @brief Procesar eventos táctiles directamente desde Hardware
 */
void UIControllerClass::_handleTouchEvent() {
  // Verificar si hay un evento táctil válido
  if (!Hardware.hasValidTouchEvent()) {
    return;
  }

  // Obtener datos del evento táctil
  uint8_t pageId = Hardware.getTouchEventPage();
  uint8_t componentId = Hardware.getTouchEventComponent();
  uint8_t eventType = Hardware.getTouchEventType();

  // Solo procesar eventos de presionado (tipo 1)
  if (eventType != 1) {
    return; // Sin Serial.println para máxima velocidad
  }

  // OPTIMIZACIÓN ESPECÍFICA PARA PÁGINA DE EDICIÓN
  if (pageId == NEXTION_PAGE_EDIT) {
    // Llamada directa sin switch para máxima velocidad
    handleEditPageEvent(componentId);
    return;
  }

  // Procesar otras páginas normalmente
  switch (pageId) {
  case NEXTION_PAGE_SELECTION:
    _handleSelectionPageEvent(componentId);
    break;

  case NEXTION_PAGE_EXECUTION:
    _handleExecutionPageEvent(componentId);
    break;

  default:
    // Sin Serial.println para máxima velocidad
    break;
  }
}

/**
 * @brief Manejar eventos de la página de selección
 */
void UIControllerClass::_handleSelectionPageEvent(uint8_t componentId) {
  if (componentId == NEXTION_ID_BTN_PROGRAM1) {
    _lastUserAction = "PROGRAM_1";
    _userActionPending = true;
    UIController.showMessage("Programa P22 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el
    // programa
  } else if (componentId == NEXTION_ID_BTN_PROGRAM2) {
    _lastUserAction = "PROGRAM_2";
    _userActionPending = true;
    UIController.showMessage("Programa P23 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el
    // programa
  } else if (componentId == NEXTION_ID_BTN_PROGRAM3) {
    _lastUserAction = "PROGRAM_3";
    _userActionPending = true;
    UIController.showMessage("Programa P24 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el
    // programa
  } else if (componentId == NEXTION_ID_BTN_START) {
    // Verificar estado de puerta para determinar acción
    if (!Sensors.isDoorClosed()) {
      // Puerta abierta - botón dice "CERRAR" - activar bloqueo de puerta
      _lastUserAction = "CLOSE_DOOR";
      _userActionPending = true;
      showMessage("Puerta cerrada", 2000);
    } else {
      // Puerta cerrada - botón dice "INICIAR" - iniciar programa
      _lastUserAction = "START";
      _userActionPending = true;
      showMessage("Programa iniciado", 2000);
    }
  } else if (componentId == NEXTION_ID_BTN_EDIT) {
    _lastUserAction = "EDIT";
    _userActionPending = true;
    showMessage("Modo edición activado", 2000);
  } else {
    showMessage("Componente no reconocido", 2000);
  }
}

/**
 * @brief Manejar eventos de la página de ejecución
 */
void UIControllerClass::_handleExecutionPageEvent(uint8_t componentId) {
  if (componentId == NEXTION_ID_BTN_PARAR) {
    _lastUserAction = "STOP";
    _userActionPending = true;
    showMessage("Programa detenido", 2000);
  } else if (componentId == NEXTION_ID_BTN_PAUSAR) {
    _lastUserAction = "PAUSE";
    _userActionPending = true;
    showMessage("Programa pausado", 2000);
  } else {
    Serial.println("   ❓ ComponentID no reconocido: " + String(componentId));
  }
}

/// Este método actualiza los componentes de la pantalla Nextion con los valores
/// del programa seleccionado.
/// @details
/// Este método toma el número del programa (0, 1 o 2) y actualiza los
/// componentes de la pantalla Nextion con los valores correspondientes de nivel
/// de agua, temperatura, tiempo y rotación. También maneja la visualización de
/// información adicional para el programa P24 que tiene múltiples fases.
/// @param programa
/// El número del programa a mostrar (0, 1 o 2).
/// Si se pasa un número fuera de este rango, no se realizará ninguna acción.
void UIControllerClass::_updateProgramInfo(uint8_t programa) {
  // Actualizar información mostrada para el programa seleccionado
  Hardware.nextionSetText(NEXTION_COMP_PROGRAMA_SEL,
                          "P" + String(programa + 22));

  // Cargar valores desde Storage
  // NOTA: Para P22/P23 (valores únicos), todas las fases devuelven el mismo
  // valor Para P24 (matriz), se usa la fase 0 como representativa para la
  // pantalla de selección
  uint8_t nivel = Storage.loadWaterLevel(programa, 0, 0);
  uint8_t temp = Storage.loadTemperature(programa, 0, 0);
  uint8_t tiempo = Storage.loadTime(programa, 0);
  uint8_t rotacion = Storage.loadRotation(programa, 0);
  uint8_t tanda = Storage.loadPhaseType(programa, 0, 0);
  uint8_t centrifugado =
      Storage.loadCentrifugado(programa, 0); // Tanda 0 para todos
  uint8_t tipoAgua = Storage.loadTipoAgua(programa, 0, 0);

  // Actualizar valores del panel derecho (sin parpadeo gracias a campos vacíos
  // en HMI)
  Hardware.nextionSetText(NEXTION_COMP_SET_NIVEL, String(nivel));
  Hardware.nextionSetText(NEXTION_COMP_SET_TEMP, String(temp) + "°C");
  Hardware.nextionSetText(NEXTION_COMP_SET_TIEMPO, String(tiempo) + " min");
  Hardware.nextionSetText(NEXTION_COMP_SET_ROTACION, String(rotacion) + " RPM");
  Hardware.nextionSetText(NEXTION_COMP_SET_FASE, String(tanda));
  Hardware.nextionSetText(NEXTION_COMP_SET_CENTRIF,
                          centrifugado ? "SI" : "NO");
  Hardware.nextionSetText(NEXTION_COMP_SET_AGUA,
                          tipoAgua ? "Caliente" : "Fria");

  // Si es el programa P24 (índice 2), mostrar información adicional de
  // múltiples fases
  if (programa == 2) {
    // Mostrar información de todas las fases
    String fasesInfo = "Fases: ";
    for (uint8_t i = 0; i < 4; i++) {
      uint8_t tiempoFase =
          Storage.loadTime(programa, i); // Cargar tiempo de cada fase
      fasesInfo += String(i + 1) + ":" + String(tiempoFase) + "m ";
    }
    Hardware.nextionSetText(NEXTION_COMP_MSG, fasesInfo);
  } else {
    // Para P22 y P23, mostrar que usan configuración única
    Hardware.nextionSetText(NEXTION_COMP_MSG, "Configuracion unica");
  }

  Serial.println("Información del programa P" + String(programa + 22) +
                 " actualizada desde Storage");
}

/**
 * @brief Actualiza SOLO el panel derecho con información del programa (sin
 * cambiar página ni botones)
 * @param programa Programa del cual mostrar información (0-2)
 */
void UIControllerClass::updateProgramPanel(uint8_t programa) {
  // Cargar valores desde Storage (igual que _updateProgramInfo pero más rápido)
  uint8_t nivel = Storage.loadWaterLevel(programa, 0, 0);
  uint8_t temp = Storage.loadTemperature(programa, 0, 0);
  uint8_t tiempo = Storage.loadTime(programa, 0);
  uint8_t rotacion = Storage.loadRotation(programa, 0);
  // uint8_t tanda = Storage.loadPhaseType(programa, 0, 0);
  uint8_t centrifugado = Storage.loadCentrifugado(programa, 0);
  uint8_t tipoAgua = Storage.loadTipoAgua(programa, 0, 0);

  // Actualizar SOLO el panel derecho (sin tocar botones ni cambiar páginas)
  Hardware.nextionSetText(NEXTION_COMP_SET_NIVEL, String(nivel));
  Hardware.nextionSetText(NEXTION_COMP_SET_TEMP, String(temp) + "°C");
  Hardware.nextionSetText(NEXTION_COMP_SET_TIEMPO, String(tiempo) + " min");
  Hardware.nextionSetText(NEXTION_COMP_SET_ROTACION, String(rotacion) + " RPM");
  // Hardware.nextionSetText(NEXTION_COMP_SET_FASE, String(tanda));
  Hardware.nextionSetText(NEXTION_COMP_SET_CENTRIF,
                          centrifugado ? "SI" : "NO");
  Hardware.nextionSetText(NEXTION_COMP_SET_AGUA,
                          tipoAgua ? "Caliente" : "Fría");
}

bool UIControllerClass::hasUserAction() { return _userActionPending; }

String UIControllerClass::getUserAction() {
  _userActionPending = false;
  return _lastUserAction;
}

bool UIControllerClass::isUIStable() {
  // La UI es estable cuando no está limpiando eventos y no hay mensajes activos
  // críticos
  return !_clearingEvents && _isEventClearingComplete();
}

/// @brief
/// Muestra un mensaje temporal en la pantalla.
/// Este método muestra un mensaje en la pantalla Nextion que desaparece después
/// de un tiempo.
/// @param message
/// El mensaje a mostrar. Si el mensaje es vacío, no se mostrará nada.
/// @param duration
/// La duración en milisegundos que el mensaje permanecerá visible. Por defecto
/// es 2000 ms (2 segundos). Si se pasa 0, el mensaje permanecerá visible hasta
/// que se oculte manualmente.
void UIControllerClass::showMessage(const String &message, uint16_t duration) {
  // Mostrar un mensaje temporal en la pantalla
  Hardware.nextionSetText(NEXTION_COMP_MSG, message);

  _messageActive = true;
  _messageTimestamp = millis();
  _messageDuration = duration;
}

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

  // No cambiar de página durante la limpieza - mantener la página actual
  // Hardware.nextionSetPage(NEXTION_PAGE_WELCOME);  // ELIMINADO - causaba
  // problemas

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

  // Forzar actualización de información del programa para reflejar cambios
  _updateProgramInfo(programa);
}

void UIControllerClass::safeTransitionToExecution(uint8_t programa,
                                                  uint8_t fase,
                                                  uint8_t nivelAgua,
                                                  uint8_t temperatura,
                                                  uint8_t rotacion,
                                                  uint8_t tanda) {
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
  showExecutionScreen(programa, fase, nivelAgua, temperatura, rotacion, tanda);
  _updateProgramInfo(programa); // Actualizar información del programa

  Serial.println("Transición segura a pantalla de ejecución completada");
}

void UIControllerClass::safeTransitionToEdit(uint8_t programa, uint8_t fase) {
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
  showEditScreen(programa, fase);

  Serial.println("Transición segura a pantalla de edición completada");
}

void UIControllerClass::safeTransitionToError(uint8_t errorCode,
                                              const String &errorMessage) {
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

/**
 * @brief Inicializar el modo de edición con los parámetros del programa y fase
 * especificados
 * @param programa Número de programa (0, 1, 2)
 * @param fase Número de fase (0-3)
 */
void UIControllerClass::initEditMode(uint8_t programa, uint8_t fase) {
  _programaEnEdicion = programa;
  _faseEnEdicion = fase;
  _parametroActual = PARAM_NIVEL; // Comenzar con el primer parámetro
  _modoEdicionActivo = true;

  // Cargar valores actuales desde storage
  _loadParametersFromStorage(programa, fase);
  showMessage("Modo edición activado para P" + String(programa + 22) + " F" +
                  String(fase + 1),
              2000);
}

/// @brief
/// Cargar los valores de los parámetros del programa y fase especificados desde
/// Storage
void UIControllerClass::updateEditDisplay() {
  if (!_modoEdicionActivo)
    return;

  char buffer[20];

  // Actualizar programa y fase en edición
  generarTextoPrograma(_programaEnEdicion, buffer, sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_SET_PROG, buffer);

  Hardware.nextionSetText(NEXTION_COMP_PROGRAMA_SEL,
                          "P" + String(_programaEnEdicion + 22));

  if (_programaEnEdicion == 0 || _programaEnEdicion == 1) {
    // P22 y P23 - tanda y agua deshabilitados, centrifugado habilitado
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_FASE) +
                                ",0"); // Deshabilitar touch
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_FASE) +
                                ".pco=33840"); // Color gris
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_AGUA) +
                                ",0"); // Deshabilitar touch
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_AGUA) +
                                ".pco=33840"); // Color gris
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_CENTRIF) +
                                ",1"); // Habilitar touch centrifugado
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_CENTRIF) +
                                ".pco=65535"); // Color normal centrifugado
  } else {
    // P24 - botones habilitados y colores normales
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_FASE) +
                                ",0"); // Deshabilitar touch
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_FASE) +
                                ".pco=33840"); // Color gris
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_AGUA) +
                                ",1"); // Habilitar touch
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_AGUA) +
                                ".pco=65535"); // Color normal
    Hardware.nextionSendCommand("tsw " + String(NEXTION_COMP_SET_CENTRIF) +
                                ",1"); // Habilitar touch centrifugado
    Hardware.nextionSendCommand(String(NEXTION_COMP_SET_CENTRIF) +
                                ".pco=65535"); // Color normal centrifugado
    // ELIMINADO: .bco para mantener fondo por defecto del HMI (azul oscuro)
  }

  // Actualizar parámetro actual y panel derecho DESPUÉS de configurar estilos
  updateParameterDisplay();
  updateEditPanel(UPDATE_FULL); // Optimizado como página de selección

  // Configurar botones de tanda según el programa y tanda actual
  updateTandaButtons(ProgramController.getCurrentEditingTanda());
}

/// @brief
/// Cargar los valores de los parámetros del programa y fase especificados desde
/// Storage
void UIControllerClass::updateParameterDisplay() {
  if (!_modoEdicionActivo)
    return;

  char buffer[20];

  // Actualizar nombre del parámetro actual
  const char *textoParam = obtenerTextoParametro(_parametroActual);
  Hardware.nextionSetText(NEXTION_COMP_PARAM_EDITAR, textoParam);

  // Actualizar valor del parámetro actual con formato
  formatearParametroConUnidad(_parametroActual,
                              _valoresTemporales[_parametroActual], buffer,
                              sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_PARAM_VALOR_EDITAR, buffer);

  Serial.println("Parámetro actual actualizado: " + String(textoParam) + " = " +
                 String(buffer));
}

/**
 * @brief Actualiza el panel de edición con diferentes niveles de optimización
 * @param updateMode Tipo de actualización:
 *   - UPDATE_FULL: Actualiza todo el panel derecho
 *   - UPDATE_SINGLE: Actualiza solo un parámetro específico
 *   - UPDATE_FAST: Actualiza parámetro principal + componente del panel
 *   - UPDATE_INSTANT: Solo valor principal (máxima velocidad)
 * @param parametro Parámetro específico (solo para UPDATE_SINGLE, UPDATE_FAST, UPDATE_INSTANT)
 */
void UIControllerClass::updateEditPanel(UpdateMode updateMode, uint8_t parametro) {
  if (!_modoEdicionActivo) return;
  
  char buffer[20];
  
  switch (updateMode) {
    case UPDATE_FULL:
      // Actualizar todos los parámetros del panel derecho
      _updateAllPanelParameters();
      Serial.println("Panel derecho actualizado (6 parámetros)");
      break;
      
    case UPDATE_SINGLE:
      // Actualizar solo un parámetro específico en panel derecho
      _updateSinglePanelParameter(parametro);
      break;
      
    case UPDATE_FAST:
      // Actualizar parámetro principal + componente específico del panel
      _updateMainParameter(parametro);
      _updateSinglePanelParameter(parametro);
      break;
      
    case UPDATE_INSTANT:
      // Solo valor principal - máxima velocidad
      _updateMainParameterInstant(parametro);
      break;
  }
}

/**
 * @brief Actualiza todos los parámetros del panel derecho
 */
void UIControllerClass::_updateAllPanelParameters() {
  char buffer[20];
  
  // Nivel
  snprintf(buffer, sizeof(buffer), "%d", _valoresTemporales[PARAM_NIVEL]);
  Hardware.nextionSetText(NEXTION_COMP_SET_NIVEL, buffer);
  
  // Temperatura
  snprintf(buffer, sizeof(buffer), "%d°C", _valoresTemporales[PARAM_TEMPERATURA]);
  Hardware.nextionSetText(NEXTION_COMP_SET_TEMP, buffer);
  
  // Tiempo
  snprintf(buffer, sizeof(buffer), "%d min", _valoresTemporales[PARAM_TIEMPO]);
  Hardware.nextionSetText(NEXTION_COMP_SET_TIEMPO, buffer);
  
  // Rotación
  snprintf(buffer, sizeof(buffer), "%d", _valoresTemporales[PARAM_ROTACION]);
  Hardware.nextionSetText(NEXTION_COMP_SET_ROTACION, buffer);
  
  // Centrifugado
  formatearParametroConUnidad(PARAM_CENTRIF, _valoresTemporales[PARAM_CENTRIF], buffer, sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_SET_CENTRIF, buffer);
  
  // Tipo de agua
  formatearParametroConUnidad(PARAM_AGUA, _valoresTemporales[PARAM_AGUA], buffer, sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_SET_AGUA, buffer);
}

/**
 * @brief Actualiza un solo parámetro en el panel derecho
 */
void UIControllerClass::_updateSinglePanelParameter(uint8_t parametro) {
  char buffer[20];
  
  switch (parametro) {
    case PARAM_NIVEL:
      snprintf(buffer, sizeof(buffer), "%d", _valoresTemporales[PARAM_NIVEL]);
      Hardware.nextionSetText(NEXTION_COMP_SET_NIVEL, buffer);
      break;
    case PARAM_TEMPERATURA:
      snprintf(buffer, sizeof(buffer), "%d°C", _valoresTemporales[PARAM_TEMPERATURA]);
      Hardware.nextionSetText(NEXTION_COMP_SET_TEMP, buffer);
      break;
    case PARAM_TIEMPO:
      snprintf(buffer, sizeof(buffer), "%d min", _valoresTemporales[PARAM_TIEMPO]);
      Hardware.nextionSetText(NEXTION_COMP_SET_TIEMPO, buffer);
      break;
    case PARAM_ROTACION:
      snprintf(buffer, sizeof(buffer), "%d", _valoresTemporales[PARAM_ROTACION]);
      Hardware.nextionSetText(NEXTION_COMP_SET_ROTACION, buffer);
      break;
    case PARAM_CENTRIF:
      formatearParametroConUnidad(PARAM_CENTRIF, _valoresTemporales[PARAM_CENTRIF], buffer, sizeof(buffer));
      Hardware.nextionSetText(NEXTION_COMP_SET_CENTRIF, buffer);
      break;
    case PARAM_AGUA:
      formatearParametroConUnidad(PARAM_AGUA, _valoresTemporales[PARAM_AGUA], buffer, sizeof(buffer));
      Hardware.nextionSetText(NEXTION_COMP_SET_AGUA, buffer);
      break;
  }
}

/**
 * @brief Actualiza el parámetro principal que se está editando
 */
void UIControllerClass::_updateMainParameter(uint8_t parametro) {
  char buffer[20];
  const char *textoParam = obtenerTextoParametro(parametro);
  
  Hardware.nextionSetText(NEXTION_COMP_PARAM_EDITAR, textoParam);
  formatearParametroConUnidad(parametro, _valoresTemporales[parametro], buffer, sizeof(buffer));
  Hardware.nextionSetText(NEXTION_COMP_PARAM_VALOR_EDITAR, buffer);
}

/**
 * @brief Actualiza solo el valor principal - máxima velocidad
 */
void UIControllerClass::_updateMainParameterInstant(uint8_t parametro) {
  char buffer[20];
  int valor = _valoresTemporales[parametro];
  
  switch (parametro) {
    case PARAM_NIVEL:
      snprintf(buffer, sizeof(buffer), "%d", valor);
      break;
    case PARAM_TEMPERATURA:
      snprintf(buffer, sizeof(buffer), "%d°C", valor);
      break;
    case PARAM_TIEMPO:
      snprintf(buffer, sizeof(buffer), "%d min", valor);
      break;
    case PARAM_ROTACION:
      snprintf(buffer, sizeof(buffer), "%d RPM", valor);
      break;
    case PARAM_FASE:
      snprintf(buffer, sizeof(buffer), "%d", valor);
      break;
    case PARAM_CENTRIF:
      strcpy(buffer, valor ? "SI" : "NO");
      break;
    case PARAM_AGUA:
      strcpy(buffer, valor ? "Caliente" : "Fría");
      break;
    default:
      snprintf(buffer, sizeof(buffer), "%d", valor);
      break;
  }
  
  Hardware.nextionSetText(NEXTION_COMP_PARAM_VALOR_EDITAR, buffer);
}





// ===== MANEJO DE EVENTOS DE EDICIÓN =====

/**
 * @brief Procesar eventos táctiles de la página de edición
 * @param componentId ID del componente que generó el evento
 */
void UIControllerClass::handleEditPageEvent(int componentId) {
  if (!_modoEdicionActivo) {
    Serial.println(
        "⚠️ Evento de edición recibido pero modo edición no está activo");
    return;
  }

  switch (componentId) {
  case NEXTION_ID_BTN_PARAM_MAS:
    handleParameterIncrement();
    break;

  case NEXTION_ID_BTN_PARAM_MENOS:
    handleParameterDecrement();
    break;

  case NEXTION_ID_BTN_PARAM_SIGUIENTE:
    handleNextParameter();
    break;

  case NEXTION_ID_BTN_PARAM_ANTERIOR:
    handlePreviousParameter();
    break;

  case NEXTION_ID_BTN_GUARDAR:
    handleSaveParameters();
    break;

  case NEXTION_ID_BTN_CANCELAR:
    handleCancelEdit();
    break;

  // Nuevos: Selección directa de parámetros
  case NEXTION_ID_PARAM_NIVEL_EDIT:
    selectParameter(PARAM_NIVEL);
    break;

  case NEXTION_ID_PARAM_TEMP_EDIT:
    selectParameter(PARAM_TEMPERATURA);
    break;

  case NEXTION_ID_PARAM_TIEMPO_EDIT:
    selectParameter(PARAM_TIEMPO);
    break;

  case NEXTION_ID_PARAM_ROTAC_EDIT:
    selectParameter(PARAM_ROTACION);
    break;

  case NEXTION_ID_PARAM_FASE_EDIT:
    // DESACTIVADO: Ahora se usan los botones TANDA1-4 para mejor UX
    // selectTanda();
    break;

  // Nuevos botones de tanda para mejor experiencia de usuario
  case NEXTION_ID_BTN_TANDA1:
    selectTandaDirecta(0);
    break;

  case NEXTION_ID_BTN_TANDA2:
    selectTandaDirecta(1);
    break;

  case NEXTION_ID_BTN_TANDA3:
    selectTandaDirecta(2);
    break;

  case NEXTION_ID_BTN_TANDA4:
    selectTandaDirecta(3);
    break;

  case NEXTION_ID_PARAM_CENTRIF_EDIT:
    selectParameter(PARAM_CENTRIF);
    break;

  case NEXTION_ID_PARAM_AGUA_EDIT:
    selectParameter(PARAM_AGUA);
    break;

  default:
    Serial.println("❓ Evento de edición no reconocido: ComponentID=" +
                   String(componentId));
    break;
  }
}
/**
 * @brief Manejar evento del botón "+" (incrementar parámetro)
 */
void UIControllerClass::handleParameterIncrement() {
  // INCREMENTO INLINE - evitar llamadas a funciones auxiliares para máxima
  // velocidad
  int &valor = _valoresTemporales[_parametroActual];

  // Incremento específico según parámetro con límites inline
  switch (_parametroActual) {
  case PARAM_NIVEL:
    if (valor < 4)
      valor++;
    break;
  case PARAM_TEMPERATURA:
    if (valor < 100)
      valor++;
    break;
  case PARAM_TIEMPO:
    if (valor < 60)
      valor++;
    break;
  case PARAM_ROTACION:
    if (valor < 4)
      valor++;
    break;
  case PARAM_FASE:
    if (valor < 4)
      valor++;
    break;
  case PARAM_CENTRIF:
    valor = (valor == 0) ? 1 : 0; // Toggle
    break;
  case PARAM_AGUA:
    valor = (valor == 0) ? 1 : 0; // Toggle
    break;
  }

  // ULTRA-EXTREMA velocidad - SOLO valor principal (1 comando Nextion)
  updateEditPanel(UPDATE_INSTANT, _parametroActual);
}

/**
 * @brief Manejar evento del botón "-" (decrementar parámetro)
 */
void UIControllerClass::handleParameterDecrement() {
  // DECREMENTO INLINE - evitar llamadas a funciones auxiliares para máxima
  // velocidad
  int &valor = _valoresTemporales[_parametroActual];

  // Decremento específico según parámetro con límites inline
  switch (_parametroActual) {
  case PARAM_NIVEL:
    if (valor > 0)
      valor--;
    break;
  case PARAM_TEMPERATURA:
    if (valor > 0)
      valor--;
    break;
  case PARAM_TIEMPO:
    if (valor > 1)
      valor--;
    break;
  case PARAM_ROTACION:
    if (valor > 0)
      valor--;
    break;
  case PARAM_FASE:
    if (valor > 0)
      valor--;
    break;
  case PARAM_CENTRIF:
    valor = (valor == 0) ? 1 : 0; // Toggle
    break;
  case PARAM_AGUA:
    valor = (valor == 0) ? 1 : 0; // Toggle
    break;
  }

  // ULTRA-EXTREMA velocidad - SOLO valor principal (1 comando Nextion)
  updateEditPanel(UPDATE_INSTANT, _parametroActual);
}

/**
 * @brief Manejar evento del botón "Siguiente" (pasar al siguiente parámetro)
 */
void UIControllerClass::handleNextParameter() {
  // Obtener el siguiente parámetro en el ciclo usando las funciones de
  // config.cpp
  _parametroActual = obtenerSiguienteParametro(_parametroActual);

  // Saltar parámetro FASE en P22 y P23 (solo editable en P24)
  if (_parametroActual == PARAM_FASE &&
      (_programaEnEdicion == 0 || _programaEnEdicion == 1)) {
    _parametroActual =
        obtenerSiguienteParametro(_parametroActual); // Saltar al siguiente
  }

  // Al cambiar parámetro, actualizar panel derecho para mostrar estado completo
  updateEditPanel(UPDATE_FAST, _parametroActual);
}

/**
 * @brief Manejar evento del botón "Anterior" (pasar al parámetro anterior)
 */
void UIControllerClass::handlePreviousParameter() {
  // Obtener el parámetro anterior en el ciclo usando las funciones de
  // config.cpp
  _parametroActual = obtenerAnteriorParametro(_parametroActual);

  // Saltar parámetro FASE en P22 y P23 (solo editable en P24)
  if (_parametroActual == PARAM_FASE &&
      (_programaEnEdicion == 0 || _programaEnEdicion == 1)) {
    _parametroActual =
        obtenerAnteriorParametro(_parametroActual); // Saltar al anterior
  }

  // Al cambiar parámetro, actualizar panel derecho para mostrar estado completo
  updateEditPanel(UPDATE_FAST, _parametroActual);
}

/**
 * @brief Manejar evento del botón "Guardar" (guardar todos los cambios)
 */
void UIControllerClass::handleSaveParameters() {
  if (!_parameterSaved) {
    // PRIMERA PRESIÓN: Guardar parámetro actual

    // Validar parámetro actual
    if (!_validateCurrentParameter()) {
      showMessage("Error: Valor no válido", 2000);
      return;
    }

    // Guardar parámetro actual en memoria temporal
    _saveCurrentParameterToTemp();
    _parameterSaved = true;

    // Mostrar confirmación
    showMessage("Parámetro guardado - Presione de nuevo para guardar programa",
                3000);

    // Actualizar display para mostrar que está pendiente de guardado final
    updateParameterDisplay();
  } else {
    // SEGUNDA PRESIÓN: Guardar programa completo

    // Validar todos los parámetros
    if (!_validateAllParameters()) {
      showMessage("Error: Valores no válidos", 3000);
      return;
    }

    // Para P24, usar la tanda actual del ProgramController
    uint8_t indexToUse = _faseEnEdicion;
    if (_programaEnEdicion == 2) { // P24
      indexToUse = ProgramController.getCurrentEditingTanda();
      Serial.println("🔍 P24 - Guardando en tanda: " + String(indexToUse + 1) +
                     " (índice: " + String(indexToUse) + ")");
    }
    _saveParametersToStorage(_programaEnEdicion, indexToUse);

    // Mostrar mensaje de confirmación
    showMessage("Programa guardado exitosamente", 2000);

    // Resetear estado y salir del modo edición
    _parameterSaved = false;
    _modoEdicionActivo = false;

    // Notificar al ProgramController que vuelva al estado de selección
    ProgramController.endEditing();

    // Volver a la página de selección
    safeTransitionToSelection(_programaEnEdicion);
  }
}
/**
 * @brief Manejar evento del botón "Cancelar" (descartar cambios y volver)
 */
void UIControllerClass::handleCancelEdit() {
  // Salir del modo edición sin guardar
  _modoEdicionActivo = false;
  _parameterSaved = false;

  // Notificar al ProgramController que vuelva al estado de selección
  ProgramController.endEditing();

  // Volver a la página de selección
  safeTransitionToSelection(_programaEnEdicion);
}

// ===== MÉTODOS INTERNOS PARA GESTIÓN DE PARÁMETROS =====

/// @brief
/// Cargar los parámetros del programa y fase especificados desde Storage
/// @param programa
/// Número de programa (0, 1, 2)
/// @param fase
/// Número de fase (0-3)
void UIControllerClass::_loadParametersFromStorage(uint8_t programa,
                                                   uint8_t fase) {
  // Cargar valores directamente desde Storage
  _valoresTemporales[PARAM_NIVEL] = Storage.loadWaterLevel(programa, 0, fase);
  _valoresTemporales[PARAM_TEMPERATURA] =
      Storage.loadTemperature(programa, 0, fase);
  _valoresTemporales[PARAM_TIEMPO] = Storage.loadTime(programa, fase);
  _valoresTemporales[PARAM_ROTACION] = Storage.loadRotation(programa, fase);
  _valoresTemporales[PARAM_FASE] = Storage.loadPhaseType(programa, 0, fase);
  _valoresTemporales[PARAM_CENTRIF] =
      Storage.loadCentrifugado(programa, fase); // fase representa tanda
  _valoresTemporales[PARAM_AGUA] = Storage.loadTipoAgua(programa, 0, fase);

  showMessage("Parámetros cargados de Storage - P" + String(programa + 22) +
                  " F" + String(fase + 1),
              2000);
}

/// @brief
/// Guardar los parámetros del programa y fase especificados en Storage
/// @details
/// Este método guarda los valores temporales de nivel de agua, temperatura,
/// tiempo y rotación en el almacenamiento permanente (Storage) para el programa
/// y fase especificados. También actualiza las matrices estáticas para mantener
/// la consistencia con los datos guardados.
/// @param programa
/// Número de programa (0, 1, 2)
/// @param fase
/// Número de fase (0-3)
void UIControllerClass::_saveParametersToStorage(uint8_t programa,
                                                 uint8_t fase) {
  // Guardar valores directamente en Storage
  Storage.saveWaterLevel(programa, 0, fase, _valoresTemporales[PARAM_NIVEL]);
  Storage.saveTemperature(programa, 0, fase,
                          _valoresTemporales[PARAM_TEMPERATURA]);
  Storage.saveTime(programa, fase, _valoresTemporales[PARAM_TIEMPO]);
  Storage.saveRotation(programa, fase, _valoresTemporales[PARAM_ROTACION]);
  Storage.savePhaseType(programa, 0, fase, _valoresTemporales[PARAM_FASE]);
  Storage.saveCentrifugado(programa, fase, _valoresTemporales[PARAM_CENTRIF]);
  Storage.saveTipoAgua(programa, 0, fase, _valoresTemporales[PARAM_AGUA]);

  // Actualizar matrices estáticas también para mantener consistencia
  _nivelAgua[programa][fase] = _valoresTemporales[PARAM_NIVEL];
  _temperaturaLim[programa][fase] = _valoresTemporales[PARAM_TEMPERATURA];
  _temporizadorLim[programa][fase] = _valoresTemporales[PARAM_TIEMPO];
  _rotacionTam[programa][fase] = _valoresTemporales[PARAM_ROTACION];
  _fasesPrograma[programa][fase] = _valoresTemporales[PARAM_FASE];
  _centrifugadoPorTanda[programa][fase] =
      _valoresTemporales[PARAM_CENTRIF]; // fase representa tanda
  _tipoAguaPrograma[programa][fase] = _valoresTemporales[PARAM_AGUA];

  Serial.println("✅ Parámetros guardados en Storage - P" +
                 String(programa + 22) + " Tanda/Fase: " + String(fase + 1) +
                 " [Nivel:" + String(_valoresTemporales[PARAM_NIVEL]) +
                 ", Temp:" + String(_valoresTemporales[PARAM_TEMPERATURA]) +
                 ", Tiempo:" + String(_valoresTemporales[PARAM_TIEMPO]) + "]");
}

/**
 * @brief Validar todos los parámetros antes de guardar
 * @return true si todos los parámetros son válidos, false si no
 */
bool UIControllerClass::_validateAllParameters() {
  for (int i = 0; i < 7; i++) // Ahora validamos los 7 parámetros (0-6)
  {
    if (!esParametroValido(i, _valoresTemporales[i])) {
      Serial.println("Parámetro inválido: " + String(obtenerTextoParametro(i)) +
                     " = " + String(_valoresTemporales[i]));
      return false;
    }
  }
  return true;
}

/**
 * @brief Verificar timeout de edición y salir automáticamente si es necesario
 */
void UIControllerClass::_checkEditTimeout() {
  // Solo sale manualmente con Guardar o Cancelar
  return;

  Serial.println("Timeout de edición alcanzado - Saliendo automáticamente");
}

/**
 * @brief Resetear el timeout de edición
 */
void UIControllerClass::_resetEditTimeout() { _editTimeoutStart = millis(); }

// === IMPLEMENTACIÓN DE MÉTODOS DE INDICADORES DE ESTADO ===

/**
 * @brief Actualiza la alerta de emergencia con efecto de parpadeo
 * @param state Estado del parpadeo (on/off)
 */
void UIControllerClass::updateEmergencyAlert(bool state) {
  // Cambiar color de fondo o mostrar alerta visual
  if (state) {
    // Fondo rojo para emergencia
    Hardware.nextionSendCommand("page0.bco=63488"); // Color rojo
    Hardware.nextionSendCommand("tEmergencia.txt=\"¡EMERGENCIA!\"");
  } else {
    // Fondo normal
    Hardware.nextionSendCommand("page0.bco=0"); // Color negro
    Hardware.nextionSendCommand("tEmergencia.txt=\"\"");
  }
}

/**
 * @brief Actualiza el display de error con efecto de parpadeo
 * @param blinkState Estado del parpadeo para crear efecto visual
 */
void UIControllerClass::updateErrorDisplay(bool blinkState) {
  if (_currentPage != NEXTION_PAGE_ERROR)
    return;

  // Hacer parpadear el texto de error
  String cmd =
      "tError.pco=" + String(blinkState ? 63488 : 65535); // Rojo : Blanco
  Hardware.nextionSendCommand(cmd);
}

/**
 * @brief Actualiza la información del programa en la pantalla de selección
 * @param programa Número de programa (1-3)
 */
void UIControllerClass::updateProgramInfo(uint8_t programa) {
  if (_currentPage != NEXTION_PAGE_SELECTION)
    return;

  // Actualizar texto descriptivo del programa usando componente mensaje común
  char buffer[100];
  generarTextoPrograma(programa, buffer, sizeof(buffer));

  Hardware.nextionSetText(NEXTION_COMP_MSG, String(buffer));
}

/**
 * @brief Muestra el estado de preparación mientras se alcanzan las condiciones
 * @param prepTime Tiempo transcurrido en la preparación (segundos)
 */
void UIControllerClass::updatePreparationStatus(unsigned long prepTime) {
  if (_currentPage != NEXTION_PAGE_EXECUTION)
    return;

  // Mostrar tiempo de preparación y estado usando el componente mensaje común
  char timeBuffer[10];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02lu:%02lu", prepTime / 60,
           prepTime % 60);

  Hardware.nextionSetText(NEXTION_COMP_MSG,
                          "Preparando... " + String(timeBuffer));
}

/**
 * @brief Limpia el estado de preparación cuando se alcanzan las condiciones
 */
void UIControllerClass::clearPreparationStatus() {
  if (_currentPage != NEXTION_PAGE_EXECUTION)
    return;

  // Limpiar mensaje de preparación
  Hardware.nextionSetText(NEXTION_COMP_MSG, "");
}

// === NUEVAS FUNCIONES PARA SELECCIÓN DIRECTA DE PARÁMETROS ===

/**
 * @brief Selecciona directamente un parámetro específico para edición
 * @param param Tipo de parámetro (PARAM_NIVEL, PARAM_TEMPERATURA, etc.)
 */
void UIControllerClass::selectParameter(uint8_t param) {
  if (!_modoEdicionActivo)
    return;

  _parametroActual = param;

  // Sin mensajes - feedback visual instantáneo es el parámetro destacado
  // Los mensajes causan delay innecesario en la respuesta

  // Actualizar display para mostrar parámetro activo
  updateParameterDisplay();
  updateEditPanel(UPDATE_FULL); // Optimizado como página de selección
}

/**
 * @brief Selecciona la fase para edición
 */
void UIControllerClass::selectPhase() {
  if (!_modoEdicionActivo)
    return;

  // P22 y P23 tienen secuencias fijas, solo P24 permite editar fases
  if (_programaEnEdicion == 0 || _programaEnEdicion == 1) {
    // Programas 22 y 23 - no permitir edición de fase
    showMessage("P" + String(_programaEnEdicion + 22) + " tiene secuencia fija",
                2000);
    return;
  }

  _parametroActual = PARAM_FASE;

  // Actualizar display para mostrar parámetro activo
  updateParameterDisplay();
  updateEditPanel(UPDATE_FULL); // Optimizado como página de selección
}

void UIControllerClass::selectTanda() {
  if (!_modoEdicionActivo)
    return;

  // Esta funcionalidad está implementada en ProgramController
  // para manejar la selección de tanda y actualizar el panel derecho
  // Sin mensajes para respuesta más rápida - el cambio visual es suficiente
  // feedback
}

void UIControllerClass::selectTandaDirecta(uint8_t tanda) {
  if (!_modoEdicionActivo)
    return;

  // Solo para P24 - P22 y P23 usan una sola tanda
  if (_programaEnEdicion != 2) {
    showMessage("P" + String(_programaEnEdicion + 22) + " usa solo 1 tanda",
                1500);
    return;
  }

  // Validar rango de tanda (0-3 para P24, que tiene 4 tandas)
  if (tanda > 3) {
    showMessage("P24 tiene 4 tandas", 1500);
    return;
  }

  // Notificar al ProgramController que cambie la tanda
  ProgramController.setEditingTanda(tanda);

  // Actualizar visualización de botones de tanda
  updateTandaButtons(tanda);

  Serial.println("🔘 Tanda " + String(tanda + 1) +
                 " seleccionada directamente");
  Serial.println("🔍 Actualizando botones: Tanda activa = " + String(tanda));
}

/**
 * @brief Selecciona parámetros de centrifugado
 */
void UIControllerClass::selectCentrifuge() {
  if (!_modoEdicionActivo)
    return;

  _parametroActual = PARAM_CENTRIF;

  // Actualizar display para mostrar parámetro activo
  updateParameterDisplay();
  updateEditPanel(UPDATE_FULL); // Optimizado como página de selección
}

/**
 * @brief Selecciona parámetros de agua
 */
void UIControllerClass::selectWater() {
  if (!_modoEdicionActivo)
    return;

  _parametroActual = PARAM_AGUA;

  // Actualizar display para mostrar parámetro activo
  updateParameterDisplay();
  updateEditPanel(UPDATE_FULL); // Optimizado como página de selección
}

// === FUNCIONES AUXILIARES PARA DOBLE GUARDADO ===

/**
 * @brief Valida solo el parámetro actualmente seleccionado
 */
bool UIControllerClass::_validateCurrentParameter() {
  int value = _valoresTemporales[_parametroActual];
  return esParametroValido(_parametroActual, value);
}

/**
 * @brief Guarda el parámetro actual en memoria temporal (sin persistir)
 */
void UIControllerClass::_saveCurrentParameterToTemp() {
  // El valor ya está en _valoresTemporales[_parametroActual]
  // Solo mostramos confirmación
  String paramName = String(obtenerTextoParametro(_parametroActual));
  int value = _valoresTemporales[_parametroActual];

  Serial.println("💾 Guardando temporalmente " + paramName + ": " +
                 String(value));
}

void UIControllerClass::updateStartButtonText() {
  // Solo actualizar si estamos en la página de selección
  if (_currentPage != NEXTION_PAGE_SELECTION) {
    return;
  }

  if (!Sensors.isDoorClosed()) {
    // Puerta abierta - cambiar botón a "CERRAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_START, "CERRAR");
    Hardware.nextionSetText(NEXTION_COMP_MSG, "PUERTA ABIERTA");
  } else {
    // Puerta cerrada - texto por defecto "INICIAR"
    Hardware.nextionSetText(NEXTION_COMP_BTN_START, "INICIAR");
    Hardware.nextionSetText(NEXTION_COMP_MSG, ""); // Limpiar mensaje
  }
}

// ===== FUNCIONES PARA MANEJO DE BOTONES DE TANDA =====

/**
 * @brief Actualiza el estado visual de los botones de tanda
 * @param tandaActiva Tanda actualmente seleccionada (0-2)
 */
void UIControllerClass::updateTandaButtons(uint8_t tandaActiva) {
  if (!_modoEdicionActivo || _currentPage != NEXTION_PAGE_EDIT)
    return;

  // Configurar colores para botones de tanda
  uint16_t colorActivo = 25919;  // 
  uint16_t colorInactivo = 2279; // 

  if (_programaEnEdicion == 2) { // P24 - 4 tandas activas
    // TANDA1 (índice 0)
    Hardware.nextionSendCommand(
        String(NEXTION_COMP_BTN_TANDA1) +
        ".bco=" + String(tandaActiva == 0 ? colorActivo : colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA1) +
                                ",1"); // Habilitado

    // TANDA2 (índice 1)
    Hardware.nextionSendCommand(
        String(NEXTION_COMP_BTN_TANDA2) +
        ".bco=" + String(tandaActiva == 1 ? colorActivo : colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA2) +
                                ",1"); // Habilitado

    // TANDA3 (índice 2)
    Hardware.nextionSendCommand(
        String(NEXTION_COMP_BTN_TANDA3) +
        ".bco=" + String(tandaActiva == 2 ? colorActivo : colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA3) +
                                ",1"); // Habilitado

    // TANDA4 (índice 3)
    Hardware.nextionSendCommand(
        String(NEXTION_COMP_BTN_TANDA4) +
        ".bco=" + String(tandaActiva == 3 ? colorActivo : colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA4) +
                                ",1"); // Habilitado
  } else {                             // P22 y P23 - Solo TANDA1 activa
    // TANDA1 - Siempre activa para P22/P23
    Hardware.nextionSendCommand(String(NEXTION_COMP_BTN_TANDA1) +
                                ".bco=" + String(colorActivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA1) +
                                ",1"); // Habilitado

    // TANDA2, TANDA3, TANDA4 - Deshabilitadas para P22/P23
    Hardware.nextionSendCommand(String(NEXTION_COMP_BTN_TANDA2) +
                                ".bco=" + String(colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA2) +
                                ",0"); // Deshabilitado

    Hardware.nextionSendCommand(String(NEXTION_COMP_BTN_TANDA3) +
                                ".bco=" + String(colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA3) +
                                ",0"); // Deshabilitado

    Hardware.nextionSendCommand(String(NEXTION_COMP_BTN_TANDA4) +
                                ".bco=" + String(colorInactivo));
    Hardware.nextionSendCommand("tsw " + String(NEXTION_ID_BTN_TANDA4) +
                                ",0"); // Deshabilitado
  }
}

/**
 * @brief Actualiza la tanda mostrada en la página de ejecución
 * @param programa Programa actual (0=P22, 1=P23, 2=P24)
 * @param tanda Tanda actual (0-3 para P24, siempre 0 para P22/P23)
 */
void UIControllerClass::updateTanda(uint8_t programa, uint8_t tanda) {
  if (_currentPage != NEXTION_PAGE_EXECUTION)
    return;
    
  if (programa == 2) { // P24 - mostrar tanda actual
    Hardware.nextionSetText(NEXTION_COMP_TANDA_EJECUCION, String(tanda + 1)); // Mostrar 1-4
  } else { // P22/P23 - siempre tanda 1
    Hardware.nextionSetText(NEXTION_COMP_TANDA_EJECUCION, "1");
  }
}
