// === EJEMPLO DE IMPLEMENTACIÓN DE PÁGINA DE EDICIÓN DE PARÁMETROS ===
// Este archivo muestra cómo usar las funciones de config.cpp para implementar
// la funcionalidad de edición en la página 3 de la pantalla Nextion

/*
CONTEXTO DE USO:
El usuario quiere editar los parámetros del programa P22:
1. param = "P22" (se muestra en la pantalla)
2. param_value = "45" (temperatura inicial, irá cambiando con btnMas/btnMenos)
3. En el panel derecho se muestran todos los valores actuales (val_temp, val_nivel, etc.)
4. Al presionar "Siguiente", pasa al siguiente parámetro
5. Al presionar "Guardar", se almacenan todos los cambios
*/

#include "config.h"

// ===== VARIABLES GLOBALES PARA PÁGINA DE EDICIÓN =====
static int programaEnEdicion = 1;          // Programa siendo editado (1=P22, 2=P23, 3=P24)
static int faseEnEdicion = 1;              // Fase siendo editada (1-4)
static int parametroActual = PARAM_NIVEL;  // Parámetro actualmente seleccionado
static int valoresTemporales[4];           // Valores temporales: [nivel, temp, tiempo, rotacion]

// ===== FUNCIONES DE EJEMPLO PARA MANEJO DE EVENTOS =====

/**
 * @brief Inicializar la página de edición con los valores actuales del programa
 * @param programa Número de programa (1, 2, 3)
 * @param fase Número de fase (1-4)
 */
void inicializarPaginaEdicion(int programa, int fase) {
    programaEnEdicion = programa;
    faseEnEdicion = fase;
    parametroActual = PARAM_NIVEL; // Comenzar con el primer parámetro
    
    // Cargar valores actuales desde storage (ejemplo)
    // valoresTemporales[PARAM_NIVEL] = Storage.getParamNivel(programa, fase);
    // valoresTemporales[PARAM_TEMPERATURA] = Storage.getParamTemp(programa, fase);
    // valoresTemporales[PARAM_TIEMPO] = Storage.getParamTiempo(programa, fase);
    // valoresTemporales[PARAM_ROTACION] = Storage.getParamRotacion(programa, fase);
    
    // Valores de ejemplo para demostración
    valoresTemporales[PARAM_NIVEL] = 2;
    valoresTemporales[PARAM_TEMPERATURA] = 45;
    valoresTemporales[PARAM_TIEMPO] = 15;
    valoresTemporales[PARAM_ROTACION] = 2;
    
    // Actualizar pantalla con valores iniciales
    actualizarPantallaEdicion();
}

/**
 * @brief Actualizar todos los componentes de la pantalla de edición
 */
void actualizarPantallaEdicion() {
    char buffer[20];
    
    // Actualizar programa en edición
    generarTextoPrograma(programaEnEdicion, buffer, sizeof(buffer));
    // Hardware.nextionSendCommand("progr_edic.txt=\"" + String(buffer) + "\"");
    
    // Actualizar fase
    snprintf(buffer, sizeof(buffer), "F%d", faseEnEdicion);
    // Hardware.nextionSendCommand("fase_edic.txt=\"" + String(buffer) + "\"");
    
    // Actualizar parámetro actual siendo editado
    const char* textoParam = obtenerTextoParametro(parametroActual);
    // Hardware.nextionSendCommand("param.txt=\"" + String(textoParam) + "\"");
    
    // Actualizar valor del parámetro actual
    formatearParametroConUnidad(parametroActual, valoresTemporales[parametroActual], buffer, sizeof(buffer));
    // Hardware.nextionSendCommand("param_value.txt=\"" + String(buffer) + "\"");
    
    // Actualizar panel derecho con todos los valores
    actualizarPanelDerecho();
}
/**
 * @brief Actualizar el panel derecho con todos los valores actuales
 */
void actualizarPanelDerecho() {
    char buffer[20];
    
    // Actualizar nivel en panel derecho
    snprintf(buffer, sizeof(buffer), "%d", valoresTemporales[PARAM_NIVEL]);
    // Hardware.nextionSendCommand("val_nivel.txt=\"" + String(buffer) + "\"");
    
    // Actualizar temperatura en panel derecho
    snprintf(buffer, sizeof(buffer), "%d°C", valoresTemporales[PARAM_TEMPERATURA]);
    // Hardware.nextionSendCommand("val_temp.txt=\"" + String(buffer) + "\"");
    
    // Actualizar tiempo en panel derecho
    snprintf(buffer, sizeof(buffer), "%d min", valoresTemporales[PARAM_TIEMPO]);
    // Hardware.nextionSendCommand("val_tiempo.txt=\"" + String(buffer) + "\"");
    
    // Actualizar rotación en panel derecho
    snprintf(buffer, sizeof(buffer), "Vel %d", valoresTemporales[PARAM_ROTACION]);
    // Hardware.nextionSendCommand("val_rotac.txt=\"" + String(buffer) + "\"");
    
    // Actualizar fase en panel derecho
    snprintf(buffer, sizeof(buffer), "F%d", faseEnEdicion);
    // Hardware.nextionSendCommand("val_fase.txt=\"" + String(buffer) + "\"");
}

// ===== FUNCIONES DE MANEJO DE EVENTOS DE BOTONES =====

/**
 * @brief Manejar evento del botón "+" (incrementar parámetro)
 */
void manejarBotonMas() {
    // Incrementar el valor del parámetro actual
    valoresTemporales[parametroActual] = incrementarParametro(parametroActual, valoresTemporales[parametroActual]);
    
    // Actualizar pantalla
    actualizarPantallaEdicion();
    
    // Mostrar feedback visual/sonoro (opcional)
    // Hardware.activarBuzzer(50);
}

/**
 * @brief Manejar evento del botón "-" (decrementar parámetro)
 */
void manejarBotonMenos() {
    // Decrementar el valor del parámetro actual
    valoresTemporales[parametroActual] = decrementarParametro(parametroActual, valoresTemporales[parametroActual]);
    
    // Actualizar pantalla
    actualizarPantallaEdicion();
    
    // Mostrar feedback visual/sonoro (opcional)
    // Hardware.activarBuzzer(50);
}
/**
 * @brief Manejar evento del botón "Siguiente" (pasar al siguiente parámetro)
 */
void manejarBotonSiguiente() {
    // Obtener el siguiente parámetro en el ciclo
    parametroActual = obtenerSiguienteParametro(parametroActual);
    
    // Actualizar pantalla para mostrar el nuevo parámetro
    actualizarPantallaEdicion();
    
    // Mostrar feedback visual (opcional)
    // Hardware.activarBuzzer(100);
}

/**
 * @brief Manejar evento del botón "Anterior" (pasar al parámetro anterior)
 */
void manejarBotonAnterior() {
    // Obtener el parámetro anterior en el ciclo
    parametroActual = obtenerAnteriorParametro(parametroActual);
    
    // Actualizar pantalla para mostrar el nuevo parámetro
    actualizarPantallaEdicion();
    
    // Mostrar feedback visual (opcional)
    // Hardware.activarBuzzer(100);
}

/**
 * @brief Manejar evento del botón "Guardar" (guardar todos los cambios)
 */
void manejarBotonGuardar() {
    // Validar todos los parámetros antes de guardar
    for (int i = 0; i < 4; i++) {
        if (!esParametroValido(i, valoresTemporales[i])) {
            // Mostrar mensaje de error
            // UIController.mostrarMensajeError("Valores no válidos");
            return;
        }
    }
    
    // Guardar valores en storage permanente
    // Storage.setParamNivel(programaEnEdicion, faseEnEdicion, valoresTemporales[PARAM_NIVEL]);
    // Storage.setParamTemp(programaEnEdicion, faseEnEdicion, valoresTemporales[PARAM_TEMPERATURA]);
    // Storage.setParamTiempo(programaEnEdicion, faseEnEdicion, valoresTemporales[PARAM_TIEMPO]);
    // Storage.setParamRotacion(programaEnEdicion, faseEnEdicion, valoresTemporales[PARAM_ROTACION]);
    
    // Mostrar mensaje de confirmación
    // UIController.mostrarMensaje("Parámetros guardados exitosamente");
    
    // Volver a la página de selección
    // UIController.safeTransitionToSelection();
    
    // Mostrar feedback visual/sonoro
    // Hardware.activarBuzzer(200);
}
/**
 * @brief Manejar evento del botón "Cancelar" (descartar cambios y volver)
 */
void manejarBotonCancelar() {
    // Mostrar mensaje de confirmación (opcional)
    // UIController.mostrarMensajeConfirmacion("¿Descartar cambios?");
    
    // Volver a la página de selección sin guardar
    // UIController.safeTransitionToSelection();
    
    // Mostrar feedback visual/sonoro
    // Hardware.activarBuzzer(150);
}

// ===== PROCESADOR PRINCIPAL DE EVENTOS DE EDICIÓN =====

/**
 * @brief Procesar eventos táctiles en la página de edición
 * @param componentId ID del componente que generó el evento
 */
void procesarEventoPaginaEdicion(int componentId) {
    switch (componentId) {
        case NEXTION_ID_BTN_PARAM_MAS:
            manejarBotonMas();
            break;
            
        case NEXTION_ID_BTN_PARAM_MENOS:
            manejarBotonMenos();
            break;
            
        case NEXTION_ID_BTN_PARAM_SIGUIENTE:
            manejarBotonSiguiente();
            break;
            
        case NEXTION_ID_BTN_PARAM_ANTERIOR:
            manejarBotonAnterior();
            break;
            
        case NEXTION_ID_BTN_GUARDAR:
            manejarBotonGuardar();
            break;
            
        case NEXTION_ID_BTN_CANCELAR:
            manejarBotonCancelar();
            break;
            
        default:
            // Evento no reconocido
            break;
    }
}

// ===== EJEMPLO DE INTEGRACIÓN CON UI_CONTROLLER =====

/*
Para integrar esta funcionalidad en UIController, se puede agregar algo como:

// En ui_controller.cpp
void UIController::handleEditPageEvent(int componentId) {
    // Llamar al procesador de eventos de edición
    procesarEventoPaginaEdicion(componentId);
}

// En el loop principal de procesamiento de eventos
if (currentState == ESTADO_EDICION) {
    int componentId = Hardware.getLastTouchEvent();
    if (componentId != -1) {
        handleEditPageEvent(componentId);
    }
}
*/