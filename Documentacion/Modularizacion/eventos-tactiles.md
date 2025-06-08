# Sistema de Eventos Táctiles - Controlador de Lavadora Industrial

## 🎯 Resumen

Se ha implementado un sistema completo de captura y procesamiento de eventos táctiles desde la pantalla Nextion hacia el ESP32. Este sistema permite que el usuario modifique los parámetros de cada programa (Nivel, Temperatura, Tiempo, Rotación) mediante botones táctiles en la interfaz.

## 📋 Componentes Implementados

### 1. **Módulo Hardware (hardware.cpp/h)**
- ✅ **Interpretación mejorada de eventos táctiles**: La función `_readNextionResponse()` ahora decodifica correctamente los códigos de evento táctil de Nextion (formato: `0x65 [PageID] [ComponentID] [EventType]`)
- ✅ **Funciones específicas para eventos**:
  - `hasValidTouchEvent()`: Verifica si hay un evento táctil válido
  - `getTouchEventPage()`: Obtiene la página del evento
  - `getTouchEventComponent()`: Obtiene el ID del componente
  - `getTouchEventType()`: Obtiene el tipo de evento (0=liberado, 1=presionado)

### 2. **Módulo Program Controller (program_controller.cpp/h)**
- ✅ **Procesamiento de eventos por página**: Nuevo método `processUserEvent()` que interpreta eventos según la página activa
- ✅ **Manejo específico por pantalla**:
  - `_handleSelectionPageEvents()`: Eventos en pantalla de selección de programa
  - `_handleEditPageEvents()`: Eventos en pantalla de edición de parámetros
  - `_handleExecutionPageEvents()`: Eventos en pantalla de ejecución
- ✅ **Edición completa de parámetros**:
  - Navegación entre parámetros (anterior/siguiente)
  - Incremento/decremento de valores con límites
  - Guardado en memoria persistente

### 3. **Configuración (config.h)**
- ✅ **IDs de componentes**: Definición de los IDs numéricos de cada botón
- ✅ **Límites de parámetros**: Valores mínimos y máximos para cada tipo de parámetro
- ✅ **Constantes de parámetros**: Definición de tipos de parámetros editables

## 🔧 Configuración Requerida en Nextion Editor

### **IMPORTANTE: Configurar Touch Press Event**
Para que los botones envíen eventos táctiles al ESP32, cada botón debe configurarse en Nextion Editor:

1. **Seleccionar el botón** en Nextion Editor
2. **En Properties**, buscar **Touch Press Event**
3. **Marcar la casilla** para habilitar el evento
4. **NO escribir código** en el campo de evento (dejar vacío)
5. **Repetir para todos los botones** que necesiten enviar eventos

### **Mapeo de IDs de Componentes**

#### Página 1 - Selección de Programa
```
ID  | Componente               | Función
----|--------------------------|---------------------------
1   | Botón "<"               | Programa anterior
2   | Botón ">"               | Programa siguiente  
3   | Botón "Comenzar"        | Iniciar programa
4   | Botón "Editar"          | Entrar en modo edición
```

#### Página 3 - Edición de Parámetros
```
ID  | Componente               | Función
----|--------------------------|---------------------------
5   | Botón "-"               | Disminuir parámetro
6   | Botón "+"               | Aumentar parámetro
7   | Botón "Param <"         | Parámetro anterior
8   | Botón "Param >"         | Parámetro siguiente
9   | Botón "Guardar"         | Guardar cambios
10  | Botón "Cancelar"        | Cancelar edición
```

#### Página 2 - Ejecución
```
ID  | Componente               | Función
----|--------------------------|---------------------------
11  | Botón "Pausar"          | Pausar/reanudar programa
12  | Botón "Parar"           | Detener programa
```

**NOTA**: Los IDs pueden variar según el orden de creación en Nextion Editor. Verificar y ajustar las constantes en `config.h` si es necesario.

## 🎮 Funcionalidad Implementada

### **Selección de Programa**
- **Botones de navegación**: Cambiar entre programas 22, 23, 24
- **Botón Comenzar**: Iniciar el programa seleccionado
- **Botón Editar**: Entrar en modo de edición de parámetros

### **Edición de Parámetros**
- **Navegación de parámetros**: Cambiar entre Nivel, Temperatura, Tiempo, Rotación
- **Modificación de valores**: Botones +/- con respeto a límites configurados
- **Guardado persistente**: Los cambios se almacenan en la memoria Preferences del ESP32
- **Validación de límites**:
  - Nivel: 1-4
  - Temperatura: 20-90°C
  - Tiempo: 1-60 minutos
  - Rotación: 1-3

### **Control de Ejecución**
- **Pausar/Reanudar**: Control de la ejecución del programa
- **Parar**: Detener completamente el programa

## 🔍 Debug y Diagnóstico

El sistema incluye logging detallado para facilitar el debug:

```cpp
// Ejemplos de mensajes de debug
"🔍 Evento Nextion Raw [4 bytes]: 0x65 0x01 0x03 0x01"
"✅ EVENTO TÁCTIL DETECTADO:"
"   Página: 1"
"   Componente: 3" 
"   Tipo: 1 (PRESIONADO)"
"🔘 Evento en página de selección - Componente: 3"
"▶️ Iniciando programa 22"
```

## 📊 Estado de Implementación

- ✅ **Hardware**: Captura e interpretación de eventos táctiles
- ✅ **Program Controller**: Procesamiento completo de eventos
- ✅ **Edición de parámetros**: Sistema completo con validación
- ✅ **Configuración**: IDs y límites definidos
- ⏳ **Pendiente**: Verificación de IDs en archivo .hmi real
- ⏳ **Pendiente**: Pruebas con hardware físico

## 🚀 Próximos Pasos

1. **Verificar IDs de componentes** en el archivo .hmi de la pantalla Nextion
2. **Ajustar constantes** en `config.h` si los IDs no coinciden
3. **Probar la funcionalidad** con hardware real
4. **Optimizar la experiencia de usuario** según los resultados de las pruebas

## 💡 Notas Técnicas

- **Formato de eventos Nextion**: `0x65 [PageID] [ComponentID] [EventType] 0xFF 0xFF 0xFF`
- **Solo se procesan eventos de botón presionado** (EventType == 1)
- **Los eventos se procesan según la página activa**, no según el estado del programa
- **Todos los cambios se guardan automáticamente** en la memoria Preferences del ESP32
- **El sistema es robusto ante eventos duplicados** o no válidos
