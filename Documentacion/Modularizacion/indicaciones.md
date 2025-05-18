### Indicaciones 
- Contexto geográfico: Arequipa, Perú.
- Controlador que se usará: ESP32-WROOM.
- Usa para análisis y generación de código el servidor mcp desktop commander que tienes instalado en Claude.
- Responde siempre en español.
- Responde como ingeniero electrónico senior.   
- Implementar según el documento: 
"C:\Users\DANIELE\Documents\itrebolsoft\Proyectos-hardware\Washer-industrial-machine-controller\Documentacion\Modularizacion\plan-implementacion.md" y actualiza el avance en porcentaje luego de cada tarea.
- Usa una librería compatible con ESP32.
- Se usa el IDE de arduino, versión 2.3.6. 
- Se usa la versión para ESP32 en arduino, versión 3.2.0.
- No usar la función delay() ni similares. En cambio usar la utilidad que creamos en utils.h.
- Comenta la utilidad de cada función o bloque importante.
- Luego de culminar cada tarea, no generes archivos .md a menos que yo te lo indique. 
- Luego de culminar cada una tarea espera que yo te indique para proseguir con otra tarea. 

### Pines
## Entradas
- **btnEmergencia**: Pin 15 (Botón de emergencia con antirrebote)

## Salidas (Actuadores)
- **MotorDirA**: Pin 12 (Control dirección derecha del motor)
- **MotorDirB**: Pin 14 (Control dirección izquierda del motor)
- **Centrifugado**: Pin 27 (Control centrifugado del motor)
- **ValvulAgua**: Pin 26 (Control de la válvula de entrada de agua)
- **ElectrovVapor**: Pin 33 (Control de la electroválvula de vapor para calentar)
- **ValvulDesfogue**: Pin 13 (Control de la válvula de drenaje)
- **MagnetPuerta**: Pin 25 (Control del bloqueo electromagnético de la puerta)

## Sensores
## Sensor de Presión (HX710B)
- **DOUT**: Pin 05
- **SCLK**: Pin 04

## Sensor de Temperatura (OneWire Dallas)
- **Pin de Datos**: Pin 23
- **Resolución**: 9 bits
- **Dirección del sensor**: 0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A

### Nextion
## Botones en el nextion
NEXTION_COMP_BTN_PROGRAM1 "btnPrograma1"
NEXTION_COMP_BTN_PROGRAM2 "btnPrograma2"
NEXTION_COMP_BTN_PROGRAM3 "btnPrograma3"
NEXTION_COMP_BTN_START "btnComenzar"
NEXTION_COMP_BTN_EDIT "btnEditar"
NEXTION_COMP_BTN_STOP "btnParar"
NEXTION_COMP_TXT_TEMP "txtTemp"
NEXTION_COMP_TXT_PRESSURE "txtPresion"
NEXTION_COMP_TXT_TIME "txtTiempo"
NEXTION_COMP_GAUGE_TEMP "gaugeTemp"
NEXTION_COMP_GAUGE_PRESSURE "gaugePresion"

- Directorio del Código: 
"C:\Users\DANIELE\Documents\itrebolsoft\Proyectos-hardware\Washer-industrial-machine-controller\Documentacion\Modularizacion"

- Archivo principal: 
C:\Users\DANIELE\Documents\itrebolsoft\Proyectos-hardware\Washer-industrial-machine-controller\mainController\mainController.ino

- No tomar en cuenta: 
    - "Documentacion\Analisis de código"
    - "Documentacion\Presupuesto"

- Directorio de la documentación:
"C:\Users\DANIELE\Documents\itrebolsoft\Proyectos hardware\Washer-industrial-machine-controller\Documentacion"