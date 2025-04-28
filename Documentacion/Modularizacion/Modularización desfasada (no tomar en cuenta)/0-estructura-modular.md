# Estructura Modular del Controlador de Lavadora Industrial

## Introducción

La presente propuesta busca reorganizar el código del controlador de lavadora industrial en módulos lógicos sin modificar su funcionalidad actual. La modularización permitirá un mejor mantenimiento, mayor facilidad para implementar nuevas características y una estructura más clara para entender el sistema.

## Analogía: Sistema Modular de Orquesta

Podemos visualizar el controlador como una orquesta donde cada sección de instrumentos (módulos) tiene su propia partitura (código) pero todos siguen la batuta del director (programa principal). Cada sección puede practicar y afinar independientemente, pero cuando toca la sinfonía completa (aplicación), todas las partes trabajan armoniosamente.

## Estructura de Archivos Propuesta

La nueva estructura agrupará el código en archivos específicos para cada funcionalidad:

```
automLavanderia_v1/
│
├── automLavanderia_v1.ino       # Archivo principal (incluye setup, loop)
├── config.h                     # Configuraciones, constantes y definiciones globales
├── hardware.cpp                 # Gestión del hardware físico
├── hardware.h
├── ui_controller.cpp            # Controlador de interfaz de usuario
├── ui_controller.h
├── program_controller.cpp       # Controlador de programas de lavado
├── program_controller.h
├── sensors.cpp                  # Gestión de sensores (temperatura, presión)
├── sensors.h
├── actuators.cpp                # Control de actuadores (motor, válvulas)
├── actuators.h
├── storage.cpp                  # Gestión de almacenamiento (EEPROM)
├── storage.h
└── utils.cpp                    # Utilidades generales
└── utils.h
```

## Beneficios de la Modularización

1. **Mantenibilidad**: Cada módulo puede ser modificado de forma independiente sin afectar a otros módulos.
2. **Escalabilidad**: Facilita la implementación de nuevas características al permitir añadir o modificar módulos específicos.
3. **Legibilidad**: Estructura clara y organizada que mejora la comprensión del código.
4. **Reutilización**: Los módulos pueden ser reutilizados en otros proyectos similares.
5. **Desarrollo en equipo**: Permite que diferentes desarrolladores trabajen en diferentes módulos simultáneamente.
6. **Pruebas**: Facilita la creación de pruebas unitarias para cada módulo.

## Principio de Responsabilidad Única

Cada módulo tendrá una única responsabilidad bien definida, siguiendo el principio de responsabilidad única (SRP) de SOLID. Esto permite:

- Reducir la complejidad
- Facilitar la comprensión
- Mejorar la mantenibilidad
- Minimizar los errores

Los siguientes documentos detallan cada uno de los módulos propuestos, su funcionalidad y estructura.
