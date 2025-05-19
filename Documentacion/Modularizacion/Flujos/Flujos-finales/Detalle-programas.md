# Detalle de Programas y Configuraciones

## Tabla de Configuración de Programas

| Programa | Descripción    | Fases | Tandas | Tipo de Agua    | Control de Temp. | Nivel de Agua | Rotación     | Velocidad    | Centrifugado |
|----------|-------------   |-------|--------|--------------   |------------------|---------------|----------    |-----------   |--------------|
| 22       | Agua Caliente  | 3     | 1      | Caliente (fijo) | Sí               | Configurable  | Configurable | Configurable | Configurable |
| 23       | Agua Fría      | 3     | 1      | Fría (fijo)     | No               | Configurable  | Configurable | Configurable | Configurable |
| 24       | Multitanda     | 3     | 4      | Configurable    | Si agua caliente | Configurable  | Configurable | Configurable | Configurable |

## Detalle de Programas y Fases

### Programa 22 (Agua Caliente)
- **Tipo de Agua**: Solo agua caliente (no configurable)
- **Control de Temperatura**: 
  - Mantiene temperatura en rango objetivo ± 2°C
  - Si temperatura > objetivo + 2°C: drena agua parcialmente y rellena con agua caliente
  - Si temperatura < objetivo - 2°C: abre válvula de vapor
- **Nivel de Agua**: Configurable para cada fase
- **Velocidad del Motor**: Configurable para cada fase
- **Rotación**: Configurable para cada fase
- **Centrifugado**: Configurable (Sí/No)
- **Fases**: 
  - Fase 0: Llenado
  - Fase 1: Lavado Principal
  - Fase 2: Drenaje

### Programa 23 (Agua Fría)
- **Tipo de Agua**: Solo agua fría (no configurable)
- **Control de Temperatura**: No (sensor solo informativo)
- **Nivel de Agua**: Configurable para cada fase
- **Velocidad del Motor**: Configurable para cada fase
- **Rotación**: Configurable para cada fase
- **Centrifugado**: Configurable (Sí/No)
- **Fases**:
  - Fase 0: Llenado
  - Fase 1: Lavado Principal
  - Fase 2: Drenaje

### Programa 24 (Multitanda)
- **Tipo de Agua**: Configurable (Fría/Caliente)
- **Control de Temperatura**: 
  - Si agua caliente: igual que Programa 22
  - Si agua fría: sin control (como Programa 23)
- **Nivel de Agua**: Configurable para cada fase y tanda
- **Velocidad del Motor**: Configurable para cada fase y tanda
- **Rotación**: Configurable para cada fase y tanda
- **Centrifugado**: Configurable (Sí/No)
- **Estructura**:
  - 4 Tandas (secuencias completas de lavado)
  - Cada tanda tiene 3 fases (como los programas básicos)
  - Total: 12 fases secuenciales
