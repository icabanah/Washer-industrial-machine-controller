// hardware.cpp
#include "hardware.h"
#include "utils.h"

// Definición de la instancia global
HardwareClass Hardware;

// Callback para completar la inicialización de la pantalla Nextion
void completeNextionInitCallback() {
  Hardware._completeNextionInit();
}

void HardwareClass::init() {
  _initEmergencyButton();
  _initOutputs();
  _initNextion();
  
  _emergencyButtonState = LOW;
  _lastEmergencyButtonState = LOW;
  _lastDebounceTime = 0;
  _debounceDelay = 50;  // 50ms para el antirrebote
  
  _nextionLastEvent = "";
  _nextionInitComplete = false;
}

void HardwareClass::_initEmergencyButton() {
  // Configurar botón de emergencia sin resistencia interna (ya tiene pulldown físico)
  pinMode(PIN_BTN_EMERGENCIA, INPUT);
  Serial.println("Botón emergencia configurado como INPUT (pulldown físico)");
}

void HardwareClass::_initOutputs() {
  // Configurar pines de salida
  pinMode(PIN_MOTOR_DIR_A, OUTPUT);
  pinMode(PIN_MOTOR_DIR_B, OUTPUT);
  pinMode(PIN_CENTRIFUGADO, OUTPUT);
  pinMode(PIN_VALVULA_AGUA, OUTPUT);
  pinMode(PIN_ELECTROV_VAPOR, OUTPUT);
  pinMode(PIN_VALVULA_DESFOGUE, OUTPUT);
  pinMode(PIN_MAGNET_PUERTA, OUTPUT);
  
  // Inicializar todas las salidas en LOW
  digitalWrite(PIN_MOTOR_DIR_A, LOW);
  digitalWrite(PIN_MOTOR_DIR_B, LOW);
  digitalWrite(PIN_CENTRIFUGADO, LOW);
  digitalWrite(PIN_VALVULA_AGUA, LOW);
  digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  digitalWrite(PIN_MAGNET_PUERTA, LOW);
}

void HardwareClass::_initNextion() {
  // Inicializar comunicación serial con la pantalla Nextion
  // En ESP32, Serial2 puede asignarse a diferentes pines
  NEXTION_SERIAL.begin(NEXTION_BAUD_RATE, SERIAL_8N1, NEXTION_RX_PIN, NEXTION_TX_PIN);
  
  // En lugar de usar delay, programamos una tarea para completar la inicialización
  // después de que la pantalla haya tenido tiempo de inicializarse
  Utils.createTimeout(500, completeNextionInitCallback);
  Serial.println("Iniciando pantalla Nextion. Completando inicialización en 500ms...");
}

void HardwareClass::_completeNextionInit() {
  // Establecer el brillo a un nivel medio (valores 0-100)
  nextionSendCommand("dim=50");
  
  // Ir a la página de bienvenida
  nextionSetPage(NEXTION_PAGE_WELCOME);
  
  // Enviar comando simple de verificación de comunicación
  nextionSendCommand("get dim");
  
  // Comando de prueba para verificar respuesta
  nextionSendCommand("page 0");
  
  _nextionInitComplete = true;
  Serial.println("✅ Pantalla Nextion inicializada completamente");
}

void HardwareClass::updateDebouncer() {
  // Leer el estado actual del botón de emergencia
  uint8_t reading = ::digitalRead(PIN_BTN_EMERGENCIA);
  
  // Si el estado ha cambiado, resetear el temporizador de antirrebote
  if (reading != _lastEmergencyButtonState) {
    _lastDebounceTime = millis();
  }
  
  // Si ha pasado suficiente tiempo desde el último cambio, actualizar el estado
  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    // Si el estado ha cambiado:
    if (reading != _emergencyButtonState) {
      _emergencyButtonState = reading;
    }
  }
  
  // Guardar la lectura para la próxima vez
  _lastEmergencyButtonState = reading;
}

bool HardwareClass::isEmergencyButtonPressed() {
  // Con pulldown físico: LOW = no presionado, HIGH = presionado
  return (_emergencyButtonState == HIGH);
}

void HardwareClass::nextionSendCommand(const String& command) {
  // Envía el comando a la pantalla Nextion
  NEXTION_SERIAL.print(command);
  _sendNextionEndCmd();
  
  // Debug mejorado de comandos
  Serial.print(">>> Nextion CMD: [");
  Serial.print(command);
  Serial.println("]");
  
  // Pequeña pausa para asegurar envío
  delay(10);
}

void HardwareClass::_sendNextionEndCmd() {
  // Enviar los tres bytes de finalización de comando FF FF FF
  // Estos bytes son necesarios para que la pantalla Nextion reconozca el fin de un comando
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

void HardwareClass::nextionSetPage(uint8_t pageId) {
  // Cambia a la página especificada en la pantalla Nextion
  nextionSendCommand("page " + String(pageId));
}

void HardwareClass::nextionSetText(const String& componentId, const String& text) {
  // Establece el texto de un componente específico en la pantalla Nextion utilizando su nombre
  nextionSendCommand(componentId + ".txt=\"" + text + "\"");
}

void HardwareClass::nextionSetValue(const String& componentId, int value) {
  // Establece el valor numérico de un componente específico en la pantalla Nextion utilizando su nombre
  nextionSendCommand(componentId + ".val=" + String(value));
}

bool HardwareClass::nextionCheckForEvents() {
  // Verifica si hay eventos disponibles para leer desde la pantalla Nextion
  if (NEXTION_SERIAL.available()) {
    return _readNextionResponse();
  }
  return false;
}

bool HardwareClass::_readNextionResponse() {
  int index = 0;
  uint8_t endCount = 0;
  unsigned long startTime = millis();
  
  // Limpiar el búfer anterior
  memset(_nextionBuffer, 0, sizeof(_nextionBuffer));
  
  // Leer la respuesta con timeout
  // Los mensajes de Nextion terminan con 3 bytes FF FF FF
  while ((millis() - startTime) < NEXTION_TIMEOUT && endCount < 3) {
    if (NEXTION_SERIAL.available()) {
      uint8_t c = NEXTION_SERIAL.read();
      if (c == 0xFF) {
        endCount++;
      } else {
        endCount = 0;
        if (index < sizeof(_nextionBuffer) - 1) {
          _nextionBuffer[index++] = c;
        }
      }
    }
  }
  
  // Procesar la respuesta si es válida
  if (endCount == 3 && index > 0) {
    _nextionLastEvent = String(_nextionBuffer);
    
    // Debug de eventos
    Serial.print("Evento Nextion: ");
    for (int i = 0; i < index; i++) {
      Serial.print(_nextionBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    return true;
  }
  
  return false;
}

String HardwareClass::nextionGetLastEvent() {
  // Retorna el último evento capturado desde la pantalla Nextion
  return _nextionLastEvent;
}

bool HardwareClass::isNextionInitComplete() {
  // Retorna si la inicialización de la pantalla Nextion está completa
  return _nextionInitComplete;
}

void HardwareClass::testNextionConnectivity() {
  Serial.println("🔧 === PRUEBA DE CONECTIVIDAD NEXTION ===");
  
  // Verificar si hay respuesta de la pantalla
  NEXTION_SERIAL.flush(); // Limpiar buffer
  
  // Enviar comando simple que debería generar respuesta
  Serial.println("Enviando comando: get sleep");
  nextionSendCommand("get sleep");
  
  // Esperar respuesta breve
  delay(100);
  
  if (NEXTION_SERIAL.available()) {
    Serial.println("✅ Nextion responde:");
    while (NEXTION_SERIAL.available()) {
      Serial.print("0x");
      Serial.print(NEXTION_SERIAL.read(), HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("❌ Sin respuesta de Nextion");
    Serial.println("Verificar:");
    Serial.println("- Conexiones RX/TX (pines 16/17)");
    Serial.println("- Alimentación de pantalla");
    Serial.println("- Baudrate (115200)");
  }
  
  // Intentar comando de página
  Serial.println("Enviando: page 0");
  nextionSendCommand("page 0");
  delay(50);
  
  // Intentar comando de texto simple
  Serial.println("Enviando comando de texto...");
  nextionSetText("lbl_titulo", "TEST");
  
  Serial.println("🔧 === FIN PRUEBA CONECTIVIDAD ===");
}

void HardwareClass::digitalWrite(uint8_t pin, uint8_t state) {
  // Wrapper para la función digitalWrite estándar
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin) {
  // Wrapper para la función digitalRead estándar
  return ::digitalRead(pin);
}