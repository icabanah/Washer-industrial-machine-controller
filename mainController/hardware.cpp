// hardware.cpp
#include "hardware.h"
#include "utils.h"

// Definición de la instancia global
HardwareClass Hardware;

// Callback para completar la inicialización de la pantalla Nextion
void completeNextionInitCallback()
{
  Hardware._completeNextionInit();
}

void HardwareClass::init()
{
  _initEmergencyButton();
  _initOutputs();
  _initNextion();

  _emergencyButtonState = LOW;
  _lastEmergencyButtonState = LOW;
  _lastDebounceTime = 0;
  _debounceDelay = 10; // 10ms para el antirrebote - optimizado para velocidad

  _nextionLastEvent = "";
  _nextionInitComplete = false;

  // Inicializar variables para eventos táctiles
  _hasValidTouch = false;
  _touchPage = 0;
  _touchComponent = 0;
  _touchEventType = 0;
}

void HardwareClass::_initEmergencyButton()
{
  // Configurar botón de emergencia sin resistencia interna (ya tiene pulldown físico)
  pinMode(PIN_BTN_EMERGENCIA, INPUT);
  Serial.println("Botón emergencia configurado como INPUT (pulldown físico)");
}

void HardwareClass::_initOutputs()
{
  // Configurar pines de salida
  pinMode(PIN_MOTOR_DIR_IZQ, OUTPUT);
  pinMode(PIN_MOTOR_DIR_DER, OUTPUT);
  pinMode(PIN_CENTRIFUGADO, OUTPUT);
  pinMode(PIN_VALVULA_AGUA, OUTPUT);
  pinMode(PIN_ELECTROV_VAPOR, OUTPUT);
  pinMode(PIN_VALVULA_DESFOGUE, OUTPUT);
  pinMode(PIN_MAGNET_PUERTA, OUTPUT);

  // Inicializar todas las salidas en LOW
  digitalWrite(PIN_MOTOR_DIR_IZQ, LOW);
  digitalWrite(PIN_MOTOR_DIR_DER, LOW);
  digitalWrite(PIN_CENTRIFUGADO, LOW);
  digitalWrite(PIN_VALVULA_AGUA, LOW);
  digitalWrite(PIN_ELECTROV_VAPOR, LOW);
  digitalWrite(PIN_VALVULA_DESFOGUE, LOW);
  digitalWrite(PIN_MAGNET_PUERTA, LOW);
}

void HardwareClass::_initNextion()
{
  // Inicializar comunicación serial con la pantalla Nextion
  // En ESP32, Serial2 puede asignarse a diferentes pines
  NEXTION_SERIAL.begin(NEXTION_BAUD_RATE, SERIAL_8N1, NEXTION_RX_PIN, NEXTION_TX_PIN);

  // En lugar de usar delay, programamos una tarea para completar la inicialización
  // después de que la pantalla haya tenido tiempo de inicializarse
  Utils.createTimeout(500, completeNextionInitCallback);
  Serial.println("Iniciando pantalla Nextion. Completando inicialización en 500ms...");
}

void HardwareClass::_completeNextionInit()
{
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

void HardwareClass::updateDebouncer()
{
  // Leer el estado actual del botón de emergencia
  uint8_t reading = ::digitalRead(PIN_BTN_EMERGENCIA);

  // Si el estado ha cambiado, resetear el temporizador de antirrebote
  if (reading != _lastEmergencyButtonState)
  {
    _lastDebounceTime = millis();
  }

  // Si ha pasado suficiente tiempo desde el último cambio, actualizar el estado
  if ((millis() - _lastDebounceTime) > _debounceDelay)
  {
    // Si el estado ha cambiado:
    if (reading != _emergencyButtonState)
    {
      _emergencyButtonState = reading;
    }
  }

  // Guardar la lectura para la próxima vez
  _lastEmergencyButtonState = reading;
}

/// @brief 
/// Verifica si el botón de emergencia está presionado.
/// Este método lee el estado del botón de emergencia y determina si está presionado o no.
/// @return 
/// Retorna `true` si el botón de emergencia está presionado (estado LOW), o `false` si no lo está (estado HIGH).
bool HardwareClass::isEmergencyButtonPressed()
{
  // Con pulldown físico: LOW = no presionado, HIGH = presionado
  return (_emergencyButtonState == LOW);
}

/// @brief 
/// Envía un comando a la pantalla Nextion.
/// @param command 
/// El comando a enviar a la pantalla Nextion.
/// Este comando debe ser una cadena de texto que la pantalla Nextion pueda interpretar.
/// Por ejemplo, para cambiar a la página 1, se enviaría el comando "page 1".
/// @note
/// Este método utiliza la comunicación serial para enviar comandos a la pantalla Nextion.
/// Se asegura de que el comando se envíe correctamente añadiendo los bytes de finalización necesarios.
/// Los comandos deben seguir el formato esperado por la pantalla Nextion, que generalmente incluye un nombre de componente seguido de una acción o valor.
/// Por ejemplo, para establecer el texto de un componente con ID `t1` a "Hola", se enviaría el comando `t1.txt="Hola"`.
void HardwareClass::nextionSendCommand(const String &command)
{
  // Envía el comando a la pantalla Nextion
  NEXTION_SERIAL.print(command);
  _sendNextionEndCmd();

  // Debug de comandos deshabilitado por performance
  // static unsigned long lastDebugTime = 0;
  // if (millis() - lastDebugTime > 3000) { // Cada 3 segundos
  //   Serial.println("📤 Comando enviado: " + command);
  //   lastDebugTime = millis();
  // }

  // Pausa reducida para evitar bloqueo del sistema
  delayMicroseconds(5000); // 5ms no bloqueante equivalente
}

void HardwareClass::_sendNextionEndCmd()
{
  // Enviar los tres bytes de finalización de comando FF FF FF
  // Estos bytes son necesarios para que la pantalla Nextion reconozca el fin de un comando
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

/// @brief 
/// Cambia a la página especificada en la pantalla Nextion.
/// Este método envía un comando a la pantalla Nextion para cambiar a una página específica.
/// @details
/// Este método utiliza el comando `page` para cambiar a la página deseada.
/// @param pageId 
/// El ID de la página a la que se desea cambiar.
/// Este ID debe corresponder a una página definida en el proyecto de la pantalla Nextion.
/// Por ejemplo, si se desea cambiar a la página 1, se llamaría a este método como `nextionSetPage(1)`.
void HardwareClass::nextionSetPage(uint8_t pageId)
{
  // Cambia a la página especificada en la pantalla Nextion
  nextionSendCommand("page " + String(pageId));
}

/// @brief 
/// Establece el texto de un componente específico en la pantalla Nextion utilizando su nombre.
/// Este método envía un comando a la pantalla Nextion para actualizar el texto de un componente.
/// @param componentId 
/// El ID del componente de texto en la pantalla Nextion.
/// Este ID debe corresponder al componente que se desea actualizar.
/// @param text 
/// El texto que se desea establecer en el componente.
/// Este texto se enviará entre comillas dobles para que sea interpretado correctamente por la pantalla.
/// Por ejemplo, si se desea establecer el texto "Hola" en un componente con ID `t1`, se llamaría a este método como `nextionSetText("t1", "Hola")`.
void HardwareClass::nextionSetText(const String &componentId, const String &text)
{
  // Establece el texto de un componente específico en la pantalla Nextion utilizando su nombre
  nextionSendCommand(componentId + ".txt=\"" + text + "\"");
}

/// @brief 
/// Establece el valor numérico de un componente específico en la pantalla Nextion utilizando su nombre.
/// Este método envía un comando a la pantalla Nextion para actualizar el valor de un componente específico.
/// @details
/// Este método utiliza el comando `val` para establecer el valor de un componente, como un campo numérico o un slider.
/// El comando se envía en el formato `componentId.val=value`, donde `componentId` es el identificador del componente y `value` es el valor numérico deseado.
/// @param componentId 
/// El ID del componente en la pantalla Nextion al que se le desea establecer el valor.
/// Este ID debe corresponder a un componente definido en el proyecto de la pantalla Nextion.
/// Por ejemplo, si el componente es un campo numérico con ID `n1`, el comando sería `n1.val=value`.
/// @param value 
/// El valor numérico que se desea establecer en el componente.
/// Este valor se enviará como un número entero y debe ser compatible con el tipo de componente.
/// Por ejemplo, si se desea establecer el valor 42 en un campo numérico, se llamaría a este método como `nextionSetValue("n1", 42)`.
void HardwareClass::nextionSetValue(const String &componentId, int value)
{
  // Establece el valor numérico de un componente específico en la pantalla Nextion utilizando su nombre
  nextionSendCommand(componentId + ".val=" + String(value));
}

/// @brief 
/// Verifica si hay eventos disponibles para leer desde la pantalla Nextion.
/// Este método lee la comunicación serial de la pantalla Nextion y procesa cualquier evento táctil o respuesta disponible.
/// @return 
/// Retorna `true` si se ha detectado un evento válido, `false` si no hay eventos disponibles.
/// @details
/// Este método utiliza la comunicación serial para leer los datos enviados por la pantalla Nextion.
/// Si se detecta un evento táctil válido, se actualizan las variables internas `_hasValidTouch`, `_touchPage`, `_touchComponent` y `_touchEventType`.
/// Si no se detecta un evento táctil, se imprime la respuesta recibida en el monitor serial para depuración.
/// Los eventos táctiles típicos tienen el formato `0x65 [PageID] [ComponentID] [EventType]`, donde:
/// - `PageID` es el ID de la página donde ocurrió el evento.
/// - `ComponentID` es el ID del componente que generó el evento.
/// - `EventType` indica el tipo de evento (1 para presionado, 0 para liberado).
bool HardwareClass::nextionCheckForEvents()
{
  // Verifica si hay eventos disponibles para leer desde la pantalla Nextion
  if (NEXTION_SERIAL.available())
  {
    return _readNextionResponse();
  }
  return false;
}

bool HardwareClass::_readNextionResponse()
{
  // Limpiar estado anterior
  _hasValidTouch = false;

  int index = 0;
  uint8_t endCount = 0;
  unsigned long startTime = millis();
  uint8_t rawBytes[16]; // Buffer para bytes raw

  // Limpiar el búfer anterior
  memset(_nextionBuffer, 0, sizeof(_nextionBuffer));
  memset(rawBytes, 0, sizeof(rawBytes));

  // Leer la respuesta con timeout
  // Los mensajes de Nextion terminan con 3 bytes FF FF FF
  while ((millis() - startTime) < NEXTION_TIMEOUT && endCount < 3)
  {
    if (NEXTION_SERIAL.available())
    {
      uint8_t c = NEXTION_SERIAL.read();
      if (c == 0xFF)
      {
        endCount++;
      }
      else
      {
        endCount = 0;
        if (index < sizeof(rawBytes) - 1)
        {
          rawBytes[index] = c;
          _nextionBuffer[index] = c;
          index++;
        }
      }
    }
  }

  // Procesar la respuesta si es válida
  if (endCount == 3 && index > 0)
  {
    _nextionLastEvent = String(_nextionBuffer);

    // Filtrar eventos automáticos/de estado de Nextion ANTES del debug
    if (index == 1) {
      if (rawBytes[0] == 0x1A) {
        // Evento 0x1A: datos de variable numérica automáticos de Nextion
        return false; // No es un evento que necesitemos procesar
      }
      if (rawBytes[0] == 0x02) {
        // Evento 0x02: mensaje de estado/respuesta automática de Nextion
        return false; // No es un evento que necesitemos procesar
      }
    }

    // Interpretar eventos táctiles específicos
    // Formato típico de evento táctil: 0x65 [PageID] [ComponentID] [EventType]
    if (index >= 4 && rawBytes[0] == 0x65)
    {
      _hasValidTouch = true;
      _touchPage = rawBytes[1];
      _touchComponent = rawBytes[2];
      _touchEventType = rawBytes[3];

      return true;
    }

    // Otros tipos de respuesta (no táctiles)
    return true;
  }

  return false;
}

/// @brief 
/// Obtiene el último evento capturado desde la pantalla Nextion.
/// Este método retorna el contenido del último evento almacenado en `_nextionLastEvent`.
/// @note
/// El evento puede ser un evento táctil o una respuesta de la pantalla Nextion.
/// Los eventos táctiles típicos tienen el formato `0x65 [PageID] [ComponentID] [EventType]`, donde:
/// - `PageID` es el ID de la página donde ocurrió el evento.
/// - `ComponentID` es el ID del componente que generó el evento.
/// - `EventType` indica el tipo de evento (1 para presionado, 0 para liberado).
/// @return 
/// Retorna el último evento como una cadena de texto.
String HardwareClass::nextionGetLastEvent()
{
  // Retorna el último evento capturado desde la pantalla Nextion
  return _nextionLastEvent;
}


/// @brief 
/// Verifica si la inicialización de la pantalla Nextion está completa  
/// @return 
/// true si la inicialización está completa, false en caso contrario
bool HardwareClass::isNextionInitComplete()
{
  // Retorna si la inicialización de la pantalla Nextion está completa
  return _nextionInitComplete;
}

/// @brief 
/// Prueba de conectividad con la pantalla Nextion
/// Este método envía comandos simples a la pantalla Nextion para verificar su conectividad y respuesta.
/// @details
/// - Envía un comando para obtener el estado de sueño (`get sleep`).
/// - Espera una breve respuesta.
/// - Si hay respuesta, imprime los bytes recibidos.
/// - Si no hay respuesta, imprime un mensaje de error y posibles causas.
/// - Intenta cambiar a la página 0 y enviar un comando de texto simple.
void HardwareClass::testNextionConnectivity()
{
  // Serial.println("🔧 === PRUEBA DE CONECTIVIDAD NEXTION ===");

  // Verificar si hay respuesta de la pantalla
  NEXTION_SERIAL.flush(); // Limpiar buffer

  // Enviar comando simple que debería generar respuesta
  Serial.println("Enviando comando: get sleep");
  nextionSendCommand("get sleep");

  // Esperar respuesta breve no bloqueante
  delayMicroseconds(10000); // 10ms no bloqueante

  if (NEXTION_SERIAL.available())
  {
    Serial.println("✅ Nextion responde:");
    while (NEXTION_SERIAL.available())
    {
      Serial.print("0x");
      Serial.print(NEXTION_SERIAL.read(), HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  else
  {
    Serial.println("❌ Sin respuesta de Nextion");
    Serial.println("Verificar:");
    Serial.println("- Conexiones RX/TX (pines 16/17)");
    Serial.println("- Alimentación de pantalla");
    Serial.println("- Baudrate (115200)");
  }

  // Intentar comando de página
  Serial.println("Enviando: page 0");
  nextionSendCommand("page 0");
  delayMicroseconds(5000); // 5ms no bloqueante

  // Intentar comando de texto simple
  Serial.println("Enviando comando de texto...");
  nextionSetText("lbl_titulo", "TEST");

  Serial.println("🔧 === FIN PRUEBA CONECTIVIDAD ===");
}

void HardwareClass::digitalWrite(uint8_t pin, uint8_t state)
{
  // Wrapper para la función digitalWrite estándar
  ::digitalWrite(pin, state);
}

uint8_t HardwareClass::digitalRead(uint8_t pin)
{
  // Wrapper para la función digitalRead estándar
  return ::digitalRead(pin);
}

// ===== FUNCIONES PARA EVENTOS TÁCTILES =====

bool HardwareClass::hasValidTouchEvent()
{
  // Retorna si hay un evento táctil válido disponible
  return _hasValidTouch;
}

uint8_t HardwareClass::getTouchEventPage()
{
  // Retorna la página del último evento táctil
  return _touchPage;
}

uint8_t HardwareClass::getTouchEventComponent()
{
  // Retorna el ID del componente del último evento táctil
  return _touchComponent;
}

uint8_t HardwareClass::getTouchEventType()
{
  // Retorna el tipo de evento táctil (0=liberado, 1=presionado)
  return _touchEventType;
}