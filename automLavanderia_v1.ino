#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX710B.h>
#include <AsyncTaskLib.h>
// #include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LiquidCrystal/src/LiquidCrystal.h"

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
uint16_t contadorBloqueo = 0;
uint16_t limiteBloqueo = 10000;
uint8_t numeroVariable = 1;
uint8_t valorVariable = 0;
uint8_t fase = 1;
uint8_t nivelEdicion = 0;
uint8_t direccion = 1;

int8_t minutos[2] = {0, 0};
int8_t segundos[2] = {0, 0};
int16_t segunderoTemporizador = 0;
int16_t segunderoMotor = 0;
uint8_t segunderoEntreFase = 0;
boolean motorON = false;
boolean temporizadorON = false;
uint8_t tiempoRotacion = 0;
uint8_t tiempoPausa = 0;
uint8_t nivelRotacionTambor = 0;

// Variables para manejo de sensor temperatura
uint8_t valorTemperatura = 0;
uint8_t valorTemperaturaLim = 0;
uint8_t rangoTemperatura = 2;
boolean sensarTemperatura = false;

// Sensor de presion
uint16_t nivelPresion1 = 601;
uint16_t nivelPresion2 = 628;
uint16_t nivelPresion3 = 645;
uint16_t nivelPresion4 = 663;

uint16_t valorPresion = 0;
uint16_t valorNivelLim = 0;
uint8_t valorNivel = 0;
uint8_t contadorNivel = 0;
boolean sensarPresion = false;

boolean botonON = false;

void pintarVariables()
{
  lcd.setCursor(6, 0);
  lcd.print(fase);

  lcd.setCursor(10, 0);
  lcd.print(valorNivelLim);

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

AsyncTask segundosTemporizador(800, true, []()
                               { 
  if (temporizadorON && !programaEnPausa)
  {
    segunderoTemporizador--;
    minutos[1] = (segunderoTemporizador / 60);
    segundos[1] = segunderoTemporizador - (minutos[1] * 60); 
  }
  else if (temporizadorON && programaEnPausa)
  {
    segunderoEntreFase--;
    minutos[1] = (segunderoEntreFase / 60);
    segundos[1] = segunderoEntreFase - (minutos[1] * 60); 
  }
  if (!programaEnPausa && motorON)
  {
    segunderoMotor++;
  }
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
  // Serial.begin(115200);
  // Serial.println("Puerto serial iniciado");

  // Iniciamos sensor de presion
  pressure_sensor.begin(DOUT, SCLK);

  // Recuperamos valores del EEPROM
  recuperarValoresEEPROM();

  // inicializamos subprocesos
  pintarVentanaSeleccion();
  // pintarConsolaSerial();
}

void loop()
{
  if (contadorBloqueo < limiteBloqueo)
  {
    // pintarConsolaSerial();
    // Eligiendo programa
    if (digitalRead(btnAumentar) == nivelActivo)
    {
      if (flagBtnAumentar == 0)
      {
        // Serial.println("btnAumentar ON");
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
        // Serial.println("btnDisminuir ON");
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
        // Serial.println("btnComenzar ON");
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
          if (digitalRead(btnParar) == nivelActivo && digitalRead(btnEditar) == nivelActivo)
          {
            if (flagBtnEditar1 == 0 && flagBtnParar1 == 0)
            {
              flagBtnEditar1 = 1;
              flagBtnParar1 = 1;
              terminarPrograma();
              // Serial.println("Programa terminado con boton");
              // eeprom_write();
              // delay(100);
            }
          }
          else
          {
            flagBtnEditar1 = 0;
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

          if (digitalRead(btnAumentar) == nivelActivo && digitalRead(btnEditar) == nivelActivo)
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

          if (digitalRead(btnDisminuir) == nivelActivo && digitalRead(btnEditar) == nivelActivo)
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
        // Serial.println("btnEditar ON");
        // Edicion de numero de variable
        flagBtnEditar1 = 1;
        editarPrograma();
      }
    }
    else
    {
      flagBtnEditar1 = 0;
    }

    // if (digitalRead(btnParar) == nivelActivo)
    // {
    //   if (flagBtnParar1 == 0)
    //   {
    //     flagBtnParar1 == 1;
    //     guardarValoresEEPROM();
    //   }
    // }
    // else
    // {
    //   flagBtnParar1 = 0;
    // }
  }
  else
  {
    pintarVentanaError();
    while (true)
    {
    }
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

  if (programa == 3)
  {
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
  else
  {
    lcd.setCursor(4, 0);
    lcd.print("Niv:");
    lcd.setCursor(8, 0);
    lcd.print(NivelAgua[programa - 1][0]);

    lcd.setCursor(10, 0);
    lcd.print("Tie:");
    lcd.setCursor(14, 0);
    lcd.print(TemporizadorLim[programa - 1][0]);

    lcd.setCursor(0, 1);
    lcd.print("Temp:");

    lcd.setCursor(5, 1);
    if (TemperaturaLim[programa - 1][0] < 10)
    {
      lcd.print(0);
      lcd.print(TemperaturaLim[programa - 1][0]);
    }
    else
    {
      // lcd.setCursor(5, 1);
      lcd.print(TemperaturaLim[programa - 1][0]);
    }

    lcd.setCursor(10, 1);
    lcd.print("Rot:");
    lcd.setCursor(14, 1);
    lcd.print(RotacionTam[programa - 1][0]);
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

void pintarVentanaError()
{
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("ERROR: ");
  lcd.setCursor(7, 1);
  lcd.print("400");
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
  if (!programaEnPausa)
  {
    // if (editandoProgramaEjecucion)
    // {
    //   segunderoTemp = TemporizadorLim[programa - 1][fase - 1];
    //   if (segunderoTemporizador < segunderoTemp)
    //   {
    //     segunderoTemporizador = segunderoTemp * 60;
    //   }
    // }
    // else
    // {
    segunderoTemporizador = TemporizadorLim[programa - 1][fase - 1] * 60;
    // }
    // Serial.print("SegunderoTemporizador en ejecucion: ");
    // Serial.println(segunderoTemporizador);
  }
  else
  {
    segunderoEntreFase = TiempoEntFase[programa - 1][fase - 1];
    // temporizadorON = true;
    // Serial.print("segunderoEntreFase en pausa: ");
    // Serial.println(segunderoEntreFase);
  }
  // Serial.println(segunderoTemporizador);
  segundosTemporizador.Start();
  // Serial.println("Temporizador iniciado");
}

void iniciarTiempoRotacion()
{
  // motorON = true;
  if (motorON)
  {
    nivelRotacionTambor = RotacionTam[programa - 1][fase - 1];
    tiempoRotacion = TiempoRotacion[nivelRotacionTambor - 1][0];
    tiempoPausa = TiempoRotacion[nivelRotacionTambor - 1][1];
    digitalWrite(MotorDirA, HIGH);
    digitalWrite(MotorDirB, LOW);
  }
  // Serial.println("Tiempo de rotacion iniciado");
}

void iniciarSensorTemperatura()
{
  sensarTemperatura = true;
  valorTemperaturaLim = TemperaturaLim[programa - 1][fase - 1];
  // Serial.println("Sensor de temperatura iniciado");
}

void iniciarSensorPresion()
{
  sensarPresion = true;
  valorNivelLim = NivelAgua[programa - 1][fase - 1];
  // Serial.println("Sensor de presion iniciado");
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
        // pintarConsolaSerial();
      }
    }
    else
    {
      if (segunderoEntreFase == 0)
      {
        segundosTemporizador.Stop();
        if (programa == 3)
        {
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
        }
        else
        {
          terminarPrograma();
        }
        // pintarConsolaSerial();
      }
    }
  }
}

// Subprocesos del motor
void controladorDireccionMotor()
{
  if (!programaTerminado || !programaEnPausa)
  {
    if (motorON)
    {

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
    // segundosMotor.Update();
    // pintarConsolaSerial();
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
  // Serial.println(valorPresion);
  if (sensarPresion)
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
    }
    else
    {
      // Serial.println("Pressure sensor not found.");
    }
  }

  if ((valorNivel >= valorNivelLim + 1) && sensarPresion)
  {
    contadorNivel++; // para evitar los falsos positivos.
  }

  if ((valorNivel >= valorNivelLim + 1) && sensarPresion && (contadorNivel >= 5))
  {
    // Serial.println("Valor de nivel seleccionado alcanzado");
    digitalWrite(ValvulAgua, LOW);
    sensarPresion = false;
    temporizadorON = true;
    if (programa == 1)
    {
      iniciarSensorTemperatura();
    }
    else if (programa == 2)
    {
      motorON = true;
      iniciarTiempoRotacion();
    }
  }
}

// Subprocesos de manejo de programas
void iniciarPrograma()
{
  // Serial.print("Iniciar programa: ");
  // Serial.println(contadorBloqueo);
  tiempoCumplido = false;
  programaTerminado = false;
  programaEnPausa = false;
  numeroVariable = 1;
  direccion = 1;
  fase = 1;
  valorNivel = 0;
  contadorNivel = 0;

  digitalWrite(MagnetPuerta, HIGH);
  digitalWrite(ValvulAgua, HIGH);
  digitalWrite(ValvulDesfogue, HIGH);

  // reiniciamos temporizadores
  segunderoTemporizador = 0;
  segunderoEntreFase = 0;
  segunderoMotor = 0;
  segundos[0] = 0;
  segundos[1] = 0;
  minutos[0] = 0;
  minutos[1] = 0;

  iniciarTemporizador();
  // segundosMotor.Start();
  if (programa == 1)
  {
    motorON = true;
    iniciarTiempoRotacion();
    // Serial.println("Sensor temperatura iniciado ");
  }
  else if (programa == 2)
  {
    iniciarSensorTemperatura();
  }
  else
  {
    motorON = true;
    iniciarSensorTemperatura();
    iniciarTiempoRotacion();
    // motorON = true;
  }

  iniciarSensorPresion();
  pintarVentanaEjecucion();
  // pintarConsolaSerial();
  // pintarConsolaSerial();
}

void reiniciarPrograma()
{
  // Serial.println("Reiniciar programa: ");
  programaEnPausa = false;
  direccion = 1;
  segunderoMotor = 0;
  motorON = true;
  valorNivel = 0;
  contadorNivel = 0;

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
    motorON = false;
    sensarPresion = false;
    valorNivel = 0;
    contadorNivel = 0;
    sensarTemperatura = false;
    contadorBloqueo++;
    temporizadorON = false;

    // reiniciamos los temporizadores
    segundos[0] = 0;
    segundos[1] = 0;
    minutos[0] = 0;
    minutos[1] = 0;
    segunderoTemporizador = 0;
    segunderoMotor = 0;
    segundosTemporizador.Stop();

    digitalWrite(MotorDirA, LOW);
    digitalWrite(MotorDirB, LOW);
    digitalWrite(MagnetPuerta, LOW);
    digitalWrite(ValvulAgua, LOW);
    digitalWrite(ElectrovVapor, LOW);
    digitalWrite(ValvulDesfogue, LOW);

    pintarVentanaSeleccion();
    // Serial.println("Programa concluido exitosamente");
  }
  // Serial.println("Programa terminado");
}

void pausarPrograma()
{
  // Serial.println("Pausar programa: ");
  // pintarConsolaSerial();
  programaEnPausa = true;
  motorON = false;
  sensarTemperatura = false;
  segunderoEntreFase = 0;
  segunderoMotor = 0;
  segunderoTemporizador = 0;
  sensarPresion = false;
  valorNivel = 0;
  contadorNivel = 0;

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
  byte highByte = EEPROM.read(4);
  byte lowByte = EEPROM.read(5);
  contadorBloqueo = (highByte << 8) | lowByte;
  // Serial.print("contadorBloque: ");
  // Serial.println(contadorBloqueo);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      NivelAgua[i][j] = EEPROM.read(6 * (i + 1) + j);
      TemperaturaLim[i][j] = EEPROM.read(6 * (i + 4) + j);
      TemporizadorLim[i][j] = EEPROM.read(6 * (i + 7) + j);
      RotacionTam[i][j] = EEPROM.read(6 * (i + 10) + j);
    }
  }
  // Serial.println("Recuperado exitosamente");
}

void guardarValoresEEPROM()
{
  EEPROM.write(0, programa);
  EEPROM.write(1, fase);
  EEPROM.write(2, minutos[1]);
  EEPROM.write(3, segundos[1]);
  byte lowByte = (contadorBloqueo & 0x00FF);
  byte highByte = ((contadorBloqueo & 0xFF00) >> 8);
  EEPROM.write(4, highByte);
  EEPROM.write(5, lowByte);

  for (uint8_t i = 0; i < 3; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      EEPROM.write(6 * (i + 1) + j, NivelAgua[i][j]);
      EEPROM.write(6 * (i + 4) + j, TemperaturaLim[i][j]);
      EEPROM.write(6 * (i + 7) + j, TemporizadorLim[i][j]);
      EEPROM.write(6 * (i + 10) + j, RotacionTam[i][j]);
    }
  }

  // for (uint8_t i = 0; i < 3; i++)
  // {
  //   for (uint8_t j = 0; j < 2; j++)
  //   {
  //     EEPROM.update(2 * (i + 32) + j, TiempoRotacion[i][j]);
  //   }
  // }
  // Serial.println("Guardado exitosamente");

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
  Serial.print("Programa en pausa: ");
  Serial.println(programaEnPausa);
  Serial.print("Segundero temporizador: ");
  Serial.println(segunderoTemporizador);
  Serial.print("MotorON: ");
  Serial.println(motorON);
  // Serial.print("Presion: ");
  // Serial.println(valorPresion);
  // Serial.print("Nivel rotacion tambor: ");
  // Serial.println(nivelRotacionTambor);
  // Serial.print("tiempo rotacion: ");
  // Serial.println(tiempoRotacion);
  // Serial.print("tiempo pausa: ");
  // Serial.println(tiempoPausa);
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
        if (programa == 3)
        {
          fase++;
          if (fase > 4)
          {
            fase = 1;
          }
          pintarVentanaEdicionMenu();
          asignarBlinkLCD();
          pintarConsolaSerial();
        }
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
        if (programa == 3)
        {
          fase--;
          if (fase < 1)
          {
            fase = 4;
          }
          pintarVentanaEdicionMenu();
          asignarBlinkLCD();
          pintarConsolaSerial();
        }
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
