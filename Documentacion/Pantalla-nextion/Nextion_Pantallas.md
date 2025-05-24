# Guía de Diseño de Pantallas Nextion

Este documento proporciona una guía para el diseño de las pantallas Nextion necesarias para el controlador de lavadora industrial.

## Consideraciones Generales

- **Resolución**: Las pantallas se diseñarán para una resolución de 320x240 píxeles (pantalla Nextion 3.2")
- **Paleta de Colores**: Utilizar una paleta consistente para toda la interfaz
  - Fondo: Azul oscuro (#000050)
  - Texto principal: Blanco (#FFFFFF)
  - Botones activos: Verde (#00FF00)
  - Botones inactivos: Gris (#808080)
  - Alertas/Emergencia: Rojo (#FF0000)
- **Fuentes**: Utilizar fuentes legibles a distancia
  - Títulos: Arial, 24pt, Bold
  - Texto general: Arial, 16pt
  - Valores numéricos: Arial, 20pt, Bold

## Diseño de Pantallas

### Página 0: Bienvenida

![Ejemplo de Pantalla de Bienvenida](placeholder)

#### Componentes:
1. **t0** - Texto (Título)
   - Texto: "RH Electronics"
   - Posición: Centro (x=160, y=80)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

2. **t1** - Texto (Subtítulo)
   - Texto: "Controlador de Lavadora Industrial"
   - Posición: Centro (x=160, y=120)
   - Fuente: Arial, 16pt
   - Color: Blanco

3. **t2** - Texto (Contacto)
   - Texto: "958970967"
   - Posición: Centro (x=160, y=160)
   - Fuente: Arial, 16pt
   - Color: Blanco

4. **anim** - Animación (Logo giratorio)
   - Posición: (x=160, y=40)
   - Tamaño: 80x80
   - Tipo: Gif animado

#### Configuración de Eventos:
- Configurar un temporizador (tm0) para cambiar automáticamente a la pantalla de selección después de 3 segundos:
```
// En el evento Timer
page 1
```

### Página 1: Selección de Programa

![Ejemplo de Pantalla de Selección](placeholder)

#### Componentes:
1. **t0** - Texto (Título)
   - Texto: "Selección de Programa"
   - Posición: Centro-Superior (x=160, y=20)
   - Fuente: Arial, 24pt, Bold
   - Color: Blanco

2. **b1, b2, b3** - Botones (Programas)
   - Texto: "P22", "P23", "P24"
   - Posición: Fila superior (y=60)
   - Tamaño: 80x40 cada uno
   - Espaciado: Equidistante
   - Color: Inactivo: Gris, Activo: Verde

3. **t10** - Texto (Programa seleccionado)
   - Texto: "P22"
   - Posición: (x=20, y=110)
   - Fuente: Arial, 18pt, Bold
   - Color: Blanco

4. **t11, t12, t13, t14** - Textos (Parámetros)
   - Texto: Información de nivel, temperatura, tiempo y rotación
   - Posición: Columna izquierda, alineados
   - Fuente: Arial, 16pt
   - Color: Blanco

5. **t15** - Texto (Información adicional)
   - Texto: Información de fases para P24
   - Posición: (x=20, y=190)
   - Fuente: Arial, 14pt
   - Color: Blanco

6. **b4** - Botón (Iniciar)
   - Texto: "INICIAR"
   - Posición: Inferior-derecha (x=240, y=200)
   - Tamaño: 100x40
   - Color: Verde

7. **b5** - Botón (Editar)
   - Texto: "EDITAR"
   - Posición: Inferior-derecha (x=130, y=200)
   - Tamaño: 100x40
   - Color: Azul

#### Configuración de Eventos:
- Para cada botón de programa (b1, b2, b3), enviar evento cuando se presiona
- Para botones Iniciar y Editar, enviar evento cuando se presiona

### Página 2: Ejecución de Programa

![Ejemplo de Pantalla de Ejecución](placeholder)

#### Componentes:
1. **t1** - Texto (Programa)
   - Texto: "P22"
   - Posición: Superior-izquierda (x=20, y=20)
   - Fuente: Arial, 24pt, Bold
   - Color: Blanco

2. **t2** - Texto (Fase)
   - Texto: "Fase: 1"
   - Posición: Superior-derecha (x=240, y=20)
   - Fuente: Arial, 18pt
   - Color: Blanco

3. **t3** - Texto (Tiempo)
   - Texto: "00:00"
   - Posición: Centro (x=160, y=60)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

4. **j0** - Barra de Progreso
   - Posición: Centro (x=160, y=100)
   - Tamaño: 280x20
   - Color: Azul-Verde

5. **t4** - Texto (Temperatura)
   - Texto: "25°C"
   - Posición: (x=60, y=140)
   - Fuente: Arial, 18pt
   - Color: Blanco

6. **t5** - Texto (Nivel de agua)
   - Texto: "Nivel: 2"
   - Posición: (x=160, y=140)
   - Fuente: Arial, 18pt
   - Color: Blanco

7. **t6** - Texto (Velocidad)
   - Texto: "Vel: 1"
   - Posición: (x=260, y=140)
   - Fuente: Arial, 18pt
   - Color: Blanco

8. **z0** - Indicador (Temperatura)
   - Tipo: Gauge
   - Posición: (x=60, y=180)
   - Tamaño: 80x80
   - Color: Rojo-Amarillo

9. **z1** - Indicador (Nivel de agua)
   - Tipo: Gauge
   - Posición: (x=160, y=180)
   - Tamaño: 80x80
   - Color: Azul

10. **motor** - Indicador (Motor)
    - Tipo: Imagen animada
    - Posición: (x=260, y=180)
    - Tamaño: 80x80

11. **b6** - Botón (Detener)
    - Texto: "DETENER"
    - Posición: Inferior-derecha (x=240, y=200)
    - Tamaño: 100x40
    - Color: Rojo

#### Configuración de Eventos:
- Botón Detener envía evento cuando se presiona

### Página 3: Edición de Programa

![Ejemplo de Pantalla de Edición](placeholder)

#### Componentes:
1. **t1** - Texto (Título)
   - Texto: "P22 - F1"
   - Posición: Superior-centro (x=160, y=20)
   - Fuente: Arial, 24pt, Bold
   - Color: Blanco

2. **t2** - Texto (Etiqueta variable)
   - Texto: "Nivel de agua"
   - Posición: (x=160, y=60)
   - Fuente: Arial, 18pt
   - Color: Blanco

3. **t3** - Texto (Valor)
   - Texto: "2"
   - Posición: (x=160, y=100)
   - Fuente: Arial, 32pt, Bold
   - Color: Blanco

4. **h0** - Slider
   - Posición: Centro (x=160, y=150)
   - Tamaño: 280x40
   - Rango: 0-4 (min-max)
   - Valor inicial: 2

5. **b7** - Botón (Guardar)
   - Texto: "GUARDAR"
   - Posición: Inferior-derecha (x=240, y=200)
   - Tamaño: 100x40
   - Color: Verde

6. **b8** - Botón (Cancelar)
   - Texto: "CANCELAR"
   - Posición: Inferior-izquierda (x=80, y=200)
   - Tamaño: 100x40
   - Color: Rojo

#### Configuración de Eventos:
- Slider h0 envía evento cuando cambia su valor
- Botones Guardar y Cancelar envían eventos cuando se presionan

### Página 4: Error

![Ejemplo de Pantalla de Error](placeholder)

#### Componentes:
1. **t1** - Texto (Título de error)
   - Texto: "ERROR 400"
   - Posición: Centro-superior (x=160, y=60)
   - Fuente: Arial, 32pt, Bold
   - Color: Rojo

2. **t2** - Texto (Descripción de error)
   - Texto: "Error de sistema"
   - Posición: Centro (x=160, y=120)
   - Fuente: Arial, 18pt
   - Color: Blanco

3. **alerta** - Indicador de alerta
   - Tipo: Imagen animada
   - Posición: Centro-superior (x=160, y=20)
   - Tamaño: 50x50
   - Animación: Parpadeo

4. **b9** - Botón (Reiniciar)
   - Texto: "REINICIAR"
   - Posición: Centro-inferior (x=160, y=180)
   - Tamaño: 150x50
   - Color: Azul

#### Configuración de Eventos:
- Botón Reiniciar envía evento cuando se presiona, debe programarse para reiniciar el sistema

### Página 5: Emergencia

![Ejemplo de Pantalla de Emergencia](placeholder)

#### Componentes:
1. **t1** - Texto (Título)
   - Texto: "PARADA DE EMERGENCIA"
   - Posición: Centro-superior (x=160, y=60)
   - Fuente: Arial, 28pt, Bold
   - Color: Rojo

2. **t2** - Texto (Instrucción)
   - Texto: "Sistema detenido por seguridad"
   - Posición: Centro (x=160, y=120)
   - Fuente: Arial, 18pt
   - Color: Blanco

3. **t3** - Texto (Instrucción)
   - Texto: "Suelte el botón de emergencia para continuar"
   - Posición: Centro-inferior (x=160, y=160)
   - Fuente: Arial, 16pt
   - Color: Blanco

4. **alarm** - Indicador de alarma
   - Tipo: Imagen animada
   - Posición: Centro-superior (x=160, y=20)
   - Tamaño: 80x80
   - Animación: Parpadeo rojo

#### Configuración de Eventos:
- No se requieren eventos específicos, ya que esta pantalla se muestra automáticamente al presionar el botón de emergencia físico

### Componente Global: Cuadro de Mensaje

#### Componentes:
1. **msgBox** - Panel de mensaje
   - Visible: No (inicialmente)
   - Posición: Centro (x=160, y=120)
   - Tamaño: 260x80
   - Color de fondo: Negro (semi-transparente)
   - Borde: Blanco

2. **msgText** - Texto del mensaje
   - Posición: Centro del panel
   - Fuente: Arial, 16pt
   - Color: Blanco
   - Alineación: Centro

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

1. **Eficiencia de Memoria**: Reutilizar imágenes y fuentes cuando sea posible
2. **Optimización de Comunicación**: Minimizar el número de actualizaciones en componentes estáticos
3. **Manejo de Errores**: Mostrar mensajes claros y específicos
4. **Coordinación con el Código**: Mantener sincronizados los IDs de componentes con el código del ESP32
5. **Pruebas Exhaustivas**: Verificar que todos los componentes funcionan correctamente

## Exportación y Carga del Diseño

1. Compilar el proyecto en el Editor Nextion
2. Verificar que no haya errores de compilación
3. Exportar el archivo .tft
4. Cargar el archivo a la pantalla mediante:
   - Tarjeta microSD: Copiar el archivo .tft a la raíz, insertar en la pantalla y encender
   - Cable Serie: Utilizar la función de carga del Editor Nextion con un adaptador UART-USB

## Conclusión

El diseño adecuado de las pantallas Nextion es crucial para una experiencia de usuario satisfactoria. Siguiendo estas guías, se obtendrá una interfaz profesional, intuitiva y funcional para el controlador de lavadora industrial.
