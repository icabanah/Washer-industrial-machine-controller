# Guía de Diseño de Pantallas Nextion

Este documento proporciona una guía para el diseño de las pantallas Nextion necesarias para el controlador de lavadora industrial.

## Nomenclatura Técnica

**IMPORTANTE:** Este documento utiliza los nombres técnicos oficiales de Nextion para las páginas:

- **Nombres técnicos:** `page0`, `page1`, `page2`, `page3`, `page4`, `page5`
- **Nombres descriptivos:** Solo para documentación y comprensión humana
- **IDs numéricos:** 0, 1, 2, 3, 4, 5 (correspondientes a cada page)

### ¿Por qué usar nombres técnicos?

Los nombres técnicos (`page0`, `page1`, etc.) son los que se utilizan realmente en:
1. **Editor Nextion** - Para crear y configurar las páginas
2. **Código ESP32** - Para enviar comandos de navegación  
3. **Comandos de comunicación** - Para cambiar entre páginas

### Equivalencia Completa

```
page0 = Página ID 0 = "Bienvenida"
page1 = Página ID 1 = "Selección de Programa"
page2 = Página ID 2 = "Ejecución de Programa"
page3 = Página ID 3 = "Edición de Programa"
page4 = Página ID 4 = "Error"
page5 = Página ID 5 = "Emergencia"
```

**Ejemplo de uso correcto:**
```cpp
// ✅ CORRECTO - Usar nombres técnicos en código
Hardware.nextionSendCommand("page page1");

// ❌ INCORRECTO - Los nombres descriptivos no funcionan
Hardware.nextionSendCommand("page Selección_de_Programa");
```

## Consideraciones Generales

- **Resolución**: Las pantallas se diseñarán para una resolución de 800x480 píxeles (pantalla Nextion NX8048P050-011 de 5.0")
- **Paleta de Colores**: Utilizar una paleta consistente para toda la interfaz
  - Fondo: Azul oscuro (#000050)
  - Texto principal: Blanco (#FFFFFF)
  - Botones activos: Verde (#00FF00)
  - Botones inactivos: Gris (#808080)
  - Alertas/Emergencia: Rojo (#FF0000)
- **Fuentes**: Utilizar fuentes legibles a distancia (ajustadas para 800x480)
  - Títulos: Arial, 32pt, Bold
  - Texto general: Arial, 20pt
  - Valores numéricos: Arial, 28pt, Bold

## Diseño de Pantallas

### Page 0: Bienvenida (page0)

![Ejemplo de Pantalla de Bienvenida](placeholder)

**Nombre técnico en Nextion:** `page0`
**Función:** Pantalla inicial del sistema que se muestra al encender

#### Componentes:
1. **lbl_titulo** - Texto (Título)
   - Texto: "RH Electronics"
   - Posición: Centro (x=360, y=150)
   - Fuente: Arial, 42pt, Bold
   - Color: Blanco

2. **lbl_subtitulo** - Texto (Subtítulo)
   - Texto: "Controlador de Lavadora Industrial"
   - Posición: Centro (x=360, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

3. **lbl_contacto** - Texto (Contacto)
   - Texto: "958970967"
   - Posición: Centro (x=360, y=280)
   - Fuente: Arial, 20pt
   - Color: Blanco

4. **anim** - Animación (Logo giratorio)
   - Posición: (x=360, y=80)
   - Tamaño: 120x120
   - Tipo: Gif animado

#### Configuración de Eventos (page0):
- **tm0** (temporizador): Configurar para 3000ms (3 segundos)
- **Acción automática**: Cambiar a `page1` después del tiempo configurado

**En Nextion Editor - Evento Timer (tm0):**
```
page page1
```

**Eventos generados:**
- Después de 3s → Cambio automático a `page1` (Selección de Programa)

### Page 1: Selección de Programa (page1)

![Ejemplo de Pantalla de Selección](placeholder)

**Nombre técnico en Nextion:** `page1`
**Función:** Selección de programa de lavado y visualización de parámetros

#### Componentes:
1. **t0** - Texto (Título)
   - Texto: "Selección de Programa"
   - Posición: Centro-Superior (x=360, y=30)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

2. **b1, b2, b3** - Botones (Programas)
   - Texto: "P22", "P23", "P24"  
   - Posición: Fila superior (y=90)
   - Tamaño: 120x60 cada uno
   - Espaciado: x=160, x=360, x=560
   - Color: Inactivo: Gris, Activo: Verde

3. **t10** - Texto (Programa seleccionado)
   - Texto: "P22"
   - Posición: (x=50, y=180)
   - Fuente: Arial, 24pt, Bold
   - Color: Blanco

4. **t11, t12, t13, t14** - Textos (Etiquetas de parámetros)
   - Texto: "Nivel:", "Temperatura:", "Tiempo:", "Rotación:"
   - Posición: Columna izquierda (x=50, y=220, 260, 300, 340)
   - Fuente: Arial, 20pt
   - Color: Blanco

5. **t21, t22, t23, t24** - Textos (Valores de parámetros)
   - Texto: Valores actuales del programa seleccionado
   - Ejemplos: "2", "45°C", "15 min", "Lento"
   - Posición: Columna derecha (x=160, y=220, 260, 300, 340)
   - Fuente: Arial, 20pt, Bold
   - Color: Verde (para destacar los valores)

6. **t15** - Texto (Información adicional)
   - Texto: Información de fases para P24
   - Posición: (x=50, y=380)
   - Fuente: Arial, 18pt
   - Color: Blanco

7. **b4** - Botón (Iniciar)
   - Texto: "INICIAR"
   - Posición: Inferior-derecha (x=610, y=400)
   - Tamaño: 130x60
   - Color: Verde

8. **b5** - Botón (Editar)
   - Texto: "EDITAR"
   - Posición: Inferior-derecha (x=460, y=400)
   - Tamaño: 130x60
   - Color: Azul

#### Configuración de Eventos (page1):
- **b1, b2, b3** (botones de programa): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)
- **b4** (INICIAR): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)  
- **b5** (EDITAR): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)

**Flujo de eventos:**
- Presionar b1 → Nextion envía ID del componente → ESP32 ejecuta lógica "PROGRAM_1"
- Presionar b2 → Nextion envía ID del componente → ESP32 ejecuta lógica "PROGRAM_2"  
- Presionar b3 → Nextion envía ID del componente → ESP32 ejecuta lógica "PROGRAM_3"
- Presionar b4 → Nextion envía ID del componente → ESP32 ejecuta lógica "START"
- Presionar b5 → Nextion envía ID del componente → ESP32 ejecuta lógica "EDIT"

**IMPORTANTE:** En Nextion NO escribir comandos como "PROGRAM_1" o "START". Los eventos se dejan vacíos y el ESP32 procesa las acciones.

### Page 2: Ejecución de Programa (page2)

![Ejemplo de Pantalla de Ejecución](placeholder)

**Nombre técnico en Nextion:** `page2`
**Función:** Monitoreo en tiempo real del programa de lavado en ejecución

#### Componentes:
1. **t1** - Texto (Programa)
   - Texto: "P22"
   - Posición: Superior-izquierda (x=50, y=30)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

2. **t2** - Texto (Fase)
   - Texto: "Fase: 1"
   - Posición: Superior-derecha (x=610, y=30)
   - Fuente: Arial, 24pt
   - Color: Blanco

3. **t3** - Texto (Tiempo)
   - Texto: "00:00"
   - Posición: Centro (x=360, y=100)
   - Fuente: Arial, 48pt, Bold
   - Color: Blanco

4. **j0** - Barra de Progreso
   - Posición: Centro (x=360, y=160)
   - Tamaño: 660x30
   - Color: Azul-Verde

5. **t4** - Texto (Temperatura)
   - Texto: "25°C"
   - Posición: (x=80, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

6. **t5** - Texto (Nivel de agua)
   - Texto: "Nivel: 2"
   - Posición: (x=360, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

7. **t6** - Texto (Velocidad)
   - Texto: "Vel: 1"
   - Posición: (x=640, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

8. **z0** - Indicador (Temperatura)
   - Tipo: Gauge
   - Posición: (x=80, y=280)
   - Tamaño: 120x120
   - Color: Rojo-Amarillo

9. **z1** - Indicador (Nivel de agua)
   - Tipo: Gauge
   - Posición: (x=360, y=280)
   - Tamaño: 120x120
   - Color: Azul

10. **motor** - Indicador (Motor)
    - Tipo: Imagen animada
    - Posición: (x=640, y=280)
    - Tamaño: 120x120

11. **b6** - Botón (Detener)
    - Texto: "DETENER"
    - Posición: Inferior-derecha (x=610, y=420)
    - Tamaño: 130x50
    - Color: Rojo

#### Configuración de Eventos (page2):
- **b6** (DETENER): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)

**Flujo de eventos:**
- Presionar b6 → Nextion envía ID del componente → ESP32 recibe evento → ESP32 ejecuta lógica de "STOP"

**IMPORTANTE:** En Nextion NO escribir comandos como "STOP". El evento se deja vacío y el ESP32 procesa la acción.

### Page 3: Edición de Programa (page3)

![Ejemplo de Pantalla de Edición](placeholder)

**Nombre técnico en Nextion:** `page3`
**Función:** Modificación de parámetros específicos de cada programa y fase

#### Componentes:
1. **t1** - Texto (Título)
   - Texto: "P22 - F1"
   - Posición: Superior-centro (x=360, y=40)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

2. **t2** - Texto (Etiqueta variable)
   - Texto: "Nivel de agua"
   - Posición: (x=360, y=120)
   - Fuente: Arial, 24pt
   - Color: Blanco

3. **t3** - Texto (Valor)
   - Texto: "2"
   - Posición: (x=360, y=180)
   - Fuente: Arial, 42pt, Bold
   - Color: Blanco

4. **h0** - Slider
   - Posición: Centro (x=360, y=260)
   - Tamaño: 560x60
   - Rango: 0-4 (min-max)
   - Valor inicial: 2

5. **b7** - Botón (Guardar)
   - Texto: "GUARDAR"
   - Posición: Inferior-derecha (x=610, y=380)
   - Tamaño: 130x60
   - Color: Verde

6. **b8** - Botón (Cancelar)
   - Texto: "CANCELAR"
   - Posición: Inferior-izquierda (x=110, y=380)
   - Tamaño: 130x60
   - Color: Rojo

#### Configuración de Eventos (page3):
- **h0** (slider): Touch Release Event → **VACÍO** (solo Send Component ID habilitado)
- **b7** (GUARDAR): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)
- **b8** (CANCELAR): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)

**Flujo de eventos:**
- Cambiar h0 → Nextion envía ID del componente → ESP32 ejecuta lógica "VALUE_CHANGE"
- Presionar b7 → Nextion envía ID del componente → ESP32 ejecuta lógica "SAVE"
- Presionar b8 → Nextion envía ID del componente → ESP32 ejecuta lógica "CANCEL"

**IMPORTANTE:** En Nextion NO escribir comandos como "SAVE" o "CANCEL". Los eventos se dejan vacíos y el ESP32 procesa las acciones.

### Page 4: Error (page4)

![Ejemplo de Pantalla de Error](placeholder)

**Nombre técnico en Nextion:** `page4`
**Función:** Visualización de errores del sistema y opciones de recuperación

#### Componentes:
1. **t1** - Texto (Título de error)
   - Texto: "ERROR 400"
   - Posición: Centro-superior (x=360, y=120)
   - Fuente: Arial, 48pt, Bold
   - Color: Rojo

2. **t2** - Texto (Descripción de error)
   - Texto: "Error de sistema"
   - Posición: Centro (x=360, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

3. **alerta** - Indicador de alerta
   - Tipo: Imagen animada
   - Posición: Centro-superior (x=360, y=40)
   - Tamaño: 80x80
   - Animación: Parpadeo

4. **b9** - Botón (Reiniciar)
   - Texto: "REINICIAR"
   - Posición: Centro-inferior (x=360, y=320)
   - Tamaño: 200x70
   - Color: Azul

#### Configuración de Eventos (page4):
- **b9** (REINICIAR): Touch Press Event → **VACÍO** (solo Send Component ID habilitado)

**Flujo de eventos:**
- Presionar b9 → Nextion envía ID del componente → ESP32 ejecuta lógica "RESTART"

**IMPORTANTE:** En Nextion NO escribir comandos como "RESTART". El evento se deja vacío y el ESP32 procesa la acción.

### Page 5: Emergencia (page5)

![Ejemplo de Pantalla de Emergencia](placeholder)

**Nombre técnico en Nextion:** `page5`
**Función:** Pantalla de parada de emergencia activada por botón físico

#### Componentes:
1. **t1** - Texto (Título)
   - Texto: "PARADA DE EMERGENCIA"
   - Posición: Centro-superior (x=360, y=120)
   - Fuente: Arial, 36pt, Bold
   - Color: Rojo

2. **t2** - Texto (Instrucción)
   - Texto: "Sistema detenido por seguridad"
   - Posición: Centro (x=360, y=220)
   - Fuente: Arial, 24pt
   - Color: Blanco

3. **t3** - Texto (Instrucción)
   - Texto: "Suelte el botón de emergencia para continuar"
   - Posición: Centro-inferior (x=360, y=300)
   - Fuente: Arial, 20pt
   - Color: Blanco

4. **alarm** - Indicador de alarma
   - Tipo: Imagen animada
   - Posición: Centro-superior (x=360, y=40)
   - Tamaño: 120x120
   - Animación: Parpadeo rojo

#### Configuración de Eventos (page5):
- **Sin eventos de usuario**: Esta página se activa automáticamente por hardware (botón físico de emergencia)
- **Activación**: El ESP32 detecta el botón de emergencia y cambia automáticamente a `page5`
- **Desactivación**: El sistema vuelve a `page1` cuando se libera el botón de emergencia

**Flujo de eventos:**
- Ninguno (página controlada por hardware, no por interfaz táctil)

## ⚠️ **REGLA FUNDAMENTAL: División de Responsabilidades**

### **❌ NUNCA escribir en Nextion:**
```javascript
// En eventos de botones - ESTOS COMANDOS NO EXISTEN EN NEXTION:
STOP
START
PROGRAM_1
PROGRAM_2
SAVE
CANCEL
RESTART
```

### **✅ SIEMPRE configurar en Nextion:**
```javascript
// En eventos de botones - CONFIGURACIÓN CORRECTA:
Touch Press Event: [DEJAR VACÍO]
Send Component ID: ✅ Habilitado
```

### **🧠 El ESP32 procesa las acciones:**
```cpp
// En ui_controller.cpp - AQUÍ van las acciones:
if (event[1] == NEXTION_COMP_BTN_STOP[0]) {
    _lastUserAction = "STOP";        // ← Aquí SÍ va "STOP"
    _userActionPending = true;
}
```

### **📡 Flujo Completo Correcto:**
```
Usuario presiona botón → Nextion envía ID componente → ESP32 recibe evento → ESP32 ejecuta acción correspondiente
```

**RECUERDA:** Nextion es solo la pantalla táctil. El ESP32 es el cerebro que toma las decisiones.

### Componentes Globales: Sistema de Mensajes

**IMPORTANTE:** En Nextion, NO existen componentes verdaderamente "globales" que aparezcan en todas las páginas automáticamente. Hay **dos opciones** para implementar mensajes que aparezcan en cualquier página:

#### **Opción 1: Duplicar en Páginas Necesarias (RECOMENDADO)**

**¿Dónde agregar el cuadro de mensaje?**
- **page1** (Selección) - Para mensajes como "Configuración guardada"
- **page2** (Ejecución) - Para mensajes como "Programa completado"  
- **page3** (Edición) - Para mensajes como "Valor guardado"

**¿Dónde NO agregar?**
- **page0** (Bienvenida) - No necesario, se cambia automáticamente
- **page4** (Error) - Ya tiene su propio sistema de mensajes
- **page5** (Emergencia) - No necesario, es crítica

#### **Componentes a Duplicar (solo en page1, page2, page3):**

1. **msgBox** - Panel de mensaje (**Tipo: Crop/Panel**)
   - **Tipo en Nextion:** Crop (recorte) o Panel
   - Visible: No (inicialmente)
   - Posición: Centro (x=360, y=240)
   - Tamaño: 560x120
   - Color de fondo: Negro (semi-transparente)
   - Borde: Blanco
   - **Función:** Crear el fondo/contenedor del mensaje

2. **msgText** - Texto del mensaje (**Tipo: Text**)
   - **Tipo en Nextion:** Text
   - Posición: Centro del panel (dentro de msgBox)
   - Fuente: Arial, 20pt
   - Color: Blanco
   - Alineación: Centro
   - **Función:** Mostrar el contenido del mensaje

#### **Opción 2: Solo en Páginas Específicas (ALTERNATIVA)**

Si prefieres mantenerlo simple, agrega el cuadro de mensaje **solo en las páginas donde realmente lo necesites**:

- **page1**: Para confirmaciones de selección
- **page3**: Para confirmaciones de guardado

#### **Cómo Implementar en Editor Nextion:**

**Paso 1:** En page1, agregar:
- Componente Crop: nombre "msgBox"
- Componente Text: nombre "msgText" (dentro de msgBox)

**Paso 2:** Copiar y pegar los mismos componentes en page2 y page3

**Paso 3:** Mantener los mismos nombres en todas las páginas para que el código ESP32 funcione igual

#### **Control desde ESP32 (funciona en cualquier página que tenga los componentes):**

```cpp
// Mostrar mensaje temporal
UIController.showMessage("Configuración guardada", 2000);

// Implementación en ui_controller.cpp:
void UIControllerClass::showMessage(const String& message, uint16_t duration) {
  // Establecer texto del mensaje
  Hardware.nextionSetText("msgText", message);
  
  // Mostrar el cuadro de mensaje
  Hardware.nextionSendCommand("msgBox.vis=1");
  
  // Programar ocultado automático después del tiempo especificado
  _messageActive = true;
  _messageTimestamp = millis();
}

// En el loop de processEvents():
if (_messageActive && (millis() - _messageTimestamp > 2000)) {
  _messageActive = false;
  Hardware.nextionSendCommand("msgBox.vis=0"); // Ocultar mensaje
}
```

## Navegación entre Páginas

### Comandos de Cambio de Página

Para cambiar entre páginas desde el ESP32, usar los siguientes comandos:

```cpp
// Cambiar a página específica
Hardware.nextionSendCommand("page page0");  // Ir a Bienvenida
Hardware.nextionSendCommand("page page1");  // Ir a Selección
Hardware.nextionSendCommand("page page2");  // Ir a Ejecución
Hardware.nextionSendCommand("page page3");  // Ir a Edición
Hardware.nextionSendCommand("page page4");  // Ir a Error
Hardware.nextionSendCommand("page page5");  // Ir a Emergencia
```

### Tabla de Correspondencia de Páginas

| **Nombre Técnico** | **Nombre Descriptivo** | **ID Numérico** | **Función Principal** | **Eventos Principales** |
|-------------------|------------------------|------------------|----------------------|------------------------|
| `page0` | Bienvenida | 0 | Pantalla inicial del sistema | Timer automático (3s) |
| `page1` | Selección de Programa | 1 | Elegir programa de lavado | b1, b2, b3, b4, b5 |
| `page2` | Ejecución de Programa | 2 | Monitorear lavado activo | b6 (Detener) |
| `page3` | Edición de Programa | 3 | Modificar parámetros | h0, b7, b8 |
| `page4` | Error | 4 | Mostrar errores del sistema | b9 (Reiniciar) |
| `page5` | Emergencia | 5 | Parada de emergencia | Sin eventos (automática) |

### Resumen de Eventos por Página

**page1 (Selección):**
- **b1, b2, b3:** Selección de programas P22, P23, P24
- **b4:** Botón INICIAR 
- **b5:** Botón EDITAR

**page2 (Ejecución):**
- **b6:** Botón DETENER

**page3 (Edición):**
- **h0:** Slider para ajustar valores
- **b7:** Botón GUARDAR
- **b8:** Botón CANCELAR

**page4 (Error):**
- **b9:** Botón REINICIAR

**page5 (Emergencia):**
- Sin eventos de usuario (se activa automáticamente)

### Flujo de Navegación Típico

```
page0 (Bienvenida)
    ↓ (automático después de 3s)
page1 (Selección)
    ↓ (usuario presiona INICIAR)
page2 (Ejecución)
    ↓ (al finalizar o usuario presiona DETENER)
page1 (Selección)

// Flujos alternativos:
page1 → page3 (usuario presiona EDITAR)
page3 → page1 (usuario presiona GUARDAR/CANCELAR)
cualquier_página → page4 (error del sistema)
cualquier_página → page5 (botón emergencia físico)
```

## Técnicas de Diseño Avanzado

### Animaciones
- Utilizar GIFs animados para mostrar actividad del motor
- Implementar parpadeo para alertas mediante temporizadores
- Transiciones entre páginas para una experiencia más fluida

### Visualización de Datos
- Utilizar indicadores de nivel para mostrar el agua
- Gráficos de temperatura con códigos de color
- Barras de progreso para visualizar el avance del programa

### Mejora de la Experiencia de Usuario
1. **Consistencia Visual**: Mantener el mismo esquema de colores y posiciones en todas las pantallas
2. **Retroalimentación**: Proporcionar confirmación visual para cada acción del usuario
3. **Navegación Intuitiva**: Botones claramente etiquetados y posicionados lógicamente
4. **Legibilidad**: Fuentes y tamaños adecuados para visualización a distancia
5. **Adaptabilidad**: Diseño que funcione bien bajo diferentes condiciones de iluminación

## Consideraciones para la Programación

### ⚠️ **REGLA MÁS IMPORTANTE: División de Responsabilidades**

**🖥️ Nextion (Pantalla Táctil):**
- Solo maneja visualización y envío de eventos
- Comandos válidos: `page`, `vis`, `txt`, `val`, `pic`
- **NUNCA** poner comandos como: `STOP`, `START`, `PROGRAM_1`, `SAVE`, etc.

**🧠 ESP32 (Controlador):**
- Procesa la lógica de negocio
- Interpreta eventos de Nextion
- Controla hardware real (motores, válvulas, sensores)
- **AQUÍ SÍ** van las acciones como "STOP", "START", etc.

**✅ Configuración correcta de eventos en Nextion:**
```javascript
// En TODOS los botones:
Touch Press Event: [DEJAR VACÍO]
Send Component ID: ✅ Habilitado
```

1. **Nomenclatura de Páginas**: Usar siempre los nombres técnicos (`page0`, `page1`, etc.) en el código del ESP32 y en el Editor Nextion para mantener consistencia
2. **Eficiencia de Memoria**: Reutilizar imágenes y fuentes cuando sea posible
3. **Optimización de Comunicación**: Minimizar el número de actualizaciones en componentes estáticos
4. **Manejo de Errores**: Mostrar mensajes claros y específicos
5. **Coordinación con el Código**: Mantener sincronizados los IDs de componentes entre el Nextion Editor y las constantes en config.h del ESP32
6. **Pruebas Exhaustivas**: Verificar que todos los componentes funcionan correctamente en cada página

### Sincronización ESP32 - Nextion

**En config.h (ESP32):**
```cpp
#define NEXTION_PAGE_WELCOME 0      // Corresponde a page0
#define NEXTION_PAGE_SELECTION 1    // Corresponde a page1  
#define NEXTION_PAGE_EXECUTION 2    // Corresponde a page2
#define NEXTION_PAGE_EDIT 3         // Corresponde a page3
#define NEXTION_PAGE_ERROR 4        // Corresponde a page4
#define NEXTION_PAGE_EMERGENCY 5    // Corresponde a page5
```

**En Nextion Editor:**
- Asegurarse de que el orden de las páginas coincida con los IDs numéricos
- Nombrar las páginas como `page0`, `page1`, `page2`, `page3`, `page4`, `page5`
- Verificar que los componentes tengan IDs consistentes con las constantes del ESP32

## Exportación y Carga del Diseño

1. Compilar el proyecto en el Editor Nextion
2. Verificar que no haya errores de compilación
3. Exportar el archivo .tft
4. Cargar el archivo a la pantalla mediante:
   - Tarjeta microSD: Copiar el archivo .tft a la raíz, insertar en la pantalla y encender
   - Cable Serie: Utilizar la función de carga del Editor Nextion con un adaptador UART-USB

## Conclusión

El diseño adecuado de las pantallas Nextion es crucial para una experiencia de usuario satisfactoria. Siguiendo estas guías actualizadas para la pantalla NX8048P050-011 (800x480 píxeles), se obtendrá una interfaz profesional, intuitiva y funcional para el controlador de lavadora industrial.

**NOTAS IMPORTANTES:**

1. **⚠️ CRÍTICO - División de Responsabilidades:** En Nextion NUNCA escribir comandos como "STOP", "START", "PROGRAM_1", etc. Solo dejar eventos vacíos con "Send Component ID" habilitado. El ESP32 procesa toda la lógica.

2. **Nomenclatura Técnica:** Este documento usa los nombres técnicos oficiales (`page0`, `page1`, etc.) que deben utilizarse tanto en el Editor Nextion como en el código ESP32.

3. **Resolución Específica:** Todas las posiciones, tamaños de fuente y dimensiones de componentes han sido calculadas específicamente para la pantalla NX8048P050-011 de 5.0" con resolución 800x480 píxeles.

4. **Sincronización Código-Diseño:** Mantener siempre sincronizados los nombres de páginas y componentes entre el diseño Nextion y las constantes del código ESP32.

5. **Flujo de Navegación:** El sistema implementa un flujo lógico de navegación que va desde `page0` (bienvenida) hasta `page2` (ejecución) con opciones de edición (`page3`) y manejo de errores (`page4`, `page5`).

### Siguiente Paso Recomendado

**ANTES de implementar en Editor Nextion:**

1. **✅ Verificar** que todas las constantes en `config.h` del ESP32 estén alineadas con los nombres de componentes definidos en este documento

2. **✅ Recordar** la regla fundamental: En Nextion solo configurar "Send Component ID", NUNCA escribir comandos como "STOP" o "START"

3. **✅ Compilar** sin errores verificando que no hay comandos inválidos en los eventos

4. **✅ Probar** la comunicación ESP32-Nextion antes de implementar toda la interfaz

**¡Siguiendo estas guías evitarás el error "Invalid command" que encontraste!** 🎯
