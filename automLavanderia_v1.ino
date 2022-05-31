#include <Adafruit_MAX31865.h>
#include <AsyncTaskLib.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Iniciamos el lcd
const int rs = 19, en = 18, d4 = 17, d5 = 16, d6 = 15, d7 = 14;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 thermo = Adafruit_MAX31865(37, 35, 33, 31);
// rdi = pin 39
#define RREF 430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL 100.0

// Entradas
#define btnParar 2
#define btnDisminuir 3
#define btnAumentar 4
#define btnEditar 24
#define btnProgramarNivelAgua 43
#define btnComenzar 26
#define sensorPuerta 23
// #define reserva 25
// #define reservaSensorPresionAlt A0

// Sensor de presion
// #define salidaSensor 27
// #define salidaCLK 29

// Salidas
#define MotorDirA 12
#define MotorDirB 11
#define ValvulAgua 10
#define ElectrovVapor 9
#define ValvulOnOff 8
#define MagnetPuerta 7
#define buzzer 41
// #define LedConfirmacion 0

// Variables de sensores
uint16_t valorSensorTemperatura = 0;

// Definimos variables de los programas
// uint8_t NivelAgua[3][4] = {{6, 6, 7, 4}, {5, 4, 4, 5}, {1, 3, 3, 2}};
// uint8_t RotacionTam[3][4] = {{1, 1, 2, 2}, {2, 3, 3, 2}, {3, 2, 3, 2}};
// uint8_t TemperaturaLim[3][4] = {{30, 0, 35, 45}, {45, 0, 40, 45}, {55, 0, 34, 30}};
// uint8_t TemporizadorLim[3][4] = {{1, 2, 1, 1}, {1, 2, 3, 1}, {1, 2, 2, 2}};
// uint8_t TiempoEntFase[3][4] = {{5, 4, 4, 5}, {3, 4, 4, 3}, {3, 4, 4, 3}};

// uint8_t NivelAgua[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
// uint8_t RotacionTam[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
// uint8_t TemperaturaLim[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
// uint8_t TemporizadorLim[3][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
// uint8_t TiempoRotacion[3][2] = {{0, 0}, {0, 0}, {0, 0}};

uint8_t NivelAgua[3][4];
uint8_t RotacionTam[3][4];
uint8_t TemperaturaLim[3][4];
uint8_t TemporizadorLim[3][4];
uint8_t TiempoRotacion[3][2] = {{3, 2}, {3, 3}, {4, 3}};
uint8_t TiempoEntFase[3][4] = {{3, 3, 3, 5}, {4, 3, 2, 3}, {4, 3, 4, 5}};
// uint8_t TiempoRotacion[3][2];
// uint8_t TiempoEntFase[3][4];

// Variables bandera
boolean tiempoCumplido = false;
boolean programaTerminado = true;
boolean programaEnPausa = false;
boolean editandoProgramaEjecucion = false;
// boolean terminarEdicion = false;
boolean nivelActivo = LOW;

// Antirebote
uint8_t flagBtnAumentar1 = 0;
uint8_t flagBtnDisminuir1 = 0;
uint8_t flagBtnComenzar1 = 0;
uint8_t flagBtnEditar1 = 0;
uint8_t flagBtnParar1 = 0;

// Variables de proceso
boolean pausa = false;
uint8_t programa = 1;
uint8_t numeroVariable = 1;
uint8_t valorVariable = 0;
uint8_t fase = 1;
uint8_t nivelEdicion = 0;
uint8_t direccion = 1;
// uint8_t PreDireccion = 0;

int8_t minutos[2] = {0, 0};
int8_t segundos[2] = {0, 0};
int16_t segunderoTemporizador = 0;
uint8_t segunderoEntreFase = 0;
uint8_t valorTemperatura = 40;
uint8_t valorPresion = 4;

AsyncTask segundosMotor(1000, true, []()
                        { segundos[0]++; });

void pintarVariables()
{
  lcd.setCursor(6, 0);
  lcd.print(fase);

  lcd.setCursor(10, 0);
  lcd.print(valorPresion);

  lcd.setCursor(14, 0);
  lcd.print(valorTemperatura);

  lcd.setCursor(3, 1);
  if (minutos[1] < 10)
  {
    lcd.print(0);
    lcd.print(minutos[1]);
  }
  else
  {
    lcd.print(minutos[1]);
  }
  lcd.setCursor(6, 1);
  if (segundos[1] < 10)
  {
    lcd.print(0);
    lcd.print(segundos[1]);
  }
  else
  {
    lcd.print(segundos[1]);
  }
}

AsyncTask segundosTemporizador(1000, true, []()
                               { 
  segunderoTemporizador--;
  // Serial.print("")
  // if (!programaEnPausa)
  // {
    minutos[1] = (segunderoTemporizador / 60);
    segundos[1] = segunderoTemporizador - (minutos[1] * 60); 
  // }
  // else{
  //   minutos[1] = 0;
  //   segundos[1] = segunderoTemporizador;
  // }
  // controladorSensorTemperatura();
  if (!editandoProgramaEjecucion){ pintarVariables(); } });

// AsyncTask delayTemporizador(100, true, []()
//                             { flagLCD = !flagLCD; });

void setup()
{
  // Definimos entradas
  pinMode(btnAumentar, INPUT);
  pinMode(btnComenzar, INPUT);
  pinMode(btnDisminuir, INPUT);
  pinMode(btnEditar, INPUT);
  pinMode(btnParar, INPUT);

  // Definimos salidas
  pinMode(MagnetPuerta, OUTPUT);
  pinMode(ValvulAgua, OUTPUT);
  pinMode(MotorDirA, OUTPUT);
  pinMode(MotorDirB, OUTPUT);
  pinMode(ElectrovVapor, OUTPUT);
  pinMode(ValvulOnOff, OUTPUT);

  // Inicializamos salidas
  digitalWrite(MagnetPuerta, LOW);
  digitalWrite(ValvulAgua, LOW);
  digitalWrite(ElectrovVapor, LOW);
  digitalWrite(ValvulOnOff, LOW);
  digitalWrite(MotorDirA, LOW);
  digitalWrite(MotorDirB, LOW);

  // Inicializamos el lcd
  lcd.begin(16, 2);

  // Iniciamos el modulo MAX31865
  thermo.begin(MAX31865_2WIRE); // set to 2WIRE or 4WIRE as necessary

  // Inicializamos el puerto serial para depurar
  Serial.begin(115200);
  Serial.println("Puerto serial iniciado");

  // Recuperamos valores del EEPROM
  recuperarValoresEEPROM();

  // inicializamos subprocesos
  pintarVentanaSeleccion();
}

void loop()
{
  // Eligiendo programa
  if (digitalRead(btnAumentar) == nivelActivo)
  {
    if (flagBtnAumentar1 == 0)
    {
      flagBtnAumentar1 = 1;
      programa++;
      if (programa > 3)
        programa = 1;
      pintarVentanaSeleccion();
    }
  }
  else
  {
    flagBtnAumentar1 = 0;
  }

  if (digitalRead(btnDisminuir) == nivelActivo)
  {
    if (flagBtnDisminuir1 == 0)
    {
      flagBtnDisminuir1 = 1;
      Serial.println("btnDisminuir");
      programa--;
      if (programa < 1)
      {
        programa = 3;
      }
      pintarVentanaSeleccion();
      // eeprom_write();
      // delay(100);
    }
  }
  else
  {
    flagBtnDisminuir1 = 0;
  }

  // Comenzando programa seleccionado
  if (digitalRead(btnComenzar) == nivelActivo)
  {
    if (flagBtnComenzar1 == 0)
    {
      flagBtnComenzar1 = 1;
      iniciarPrograma();
      while (!tiempoCumplido)
      {
        controladorTemporizador();
        controladorDireccionMotor();
        // controladorSensorTemperatura();
        if (digitalRead(btnParar) == nivelActivo)
        {
          if (flagBtnParar1 == 0)
          {
            Serial.println("Programa terminado con boton");
            flagBtnParar1 = 1;
            programaTerminado = true;
            terminarPrograma();
            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flagBtnParar1 = 0;
        }

        if (digitalRead(btnEditar) == nivelActivo)
        {
          if (flagBtnEditar1 == 0)
          {
            flagBtnEditar1 = 1;
            editandoProgramaEjecucion = true;
            editarPrograma();
          }
        }
        else
        {
          flagBtnEditar1 = 0;
        }
      }
    }
  }
  else
  {
    flagBtnComenzar1 = 0;
  }

  // Edicion de programa
  if (digitalRead(btnEditar) == nivelActivo)
  {
    if (flagBtnEditar1 == 0)
    {
      // Edicion de numero de variable
      flagBtnEditar1 = 1;
      editarPrograma();
    }
  }
  else
  {
    flagBtnEditar1 = 0;
  }

  if (digitalRead(btnParar) == nivelActivo)
  {
    if (flagBtnParar1 == 0)
    {
      guardarValoresEEPROM();
    }
  }
  else
  {
    flagBtnParar1 = 0;
  }
}

// Subprocesos de pintar el LCD
void pintarVentanaSeleccion()
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(programa + 21);

  lcd.setCursor(4, 0);
  lcd.print("N");
  for (uint8_t i = 0; i < 4; i++)
  {
    lcd.setCursor(i + 5, 0);
    lcd.print(NivelAgua[programa - 1][i]);
  }

  lcd.setCursor(10, 0);
  lcd.print("T");
  for (uint8_t i = 0; i < 4; i++)
  {
    lcd.setCursor(i + 11, 0);
    lcd.print(TemporizadorLim[programa - 1][i]);
  }

  // Segunda fila
  lcd.setCursor(0, 1);
  lcd.print("T");
  lcd.setCursor(2, 1);
  lcd.print("/");
  lcd.setCursor(5, 1);
  lcd.print("/");
  lcd.setCursor(8, 1);
  lcd.print("/");
  for (uint8_t i = 0; i < 4; i++)
  {
    lcd.setCursor(3 * i, 1);
    lcd.print(TemperaturaLim[programa - 1][i]);
  }

  lcd.setCursor(12, 1);
  lcd.print("R");
  for (uint8_t i = 0; i < 3; i++)
  {
    lcd.setCursor(i + 13, 1);
    lcd.print(RotacionTam[programa - 1][i]);
  }
}

void pintarVentanaEdicionMenu()
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(programa + 21);

  lcd.setCursor(4, 0);
  lcd.print("Seleccione:");

  // Segunda fila
  switch (numeroVariable)
  {
  case 1:
    lcd.setCursor(0, 1);
    lcd.print("Fase:");
    lcd.setCursor(5, 1);
    lcd.print(fase);
    lcd.setCursor(8, 1);
    lcd.print("Nivel:");
    lcd.setCursor(14, 1);
    lcd.print(NivelAgua[programa - 1][fase - 1]);
    break;

  case 2:
    lcd.setCursor(0, 1);
    lcd.print("Fase:");
    lcd.setCursor(5, 1);
    lcd.print(fase);
    lcd.setCursor(8, 1);
    lcd.print("Tiemp:");
    lcd.setCursor(14, 1);
    lcd.print(TemporizadorLim[programa - 1][fase - 1]);
    break;

  case 3:
    lcd.setCursor(0, 1);
    lcd.print("Fase:");
    lcd.setCursor(5, 1);
    lcd.print(fase);
    lcd.setCursor(8, 1);
    lcd.print("Tempe:");
    lcd.setCursor(14, 1);
    lcd.print(TemperaturaLim[programa - 1][fase - 1]);
    break;

  case 4:
    lcd.setCursor(0, 1);
    lcd.print("Fase:");
    lcd.setCursor(5, 1);
    lcd.print(fase);
    lcd.setCursor(8, 1);
    lcd.print("VRota:");
    lcd.setCursor(14, 1);
    lcd.print(RotacionTam[programa - 1][fase - 1]);
    break;

  default:
    break;
  }
}

void pintarVentanaEjecucion()
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(programa + 21);

  lcd.setCursor(4, 0);
  lcd.print("F:");
  lcd.setCursor(6, 0);
  lcd.print(fase);

  lcd.setCursor(8, 0);
  lcd.print("N:");
  lcd.setCursor(10, 0);
  lcd.print(valorPresion);

  lcd.setCursor(12, 0);
  lcd.print("T:");
  lcd.setCursor(14, 0);
  lcd.print(valorTemperatura);

  // Segunda fila
  lcd.setCursor(0, 1);
  lcd.print("Ti:");
  lcd.setCursor(3, 1);
  if (minutos[1] < 10)
  {
    lcd.print(0);
    lcd.print(minutos[1]);
  }
  else
  {
    lcd.print(minutos[1]);
  }
  lcd.setCursor(5, 1);
  lcd.print(":");
  lcd.setCursor(6, 1);
  if (segundos[1] < 10)
  {
    lcd.print(0);
    lcd.print(segundos[1]);
  }
  else
  {
    lcd.print(segundos[1]);
  }
  lcd.setCursor(12, 1);
  lcd.print("R:");
  lcd.setCursor(14, 1);
  lcd.print(RotacionTam[programa - 1][fase - 1]);
}

// Subprocesos de obtencion/edicin de variables
void editarValorVariable()
{
  switch (numeroVariable)
  {
  case 1:
    NivelAgua[programa - 1][fase - 1] = valorVariable;
    break;

  case 2:
    TemporizadorLim[programa - 1][fase - 1] = valorVariable;
    break;

  case 3:
    TemperaturaLim[programa - 1][fase - 1] = valorVariable;
    break;

  case 4:
    RotacionTam[programa - 1][fase - 1] = valorVariable;
    break;

  default:
    break;
  }
}

void obtenerValorVariable()
{
  switch (numeroVariable)
  {
  case 1:
    valorVariable = NivelAgua[programa - 1][fase - 1];
    break;

  case 2:
    valorVariable = TemporizadorLim[programa - 1][fase - 1];
    break;

  case 3:
    valorVariable = TemperaturaLim[programa - 1][fase - 1];
    break;

  case 4:
    valorVariable = RotacionTam[programa - 1][fase - 1];
    break;

  default:
    break;
  }
}

// subprecesos del temporizador
void iniciarTemporizador()
{
  // minutos[1] = min;
  if (!programaEnPausa)
  {
    // segunderoTemporizador = TemporizadorLim[programa - 1][fase - 1] * 60;
    segunderoTemporizador = TemporizadorLim[programa - 1][fase - 1] * 10;
    // Serial.print("SegunderoTemporizador en ejecucion: ");
    // Serial.println(segunderoTemporizador);
  }
  else
  {
    segunderoTemporizador = TiempoEntFase[programa - 1][fase - 1];
    // Serial.print("SegunderoTemporizador en pausa: ");
    // Serial.println(segunderoTemporizador);
  }
  // Serial.println(segunderoTemporizador);
  segundosTemporizador.Start();
}

void controladorTemporizador()
{
  if (!programaTerminado)
  {
    segundosTemporizador.Update();
    if (!programaEnPausa)
    {
      if (segunderoTemporizador == 0)
      {
        programaEnPausa = true;
        programaTerminado = false;
        terminarPrograma();
        iniciarTemporizador();
        // Serial.println("Programa en pausa");
      }
    }
    else
    {
      if (segunderoTemporizador == 0)
      {
        segundosTemporizador.Stop();
        programaEnPausa = false;
        iniciarPrograma();
        fase++;
        if (fase > 4)
        {
          programaEnPausa = false;
          programaTerminado = true;
          terminarPrograma();
          fase = 1;
        }
        iniciarTemporizador();
        // Serial.println("Programa reanudado");
      }
    }

    if (!programaEnPausa)
    {
      if (segunderoTemporizador == 0)
      {
        segundosTemporizador.Stop();
        programaEnPausa = true;
        terminarPrograma();
        segunderoTemporizador = TiempoEntFase[programa - 1][fase - 1];
        segunderoTemporizador = segunderoTemporizador / 60;
        iniciarTemporizador();
      }
    }
    else
    {
      if (segunderoTemporizador == 0)
      {
        segundosTemporizador.Stop();
        programaEnPausa = false;
        fase++;
        iniciarTemporizador();
        iniciarPrograma();
        if (fase > 4)
        {
          terminarPrograma();
        }
      }
    }
  }
}

// Subprocesos del motor
void controladorDireccionMotor()
{
  if (!programaTerminado || !programaEnPausa)
  {
    segundosMotor.Update();
    uint8_t tiempoRotacionTemp = TiempoRotacion[RotacionTam[programa - 1][fase - 1] - 1][0];
    uint8_t tiempoPausaTemp = TiempoRotacion[RotacionTam[programa - 1][fase - 1] - 1][1];
    switch (direccion)
    {
    case 1:
      digitalWrite(MotorDirA, HIGH);
      digitalWrite(MotorDirB, LOW);
      if (segundos[0] == tiempoRotacionTemp)
      {
        direccion = 2;
        pausa = true;
        segundos[0] = 0;
        // Serial.print("Controlador motor - direccion");
        // Serial.println(direccion);
      }
      break;

    case 2:
      digitalWrite(MotorDirA, LOW);
      digitalWrite(MotorDirB, LOW);
      if (segundos[0] == tiempoPausaTemp)
      {
        if (pausa == true)
        {
          direccion = 3;
        }
        else
        {
          direccion = 1;
        }
        segundos[0] = 0;
      }
      // Serial.print("Controlador motor - direccion");
      // Serial.println(direccion);
      break;

    case 3:
      digitalWrite(MotorDirA, LOW);
      digitalWrite(MotorDirB, HIGH);
      if (segundos[0] == tiempoRotacionTemp)
      {
        direccion = 2;
        pausa = false;
        segundos[0] = 0;
      }
      break;

    default:
      break;
    }
  }
}

// Controlador del sensor de temperatura
void controladorSensorTemperatura()
{
  if (!programaTerminado)
  {
    uint16_t rtd = thermo.readRTD();
    float ratio = rtd;
    ratio /= 32768;
    Serial.print("Temperature = ");
    Serial.println(thermo.temperature(RNOMINAL, RREF));
    Serial.print("Ratio = ");
    Serial.println(ratio, 8);
    Serial.print("Resistance = ");
    Serial.println(RREF * ratio, 8);
    Serial.println();
    // Check and print any faults
    // uint8_t fault = thermo.readFault();
    // if (fault)
    // {
    //   Serial.print("Fault 0x");
    //   Serial.println(fault, HEX);
    //   if (fault & MAX31865_FAULT_HIGHTHRESH)
    //   {
    //     Serial.println("RTD High Threshold");
    //   }
    //   if (fault & MAX31865_FAULT_LOWTHRESH)
    //   {
    //     Serial.println("RTD Low Threshold");
    //   }
    //   if (fault & MAX31865_FAULT_REFINLOW)
    //   {
    //     Serial.println("REFIN- > 0.85 x Bias");
    //   }
    //   if (fault & MAX31865_FAULT_REFINHIGH)
    //   {
    //     Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    //   }
    //   if (fault & MAX31865_FAULT_RTDINLOW)
    //   {
    //     Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    //   }
    //   if (fault & MAX31865_FAULT_OVUV)
    //   {
    //     Serial.println("Under/Over voltage");
    //   }
    //   thermo.clearFault();
    // }
    // Serial.println();
  }
}

// Subprocesos de manejo de programas
void iniciarPrograma()
{
  if (!programaEnPausa && programaTerminado)
  {
    tiempoCumplido = false;
    programaTerminado = false;
    numeroVariable = 1;
    direccion = 1;
    fase = 1;

    digitalWrite(MagnetPuerta, HIGH);
    digitalWrite(ValvulAgua, HIGH);
    digitalWrite(ElectrovVapor, HIGH);
    digitalWrite(ValvulOnOff, HIGH);

    // reiniciamos temporizadores
    segundos[0] = 0;
    segundos[1] = 0;

    // desactivamos funciones asincronicas
    segundosMotor.Start();
    iniciarTemporizador();
    pintarVentanaEjecucion();
  }
  else if (!programaEnPausa && !programaTerminado)
  {
    digitalWrite(ValvulAgua, HIGH);
    digitalWrite(ElectrovVapor, HIGH);
    digitalWrite(ValvulOnOff, HIGH);
    segundosMotor.Start();
    segundosTemporizador.Start();
  }
}

void terminarPrograma()
{
  if (programaEnPausa && !programaTerminado)
  {
    segundosMotor.Stop();
    direccion = 2;
    digitalWrite(MotorDirA, LOW);
    digitalWrite(MotorDirB, LOW);
    digitalWrite(ValvulAgua, LOW);
    digitalWrite(ElectrovVapor, LOW);
    digitalWrite(ValvulOnOff, LOW);
  }
  else if (!programaEnPausa && programaTerminado)
  {
    tiempoCumplido = true;
    programaTerminado = true;
    numeroVariable = 1;
    fase = 1;

    // reiniciamos los temporizadores
    minutos[1] = 0;
    segundos[1] = 0;
    minutos[0] = 0;
    segundos[0] = 0;
    segunderoTemporizador = 0;
    segundosMotor.Stop();
    segundosTemporizador.Stop();

    // reiniciamos temporizadores
    segundos[0] = 0;
    segundos[1] = 0;

    digitalWrite(MotorDirA, LOW);
    digitalWrite(MotorDirB, LOW);
    digitalWrite(MagnetPuerta, LOW);
    digitalWrite(ValvulAgua, LOW);
    digitalWrite(ElectrovVapor, LOW);
    digitalWrite(ValvulOnOff, LOW);

    pintarVentanaSeleccion();
    Serial.println("Programa concluido exitosamente");
  }
  // mostramos nueva pantalla en el LCD
}

// Subprocesos de manejo del EEPROM
void escribirVariableEEPROM(uint8_t indice, uint8_t valor)
{
  EEPROM.write(indice, valor);
  // EEPROM.write(2, fase);
  // EEPROM.write(3, );
  // EEPROM.write(4, (Counter / 1000) % 10);
  // EEPROM.write(4, 7);
}

void recuperarValoresEEPROM()
{
  programa = EEPROM.read(0);
  fase = EEPROM.read(1);
  minutos[1] = EEPROM.read(2);
  segundos[1] = EEPROM.read(3);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      NivelAgua[i][j] = EEPROM.read(4 * (i + 1) + j);
      TemperaturaLim[i][j] = EEPROM.read(4 * (i + 4) + j);
      TemporizadorLim[i][j] = EEPROM.read(4 * (i + 7) + j);
      RotacionTam[i][j] = EEPROM.read(4 * (i + 10) + j);
      // TiempoEntFase[i][j] = EEPROM.read(4 * (i + 13) + j);
    }
  }

  // for (uint8_t i = 0; i < 3; i++)
  // {
  //   for (uint8_t j = 0; j < 2; j++)
  //   {
  //     TiempoRotacion[i][j] = EEPROM.read(2 + (i + 32) + j);
  //   }
  // }
  Serial.println("Recuperado exitosamente");
}

void guardarValoresEEPROM()
{
  EEPROM.update(0, programa);
  EEPROM.update(1, fase);
  EEPROM.update(2, minutos[1]);
  EEPROM.update(3, segundos[1]);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      EEPROM.update(4 * (i + 1) + j, NivelAgua[i][j]);
      EEPROM.update(4 * (i + 4) + j, TemperaturaLim[i][j]);
      EEPROM.update(4 * (i + 7) + j, TemporizadorLim[i][j]);
      EEPROM.update(4 * (i + 10) + j, RotacionTam[i][j]);
    }
  }

  // for (uint8_t i = 0; i < 3; i++)
  // {
  //   for (uint8_t j = 0; j < 2; j++)
  //   {
  //     EEPROM.update(2 * (i + 32) + j, TiempoRotacion[i][j]);
  //   }
  // }
  Serial.println("Guardado exitosamente");

  // sensorValue = ReadSensor(); //Lectura simulada del sensor
  // EEPROM.update( eeAddress, sensorValue );  //Grabamos el valor
  // eeAddress += sizeof(float);  //Obtener la siguiente posicion para escribir
  // if(eeAddress >= EEPROM.length()) eeAddress = 0;  //Comprobar que no hay desbordamiento
  // delay(30000); //espera 30 segunos
}

void pintarConsolaSerial()
{
  Serial.println("-------------------");
  Serial.print("Nivel de edicion: ");
  switch (nivelEdicion)
  {
  case 1:
    Serial.println("Edicion de fase");
    break;

  case 2:
    Serial.println("Edicion de numero de variable");
    break;

  case 3:
    Serial.println("Edicion de valor de variable");
    break;

  default:
    break;
  }
  Serial.print("Programa: ");
  Serial.println(programa);
  Serial.print("Fase: ");
  Serial.println(fase);
  Serial.print("Numero variable: ");
  Serial.println(numeroVariable);
  Serial.print("Valor obtenido: ");
  Serial.println(valorVariable);
}

void asignarBlinkLCD()
{
  switch (nivelEdicion)
  {
  case 1:
    lcd.setCursor(5, 1);
    break;

  case 2:
    lcd.setCursor(8, 1);
    break;

  case 3:
    lcd.setCursor(14, 1);
    break;

  default:
    break;
  }
}

void editarPrograma()
{
  uint8_t faseTemp = fase;
  numeroVariable = 1;
  nivelEdicion = 1;
  obtenerValorVariable();
  pintarVentanaEdicionMenu();
  lcd.blink();
  // pintarConsolaSerial();
  asignarBlinkLCD();
  while (nivelEdicion == 1)
  {
    if (digitalRead(btnAumentar) == nivelActivo)
    {
      if (flagBtnAumentar1 == 0)
      {
        flagBtnAumentar1 = 1;
        fase++;
        if (fase > 4)
        {
          fase = 1;
        }
        pintarVentanaEdicionMenu();
        asignarBlinkLCD();
        // pintarConsolaSerial();
      }
    }
    else
    {
      flagBtnAumentar1 = 0;
    }

    if (digitalRead(btnDisminuir) == nivelActivo)
    {
      if (flagBtnDisminuir1 == 0)
      {
        flagBtnDisminuir1 = 1;
        fase--;
        if (fase < 1)
        {
          fase = 4;
        }
        pintarVentanaEdicionMenu();
        asignarBlinkLCD();
        // pintarConsolaSerial();
      }
    }
    else
    {
      flagBtnDisminuir1 = 0;
    }

    if (digitalRead(btnEditar) == nivelActivo)
    {
      if (flagBtnEditar1 == 0)
      {
        flagBtnEditar1 = 1;
        nivelEdicion = 2;

        obtenerValorVariable();
        // pintarConsolaSerial();
        asignarBlinkLCD();

        // Edicion de valor de numeroVariable
        while (nivelEdicion == 2)
        {
          if (editandoProgramaEjecucion)
          {
            controladorDireccionMotor();
            controladorTemporizador();
          }
          if (digitalRead(btnAumentar) == nivelActivo)
          {
            if (flagBtnAumentar1 == 0)
            {
              flagBtnAumentar1 = 1;
              numeroVariable++;
              if (numeroVariable > 4)
              {
                numeroVariable = 1;
              }
              obtenerValorVariable();
              pintarVentanaEdicionMenu();
              // pintarConsolaSerial();
              asignarBlinkLCD();
            }
          }
          else
          {
            flagBtnAumentar1 = 0;
          }

          if (digitalRead(btnDisminuir) == nivelActivo)
          {
            if (flagBtnDisminuir1 == 0)
            {
              flagBtnDisminuir1 = 1;
              numeroVariable--;
              if (numeroVariable < 1)
              {
                numeroVariable = 4;
              }
              obtenerValorVariable();
              pintarVentanaEdicionMenu();
              // pintarConsolaSerial();
              asignarBlinkLCD();
            }
          }
          else
          {
            flagBtnDisminuir1 = 0;
          }

          if (digitalRead(btnEditar) == nivelActivo)
          {
            if (flagBtnEditar1 == 0)
            {
              flagBtnEditar1 = 1;
              nivelEdicion = 3;

              // pintarConsolaSerial();
              asignarBlinkLCD();
              // Edicion valor de variables
              while (nivelEdicion == 3)
              {
                if (editandoProgramaEjecucion)
                {
                  controladorDireccionMotor();
                  controladorTemporizador();
                }
                if (digitalRead(btnAumentar) == nivelActivo)
                {
                  if (flagBtnAumentar1 == 0)
                  {
                    flagBtnAumentar1 = 1;
                    valorVariable++;

                    if (numeroVariable == 4)
                    {
                      if (valorVariable > 3)
                      {
                        valorVariable = 1;
                      }
                    }
                    else if (numeroVariable == 1)
                    {
                      if (valorVariable > 10)
                      {
                        valorVariable = 1;
                      }
                    }
                    editarValorVariable();
                    pintarVentanaEdicionMenu();
                    // pintarConsolaSerial();
                    asignarBlinkLCD();
                  }
                }
                else
                {
                  flagBtnAumentar1 = 0;
                }

                if (digitalRead(btnDisminuir) == nivelActivo)
                {
                  if (flagBtnDisminuir1 == 0)
                  {
                    flagBtnDisminuir1 = 1;
                    valorVariable--;
                    if (numeroVariable == 4)
                    {
                      // valorVariable = 1;
                      if (valorVariable < 1)
                      {
                        valorVariable = 3;
                      }
                    }
                    else if (numeroVariable == 1)
                    {
                      if (valorVariable < 1)
                      {
                        valorVariable = 10;
                      }
                    }
                    else
                    {
                      if (valorVariable < 0)
                      {
                        valorVariable = 0;
                      }
                    }
                    editarValorVariable();
                    pintarVentanaEdicionMenu();
                    // pintarConsolaSerial();
                    asignarBlinkLCD();
                  }
                }
                else
                {
                  flagBtnDisminuir1 = 0;
                }

                if (digitalRead(btnParar) == nivelActivo)
                {
                  if (flagBtnParar1 == 0)
                  {
                    flagBtnParar1 = 1;
                    nivelEdicion = 2;
                    pintarConsolaSerial();
                    asignarBlinkLCD();
                  }
                }
                else
                {
                  flagBtnParar1 = 0;
                }
              }
            }
          }
          else
          {
            flagBtnEditar1 = 0;
          }

          if (digitalRead(btnParar) == nivelActivo)
          {
            if (flagBtnParar1 == 0)
            {
              flagBtnParar1 = 1;
              nivelEdicion = 1;
              // pintarConsolaSerial();
              asignarBlinkLCD();
            }
          }
          else
          {
            flagBtnParar1 = 0;
          }
        }
      }
    }
    else
    {
      flagBtnEditar1 = 0;
    }

    if (digitalRead(btnComenzar) == nivelActivo)
    {
      if (flagBtnComenzar1 == 0)
      {
        flagBtnComenzar1 = 1;
        fase = faseTemp;
        if (editandoProgramaEjecucion)
        {
          pintarVentanaEjecucion();
        }
        else
        {
          pintarVentanaSeleccion();
        }
        guardarValoresEEPROM();
        editarValorVariable();
        nivelEdicion = 0;
        numeroVariable = 1;
        editandoProgramaEjecucion = false;
        lcd.noBlink();
      }
    }
    else
    {
      flagBtnComenzar1 = 0;
    }
  }
}