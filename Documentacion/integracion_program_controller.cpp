// === EJEMPLO DE INTEGRACIÓN PROGRAM CONTROLLER → UI CONTROLLER ===
// Muestra cómo usar las nuevas funciones de edición desde Program Controller
// Autor: Ingeniero Electrónico Senior

/*
Este archivo muestra los cambios necesarios en program_controller.cpp
para usar las nuevas funciones de edición del UI Controller
*/

// ===== CAMBIOS EN PROGRAM_CONTROLLER.H =====
/*
// Agregar en la clase ProgramController:
private:
    // Estado de edición
    bool _editModeRequested;
    uint8_t _programToEdit;
    uint8_t _phaseToEdit;
    
    // Métodos para edición
    void handleEditRequest();
    void checkEditCompletion();
*/

// ===== CAMBIOS EN PROGRAM_CONTROLLER.CPP =====

/*
// En el método que maneja eventos de selección de programa:
void ProgramController::handleSelectionPageEvent(int componentId) {
    switch (componentId) {
        case NEXTION_ID_BTN_PROGRAM1:
            currentProgram = 1;
            UIController.showSelectionScreen(1);
            break;
            
        case NEXTION_ID_BTN_PROGRAM2:
            currentProgram = 2;
            UIController.showSelectionScreen(2);
            break;
            
        case NEXTION_ID_BTN_PROGRAM3:
            currentProgram = 3;
            UIController.showSelectionScreen(3);
            break;
            
        case NEXTION_ID_BTN_EDIT:
            // === NUEVA FUNCIONALIDAD DE EDICIÓN ===
            handleEditRequest();
            break;
            
        case NEXTION_ID_BTN_START:
            // Comenzar programa seleccionado
            startProgram(currentProgram);
            break;
    }
}

// Nueva función para manejar solicitudes de edición:
void ProgramController::handleEditRequest() {
    if (currentProgram >= 1 && currentProgram <= 3) {
        // Por ahora editamos la fase 1, pero se puede extender para seleccionar fase
        uint8_t faseAEditar = 1;
        
        Serial.println("Iniciando edición de P" + String(currentProgram + 21) + " F" + String(faseAEditar));
        
        // Usar transición segura para evitar eventos pendientes
        UIController.safeTransitionToEdit(currentProgram, faseAEditar);
        
        // Cambiar estado del controlador
        currentState = ESTADO_EDICION;
        
        // Guardar información de edición
        _editModeRequested = true;
        _programToEdit = currentProgram;
        _phaseToEdit = faseAEditar;
    } else {
        Serial.println("Error: No hay programa seleccionado para editar");
        UIController.showMessage("Seleccione un programa primero", 2000);
    }
}

// En el loop principal, agregar verificación de edición:
void ProgramController::update() {
    // ... código existente ...
    
    switch (currentState) {
        case ESTADO_SELECCION:
            // ... código existente ...
            break;
            
        case ESTADO_EDICION:
            // === NUEVO MANEJO DE ESTADO EDICIÓN ===
            checkEditCompletion();
            break;
            
        case ESTADO_EJECUCION:
            // ... código existente ...
            break;
            
        // ... otros estados ...
    }
}

// Nueva función para verificar cuando la edición termina:
void ProgramController::checkEditCompletion() {
    // El UI Controller maneja automáticamente la edición
    // Solo necesitamos verificar si hemos salido del modo edición
    
    if (_editModeRequested && UIController.isUIStable()) {
        // Verificar si UIController ha terminado la edición
        // (esto se puede implementar agregando un método isEditMode() en UIController)
        
        // Por ahora, asumimos que si UIController está estable y
        // ya no estamos en la página de edición, la edición terminó
        
        _editModeRequested = false;
        currentState = ESTADO_SELECCION;
        
        Serial.println("Edición completada, volviendo a selección");
        
        // Actualizar la pantalla de selección con los nuevos valores
        UIController.showSelectionScreen(_programToEdit);
    }
}
*/

// ===== EJEMPLO DE USO AVANZADO =====

/*
// Para implementar edición de múltiples fases (programa P24):
void ProgramController::handleMultiPhaseEdit() {
    uint8_t programa = 3; // P24
    
    for (uint8_t fase = 1; fase <= 4; fase++) {
        Serial.println("Editando fase " + String(fase) + " de P24");
        
        // Esperar a que UI esté estable
        while (!UIController.isUIStable()) {
            delay(10);
        }
        
        // Iniciar edición de esta fase
        UIController.safeTransitionToEdit(programa, fase);
        
        // Esperar a que termine la edición
        // (esto requeriría extender UIController con señales de finalización)
    }
}

// Para validar cambios antes de permitir ejecución:
bool ProgramController::validateProgramParameters(uint8_t programa) {
    // Esta función se podría implementar usando las funciones de config.cpp
    // para verificar que todos los parámetros del programa estén en rangos válidos
    
    for (uint8_t fase = 1; fase <= 4; fase++) {
        // Cargar parámetros de la fase
        // int nivel = Storage.getParamNivel(programa, fase);
        // int temp = Storage.getParamTemp(programa, fase);
        // int tiempo = Storage.getParamTiempo(programa, fase);
        // int rotacion = Storage.getParamRotacion(programa, fase);
        
        // Validar usando funciones de config.cpp
        // if (!esParametroValido(PARAM_NIVEL, nivel) ||
        //     !esParametroValido(PARAM_TEMPERATURA, temp) ||
        //     !esParametroValido(PARAM_TIEMPO, tiempo) ||
        //     !esParametroValido(PARAM_ROTACION, rotacion)) {
        //     return false;
        // }
    }
    
    return true;
}
*/

// ===== BENEFICIOS DE LA NUEVA ARQUITECTURA =====

/*
1. SEPARACIÓN DE RESPONSABILIDADES:
   - UI Controller: Maneja toda la lógica de interfaz y edición
   - Program Controller: Se enfoca en lógica de programa y estados
   - Config: Centraliza validación y límites

2. ROBUSTEZ:
   - Transiciones seguras entre pantallas
   - Validación automática de parámetros
   - Timeout de seguridad en edición
   - Limpieza automática de eventos pendientes

3. EXTENSIBILIDAD:
   - Fácil agregar nuevos parámetros
   - Modificar límites sin cambiar lógica
   - Agregar nuevas pantallas de edición

4. MANTENIBILIDAD:
   - Código modular y bien documentado
   - Funciones reutilizables
   - Logging detallado para debugging

5. EXPERIENCIA DE USUARIO:
   - Feedback visual y sonoro inmediato
   - Navegación intuitiva entre parámetros
   - Actualización en tiempo real de valores
   - Protección contra pérdida accidental de datos
*/