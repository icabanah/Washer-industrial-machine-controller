# Flujo de Ejecución de Programas del Controlador de Lavadora Industrial

## Introducción

Este documento describe en detalle el flujo de ejecución de cada programa disponible en el controlador de lavadora industrial basado en ESP32. Se explica la secuencia de operaciones, los parámetros específicos de cada fase y las diferencias entre los tres programas predefinidos.

## Estructura General de Programas

El sistema cuenta con **3 programas** predefinidos, cada uno diseñado para diferentes tipos de lavado:

1. **Programa 22**: Lavado con agua caliente con gestión activa de temperatura
2. **Programa 23**: Lavado con agua fría sin gestión de temperatura
3. **Programa 24**: Lavado configurable (agua caliente o fría) con múltiples tandas

Cada programa está compuesto por una serie de fases que varían según el programa específico:

* **Fases estándar** (Programas 22 y 23): 
  1. Llenado
  2. Lavado
  3. Drenaje
  4. Centrifugado (opcional)

* **Programa 24**: 
  - 4 tandas, cada una compuesta por las 3 fases básicas (llenado, lavado, drenaje)
  - Fase final de centrifugado (opcional)

## Parámetros de Control

Cada programa está definido por varios parámetros configurables:

1. **Nivel de agua** (valores 1-4):
   - 1: Nivel bajo
   - 2: Nivel medio-bajo
   - 3: Nivel medio-alto
   - 4: Nivel alto

2. **Temperatura** (en °C):
   - Rango típico: 30°C - 70°C
   - Tolerancia: ±2°C (en programas con control de temperatura)

3. **Tiempo** (en minutos):
   - Define la duración de cada fase

4. **Rotación** (valores 1-3):
   - 1: Rotación suave (menos RPM, tiempos de pausa más largos)
   - 2: Rotación media
   - 3: Rotación intensa (más RPM, tiempos de pausa más cortos)

5. **Tipo de agua** (solo en Programa 24):
   - Caliente: Con gestión activa de temperatura
   - Fría: Sin gestión de temperatura

6. **Centrifugado** (configurable en todos los programas):
   - Habilitar/deshabilitar fase final de centrifugado
   - Intensidad de centrifugado

## Programa 22: Lavado con Agua Caliente

### Características Principales
- Utiliza **exclusivamente agua caliente**
- Implementa **gestión activa de temperatura** 
- Consta de **3 fases principales** más una fase opcional de centrifugado
- Todos los parámetros son configurables

### Flujo de Ejecución

1. **Fase 1: Llenado**
   - Solo se abre la válvula de agua caliente
   - El llenado continúa hasta alcanzar el nivel configurado
   - El sistema monitorea la temperatura constantemente
   - Si la temperatura del agua desciende más de 2°C por debajo del valor configurado:
     * Se drena parcialmente el agua
     * Se introduce nueva agua caliente
     * Este proceso mantiene la temperatura dentro del rango de tolerancia

2. **Fase 2: Lavado**
   - Activación del motor con el patrón de rotación configurado
   - Mantenimiento activo de la temperatura durante todo el ciclo
   - Monitoreo constante del nivel de agua
   - El temporizador de lavado se ejecuta durante el tiempo configurado

3. **Fase 3: Drenaje**
   - Se abre la válvula de drenaje hasta evacuación completa del agua
   - El motor puede continuar funcionando a baja velocidad para facilitar el drenaje
   - El sistema monitorea el sensor de presión para confirmar el vaciado completo

4. **Fase 4: Centrifugado (opcional)**
   - Si está habilitado, se activa el modo de centrifugado
   - La intensidad y duración dependen de la configuración
   - El motor funciona a alta velocidad para extraer la humedad residual

### Control de Temperatura
- Durante las fases de llenado y lavado, el sistema mantiene la temperatura configurada con una tolerancia de ±2°C
- Si la temperatura desciende, se drena parcialmente y se agrega agua caliente nueva
- La válvula de agua fría no se utiliza en este programa

## Programa 23: Lavado con Agua Fría

### Características Principales
- Utiliza **exclusivamente agua fría**
- **No realiza gestión de temperatura** (el sensor solo cumple función informativa)
- Consta de **3 fases principales** más una fase opcional de centrifugado
- Todos los parámetros son configurables excepto el tipo de agua

### Flujo de Ejecución

1. **Fase 1: Llenado**
   - Solo se abre la válvula de agua fría
   - El llenado continúa hasta alcanzar el nivel configurado
   - El sistema monitorea la temperatura solo con fines informativos
   - No hay control activo de la temperatura del agua

2. **Fase 2: Lavado**
   - Activación del motor con el patrón de rotación configurado
   - Monitoreo constante del nivel de agua
   - El temporizador de lavado se ejecuta durante el tiempo configurado
   - La temperatura se muestra pero no se controla

3. **Fase 3: Drenaje**
   - Se abre la válvula de drenaje hasta evacuación completa del agua
   - El motor puede continuar funcionando a baja velocidad para facilitar el drenaje
   - El sistema monitorea el sensor de presión para confirmar el vaciado completo

4. **Fase 4: Centrifugado (opcional)**
   - Si está habilitado, se activa el modo de centrifugado
   - La intensidad y duración dependen de la configuración
   - El motor funciona a alta velocidad para extraer la humedad residual

### Monitoreo de Temperatura
- La temperatura del agua se mide y muestra en la interfaz
- No hay acciones de control basadas en la temperatura
- El sensor cumple únicamente función informativa

## Programa 24: Lavado Configurable con Múltiples Tandas

### Características Principales
- Permite configurar el **tipo de agua** (caliente o fría)
- Ejecuta **4 tandas consecutivas** de las 3 fases básicas
- Incluye una fase final opcional de centrifugado
- Todos los parámetros son configurables

### Flujo de Ejecución

Para cada una de las 4 tandas:

1. **Fase 1: Llenado**
   - Si se configura agua caliente:
     * Solo se abre la válvula de agua caliente
     * Se implementa gestión activa de temperatura (como en Programa 22)
   - Si se configura agua fría:
     * Solo se abre la válvula de agua fría
     * No hay gestión de temperatura (como en Programa 23)
   - El llenado continúa hasta alcanzar el nivel configurado

2. **Fase 2: Lavado**
   - Activación del motor con el patrón de rotación configurado
   - Si se usa agua caliente, se mantiene activa la gestión de temperatura
   - El temporizador de lavado se ejecuta durante el tiempo configurado

3. **Fase 3: Drenaje**
   - Se abre la válvula de drenaje hasta evacuación completa del agua
   - El motor puede continuar funcionando a baja velocidad para facilitar el drenaje
   - El sistema monitorea el sensor de presión para confirmar el vaciado completo

Al finalizar las 4 tandas:

4. **Fase Final: Centrifugado (opcional)**
   - Si está habilitado, se activa el modo de centrifugado
   - La intensidad y duración dependen de la configuración
   - El motor funciona a alta velocidad para extraer la humedad residual

### Gestión de Temperatura
- Si se configura agua caliente:
  * Se implementa el mismo control de temperatura que en el Programa 22
  * Mantenimiento de temperatura con tolerancia de ±2°C
  * Drenaje parcial y nuevo llenado para mantener temperatura
- Si se configura agua fría:
  * No hay control activo de temperatura
  * El sensor solo cumple función informativa

## Controladores y Flujo de Señales

El flujo de ejecución implica la interacción de varios módulos:

```
[UIController] <----> [ProgramController] <----> [Storage]
       ^                     ^   ^
       |                     |   |
       v                     v   v
[Interfaz Nextion]       [Sensors] [Actuators]
                             ^         ^
                             |         |
                             v         v
                        [Sensores]  [Hardware]
```

### Secuencia de Procesamiento

1. **Selección de programa**: El usuario selecciona un programa (22, 23 o 24) a través de la interfaz en pantalla Nextion
2. **Configuración de parámetros**: El usuario puede ajustar los parámetros específicos según el programa
3. **Inicio de programa**: ProgramController configura las condiciones iniciales y activa la secuencia
4. **Control de fase**: 
   - Se establece el nivel objetivo, temperatura y patrón de rotación
   - Sensors monitorea continuamente las condiciones actuales
   - Actuators ajusta válvulas y motor según sea necesario
5. **Progresión de fase**: 
   - El temporizador se ejecuta cuando se alcanzan las condiciones objetivo
   - UIController actualiza la información en pantalla
   - Al finalizar el tiempo, se avanza a la siguiente fase
6. **Finalización**: Al completar todas las fases, se realiza la secuencia de finalización

## Control y Gestión de Emergencias

En cualquier fase de cualquier programa, existen dos mecanismos de interrupción:

1. **Botón de emergencia físico**: 
   - Detiene inmediatamente todas las operaciones
   - Activa drenaje de seguridad
   - Muestra pantalla de emergencia
   - Requiere reinicio manual

2. **Detección de errores automática**:
   - Monitoreo continuo de sensores
   - Detección de condiciones anómalas (sobrecalentamiento, fugas)
   - Transición automática a estado de error

## Personalización

El sistema permite la edición de todos los parámetros para cada programa:

1. **Selección de programa a editar**
2. **Ajuste de parámetros**:
   - Nivel de agua para cada fase
   - Temperatura (para programas con gestión de temperatura)
   - Tiempo de cada fase
   - Patrón de rotación
   - Habilitación y configuración de centrifugado
   - Tipo de agua (solo en Programa 24)
3. **Almacenamiento**: Los cambios se guardan en memoria no volátil (ESP32 Preferences)
4. **Persistencia**: Configuración preservada incluso tras pérdidas de energía

## Tiempos Aproximados de Programa

1. **Programa 22 (Agua Caliente)**: 
   - Tiempo base: 25-30 minutos (sin centrifugado)
   - Con centrifugado: 28-33 minutos

2. **Programa 23 (Agua Fría)**: 
   - Tiempo base: 20-25 minutos (sin centrifugado)
   - Con centrifugado: 23-28 minutos

3. **Programa 24 (Múltiples Tandas)**: 
   - Tiempo base: 60-70 minutos (sin centrifugado)
   - Con centrifugado: 63-73 minutos

> **Nota**: Los tiempos efectivos pueden ser ligeramente mayores debido al tiempo adicional necesario para alcanzar las condiciones de nivel de agua y temperatura antes de iniciar cada fase, especialmente en programas con gestión activa de temperatura.
