# Plan de Implementación para el Controlador de Lavadora Industrial

## Resumen Contextual

El proyecto consiste en modernizar un controlador de lavadora industrial, migrándolo de una implementación monolítica basada en Arduino y LCD a una arquitectura modular utilizando ESP32-WROOM y pantalla táctil Nextion. El sistema debe soportar tres programas de lavado específicos (22: Agua Caliente, 23: Agua Fría, y 24: Multitanda) con sus respectivas configuraciones y comportamientos.

## Enfoque de Modularización

La estrategia se basa en dividir el sistema en módulos independientes pero interconectados, siguiendo el principio de responsabilidad única. Esto permitirá un desarrollo incremental, donde cada módulo puede ser implementado y probado individualmente antes de ser integrado en el sistema completo. La arquitectura implementa una máquina de estados completa para gestionar el flujo de ejecución, con especial énfasis en la gestión activa de temperatura para programas que utilizan agua caliente.

## Progreso General del Proyecto: 52% (Actualizado)

## Etapas de Implementación

### Etapa 1: Preparación y Estructura Base (2 semanas) - 80% completado

1. **Configuración del Entorno de Desarrollo (3 días)** - 100% completado
   - Configurar el IDE de Arduino con soporte para ESP32
   - Instalar las bibliotecas necesarias (AsyncTaskLib, OneWire, DallasTemperature, HX710B, EEPROM)
   - Configurar la comunicación serial con la pantalla Nextion

2. **Creación de la Estructura de Archivos (2 días)** - 100% completado
   - Crear los archivos de cabecera (.h) y de implementación (.cpp) para cada módulo
   - Organizar la estructura de directorios del proyecto

3. **Implementación del Módulo de Configuración (2 días)** - 100% completado
   - Definir todas las constantes, parámetros y configuraciones en config.h
   - Adaptar definiciones para ESP32 y pantalla Nextion
   - Documentar cada constante para facilitar futuras modificaciones

4. **Preparación del Archivo Principal (2 días)** - 100% completado
   - Crear la estructura mínima del archivo principal mainController.ino
   - Implementar las funciones setup() y loop() con la estructura básica
   - Incluir todos los módulos necesarios

5. **Validación de la Estructura Base (3 días)** - 20% completado
   - Verificar que el proyecto compile correctamente
   - Comprobar la comunicación básica con el hardware
   - Establecer mecanismos de depuración inicial

### Etapa 2: Implementación de Módulos Fundamentales (3 semanas) - 70% completado

1. **Módulo de Hardware (5 días)** - 100% completado
   - Implementar interfaz con hardware físico (ESP32, Nextion, botón emergencia)
   - Crear métodos para controlar pines de entrada/salida
   - Implementar comunicación serial con pantalla Nextion
   - Desarrollar sistema de detección y antirrebote para botón de emergencia
   - Verificar comunicación con todos los componentes hardware
   - Integrar métodos específicos para control de válvulas de agua caliente y fría

2. **Módulo de Utilidades (4 días)** - 100% completado
   - Desarrollar funciones auxiliares y herramientas genéricas ✓
   - Implementar gestión de temporizadores para operaciones asíncronas ✓
   - Crear funciones para manejo y conversión de tiempo ✓
   - Implementar utilidades para depuración con niveles de detalle configurables ✓
   - Implementar sistema de callbacks para temporizadores ✓
   - Implementar sistema centralizado de registro de callbacks para comunicación entre módulos ✓
   - Realizar pruebas de carga para evaluar rendimiento con múltiples tareas ✓

3. **Módulo de Almacenamiento (4 días)** - 80% completado
   - Implementar acceso a Preferences (ESP32) para persistencia de datos ✓
   - Desarrollar estructura para almacenar configuraciones específicas de programas ✓
   - Crear estructura de datos para gestión de múltiples tandas (Programa 24) ✓
   - Implementar almacenamiento de parámetros de temperatura y tipo de agua ✓
   - Implementar métodos de carga/guardado masivo de configuraciones ✓
   - Implementar validación de datos básica ✓
   - Pendiente: Implementar recuperación de errores avanzada
   - Pendiente: Crear sistema de respaldo para configuraciones críticas

4. **Integración de Módulos Fundamentales (2 días)** - 100% completado
   - Conectar módulos Hardware, Utilidades y Almacenamiento ✓
   - Implementar sistema de registro de callbacks para centralizar temporizadores ✓
   - Mejorar gestión de eventos desde la pantalla Nextion ✓
   - Crear métodos para facilitar la comunicación entre módulos ✓
   - Verificar interacción correcta entre los módulos ✓
   - Realizar pruebas de integración básicas ✓

### Etapa 3: Implementación de Módulos Físicos (2 semanas) - 30% completado

1. **Módulo de Sensores (5 días)** - 50% completado
   - Implementar interfaz para sensor de temperatura (OneWire/DallasTemperature)
   - Desarrollar interfaz para sensor de presión/nivel de agua (HX710B)
   - Crear sistema de calibración para sensor de presión
   - Implementar lógica de conversión de valores brutos a unidades prácticas
   - Implementar monitoreo continuo y asincrónico de sensores
   - Desarrollar métodos para gestión de temperaturas en programas de agua caliente
   - Pendiente: Completar algoritmos de estabilización de lecturas
   - Pendiente: Implementar detección de anomalías en sensores

2. **Módulo de Actuadores (5 días)** - 40% completado
   - Implementar control del motor bidireccional con patrones de rotación configurables
   - Desarrollar gestión diferenciada de válvulas (agua fría, agua caliente, desagüe, vapor)
   - Crear manejo del mecanismo de seguridad de puerta con verificación
   - Implementar control de centrifugado variable
   - Desarrollar sistema de gestión de temperatura para programas con agua caliente
   - Pendiente: Implementar secuencias de drenaje parcial para ajuste de temperatura
   - Pendiente: Completar algoritmos de control de centrifugado

3. **Integración de Módulos Físicos (2 días)** - 0% completado
   - Conectar módulos de sensores y actuadores
   - Verificar respuesta de actuadores basada en lecturas de sensores
   - Realizar pruebas de comunicación entre módulos físicos y fundamentales

### Etapa 4: Implementación de Módulos de Control e Interfaz (3 semanas) - 55% completado

1. **Módulo de UI Controller (7 días)** - 75% completado
   - Estructura de la clase implementada con enfoque a máquina de estados ✓
   - Definición de métodos específicos para distintas pantallas (selección, configuración, ejecución) ✓
   - Implementación de pantallas específicas para programas 22, 23 y 24 ✓
   - Desarrollo de interfaz para configuración de programas con múltiples tandas ✓
   - Implementación de pantallas para mostrar información de temperatura y presión ✓
   - **NUEVO**: Sistema de mensajes y alertas avanzado implementado ✓
   - **NUEVO**: Pantalla de mantenimiento con controles manuales ✓
   - **NUEVO**: Protocolo de comunicación bidireccional expandido ✓
   - **NUEVO**: Optimizaciones de rendimiento para ESP32 ✓
   - **NUEVO**: Integración completa con AsyncTaskLib ✓
   - Pendiente: Procesamiento completo de eventos táctiles para mantenimiento
   - Pendiente: Pruebas finales de integración con hardware real

2. **Módulo de Program Controller (7 días)** - 40% completado
   - Implementar máquina de estados principal del sistema con 7 estados
   - Desarrollar lógica para los tres programas específicos (22, 23, 24)
   - Implementar flujo específico para control de temperatura en agua caliente
   - Crear gestión especial para programa de múltiples tandas
   - Desarrollar cálculo dinámico de tiempos según configuración
   - Implementar sistema de manejo de fases para cada programa
   - Pendiente: Completar manejo detallado de centrifugado
   - Pendiente: Implementar manejo completo de emergencias
   - Pendiente: Desarrollar algoritmos específicos para cada programa

3. **Integración de Módulos de Control e Interfaz (3 días)** - 0% completado
   - Conectar UI Controller con Program Controller
   - Verificar respuesta de la interfaz ante cambios de estado
   - Probar interacciones del usuario y efectos en el sistema

### Etapa 5: Integración y Pruebas Finales (2 semanas) - 15% completado

1. **Integración Completa de Módulos (5 días)** - 30% completado
   - Desarrollar flujo de mensajes entre todos los módulos ✓
   - Implementar el sistema central de gestión de tiempo ✓
   - Establecer canales de comunicación entre módulos ✓
   - Implementar ciclo completo para los tres programas (en proceso)
   - Crear lógica de gestión especial para programa 24 (multitanda)
   - Integrar control de temperatura para programas con agua caliente
   - Pendiente: Implementar flujo de emergencia que atraviese todos los módulos
   - Pendiente: Integrar sistema de gestión de errores a nivel global

2. **Optimización (3 días)** - 0% completado
   - Revisar uso de memoria y optimizar si es necesario
   - Mejorar tiempos de respuesta del sistema
   - Optimizar comunicación entre módulos
   - Reducir redundancias en el código

3. **Pruebas del Sistema Completo (5 días)** - 0% completado
   - Probar ciclo completo para programa 22 (agua caliente)
   - Probar ciclo completo para programa 23 (agua fría)
   - Probar ciclo completo para programa 24 con múltiples tandas
   - Verificar control activo de temperatura en programas con agua caliente
   - Comprobar comportamiento ante emergencias
   - Realizar pruebas de larga duración para verificar estabilidad

4. **Documentación Final (2 días)** - 20% completado
   - Diagramas de flujo completados
   - Descripción técnica de programas y fases completada
   - Pendiente: Consolidar documentación de todos los módulos
   - Pendiente: Preparar manual técnico detallado
   - Pendiente: Desarrollar diagramas adicionales para tecnicos de mantenimiento

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

Para asegurar una implementación adecuada, es importante entender las dependencias entre módulos y cómo se han integrado:

1. **Hardware**: No depende de otros módulos
2. **Utils**: Depende minimalmente de otros módulos para callbacks específicos
3. **Storage**: Implementa métodos para trabajar con los datos del Program Controller 
4. **Sensors**: Ahora se integra con Utils para los temporizadores y notificaciones
5. **Actuators**: Utiliza Hardware para control de pines y Utils para temporizadores
6. **UI Controller**: Depende de Hardware y Program Controller
7. **Program Controller**: Depende de Sensors, Actuators, Storage y Utils

La integración ahora utiliza un enfoque centralizado de temporizadores donde Utils.updateTasks() gestiona todos los eventos temporizados del sistema. Esto permite una mejor sincronización y evita problemas de timing entre módulos.

## Recomendaciones Adicionales

1. **Desarrollo Incremental**: Implementar y probar un módulo a la vez
2. **Pruebas Unitarias**: Crear pequeños programas de prueba para cada módulo
3. **Control de Versiones**: Usar sistema de control de versiones para seguir el progreso
4. **Simulación**: Cuando sea posible, simular componentes hardware para pruebas
5. **Documentación Continua**: Documentar cada módulo a medida que se implementa
