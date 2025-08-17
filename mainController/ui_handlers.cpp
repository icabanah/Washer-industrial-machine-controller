#include "ui_handlers.h"
#include "ui_controller.h"
#include "config.h"
#include "sensors.h"
#include "debug.h"

/**
 * @file ui_handlers.cpp
 * @brief Implementación de handlers de eventos de UI
 * 
 * Contiene las implementaciones de los handlers de eventos extraídos de
 * ui_controller.cpp para optimizar el tamaño del archivo principal.
 */

// === HANDLERS DE EVENTOS POR PÁGINA ===

/**
 * @brief Maneja eventos de la página de selección de programas
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleSelectionEvents(uint8_t componentId, UIControllerClass& uiController) {
  if (componentId == NEXTION_ID_BTN_PROGRAM1) {
    uiController._lastUserAction = "PROGRAM_1";
    uiController._userActionPending = true;
    uiController.showMessage("Programa P22 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el programa
    
  } else if (componentId == NEXTION_ID_BTN_PROGRAM2) {
    uiController._lastUserAction = "PROGRAM_2";
    uiController._userActionPending = true;
    uiController.showMessage("Programa P23 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el programa
    
  } else if (componentId == NEXTION_ID_BTN_PROGRAM3) {
    uiController._lastUserAction = "PROGRAM_3";
    uiController._userActionPending = true;
    uiController.showMessage("Programa P24 seleccionado", 2000);
    // Nota: No actualizar aquí - se hace en ProgramController al cambiar el programa
    
  } else if (componentId == NEXTION_ID_BTN_START) {
    // Verificar estado de puerta para determinar acción
    if (!Sensors.isDoorClosed()) {
      // Puerta abierta - botón dice "CERRAR" - activar bloqueo de puerta
      uiController._lastUserAction = "CLOSE_DOOR";
      uiController._userActionPending = true;
      uiController.showMessage("Puerta cerrada", 2000);
    } else {
      // Puerta cerrada - botón dice "INICIAR" - iniciar programa
      uiController._lastUserAction = "START";
      uiController._userActionPending = true;
      uiController.showMessage("Programa iniciado", 2000);
    }
    
  } else if (componentId == NEXTION_ID_BTN_EDIT) {
    uiController._lastUserAction = "EDIT";
    uiController._userActionPending = true;
    uiController.showMessage("Modo edición activado", 2000);
    
  } else {
    uiController.showMessage("Componente no reconocido", 2000);
  }
}

/**
 * @brief Maneja eventos de la página de ejecución de programas
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleExecutionEvents(uint8_t componentId, UIControllerClass& uiController) {
  if (componentId == NEXTION_ID_BTN_PARAR) {
    uiController._lastUserAction = "STOP";
    uiController._userActionPending = true;
    uiController.showMessage("Programa detenido", 2000);
    
  } else if (componentId == NEXTION_ID_BTN_PAUSAR) {
    uiController._lastUserAction = "PAUSE";
    uiController._userActionPending = true;
    uiController.showMessage("Programa pausado", 2000);
    
  } else {
    Debug.print("ComponentID no reconocido: " + String(componentId));
  }
}

/**
 * @brief Maneja eventos de la página de edición de parámetros
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleEditEvents(uint8_t componentId, UIControllerClass& uiController) {
  // Verificar que el modo edición esté activo
  if (!uiController._modoEdicionActivo) {
    Debug.print("Evento de edición recibido pero modo edición no está activo");
    return;
  }

  switch (componentId) {
  case NEXTION_ID_BTN_PARAM_MAS:
    uiController.handleParameterIncrement();
    break;

  case NEXTION_ID_BTN_PARAM_MENOS:
    uiController.handleParameterDecrement();
    break;

  case NEXTION_ID_BTN_PARAM_SIGUIENTE:
    uiController.handleNextParameter();
    break;

  case NEXTION_ID_BTN_PARAM_ANTERIOR:
    uiController.handlePreviousParameter();
    break;

  case NEXTION_ID_BTN_GUARDAR:
    uiController.handleSaveParameters();
    break;

  case NEXTION_ID_BTN_CANCELAR:
    uiController.handleCancelEdit();
    break;

  // Selección directa de parámetros
  case NEXTION_ID_PARAM_NIVEL_EDIT:
    uiController.selectParameter(PARAM_NIVEL);
    break;

  case NEXTION_ID_PARAM_TEMP_EDIT:
    uiController.selectParameter(PARAM_TEMPERATURA);
    break;

  case NEXTION_ID_PARAM_TIEMPO_EDIT:
    uiController.selectParameter(PARAM_TIEMPO);
    break;

  case NEXTION_ID_PARAM_ROTAC_EDIT:
    uiController.selectParameter(PARAM_ROTACION);
    break;

  case NEXTION_ID_PARAM_FASE_EDIT:
    // DESACTIVADO: Ahora se usan los botones TANDA1-4 para mejor UX
    break;

  // Botones de tanda para mejor experiencia de usuario
  case NEXTION_ID_BTN_TANDA1:
    uiController.selectTandaDirecta(0);
    break;

  case NEXTION_ID_BTN_TANDA2:
    uiController.selectTandaDirecta(1);
    break;

  case NEXTION_ID_BTN_TANDA3:
    uiController.selectTandaDirecta(2);
    break;

  case NEXTION_ID_BTN_TANDA4:
    uiController.selectTandaDirecta(3);
    break;

  case NEXTION_ID_PARAM_CENTRIF_EDIT:
    uiController.selectParameter(PARAM_CENTRIF);
    break;

  case NEXTION_ID_PARAM_AGUA_EDIT:
    uiController.selectParameter(PARAM_AGUA);
    break;

  default:
    Debug.print("Evento de edición no reconocido: ComponentID=" + String(componentId));
    break;
  }
}