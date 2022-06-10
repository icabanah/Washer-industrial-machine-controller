#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>
#include <AsyncTaskLib.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Iniciamos el lcd
const int rs = 19, en = 18, d4 = 17, d5 = 16, d6 = 15, d7 = 14; //
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Temperatura
OneWire bus(25);
DallasTemperature thermo(&bus);
DeviceAddress sensorTemperatura = {0x28, 0xFF, 0x7, 0x3, 0x93, 0x16, 0x4, 0x7A};
uint8_t resolucion = 9;

// Configuramos sensor de presion
const int DOUT = 42; // sensor data pin
const int SCLK = 40; // sensor clock pin
HX710B pressure_sensor;

// Entradas
#define btnParar 2               //
#define btnDisminuir 3           //
#define btnAumentar 4            //
#define btnEditar 26             //
#define btnProgramarNivelAgua 30 //
#define btnComenzar 28           //
// #define btnOptoacoplador 32 //
// #define sensorPuerta 23 //

// Salidas
#define MotorDirA 12     //
#define MotorDirB 11     //
#define ValvulAgua 10    //
#define ElectrovVapor 9  //
#define ValvulDesfogue 8 //
#define MagnetPuerta 7   //
#define buzzer 34
// #define sensorMPX2010 A0

// Definimos variables de los programas
uint8_t NivelAgua[3][4];
uint8_t RotacionTam[3][4];
uint8_t TemperaturaLim[3][4];
uint8_t TemporizadorLim[3][4];
uint8_t TiempoRotacion[3][2] = {{4, 2}, {10, 4}, {20, 6}};
uint8_t TiempoEntFase[3][4] = {{60, 60, 60, 60}, {60, 60, 60, 60}, {60, 60, 60, 60}};

// Variables bandera
boolean tiempoCumplido = false;
boolean programaTerminado = true;
boolean programaEnPausa = false;
boolean editandoProgramaEjecucion = false;
boolean nivelActivo = LOW;

// Antirebote
uint8_t flagBtnAumentar = 0;
uint8_t flagBtnDisminuir = 0;
uint8_t flagBtnComenzar1 = 0;
uint8_t flagBtnEditar1 = 0;
uint8_t flagBtnParar1 = 0;

// Variables de proceso
boolean pausa = false;
uint8_t programa = 1;
uint16_t contadorBloqueo = 1498;
uint16_t limiteBloqueo = 1500;
uint8_t numeroVariable = 1;
uint8_t valorVariable = 0;
uint8_t fase = 1;
uint8_t nivelEdicion = 0;
uint8_t direccion = 1;

int8_t minutos[2] = {0, 0};
int8_t segundos[2] = {0, 0};
int16_t segunderoTemporizador = 0;
int16_t segunderoMotor = 0;
uint8_t tiempoRotacion = 0;
uint8_t tiempoPausa = 0;
uint8_t nivelRotacionTambor = 0;
uint8_t segunderoEntreFase = 0;

// Variables para manejo de sensor temperatura
uint8_t valorTemperatura = 0;
uint8_t valorTemperaturaLim = 0;
uint8_t rangoTemperatura = 2;
boolean sensarTemperatura = false;

// Sensor de presion
uint16_t nivelPresion1 = 650;
uint16_t nivelPresion2 = 750;
uint16_t nivelPresion3 = 850;
uint16_t nivelPresion4 = 950;

uint16_t valorPresion = 0;
uint16_t valorNivelLim = 0;
uint8_t valorNivel = 1;
boolean sensarPresion = false;

// AsyncTask segundosMotor(1000, true, []()
//                         {
//   segunderoMotor++;
//   segundos[0] = segunderoMotor;
//   });

void pintarVariables()
{
  lcd.setCursor(6, 0);
  lcd.print(fase);

  // lcd.setCursor(10, 0);
  // lcd.print(valorNivelLim);

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

AsyncTask segundosTemporizador(200, true, []()
                               { 
  segunderoTemporizador--;
  if (!programaEnPausa)
  {
    segunderoMotor++;
  }
  minutos[1] = (segunderoTemporizador / 60);
  segundos[1] = segunderoTemporizador - (minutos[1] * 60); 
  controladorSensorTemperatura();
  controladorSensorPresion();
  // pintarConsolaSerial();
  if (!editandoProgramaEjecucion){ pintarVariables(); 
  } });

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
  pinMode(ValvulDesfogue, OUTPUT);

  // Inicializamos salidas
  digitalWrite(MagnetPuerta, LOW);
  digitalWrite(ValvulAgua, LOW);
  digitalWrite(ElectrovVapor, LOW);
  digitalWrite(ValvulDesfogue, LOW);
  digitalWrite(MotorDirA, LOW);
  digitalWrite(MotorDirB, LOW);
  digitalWrite(buzzer, LOW);

  // Inicializamos el lcd
  lcd.begin(16, 2);
  pintarPantallaInicio();
  delay(3000);

  // Iniciamos el sensor de temperatura
  thermo.begin();
  thermo.setResolution(resolucion);

  // Inicializamos el puerto serial para depurar
  Serial.begin(115200);
  Serial.println("Puerto serial iniciado");

  // Iniciamos sensor de presion
  pressure_sensor.begin(DOUT, SCLK);

  // Recuperamos valores del EEPROM
  recuperarValoresEEPROM();

  // inicializamos subprocesos
  pintarVentanaSeleccion();
  pintarConsolaSerial();
}

void loop()
{
  // Eligiendo programa
  if (digitalRead(btnAumentar) == nivelActivo)
  {
    if (flagBtnAumentar == 0)
    {
      flagBtnAumentar = 1;
      programa++;
      if (programa > 3)
        programa = 1;
      pintarVentanaSeleccion();
      // pintarConsolaSerial();
    }
  }
  else
  {
    flagBtnAumentar = 0;
  }

  if (digitalRead(btnDisminuir) == nivelActivo)
  {
    if (flagBtnDisminuir == 0)
    {
      flagBtnDisminuir = 1;
      programa--;
      if (programa < 1)
      {
        programa = 3;
      }
      pintarVentanaSeleccion();
      // pintarConsolaSerial();
    }
  }
  else
  {
    flagBtnDisminuir = 0;
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
        if (!programaEnPausa)
        {
          controladorDireccionMotor();
        }
        if (programaTerminado)
        {
          digitalWrite(buzzer, HIGH);
          delay(2000);
          digitalWrite(buzzer, LOW);
        }
        if (digitalRead(btnParar) == nivelActivo)
        {
          if (flagBtnParar1 == 0)
          {
            flagBtnParar1 = 1;
            terminarPrograma();
            Serial.println("Programa terminado con boton");
            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flagBtnParar1 = 0;
        }

        // if (digitalRead(btnEditar) == nivelActivo)
        // {
        //   if (flagBtnEditar1 == 0)
        //   {
        //     flagBtnEditar1 = 1;
        //     editandoProgramaEjecucion = true;
        //     editarPrograma();
        //   }
        // }
        // else
        // {
        //   flagBtnEditar1 = 0;
        // }

        if (digitalRead(btnAumentar) == nivelActivo)
        {
          if (flagBtnAumentar == 0)
          {
            flagBtnAumentar = 1;
            aumentarAgua();
          }
        }
        else
        {
          flagBtnAumentar = 0;
        }

        if (digitalRead(btnDisminuir) == nivelActivo)
        {
          if (flagBtnDisminuir == 0)
          {
            flagBtnDisminuir = 1;
            disminuirAgua();
          }
        }
        else
        {
          flagBtnDisminuir = 0;
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
void pintarPantallaInicio()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("RH Electronics");
  lcd.setCursor(3, 1);
  lcd.print("958970967");
}

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
    if (TemperaturaLim[programa - 1][i] < 10)
    {
      lcd.setCursor(3 * i, 1);
      lcd.print(0);
      lcd.print(TemperaturaLim[programa - 1][i]);
    }
    else
    {
      lcd.setCursor(3 * i, 1);
      lcd.print(TemperaturaLim[programa - 1][i]);
    }
  }

  lcd.setCursor(11, 1);
  lcd.print("R");
  for (uint8_t i = 0; i < 4; i++)
  {
    lcd.setCursor(i + 12, 1);
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
  lcd.print(valorNivelLim);

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
  uint16_t segunderoTemp = 0;
  if (!programaEnPausa)
  {
    if (editandoProgramaEjecucion)
    {
      segunderoTemp = TemporizadorLim[programa - 1][fase - 1];
      if (segunderoTemporizador < segunderoTemp)
      {
        segunderoTemporizador = segunderoTemp * 60;
      }
    }
    else
    {
      segunderoTemporizador = TemporizadorLim[programa - 1][fase - 1] * 60;
    }
    // Serial.print("SegunderoTemporizador en ejecucion: ");
    // Serial.println(segunderoTemporizador);
  }
  else
  {
    segunderoTemp = TiempoEntFase[programa - 1][fase - 1];
    segunderoTemporizador = segunderoTemp;
    // Serial.print("SegunderoTemporizador en pausa: ");
    // Serial.println(segunderoTemporizador);
  }
  // Serial.println(segunderoTemporizador);
  segundosTemporizador.Start();
}

void iniciarTiempoRotacion()
{
  if (editandoProgramaEjecucion)
  {
    nivelRotacionTambor = RotacionTam[programa - 1][fase - 1];
    tiempoRotacion = TiempoRotacion[nivelRotacionTambor - 1][0];
    tiempoPausa = TiempoRotacion[nivelRotacionTambor - 1][1];
  }
  else
  {
    nivelRotacionTambor = RotacionTam[programa - 1][fase - 1];
    tiempoRotacion = TiempoRotacion[nivelRotacionTambor - 1][0];
    tiempoPausa = TiempoRotacion[nivelRotacionTambor - 1][1];
    digitalWrite(MotorDirA, HIGH);
    digitalWrite(MotorDirB, LOW);
  }
}

void iniciarSensorTemperatura()
{
  sensarTemperatura = true;
  valorTemperaturaLim = TemperaturaLim[programa - 1][fase - 1];
}

void iniciarSensorPresion()
{
  sensarPresion = true;
  valorNivelLim = NivelAgua[programa - 1][fase - 1];
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
        pausarPrograma();
        iniciarTemporizador();
        pintarConsolaSerial();
      }
    }
    else
    {
      if (segunderoTemporizador == 0)
      {
        segundosTemporizador.Stop();
        fase++;
        reiniciarPrograma();
        if (fase > 4)
        {
          terminarPrograma();
        }
        else
        {
          iniciarTemporizador();
        }
        pintarConsolaSerial();
      }
    }
  }
}

// Subprocesos del motor
void controladorDireccionMotor()
{
  if (!programaTerminado || !programaEnPausa)
  {
    // segundosMotor.Update();
    // pintarConsolaSerial();
    switch (direccion)
    {
    case 1:
      if (segunderoMotor == tiempoRotacion)
      {
        direccion = 2;
        pausa = true;
        segunderoMotor = 0;
        digitalWrite(MotorDirA, LOW);
        digitalWrite(MotorDirB, LOW);
        // pintarConsolaSerial();
        // Serial.print("Controlador motor - direccion");
        // Serial.println(direccion);
      }
      break;

    case 2:
      if (segunderoMotor == tiempoPausa)
      {
        if (pausa == true)
        {
          direccion = 3;
          digitalWrite(MotorDirA, LOW);
          digitalWrite(MotorDirB, HIGH);
        }
        else
        {
          direccion = 1;
          digitalWrite(MotorDirA, HIGH);
          digitalWrite(MotorDirB, LOW);
        }
        segunderoMotor = 0;
        // pintarConsolaSerial();
      }
      // Serial.print("Controlador motor - direccion");
      // Serial.println(direccion);
      break;

    case 3:
      if (segunderoMotor == tiempoRotacion)
      {
        direccion = 2;
        pausa = false;
        segunderoMotor = 0;
        digitalWrite(MotorDirA, LOW);
        digitalWrite(MotorDirB, LOW);
        // pintarConsolaSerial();
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
    thermo.requestTemperatures();
    valorTemperatura = round(thermo.getTempCByIndex(0));
    // Serial.print("Valor temperatura (Celcius): ");
    // Serial.println(valorTemperatura);
    if ((valorTemperatura >= (valorTemperaturaLim + rangoTemperatura)) && sensarTemperatura)
    {
      // sensarTemperatura = false;
      digitalWrite(ElectrovVapor, LOW);
    }
    else if ((valorTemperatura <= (valorTemperaturaLim - rangoTemperatura)) && sensarTemperatura)
    {
      // sensarTemperatura = false;
      digitalWrite(ElectrovVapor, HIGH);
    }
  }
}

// Controlador sensor de presion
void controladorSensorPresion()
{
  if (pressure_sensor.is_ready())
  {
    valorPresion = pressure_sensor.pascal();
    // Serial.print("Valor presion (Pascal): ");
    if (valorPresion <= nivelPresion1)
    {
      valorNivel = 1;
    }
    else if (valorPresion > nivelPresion1 && valorPresion <= nivelPresion2)
    {
      valorNivel = 2;
    }
    else if (valorPresion > nivelPresion2 && valorPresion <= nivelPresion3)
    {
      valorNivel = 3;
    }
    else if (valorPresion > nivelPresion3)
    {
      valorNivel = 4;
    }

    if ((valorNivel >= valorNivelLim) && sensarPresion)
    {
      sensarPresion = false;
      if (programa == 1)
      {
        iniciarSensorTemperatura();
      }
      digitalWrite(ValvulAgua, LOW);
    }
    // Serial.println(valorPresion);
    // Serial.println(valorNivel);
    // Serial.println(valorNivelLim);
    // else if ((valorNivel <= valorNivelLim) && sensarPresion)
    // {
    //   // sensarTemperatura = false;
    //   digitalWrite(ElectrovVapor, HIGH);
    // }
  }
  else
  {
    Serial.println("Pressure sensor not found.");
  }
}

// Subprocesos de manejo de programas
void iniciarPrograma()
{
  Serial.print("Contador: ");
  Serial.println(contadorBloqueo);
  if (contadorBloqueo < limiteBloqueo)
  {
    tiempoCumplido = false;
    programaTerminado = false;
    programaEnPausa = false;
    numeroVariable = 1;
    direccion = 1;
    fase = 1;

    digitalWrite(MagnetPuerta, HIGH);
    digitalWrite(ValvulAgua, HIGH);
    digitalWrite(ValvulDesfogue, HIGH);

    // reiniciamos temporizadores
    segundos[0] = 0;
    segundos[1] = 0;
    minutos[0] = 0;
    minutos[1] = 0;

    // segundosMotor.Start();
    iniciarTemporizador();
    if (programa != 1)
    {
      iniciarSensorTemperatura();
      // Serial.println("Sensor temperatura iniciado ");
    }
    iniciarSensorPresion();
    iniciarTiempoRotacion();
    pintarVentanaEjecucion();
    pintarConsolaSerial();
  }
}

void reiniciarPrograma()
{
  programaEnPausa = false;
  direccion = 1;
  segunderoMotor = 0;
  iniciarTemporizador();
  iniciarTiempoRotacion();
  digitalWrite(ValvulAgua, HIGH);

  valorTemperaturaLim = TemperaturaLim[programa - 1][fase - 1];
  valorNivelLim = NivelAgua[programa - 1][fase - 1];
  if (valorTemperaturaLim > 0)
  {
    iniciarSensorTemperatura();
    digitalWrite(ElectrovVapor, HIGH);
  }
  if (valorNivelLim > 0)
  {
    iniciarSensorPresion();
    digitalWrite(ValvulAgua, HIGH);
  }
  digitalWrite(ValvulDesfogue, HIGH);
  // segundosMotor.Start();
  segundosTemporizador.Start();
}

void terminarPrograma()
{
  if (!programaTerminado)
  {
    tiempoCumplido = true;
    programaTerminado = true;
    programaEnPausa = false;
    numeroVariable = 1;
    fase = 1;
    sensarPresion = false;
    sensarTemperatura = false;
    contadorBloqueo++;

    // reiniciamos los temporizadores
    segundos[0] = 0;
    segundos[1] = 0;
    minutos[0] = 0;
    minutos[1] = 0;
    segunderoTemporizador = 0;
    segundosTemporizador.Stop();

    digitalWrite(MotorDirA, LOW);
    digitalWrite(MotorDirB, LOW);
    digitalWrite(MagnetPuerta, LOW);
    digitalWrite(ValvulAgua, LOW);
    digitalWrite(ElectrovVapor, LOW);
    digitalWrite(ValvulDesfogue, LOW);

    pintarVentanaSeleccion();
    Serial.println("Programa concluido exitosamente");
  }
}

void pausarPrograma()
{
  programaEnPausa = true;
  sensarTemperatura = false;
  sensarPresion = false;
  digitalWrite(MotorDirA, LOW);
  digitalWrite(MotorDirB, LOW);
  digitalWrite(ValvulAgua, LOW);
  digitalWrite(ElectrovVapor, LOW);
  digitalWrite(ValvulDesfogue, LOW);
}

void recuperarValoresEEPROM()
{
  programa = EEPROM.read(0);
  fase = EEPROM.read(1);
  minutos[1] = EEPROM.read(2);
  segundos[1] = EEPROM.read(3);
  contadorBloqueo = EEPROM.get(4, contadorBloqueo);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      NivelAgua[i][j] = EEPROM.read(6 * (i + 1) + j);
      TemperaturaLim[i][j] = EEPROM.read(6 * (i + 4) + j);
      TemporizadorLim[i][j] = EEPROM.read(6 * (i + 7) + j);
      RotacionTam[i][j] = EEPROM.read(6 * (i + 10) + j);
      // TiempoEntFase[i][j] = EEPROM.read(4 * (i + 13) + j);
    }
  }
  Serial.println("Recuperado exitosamente");
}

void guardarValoresEEPROM()
{
  EEPROM.update(0, programa);
  EEPROM.update(1, fase);
  EEPROM.update(2, minutos[1]);
  EEPROM.update(3, segundos[1]);
  EEPROM.put(4, contadorBloqueo);
  // EEPROM.update(5, segundos[1]);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      EEPROM.update(6 * (i + 1) + j, NivelAgua[i][j]);
      EEPROM.update(6 * (i + 4) + j, TemperaturaLim[i][j]);
      EEPROM.update(6 * (i + 7) + j, TemporizadorLim[i][j]);
      EEPROM.update(6 * (i + 10) + j, RotacionTam[i][j]);
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

  // valorTemperatura = ReadSensor(); //Lectura simulada del sensor
  // EEPROM.update( eeAddress, valorTemperatura );  //Grabamos el valor
  // eeAddress += sizeof(float);  //Obtener la siguiente posicion para escribir
  // if(eeAddress >= EEPROM.length()) eeAddress = 0;  //Comprobar que no hay desbordamiento
  // delay(30000); //espera 30 segunos
}

void pintarConsolaSerial()
{
  Serial.println("-------------------");
  Serial.print("Programa: ");
  Serial.println(programa);
  Serial.print("Fase: ");
  Serial.println(fase);
  Serial.print("Direccion de motor: ");
  Serial.println(direccion);
  Serial.print("temporizador motor: ");
  Serial.println(segunderoMotor);
  Serial.print("Nivel rotacion tambor: ");
  Serial.println(nivelRotacionTambor);
  Serial.print("tiempo rotacion: ");
  Serial.println(tiempoRotacion);
  Serial.print("tiempo pausa: ");
  Serial.println(tiempoPausa);
  Serial.println();
  // Serial.print("Valor obtenido: ");
  // Serial.println(valorVariable);
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
      if (flagBtnAumentar == 0)
      {
        flagBtnAumentar = 1;
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
      flagBtnAumentar = 0;
    }

    if (digitalRead(btnDisminuir) == nivelActivo)
    {
      if (flagBtnDisminuir == 0)
      {
        flagBtnDisminuir = 1;
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
      flagBtnDisminuir = 0;
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
            if (flagBtnAumentar == 0)
            {
              flagBtnAumentar = 1;
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
            flagBtnAumentar = 0;
          }

          if (digitalRead(btnDisminuir) == nivelActivo)
          {
            if (flagBtnDisminuir == 0)
            {
              flagBtnDisminuir = 1;
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
            flagBtnDisminuir = 0;
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
                  if (flagBtnAumentar == 0)
                  {
                    flagBtnAumentar = 1;
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
                      if (valorVariable > 4)
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
                  flagBtnAumentar = 0;
                }

                if (digitalRead(btnDisminuir) == nivelActivo)
                {
                  if (flagBtnDisminuir == 0)
                  {
                    flagBtnDisminuir = 1;
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
                        valorVariable = 4;
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
                  flagBtnDisminuir = 0;
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
          iniciarTemporizador();
          iniciarSensorTemperatura();
          iniciarSensorPresion();
          iniciarTiempoRotacion();
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

void aumentarAgua()
{
  digitalWrite(ValvulAgua, HIGH);
}

void disminuirAgua()
{
  digitalWrite(ValvulAgua, LOW);
}
