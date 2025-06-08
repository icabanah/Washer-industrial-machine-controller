// === CORRECCIONES DE IDENTIFICADORES NEXTION - UI CONTROLLER ===
// Documentación de errores corregidos en identificadores de componentes Nextion
// Proyecto: Controlador de Lavadora Industrial - ESP32 + Nextion
// Autor: Ingeniero Electrónico Senior

/*
ERRORES ENCONTRADOS Y CORREGIDOS:

El problema principal era que ui_controller.cpp usaba nombres de componentes
que no estaban definidos en config.h, causando errores de compilación.

ERRORES ESPECÍFICOS:
1. "el identificador NEXTION_COMP_VAL_TEMP no está definido"
2. "el identificador NEXTION_COMP_VAL_NIVEL no está definido"
3. "el identificador NEXTION_COMP_VAL_ROTACION no está definido"
4. "el identificador NEXTION_COMP_VAL_TIEMPO no está definido"
5. "el identificador NEXTION_COMP_BTN_STOP no está definido"
6. "el identificador NEXTION_COMP_INFO_FASES no está definido"

SOLUCIÓN IMPLEMENTADA:
Se agregaron las definiciones faltantes en config.h usando una estrategia de aliases
que mantiene compatibilidad con diferentes nombres de componentes.
*/

// ===== ESTRATEGIA DE ALIASES IMPLEMENTADA =====

/*
PROBLEMA ORIGINAL:
- En config.h: #define NEXTION_COMP_SEL_TEMP "val_temp"
- En ui_controller.cpp: Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, texto);
- Resultado: Error de compilación porque NEXTION_COMP_VAL_TEMP no existe

SOLUCIÓN CON ALIASES:
- En config.h se agregó: #define NEXTION_COMP_VAL_TEMP NEXTION_COMP_SEL_TEMP
- Ahora ui_controller.cpp puede usar ambos nombres indistintamente
- Mantiene compatibilidad hacia atrás y hacia adelante

ALIASES AGREGADOS:
1. NEXTION_COMP_VAL_NIVEL → NEXTION_COMP_SEL_NIVEL
2. NEXTION_COMP_VAL_TEMP → NEXTION_COMP_SEL_TEMP  
3. NEXTION_COMP_VAL_TIEMPO → NEXTION_COMP_SEL_TIEMPO
4. NEXTION_COMP_VAL_ROTACION → NEXTION_COMP_SEL_ROTACION
5. NEXTION_COMP_BTN_STOP → NEXTION_COMP_BTN_PARAR
6. NEXTION_COMP_INFO_FASES → "lbl_fases" (nueva definición)
*/

// ===== VENTAJAS DE LA ESTRATEGIA DE ALIASES =====

/*
1. COMPATIBILIDAD TOTAL:
   - Código existente sigue funcionando sin cambios
   - Nuevos desarrollos pueden usar cualquier convención de nombres
   - No requiere refactoring masivo de código existente

2. FLEXIBILIDAD:
   - Permite diferentes estilos de nomenclatura (SEL_, VAL_, etc.)
   - Facilita colaboración entre diferentes desarrolladores
   - Reduce errores por inconsistencias de nombres

3. MANTENIBILIDAD:
   - Un solo lugar para definir cada componente físico
   - Fácil cambio de nombres de componentes en la pantalla Nextion
   - Documentación clara de qué alias apunta a qué componente

4. EXTENSIBILIDAD:
   - Fácil agregar nuevos aliases cuando sea necesario
   - Soporte para diferentes versiones de interfaz
   - Preparado para futuras expansiones de la UI
*/

// ===== MAPA DE COMPONENTES NEXTION =====

/*
PÁGINA 1 - SELECCIÓN:
Físico en Nextion    | Definición Principal      | Aliases
---------------------|---------------------------|------------------
val_nivel           | NEXTION_COMP_SEL_NIVEL    | NEXTION_COMP_VAL_NIVEL
val_temp            | NEXTION_COMP_SEL_TEMP     | NEXTION_COMP_VAL_TEMP
val_tiempo          | NEXTION_COMP_SEL_TIEMPO   | NEXTION_COMP_VAL_TIEMPO
val_rotac           | NEXTION_COMP_SEL_ROTACION | NEXTION_COMP_VAL_ROTACION
lbl_fases           | NEXTION_COMP_INFO_FASES   | -

PÁGINA 2 - EJECUCIÓN:
Físico en Nextion    | Definición Principal      | Aliases
---------------------|---------------------------|------------------
btnParar            | NEXTION_COMP_BTN_PARAR    | NEXTION_COMP_BTN_STOP
gauge_temp          | NEXTION_COMP_GAUGE_TEMP_EJECUCION | -
barra_nivel         | NEXTION_COMP_BARRA_NIVEL_EJECUCION | -

PÁGINA 3 - EDICIÓN:
Físico en Nextion    | Definición Principal      | Aliases
---------------------|---------------------------|------------------
val_nivel           | NEXTION_COMP_VAL_NIVEL_EDIT | NEXTION_COMP_VAL_NIVEL
val_temp            | NEXTION_COMP_VAL_TEMP_EDIT  | NEXTION_COMP_VAL_TEMP
val_tiempo          | NEXTION_COMP_VAL_TIEMPO_EDIT| NEXTION_COMP_VAL_TIEMPO
val_rotac           | NEXTION_COMP_VAL_ROTAC_EDIT | NEXTION_COMP_VAL_ROTACION
*/

// ===== EJEMPLOS DE USO =====

/*
ANTES DE LA CORRECCIÓN (ERROR):
Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, "45°C");
// Error: NEXTION_COMP_VAL_TEMP no está definido

DESPUÉS DE LA CORRECCIÓN (FUNCIONA):
Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, "45°C");
// Funciona: NEXTION_COMP_VAL_TEMP es alias de NEXTION_COMP_SEL_TEMP

TAMBIÉN FUNCIONA:
Hardware.nextionSetText(NEXTION_COMP_SEL_TEMP, "45°C");
// Funciona: Usando la definición principal directamente

FLEXIBILIDAD:
// Ambas líneas hacen exactamente lo mismo:
Hardware.nextionSetText(NEXTION_COMP_VAL_TEMP, temperatura);
Hardware.nextionSetText(NEXTION_COMP_SEL_TEMP, temperatura);
*/

// ===== RECOMENDACIONES PARA FUTUROS DESARROLLOS =====

/*
1. USAR ALIASES CONSISTENTEMENTE:
   - Para nuevos desarrollos, usar preferiblemente los aliases más cortos
   - NEXTION_COMP_VAL_* para valores mostrados al usuario
   - NEXTION_COMP_BTN_* para botones
   - NEXTION_COMP_LBL_* para etiquetas estáticas

2. DOCUMENTAR NUEVOS COMPONENTES:
   - Siempre documentar qué componente físico representa cada definición
   - Incluir el nombre exacto como aparece en el archivo .hmi de Nextion
   - Especificar en qué página se encuentra el componente

3. MANTENER CONSISTENCIA:
   - Si se agrega un nuevo componente similar, seguir la misma convención
   - Crear aliases apropiados para mantener flexibilidad
   - Actualizar la documentación de mapeo de componentes

4. VERIFICAR COMPATIBILIDAD:
   - Antes de cambiar definiciones existentes, verificar uso en todo el código
   - Usar aliases en lugar de eliminar definiciones antiguas
   - Probar compilación después de cada cambio en config.h
*/

// ===== TROUBLESHOOTING =====

/*
SÍNTOMAS DE PROBLEMAS SIMILARES:
- Error: "el identificador NEXTION_COMP_* no está definido"
- Error de compilación en ui_controller.cpp, hardware.cpp, o program_controller.cpp
- Referencias a componentes que "deberían existir" pero no compilan

PASOS PARA RESOLVER:
1. Identificar el nombre exacto del componente que falta
2. Verificar si existe una definición similar en config.h
3. Agregar el componente faltante o crear un alias apropiado
4. Documentar el nuevo componente en este archivo
5. Verificar que el componente exista realmente en el archivo .hmi

HERRAMIENTAS ÚTILES:
- Buscar en config.h: Ctrl+F para encontrar definiciones similares
- Buscar en todo el proyecto: Para ver dónde se usa cada componente
- Editor Nextion: Para verificar nombres exactos de componentes
*/