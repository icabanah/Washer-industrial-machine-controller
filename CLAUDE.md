# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Industrial washing machine controller for ESP32-WROOM with Nextion touchscreen interface. Developed by itrebolsoft for "Aureliano" industrial washing machines. The system implements three specialized washing programs with sophisticated state machine control and hardware abstraction.

## Build and Development Commands

### Compilation
```bash
# Verify/compile the Arduino code
arduino-cli compile --fqbn esp32:esp32:esp32-wroom-da ./mainController/

# Upload to ESP32 (requires port configuration)
arduino-cli upload -p [PORT] --fqbn esp32:esp32:esp32-wroom-da ./mainController/
```

### VS Code Tasks
- **Arduino: Verify** - Compiles the main controller code
- **Arduino: Upload** - Uploads compiled code to ESP32

## Core Architecture

### Modular Design (8 modules)
The system follows a sophisticated modular architecture with clear separation of concerns:

1. **Hardware Layer** (`hardware.h/.cpp`) - ESP32 and Nextion communication abstraction
2. **Program Controller** (`program_controller.h/.cpp`) - State machine for washing cycles  
3. **UI Controller** (`ui_controller.h/.cpp`) - Nextion touchscreen interface management
4. **Sensors** (`sensors.h/.cpp`) - Temperature (OneWire Dallas) and pressure (HX710B) management
5. **Actuators** (`actuators.h/.cpp`) - Motor, valve, and centrifuge control
6. **Storage** (`storage.h/.cpp`) - ESP32 Preferences-based persistent configuration
7. **Utils** (`utils.h/.cpp`) - Asynchronous task management and utilities
8. **Main Controller** (`mainController.ino`) - Arduino setup/loop with module coordination

### State Machine (7 states)
- IDLE, SELECTION, EDITING, EXECUTION, PAUSE, ERROR, EMERGENCY
- Emergency override capability with highest priority
- Non-blocking asynchronous timer system

### Three Washing Programs
- **Program 22**: Hot water with active temperature control (±2°C precision)
- **Program 23**: Cold water (temperature monitoring only)  
- **Program 24**: Multi-cycle configurable program (4 consecutive cycles)

Each program has 4 phases: Fill → Wash → Drain → Centrifuge (optional)

## Hardware Configuration

### Key Pin Assignments (config.h:7-34)
- Emergency Button: Pin 15 (hardware debounced)
- Motor Control: Bidirectional (pins 12, 14) + centrifuge (pin 27)
- Valves: Water inlet (26), steam/vapor (33), drainage (13)
- Door Lock: Electromagnetic (pin 25)
- Nextion Display: Serial2 (pins 16, 17) at 9600 baud
- Temperature Sensor: OneWire Dallas DS18B20 (pin 23)
- Pressure Sensor: HX710B (pins 4, 5)

### Critical Hardware Features
- Temperature control with ±2°C precision for hot water programs
- Multi-threshold water level monitoring via pressure sensor
- Bidirectional motor control with centrifuge safety sequences
- Emergency stop system with immediate hardware shutdown

## Development Guidelines

### File Structure
- `mainController/` - Main Arduino IDE project with all source code
- `Documentacion/Modularizacion/` - 12+ detailed module documentation files
- `Nextion/` - HMI interface files for different machine variants
- `Librerias/` - Required Arduino libraries (zipped format)

### Code Conventions
- All pin definitions and constants in `config.h`
- Hardware abstraction through dedicated hardware layer
- Asynchronous task management using AsyncTaskLib
- State transitions with comprehensive error handling
- Temperature sensor configured for 9-bit resolution
- Persistent storage using ESP32 Preferences API

### Key Dependencies
Libraries in `/Librerias/` (must be installed in Arduino IDE):
- AsyncTaskLib.zip - Asynchronous task management
- DallasTemperature.zip + OneWire.zip - Temperature sensor
- HX710B.zip - Pressure sensor interface
- Adafruit_BusIO.zip - I2C/SPI communication

### Safety Systems
- Emergency button overrides all operations immediately
- Temperature-based safety interlocks prevent overheating
- Water level monitoring with multiple safety thresholds
- Door lock system prevents operation with open door
- Comprehensive error states with automatic recovery

### Nextion HMI Interface
- Serial communication at 9600 baud on Serial2
- Custom command protocol with 0xFF,0xFF,0xFF termination
- Real-time display of temperature, pressure, and cycle progress
- Touch-based program selection and parameter editing
- Multi-language support preparation

## Testing and Validation

No automated test framework - validation through:
- Hardware-in-the-loop testing with actual sensors/actuators
- Nextion display simulation for UI testing
- State machine validation through documented test sequences
- Temperature control precision testing (±2°C requirement)
- Emergency system response time validation

## Documentation

Extensive documentation in `/Documentacion/`:
- Module architecture diagrams and interaction flows
- State machine documentation with transition conditions  
- Hardware interface specifications and pin assignments
- Program flow diagrams for each washing cycle
- Nextion HMI interface documentation with screen layouts

Current project progress: 89% complete according to plan-implementacion.md.