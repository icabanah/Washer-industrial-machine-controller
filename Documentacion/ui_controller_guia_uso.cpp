// === DOCUMENTACIÓN DE USO - UI CONTROLLER ACTUALIZADO ===
// Guía para usar las nuevas funciones de edición de parámetros
// Integración con config.cpp para validación y gestión de parámetros
// Autor: Ingeniero Electrónico Senior
// Proyecto: Controlador de Lavadora Industrial - ESP32 + Nextion

/*
RESUMEN DE ACTUALIZACIONES EN UI CONTROLLER:

1. INTEGRACIÓN COMPLETA CON CONFIG.CPP
   - Todas las funciones de validación, incremento y navegación están integradas
   - Los límites y rangos se gestionan automáticamente
   - Formateo con unidades automático para display

2. NUEVA ARQUITECTURA DE EDICIÓN
   - Modo de edición completamente encapsulado
   - Timeout automático para seguridad
   - Validación antes de guardar
   - Feedback visual y sonoro

3. MANEJO ROBUSTO DE EVENTOS
   - Procesamiento específico por tipo de evento
   - Limpieza de eventos pendientes en transiciones
   - Reseteo automático de timeout en interacciones
*/

#include "ui_controller.h"
#include "config.h"

// ===== EJEMPLO DE USO DESDE PROGRAM CONTROLLER =====

/*
// En program_controller.cpp - Ejemplo de cómo iniciar edición
void ProgramController::handleEditRequest(uint8_t programa, uint8_t fase) {
    // Usar transición segura para edición
    UIController.safeTransitionToEdit(programa, fase);
    
    // El UIController se encarga automáticamente de:
    // 1. Cargar valores actuales desde storage
    // 2. Inicializar modo de edición
    // 3. Mostrar la pantalla con todos los valores
    // 4. Comenzar con el primer parámetro (Nivel)
}

// En program_controller.cpp - Verificar si la UI está estable
void ProgramController::checkUIStability() {
    if (UIController.isUIStable()) {
        // La UI está lista para procesar eventos críticos
        processUserActions();
    }
}
*/

// ===== FLUJO COMPLETO DE EDICIÓN =====

/*
EJEMPLO PRÁCTICO: Editar temperatura del programa P22, fase F1

1. INICIO DE EDICIÓN:
   UIController.safeTransitionToEdit(1, 1);  // P22 (programa 1), F1 (fase 1)
   
2. EL USUARIO VE EN PANTALLA:
   - progr_edic: "P22"
   - fase_edic: "F1" 
   - param: "Nivel"           (primer parámetro)
   - param_value: "2"         (valor actual del nivel)
   - Panel derecho con todos los valores actuales

3. NAVEGACIÓN A TEMPERATURA:
   - Usuario presiona "Siguiente" 
   - Evento procesado: handleNextParameter()
   - param cambia a: "Temperatura"
   - param_value cambia a: "45°C"

4. MODIFICACIÓN DEL VALOR:
   - Usuario presiona "+" varias veces
   - Eventos procesados: handleParameterIncrement()
   - param_value cambia: "45°C" → "50°C" → "55°C"
   - Panel derecho se actualiza automáticamente: val_temp muestra "55°C"

5. GUARDADO:
   - Usuario presiona "Guardar"
   - Evento procesado: handleSaveParameters()
   - Validación automática de todos los parámetros
   - Guardado en storage (cuando esté completamente integrado)
   - Mensaje de confirmación
   - Retorno automático a página de selección
*/

// ===== CARACTERÍSTICAS AVANZADAS =====

/*
VALIDACIÓN AUTOMÁTICA:
- Cada incremento/decremento respeta los límites definidos en config.h
- Nivel: 1-4, Temperatura: 20-90°C, Tiempo: 1-60 min, Rotación: 1-3
- Validación completa antes de guardar

NAVEGACIÓN CIRCULAR:
- Nivel → Temperatura → Tiempo → Rotación → Nivel (ciclo completo)
- Funciona en ambas direcciones (Siguiente/Anterior)

TIMEOUT DE SEGURIDAD:
- 30 segundos sin actividad → salida automática sin guardar
- Reseteo automático del timeout en cualquier interacción
- Mensaje de advertencia al usuario

FEEDBACK VISUAL/SONORO:
- Sonidos diferentes para confirmación, advertencia, error
- Mensajes temporales para feedback del usuario
- Actualización en tiempo real del panel derecho

PANEL DERECHO DINÁMICO:
- val_nivel, val_temp, val_tiempo, val_rotac, val_fase
- Se actualiza inmediatamente con cada cambio
- Muestra siempre los valores más recientes (incluso sin guardar)
*/

// ===== INTEGRACIÓN CON OTROS MÓDULOS =====

/*
INTEGRACIÓN CON STORAGE (Cuando esté listo):
En _loadParametersFromStorage():
   _valoresTemporales[PARAM_NIVEL] = Storage.getParamNivel(programa, fase);
   _valoresTemporales[PARAM_TEMPERATURA] = Storage.getParamTemp(programa, fase);
   _valoresTemporales[PARAM_TIEMPO] = Storage.getParamTiempo(programa, fase);
   _valoresTemporales[PARAM_ROTACION] = Storage.getParamRotacion(programa, fase);

En _saveParametersToStorage():
   Storage.setParamNivel(programa, fase, _valoresTemporales[PARAM_NIVEL]);
   Storage.setParamTemp(programa, fase, _valoresTemporales[PARAM_TEMPERATURA]);
   Storage.setParamTiempo(programa, fase, _valoresTemporales[PARAM_TIEMPO]);
   Storage.setParamRotacion(programa, fase, _valoresTemporales[PARAM_ROTACION]);

INTEGRACIÓN CON HARDWARE:
- Todos los comandos Nextion se envían automáticamente
- IDs de componentes definidos en config.h
- Limpieza de eventos garantizada en transiciones

INTEGRACIÓN CON PROGRAM CONTROLLER:
- Transiciones seguras desde cualquier estado
- Verificación de estabilidad de UI
- Callback automático después de guardar/cancelar
*/

// ===== MANTENIMIENTO Y EXTENSIBILIDAD =====

/*
PARA AGREGAR NUEVOS PARÁMETROS:
1. Definir en config.h: #define PARAM_NUEVO_PARAMETRO 4
2. Agregar límites: #define MIN_NUEVO_PARAMETRO, MAX_NUEVO_PARAMETRO
3. Actualizar funciones en config.cpp para incluir el nuevo parámetro
4. El UI Controller funcionará automáticamente

PARA MODIFICAR LÍMITES:
- Solo cambiar valores en config.h
- Las funciones de validación se ajustan automáticamente

PARA AGREGAR NUEVOS COMPONENTES NEXTION:
- Definir en config.h: #define NEXTION_COMP_NUEVO "nombre_componente"
- Usar en updateEditDisplay() o updateRightPanel()

PARA DEBUGGING:
- Todos los eventos importantes se loguean en Serial
- Verificar _modoEdicionActivo para estado de edición
- Usar isUIStable() para verificar estado de UI
*/