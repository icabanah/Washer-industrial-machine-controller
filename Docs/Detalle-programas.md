# Detalle de Programas y Configuraciones

## Tabla de Configuración de Programas

| Programa | Descripción    | Fases | Tandas | Tipo de Agua    | Control de Temp. | Nivel de Agua | Rotación     | Velocidad    | Centrifugado |
|----------|-------------   |-------|--------|--------------   |------------------|---------------|----------    |-----------   |--------------|
| 22       | Agua Caliente  | 3     | 1      | Caliente (fijo) | Sí               | Configurable  | Fijo         | Fijo         | Configurable |
| 23       | Agua Fría      | 3     | 1      | Fría (fijo)     | No               | Configurable  | Fijo         | Fijo         | Configurable |
| 24       | Multitanda     | 3     | 4      | Configurable    | Si agua caliente | Configurable  | Fijo         | Fijo         | Configurable |

## Detalle de Programas y Fases

### Programa 22 (Agua Caliente)
- **Tipo de Agua**: Solo agua caliente (no configurable)
- **Control de Temperatura**: 
  - Mantiene temperatura en rango objetivo ± 2°C
  - Si temperatura > objetivo + 2°C: drena agua parcialmente y rellena con agua caliente
  - Si temperatura < objetivo - 2°C: abre válvula de vapor y drenaje para mantener el nivel. 
- **Nivel de Agua**: Configurable para fase "llenado", en las demás fases se mantiene. 
- **Velocidad del Motor**: Fijo para cada fase
- **Rotación**: Fijo para cada fase
- **Centrifugado**: Configurable (Sí/No)
- **Comentarios**: 
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro programa.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa.
- **Fases**:
  - Fase 0: FASE_LLENANDO (Llena agua) 
  - Fase 1: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 2: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 3: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 4: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua).

### Programa 23 (Agua Fría)
- **Tipo de Agua**: Solo agua fría (no configurable)
- **Control de Temperatura**: No (sensor solo informativo)
- **Nivel de Agua**: Configurable para fase "llenado", en las demás fases se mantiene. 
- **Velocidad del Motor**: Fijo para cada fase
- **Rotación**: Fijo para cada fase
- **Centrifugado**: Configurable (Sí/No)
- **Comentarios**: 
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro programa.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa.
- **Fases**:
  - Fase 0: FASE_LLENANDO (Llena agua) 
  - Fase 1: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 2: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 3: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 4: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua).

### Programa 24 (Multiproceso)
- **Tipo de Agua**: Configurable (Fría/Caliente)
- **Control de Temperatura**: 
  - Si agua caliente: igual que Programa 22
  - Si agua fría: sin control (como Programa 23)
- **Nivel de Agua**: Configurable para cada fase de "llenado", en las demás fases se mantiene. 
- **Velocidad del Motor**: Fijo para cada fase y proceso
- **Rotación**: Fijo para cada fase y proceso
- **Centrifugado**: Configurable (Sí/No)
- **Comentarios**: 
  - Drenaje se cierra al comienzo del programa y solo se abre al inicio de la fase "FASE_DRENAJE" y permanece abierto hasta que se inicie otro proceso (tanda), al inicio de la fase de llenado.
  - La puerta se cierra al comienzo del programa y solo se abre al final de la fase "FASE_ENFRIAMIENTO" y permanece abierto hasta que se inicie otro programa.
- **Estructura**:
  - 4 procesos (Cada una con sus fases detallados abajo)
  **Fases**:
  - Fase 0: FASE_LLENANDO (Llena agua) 
  - Fase 1: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 2: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 3: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 4: FASE_LLENANDO (Llena agua) 
  - Fase 5: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 6: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 7: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 8: FASE_LLENANDO (Llena agua) 
  - Fase 9: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 10: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 11: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 12: FASE_LLENANDO (Llena agua) 
  - Fase 13: FASE_LAVADO (La lavadora activa los motores permutando entre giro y giro)
  - Fase 14: FASE_CENTRIFUGA (Depende de si activaron esta opción en el programa, si esta activa drenaje permanece cerrado y puerta permanece cerrado)
  - Fase 15: FASE_DRENAJE (Activa drenaje de agua de lavadora)
  - Fase 16: FASE_ENFRIAMIENTO (Para estabilizar el agua y discurrimiento de agua).
