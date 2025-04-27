# Descripción General del Controlador para Lavadoras Industriales

## Introducción

El sistema "automLavanderia_v1" es un controlador de lavadoras industriales diseñado para automatizar y gestionar ciclos de lavado en entornos industriales. Este controlador está basado en la plataforma Arduino y proporciona una solución completa para el manejo de los diferentes componentes y procesos involucrados en el funcionamiento de una lavadora industrial.

## Analogía Conceptual

Para entender el funcionamiento del sistema, podemos usar la analogía de un director de orquesta. Al igual que un director coordina a los diferentes músicos para que toquen en el momento adecuado y con la intensidad correcta, el controlador coordina los diferentes componentes de la lavadora (motor, válvulas, calentador) para que funcionen en el momento preciso y con los parámetros correctos según el programa seleccionado.

## Componentes Principales

El sistema está compuesto por los siguientes elementos principales:

1. **Unidad de Control**: Microcontrolador Arduino que gestiona toda la lógica del sistema.
2. **Interfaz de Usuario**: Pantalla LCD 16x2 y conjunto de botones para la interacción con el usuario.
3. **Sensores**:
   - Sensor de temperatura (Dallas Temperature) para medir la temperatura del agua.
   - Sensor de presión (HX710B) para determinar el nivel de agua.
4. **Actuadores**:
   - Motor bidireccional para el tambor de la lavadora (controlado por pines MotorDirA y MotorDirB).
   - Válvula de agua para el llenado del tambor.
   - Electroválvula de vapor para el calentamiento del agua.
   - Válvula de desfogue para el vaciado del tambor.
   - Sistema de bloqueo magnético de puerta para seguridad.
   - Buzzer para señalización acústica.
5. **Almacenamiento**: Memoria EEPROM para guardar configuraciones y programas.

## Capacidades del Sistema

El controlador permite:

1. **Gestión de Programas**: Selección entre 3 programas de lavado diferentes.
2. **Control de Ciclos**: Cada programa puede tener hasta 4 fases o ciclos.
3. **Parametrización**: Para cada fase se pueden configurar:
   - Nivel de agua
   - Temperatura
   - Tiempo de ejecución
   - Velocidad y patrón de rotación del tambor
4. **Monitorización**: Control constante de temperatura y nivel de agua.
5. **Seguridad**: Sistema de bloqueo de puerta y detección de errores.
6. **Persistencia**: Almacenamiento de configuraciones en memoria no volátil.

## Paradigma de Funcionamiento

El sistema funciona siguiendo un paradigma de máquina de estados, donde en cada momento se encuentra en uno de los siguientes estados principales:

1. **Estado de Selección**: El usuario puede seleccionar el programa deseado.
2. **Estado de Edición**: El usuario puede modificar los parámetros del programa.
3. **Estado de Ejecución**: El sistema está ejecutando un programa de lavado.
4. **Estado de Pausa**: El programa está temporalmente detenido entre fases.
5. **Estado de Error**: Se ha detectado una condición anómala que requiere atención.

La transición entre estos estados está determinada por las acciones del usuario (a través de los botones) y por el cumplimiento de las condiciones programadas (tiempo, temperatura, nivel).

En las siguientes secciones se describirá en detalle cada uno de los componentes y procesos del sistema.
