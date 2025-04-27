# Flujo de Ejecución del Programa

Este documento describe el flujo de ejecución del controlador de lavadoras industriales "automLavanderia_v1", explicando las diferentes etapas y transiciones del programa.

## Analogía del Flujo del Programa

El flujo de ejecución del programa puede compararse con un viaje en tren:

- **Estación de Inicio (Setup)**: Es donde comienza el viaje, con todas las preparaciones necesarias.
- **Líneas Principales (Loop)**: Son las vías principales por donde circula el tren.
- **Estaciones (Estados)**: Son puntos específicos donde el tren se detiene para realizar ciertas acciones.
- **Cambios de Vía (Transiciones)**: Son las decisiones que determinan hacia dónde se dirigirá el tren a continuación.

## 1. Inicialización del Sistema (Setup)

```
Inicio
  |
  ↓
Configuración de pines (INPUT/OUTPUT)
  |
  ↓
Inicialización de salidas (todas LOW)
  |
  ↓
Inicialización LCD
  |
  ↓
Mostrar pantalla de bienvenida
  |
  ↓
Inicialización de sensores (temperatura, presión)
  |
  ↓
Recuperación de valores de EEPROM
  |
  ↓
Mostrar pantalla de selección de programa
  |
  ↓
Entrada al bucle principal
```

Durante esta fase, el sistema:
1. Define los pines como entradas o salidas
2. Inicializa los estados de los actuadores (todos en LOW)
3. Configura la pantalla LCD
4. Muestra un mensaje de bienvenida
5. Inicializa los sensores
6. Carga los valores guardados en la EEPROM
7. Prepara la primera pantalla interactiva

## 2. Bucle Principal (Loop)

El bucle principal funciona como un despachador que, según el estado actual del sistema, dirige la ejecución hacia diferentes caminos:

```
Bucle Principal
  |
  ↓
Verificar contador de bloqueo < límite
  |
  ↓
  └─── Si es falso → Mostrar error y detener
  |
  ↓
Detección de botones y gestión de estados
  |
  ↓
  ├─── btnAumentar/btnDisminuir → Cambiar programa
  |
  ├─── btnComenzar → Iniciar programa seleccionado
  |
  └─── btnEditar → Entrar en modo edición
```

El bucle principal está pendiente de:
- Las pulsaciones de botones
- El estado del sistema (selección, edición, ejecución)
- Condiciones de error (contador de bloqueo)

## 3. Modo de Selección de Programa

Cuando el sistema está en modo de selección, el usuario puede navegar entre los programas disponibles:

```
Modo Selección
  |
  ↓
Visualizar programa actual en LCD
  |
  ↓
Esperar entrada del usuario
  |
  ↓
  ├─── btnAumentar → Incrementar programa
  |
  ├─── btnDisminuir → Decrementar programa
  |
  ├─── btnEditar → Entrar en modo edición
  |
  └─── btnComenzar → Iniciar programa seleccionado
```

## 4. Modo de Edición de Programa

El modo de edición permite modificar los parámetros de los programas:

```
Modo Edición
  |
  ↓
Seleccionar variable a editar
  |
  ↓
  ├─── btnAumentar/btnDisminuir → Cambiar variable seleccionada
  |
  ├─── btnEditar → Entrar en edición de valor
  |     |
  |     ↓
  |   Editar valor de la variable
  |     |
  |     ↓
  |     ├─── btnAumentar/btnDisminuir → Cambiar valor
  |     |
  |     └─── btnParar → Volver a selección de variable
  |
  ├─── btnParar → Volver a selección de programa
  |
  └─── btnComenzar → Guardar cambios y volver
```

El modo de edición tiene una estructura jerárquica con tres niveles:
1. Selección de programa
2. Selección de variable a editar (Nivel, Temperatura, Tiempo, Rotación)
3. Modificación del valor de la variable

## 5. Ejecución de Programa

La ejecución de un programa sigue una secuencia detallada de pasos:

```
Iniciar Programa
  |
  ↓
Inicializar variables y activar seguridad
  |
  ↓
Abrir válvulas necesarias
  |
  ↓
Iniciar temporizador
  |
  ↓
Iniciar procesos según programa
  |
  ↓
  ├─── Programa 1: Motor + Temperatura
  |
  ├─── Programa 2: Temperatura + Nivel
  |
  └─── Programa 3: Motor + Temperatura + Nivel
  |
  ↓
Bucle de ejecución
  |
  ↓
  ├─── controladorTemporizador() → Gestión del tiempo
  |
  ├─── controladorDireccionMotor() → Control del motor
  |
  ├─── btnParar + btnEditar → Terminar programa
  |
  ├─── btnAumentar + btnEditar → Aumentar agua
  |
  └─── btnDisminuir + btnEditar → Disminuir agua
```

Durante la ejecución, el sistema está constantemente:
- Monitorizando el tiempo transcurrido
- Controlando la dirección y estado del motor
- Verificando la temperatura y el nivel de agua
- Respondiendo a comandos del usuario

## 6. Control del Motor

El control del motor sigue un patrón específico:

```
controladorDireccionMotor()
  |
  ↓
Verificar estado del motor (motorON)
  |
  ↓
  └─── Si está apagado → Salir
  |
  ↓
Verificar dirección actual
  |
  ↓
  ├─── Dirección 1 (A) → Verificar tiempo de rotación
  |     |
  |     ↓
  |     └─── Si tiempo cumplido → Cambiar a dirección 2 (Pausa)
  |
  ├─── Dirección 2 (Pausa) → Verificar tiempo de pausa
  |     |
  |     ↓
  |     └─── Si tiempo cumplido → Cambiar a dirección 1 o 3 según estado
  |
  └─── Dirección 3 (B) → Verificar tiempo de rotación
        |
        ↓
        └─── Si tiempo cumplido → Cambiar a dirección 2 (Pausa)
```

El motor alterna entre:
- Rotación en dirección A
- Pausa
- Rotación en dirección B
- Pausa

Siguiendo un patrón de tiempo definido por el programa.

## 7. Control de Temperatura

El control de temperatura opera continuamente:

```
controladorSensorTemperatura()
  |
  ↓
Leer temperatura actual
  |
  ↓
Comparar con temperatura objetivo
  |
  ↓
  ├─── Si temperatura > (objetivo + rango) → Apagar calentamiento
  |
  └─── Si temperatura < (objetivo - rango) → Encender calentamiento
```

Funciona como un termostato con histéresis, evitando cambios rápidos de estado.

## 8. Control de Nivel de Agua

El control del nivel de agua sigue este flujo:

```
controladorSensorPresion()
  |
  ↓
Leer presión actual
  |
  ↓
Determinar nivel de agua (1-4)
  |
  ↓
Comparar con nivel objetivo
  |
  ↓
  └─── Si nivel >= (objetivo + 1) y contador > 5 → Cerrar válvula y continuar
```

El sistema usa un contador para confirmar lecturas y evitar falsos positivos.

## 9. Transiciones entre Fases

Cuando se completa una fase, el sistema realiza una transición:

```
Fase completa (tiempo = 0)
  |
  ↓
pausarPrograma()
  |
  ↓
  ├─── Si es programa 3 y no última fase → Incrementar fase y reiniciarPrograma()
  |
  └─── En otros casos → terminarPrograma()
```

En el programa 3 (completo), el sistema puede tener hasta 4 fases, mientras que los programas 1 y 2 tienen una sola fase.

## 10. Finalización del Programa

Al finalizar un programa:

```
terminarPrograma()
  |
  ↓
Actualizar variables de estado
  |
  ↓
Detener motores y cerrar válvulas
  |
  ↓
Desbloquear puerta
  |
  ↓
Activar buzzer (2 segundos)
  |
  ↓
Mostrar pantalla de selección
```

El sistema vuelve a su estado inicial, permitiendo comenzar un nuevo ciclo.

## Estados y Transiciones

El programa opera como una máquina de estados finitos con los siguientes estados principales:

1. **Estado Inicial**: Pantalla de bienvenida y configuración
2. **Estado de Selección**: Selección de programa
3. **Estado de Edición**: Configuración de parámetros
4. **Estado de Ejecución**: Programa en funcionamiento
5. **Estado de Pausa**: Entre fases o por comando
6. **Estado de Finalización**: Término de programa
7. **Estado de Error**: Cuando se detecta un problema

Las transiciones entre estados están controladas por:
- Comandos del usuario (botones)
- Temporizadores internos
- Lecturas de sensores
- Lógica programada

Este flujo garantiza un funcionamiento ordenado y predecible del sistema, incluso ante diferentes condiciones y programas.
