# Flujo de Ejecución de Programas del Controlador de Lavadora Industrial

## Introducción

Este documento describe en detalle el flujo de ejecución de cada programa disponible en el controlador de lavadora industrial basado en ESP32. Se explica la secuencia de operaciones, los parámetros específicos de cada fase y las diferencias entre los tres programas predefinidos.

## Estructura General de Programas

El sistema cuenta con **3 programas** predefinidos, cada uno diseñado para diferentes tipos de lavado:

1. **Programa 0**: Lavado normal/estándar
2. **Programa 1**: Lavado intensivo/pesado
3. **Programa 2**: Lavado delicado/económico

Cada programa está dividido en **4 fases** secuenciales:

1. **Fase 0**: Prelavado/Remojo
2. **Fase 1**: Lavado principal
3. **Fase 2**: Enjuague
4. **Fase 3**: Centrifugado/Drenaje

## Parámetros de Control

Cada fase de cada programa está definida por cuatro parámetros principales:

1. **Nivel de agua** (valores 1-4):
   - 1: Nivel bajo
   - 2: Nivel medio-bajo
   - 3: Nivel medio-alto
   - 4: Nivel alto

2. **Temperatura** (en °C):
   - Rango típico: 30°C - 70°C

3. **Tiempo** (en minutos):
   - Define la duración de cada fase

4. **Rotación** (valores 1-3):
   - 1: Rotación suave (menos RPM, tiempos de pausa más largos)
   - 2: Rotación media
   - 3: Rotación intensa (más RPM, tiempos de pausa más cortos)

## Flujo General de Ejecución

Independientemente del programa seleccionado, el flujo de ejecución sigue este patrón general:

1. **Inicio del programa**:
   - Bloqueo de la puerta
   - Verificación de condiciones iniciales

2. **Para cada fase**:
   - Configuración de actuadores según parámetros de la fase
   - Llenado de agua hasta el nivel objetivo
   - Calentamiento hasta la temperatura objetivo
   - Inicio de rotación según nivel configurado
   - Ejecución del temporizador de fase cuando se alcanzan las condiciones
   - Finalización de la fase cuando el temporizador llega a cero

3. **Finalización del programa**:
   - Drenaje final
   - Señal acústica indicando fin de ciclo
   - Desbloqueo de la puerta
   - Retorno a pantalla de selección

## Detalle por Programa y Fase

### Programa 0: Lavado Normal/Estándar

#### Fase 0: Prelavado
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 40°C 
- **Tiempo**: 5 minutos
- **Rotación**: 1 (suave)
- **Acciones**:
  - Llenado inicial de agua
  - Calentamiento moderado
  - Rotación suave para remojo

#### Fase 1: Lavado Principal
- **Nivel de agua**: 3 (medio-alto)
- **Temperatura**: 60°C
- **Tiempo**: 10 minutos
- **Rotación**: 2 (media)
- **Acciones**:
  - Mayor nivel de agua
  - Temperatura más alta para acción del detergente
  - Rotación media para limpieza efectiva

#### Fase 2: Enjuague
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 40°C
- **Tiempo**: 5 minutos
- **Rotación**: 1 (suave)
- **Acciones**:
  - Nivel de agua moderado
  - Temperatura media para eliminar detergente
  - Rotación suave para evitar arrugas

#### Fase 3: Centrifugado/Drenaje
- **Nivel de agua**: 1 (bajo) - principalmente para drenaje
- **Temperatura**: 30°C (sin calentamiento activo)
- **Tiempo**: 3 minutos
- **Rotación**: 3 (intensa)
- **Acciones**:
  - Drenaje del agua
  - Centrifugado rápido para eliminar exceso de agua

### Programa 1: Lavado Intensivo/Pesado

#### Fase 0: Prelavado
- **Nivel de agua**: 3 (medio-alto)
- **Temperatura**: 45°C
- **Tiempo**: 6 minutos
- **Rotación**: 2 (media)
- **Acciones**:
  - Mayor nivel de agua
  - Temperatura más alta desde el inicio
  - Rotación media para remojo más intenso

#### Fase 1: Lavado Principal
- **Nivel de agua**: 3 (medio-alto)
- **Temperatura**: 70°C
- **Tiempo**: 12 minutos
- **Rotación**: 3 (intensa)
- **Acciones**:
  - Nivel de agua alto
  - Temperatura máxima para eliminar manchas difíciles
  - Rotación intensa para limpieza profunda

#### Fase 2: Enjuague
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 45°C
- **Tiempo**: 6 minutos
- **Rotación**: 2 (media)
- **Acciones**:
  - Nivel de agua moderado
  - Temperatura media para enjuague efectivo
  - Rotación media para eliminar residuos

#### Fase 3: Centrifugado/Drenaje
- **Nivel de agua**: 1 (bajo)
- **Temperatura**: 30°C
- **Tiempo**: 3 minutos
- **Rotación**: 3 (intensa)
- **Acciones**:
  - Drenaje completo
  - Centrifugado intenso para eliminar máxima humedad

### Programa 2: Lavado Delicado/Económico

#### Fase 0: Prelavado
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 30°C
- **Tiempo**: 4 minutos
- **Rotación**: 1 (suave)
- **Acciones**:
  - Nivel de agua moderado
  - Baja temperatura para proteger tejidos
  - Rotación muy suave para prendas delicadas

#### Fase 1: Lavado Principal
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 50°C
- **Tiempo**: 8 minutos
- **Rotación**: 2 (media)
- **Acciones**:
  - Nivel de agua moderado (economiza agua)
  - Temperatura media para cuidar tejidos
  - Rotación media para limpieza suficiente

#### Fase 2: Enjuague
- **Nivel de agua**: 2 (medio-bajo)
- **Temperatura**: 40°C
- **Tiempo**: 4 minutos
- **Rotación**: 1 (suave)
- **Acciones**:
  - Nivel de agua moderado
  - Temperatura media-baja
  - Rotación suave para proteger las prendas

#### Fase 3: Centrifugado/Drenaje
- **Nivel de agua**: 1 (bajo)
- **Temperatura**: 30°C
- **Tiempo**: 3 minutos
- **Rotación**: 2 (media)
- **Acciones**:
  - Drenaje suave
  - Centrifugado menos intenso para tejidos delicados

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

1. **Selección de programa**: El usuario selecciona un programa a través de la interfaz en pantalla Nextion
2. **Inicio de programa**: ProgramController configura las condiciones iniciales y activa la secuencia
3. **Control de fase**: 
   - Se establece el nivel objetivo, temperatura y patrón de rotación
   - Sensors monitorea continuamente las condiciones actuales
   - Actuators ajusta válvulas y motor según sea necesario
4. **Progresión de fase**: 
   - El temporizador se ejecuta cuando se alcanzan las condiciones objetivo
   - UIController actualiza la información en pantalla
   - Al finalizar el tiempo, se avanza a la siguiente fase
5. **Finalización**: Al completar todas las fases, se realiza la secuencia de finalización

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

El sistema permite la edición de todos los parámetros para cada fase de cada programa:

1. **Selección de programa/fase a editar**
2. **Ajuste de parámetros**: nivel, temperatura, tiempo, rotación
3. **Almacenamiento**: Los cambios se guardan en memoria no volátil (ESP32 Preferences)
4. **Persistencia**: Configuración preservada incluso tras pérdidas de energía

## Diagrama de Secuencia Típico

```
                   Inicio
                      │
                      ▼
            ┌──────────────────┐
            │ Selección Program│
            └────────┬─────────┘
                     │
                     ▼
            ┌──────────────────┐
            │  Inicio Program  │
            └────────┬─────────┘
                     │
                     ▼
            ┌──────────────────┐
            │    Fase 0        │◄────┐
            └────────┬─────────┘     │
                     │               │
                     ▼               │
            ┌──────────────────┐     │
            │ Llenar Agua      │     │
            └────────┬─────────┘     │
                     │               │
                     ▼               │
            ┌──────────────────┐     │
            │ Calentar         │     │
            └────────┬─────────┘     │
                     │               │
                     ▼               │
            ┌──────────────────┐     │
            │ Iniciar Rotación │     │
            └────────┬─────────┘     │
                     │               │
                     ▼               │
            ┌──────────────────┐     │
            │Ejecutar Temporizad     │
            └────────┬─────────┘     │
                     │               │
┌───────────────┐    ▼               │
│ Siguiente Fase├───◄ Fin Fase?  No ─┘
└───────┬───────┘   │
        │           │ Sí
        │           ▼
        │    ┌──────────────────┐
        └───►│ Última Fase?  No │
             └────────┬─────────┘
                      │ Sí
                      ▼
             ┌──────────────────┐
             │ Finalizar Program│
             └──────────────────┘
```

## Tiempos Totales de Programa

1. **Programa 0 (Normal)**: 
   - Tiempo total: 23 minutos
   - Desglose: 5 + 10 + 5 + 3 minutos

2. **Programa 1 (Intensivo)**: 
   - Tiempo total: 27 minutos
   - Desglose: 6 + 12 + 6 + 3 minutos

3. **Programa 2 (Delicado)**: 
   - Tiempo total: 19 minutos
   - Desglose: 4 + 8 + 4 + 3 minutos

> **Nota**: Los tiempos efectivos pueden ser ligeramente mayores debido al tiempo adicional necesario para alcanzar las condiciones de nivel de agua y temperatura antes de iniciar cada fase.
