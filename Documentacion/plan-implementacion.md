# Plan de Implementación para el Controlador de Lavadora Industrial

## Resumen Contextual

El proyecto consiste en modernizar un controlador de lavadora industrial, migrándolo de una implementación monolítica basada en Arduino y LCD a una arquitectura modular utilizando ESP32-WROOM y pantalla táctil Nextion. 

## Enfoque de Modularización

La estrategia se basa en dividir el sistema en módulos independientes pero interconectados, siguiendo el principio de responsabilidad única. Esto permitirá un desarrollo incremental, donde cada módulo puede ser implementado y probado individualmente antes de ser integrado en el sistema completo.

## Progreso General del Proyecto: 25%

## Etapas de Implementación

### Etapa 1: Preparación y Estructura Base (2 semanas) - 60% completado

1. **Configuración del Entorno de Desarrollo (3 días)** - 100% completado
   - Configurar el IDE de Arduino con soporte para ESP32
   - Instalar las bibliotecas necesarias (AsyncTaskLib, OneWire, DallasTemperature, HX710B, EEPROM)
   - Configurar la comunicación serial con la pantalla Nextion

2. **Creación de la Estructura de Archivos (2 días)** - 80% completado
   - Crear los archivos de cabecera (.h) y de implementación (.cpp) para cada módulo
   - Organizar la estructura de directorios del proyecto
   - Pendiente: completar estructura para otros módulos

3. **Implementación del Módulo de Configuración (2 días)** - 100% completado
   - Definir todas las constantes, parámetros y configuraciones en config.h
   - Adaptar definiciones para ESP32 y pantalla Nextion
   - Documentar cada constante para facilitar futuras modificaciones

4. **Preparación del Archivo Principal (2 días)** - 0% completado
   - Crear la estructura mínima del archivo principal automLavanderia_v1.ino
   - Implementar las funciones setup() y loop() con la estructura básica
   - Incluir todos los módulos necesarios

5. **Validación de la Estructura Base (3 días)** - 20% completado
   - Verificar que el proyecto compile correctamente
   - Comprobar la comunicación básica con el hardware
   - Establecer mecanismos de depuración inicial

### Etapa 2: Implementación de Módulos Fundamentales (3 semanas) - 30% completado

1. **Módulo de Hardware (5 días)** - 70% completado
   - Implementar interfaz con hardware físico (ESP32, Nextion, botón emergencia)
   - Crear métodos para controlar pines de entrada/salida
   - Implementar comunicación serial con pantalla Nextion
   - Desarrollar sistema de detección y antirrebote para botón de emergencia
   - Pendiente: Probar la comunicación básica con todos los componentes hardware

2. **Módulo de Utilidades (4 días)** - 0% completado
   - Desarrollar funciones auxiliares y herramientas genéricas
   - Implementar gestión de temporizadores con AsyncTaskLib
   - Crear funciones para manejo y conversión de tiempo
   - Implementar utilidades para depuración
   - Probar funcionalidades básicas de forma aislada

3. **Módulo de Almacenamiento (4 días)** - 0% completado
   - Implementar acceso a EEPROM para persistencia de datos
   - Desarrollar métodos para guardar y recuperar configuraciones
   - Crear funciones de validación para datos almacenados
   - Establecer estructura de datos para programas y configuraciones
   - Verificar integridad de datos en operaciones de lectura/escritura

4. **Integración de Módulos Fundamentales (2 días)** - 0% completado
   - Conectar módulos Hardware, Utilidades y Almacenamiento
   - Verificar interacción correcta entre los módulos
   - Realizar pruebas de integración básicas

### Etapa 3: Implementación de Módulos Físicos (2 semanas) - 0% completado

1. **Módulo de Sensores (5 días)** - 0% completado
   - Implementar interfaz para sensor de temperatura (OneWire/DallasTemperature)
   - Desarrollar interfaz para sensor de presión/nivel de agua (HX710B)
   - Crear métodos para iniciar/detener monitorización
   - Implementar funciones para procesar y convertir lecturas
   - Añadir lógica para determinar cuándo se alcanzan niveles objetivo
   - Probar cada sensor individualmente y en conjunto

2. **Módulo de Actuadores (5 días)** - 0% completado
   - Implementar control del motor bidireccional
   - Desarrollar gestión de válvulas (agua, desagüe, vapor)
   - Crear manejo del mecanismo de seguridad de puerta
   - Implementar control del zumbador para alertas
   - Establecer estados de seguridad para cada actuador
   - Probar cada actuador individualmente y en conjunto

3. **Integración de Módulos Físicos (2 días)** - 0% completado
   - Conectar módulos de sensores y actuadores
   - Verificar respuesta de actuadores basada en lecturas de sensores
   - Realizar pruebas de comunicación entre módulos físicos y fundamentales

### Etapa 4: Implementación de Módulos de Control e Interfaz (3 semanas) - 20% completado

1. **Módulo de UI Controller (7 días)** - 40% completado
   - Estructura de la clase implementada
   - Definición de métodos para distintas pantallas
   - Pendiente: Implementación completa de las funciones
   - Pendiente: Comunicación con la pantalla Nextion
   - Pendiente: Procesamiento de eventos táctiles
   - Pendiente: Sistema de mensajes y retroalimentación
   - Pendiente: Pruebas de pantallas y transiciones

2. **Módulo de Program Controller (7 días)** - 0% completado
   - Implementar lógica central del sistema
   - Desarrollar máquina de estados para controlar el flujo del programa
   - Crear gestión de programas y fases de lavado
   - Implementar manejo de temporizadores para fases y rotaciones
   - Desarrollar procesamiento de eventos de usuario
   - Añadir sistema de manejo de emergencias
   - Probar cada estado y transición individualmente

3. **Integración de Módulos de Control e Interfaz (3 días)** - 0% completado
   - Conectar UI Controller con Program Controller
   - Verificar respuesta de la interfaz ante cambios de estado
   - Probar interacciones del usuario y efectos en el sistema

### Etapa 5: Integración y Pruebas Finales (2 semanas) - 0% completado

1. **Integración Completa de Módulos (5 días)** - 0% completado
   - Conectar todos los módulos según la arquitectura diseñada
   - Implementar interacciones entre todos los componentes
   - Verificar flujo de información a través del sistema completo
   - Resolver dependencias circulares si existen

2. **Optimización (3 días)** - 0% completado
   - Revisar uso de memoria y optimizar si es necesario
   - Mejorar tiempos de respuesta del sistema
   - Optimizar comunicación entre módulos
   - Reducir redundancias en el código

3. **Pruebas del Sistema Completo (5 días)** - 0% completado
   - Realizar pruebas de todas las funcionalidades
   - Verificar todos los programas y fases de lavado
   - Probar situaciones de error y recuperación
   - Comprobar respuesta ante situaciones de emergencia
   - Realizar pruebas de larga duración para verificar estabilidad

4. **Documentación Final (2 días)** - 0% completado
   - Completar documentación de todos los módulos
   - Crear diagrama final de interacción entre módulos
   - Documentar protocolo de comunicación con Nextion
   - Preparar manual técnico del sistema

## Consideraciones Técnicas Específicas

### Uso de AsyncTaskLib

En lugar de utilizar `delay()`, que bloquea la ejecución, todas las operaciones temporizadas deben implementarse con AsyncTaskLib. Esto es crucial para:

1. **Temporizador Principal**: Control del tiempo de cada fase de lavado
   ```cpp
   // Ejemplo de implementación en Utils
   AsyncTask mainTimer(INTERVALO_TEMPORIZADOR, true, []() {
     // Código para actualizar temporizadores
     ProgramController.updateTimers();
   });
   ```

2. **Control del Motor**: Gestión de la rotación y pausas del tambor
   ```cpp
   // Dentro de la actualización de temporizadores
   if (Actuators.isMotorRunning()) {
     _motorSeconds++;
     // Código para controlar dirección según tiempo transcurrido
   }
   ```

3. **Monitoreo de Sensores**: Lectura periódica sin bloquear el sistema
   ```cpp
   // En módulo de sensores
   AsyncTask sensorTimer(500, true, []() {
     Sensors.updateTemperature();
     Sensors.updatePressure();
   });
   ```

### Comunicación con Pantalla Nextion

La interfaz con la pantalla táctil Nextion requiere comunicación serial bidireccional:

1. **Envío de Comandos**: Para actualizar la interfaz
   ```cpp
   // En Hardware.nextionSendCommand
   NEXTION_SERIAL.print(command);
   NEXTION_SERIAL.write(0xFF);
   NEXTION_SERIAL.write(0xFF);
   NEXTION_SERIAL.write(0xFF);
   ```

2. **Recepción de Eventos**: Para procesar interacciones del usuario
   ```cpp
   // En Hardware.nextionCheckForEvents
   if (NEXTION_SERIAL.available()) {
     // Código para leer y procesar respuesta
   }
   ```

### Sistema de Manejo de Emergencias

El botón de emergencia debe tener la máxima prioridad:

```cpp
// En loop principal
Hardware.updateDebouncer();
if (Hardware.isEmergencyButtonPressed()) {
  ProgramController.handleEmergency();
  return;
}
```

## Dependencias Entre Módulos

Para asegurar una implementación adecuada, es importante entender las dependencias entre módulos:

1. **Hardware**: No depende de otros módulos
2. **Utils**: No depende de otros módulos
3. **Storage**: Depende de Program Controller para acceder a los datos
4. **Sensors**: Depende de Actuators para control basado en lecturas
5. **Actuators**: Depende de Hardware para control de pines
6. **UI Controller**: Depende de Hardware y Program Controller
7. **Program Controller**: Depende de Sensors, Actuators, Storage y Utils

## Recomendaciones Adicionales

1. **Desarrollo Incremental**: Implementar y probar un módulo a la vez
2. **Pruebas Unitarias**: Crear pequeños programas de prueba para cada módulo
3. **Control de Versiones**: Usar sistema de control de versiones para seguir el progreso
4. **Simulación**: Cuando sea posible, simular componentes hardware para pruebas
5. **Documentación Continua**: Documentar cada módulo a medida que se implementa
