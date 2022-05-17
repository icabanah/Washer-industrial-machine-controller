#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "AsyncTaskLib.h"

// Iniciamos el lcd
LiquidCrystal_I2C lcd(0x20, 16, 2);
// LiquidCrystal_I2C lcd(0x27, 16, 2);

// Entradas
#define btnComenzar A1
#define btnEditar A2
#define btnDisminuir A3
#define btnAumentar A4
#define btnParar A5
#define sensorTemp A6
#define sensorPresion A7

// Salidas
#define MagnetPuerta A8
#define ValvulAgua A9
#define MotorDirA A10
#define MotorDirB A11
#define ElectrovVapor A12
#define ValvulOnOff A13

// Definimos variables de los programas
uint8_t NivelAgua[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};
uint8_t RotacionTam[3][4] = {{1, 2, 2, 2}, {1, 2, 3, 1}, {1, 2, 3, 2}};
uint8_t TemperaturaLim[3][4] = {{30, 0, 0, 0}, {45, 0, 0, 0}, {55, 0, 0, 0}};
uint8_t TemporizadorLim[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};
uint8_t RotacionIzq[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};
uint8_t RotacionPau[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};
uint8_t RotacionDer[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};
uint8_t TiempoEntFase[3][4] = {{1, 3, 4, 5}, {1, 3, 4, 5}, {1, 3, 4, 5}};

// Variables bandera
boolean tiempoCumplido = false;
// boolean terminarEdicion = false;
boolean nivelActivo = HIGH;

// Antirebote
int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0, flag5 = 0;

// Variables de proceso
uint8_t programa = 1;
uint8_t numeroVariable = 1;
uint8_t valorVariable = 0;
uint8_t fase = 1;
uint8_t nivelEdicion = 0;

uint8_t horas[2] = {0, 0};
uint8_t minutos[2] = {0, 0};
uint8_t segundos[2] = {0, 0};
uint8_t valorTemperatura = 40;
uint8_t valorPresion = 40;

// AsyncTask temporizadorMinutos (time, []() { tiempoCumplido = true });
// AsyncTask asyncTask2(2000, []() {

//  });
AsyncTask segundosMotor(1000, true, []()
                        { segundos[0]++; });

AsyncTask segundosTemporizador(1000, true, []()
                               { segundos[1]++; });

// AsyncTask asyncTask4(1000, []() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("Linea 2");
// });
void setup()
{
  // Inicializamos salidas
  pinMode(btnAumentar, INPUT);
  pinMode(btnComenzar, INPUT);
  pinMode(btnDisminuir, INPUT);
  pinMode(btnEditar, INPUT);
  pinMode(btnParar, INPUT);
  pinMode(sensorTemp, INPUT);
  pinMode(sensorPresion, INPUT);

  // digitalWrite(habilitar,nivelOn);

  // Inicializamos el lcd
  lcd.begin();
  lcd.backlight();
  // lcd.cursor();
  // lcd.leftToRight();

  // inicializamos subprocesos
  pintarVentanaSeleccion(programa);
}

void loop()
{
  if (digitalRead(btnAumentar) == nivelActivo)
  {
    if (flag1 == 0)
    {
      flag1 = 1;
      programa++;
      if (programa < 0)
        programa = 2;
      if (programa > 2)
        programa = 0;
      pintarVentanaSeleccion(programa);

      // eeprom_write();
      // delay(100);
    }
  }
  else
  {
    flag1 = 0;
  }

  if (digitalRead(btnDisminuir) == nivelActivo)
  {
    if (flag2 == 0)
    {
      flag2 = 1;
      programa--;
      if (programa < 0)
        programa = 2;
      if (programa > 2)
        programa = 0;
      pintarVentanaSeleccion(programa);
      // eeprom_write();
      // delay(100);
    }
  }
  else
  {
    flag2 = 0;
  }

  if (digitalRead(btnComenzar) == nivelActivo)
  {
    if (flag3 == 0)
    {
      segundosTemporizador.Start();
      pintarVentanaEjecucion(programa, fase);
      while (!tiempoCumplido)
      {
        temporizador(TemporizadorLim[programa][0]);
        pintarVariablesVentana2(programa, 1);
      }
    }
  }
  else
  {
    flag3 = 0;
  }

  // Edicion de programa
  if (digitalRead(btnEditar) == nivelActivo)
  {
    if (flag4 == 0)
    {
      // Edicion de numero de variable
      while (nivelEdicion == 1)
      {
        if (digitalRead(btnAumentar) == nivelActivo)
        {
          if (flag1 == 0)
          {
            flag1 = 1;
            numeroVariable++;
            if (numeroVariable < 0)
              numeroVariable = 4;
            if (numeroVariable > 4)
              numeroVariable = 0;
            pintarVentanaEdicionMenu(programa, fase, numeroVariable);

            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flag1 = 0;
        }

        if (digitalRead(btnDisminuir) == nivelActivo)
        {
          if (flag2 == 0)
          {
            flag2 = 1;
            numeroVariable--;
            if (numeroVariable < 0)
              numeroVariable = 4;
            if (numeroVariable > 4)
              numeroVariable = 0;
            pintarVentanaEdicionMenu(programa, fase, numeroVariable);
            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flag2 = 0;
        }

        if (digitalRead(btnEditar) == nivelActivo)
        {
          if (flag4 == 0)
          {
            nivelEdicion = 2;
            valorVariable = obtenerValorVariable(programa, fase, numeroVariable);
            // Edicion de valor de numeroVariable
            while (nivelEdicion == 2)
            {
              if (digitalRead(btnAumentar) == nivelActivo)
              {
                if (flag1 == 0)
                {
                  flag1 = 1;
                  valorVariable++;
                  if (valorVariable < 0)
                    valorVariable = 4;
                  if (valorVariable > 4)
                    valorVariable = 0;
                  pintarVentanaEdicionMenu(programa, fase, numeroVariable);
                  // eeprom_write();
                  // delay(100);
                }
              }
              else
              {
                flag1 = 0;
              }

              if (digitalRead(btnDisminuir) == nivelActivo)
              {
                if (flag2 == 0)
                {
                  flag2 = 1;
                  valorVariable--;
                  if (valorVariable < 0)
                    valorVariable = 4;
                  if (valorVariable > 4)
                    valorVariable = 0;
                  pintarVentanaEdicionMenu(programa, fase, numeroVariable);
                  // eeprom_write();
                  // delay(100);
                }
              }
              else
              {
                flag2 = 0;
              }

              if (digitalRead(btnEditar) == nivelActivo)
              {
                if (flag4 == 0)
                {
                  nivelEdicion = 3;
                  // fase = 0;
                  // Editar valor de fase

                  while (nivelEdicion == 3)
                  {
                    if (digitalRead(btnAumentar) == nivelActivo)
                    {
                      if (flag1 == 0)
                      {
                        flag1 = 1;
                        fase++;
                        if (fase < 0)
                          fase = 4;
                        if (fase > 4)
                          fase = 0;
                        pintarVentanaEdicionMenu(programa, fase, numeroVariable);

                        // eeprom_write();
                        // delay(100);
                      }
                    }
                    else
                    {
                      flag1 = 0;
                    }

                    if (digitalRead(btnDisminuir) == nivelActivo)
                    {
                      if (flag2 == 0)
                      {
                        flag2 = 1;
                        fase--;
                        if (fase < 0)
                          fase = 4;
                        if (fase > 4)
                          fase = 0;
                        pintarVentanaEdicionMenu(programa, fase, numeroVariable);
                        // eeprom_write();
                        // delay(100);
                      }
                    }
                    else
                    {
                      flag2 = 0;
                    }

                    if (digitalRead(btnParar) == nivelActivo)
                    {
                      if (flag5 == 0)
                      {
                        nivelEdicion = 2;
                        // fase = 0;
                        // eeprom_write();
                        // delay(100);
                      }
                    }
                    else
                    {
                      flag5 = 0;
                    }
                  }

                  // eeprom_write();
                  // delay(100);
                }
              }
              else
              {
                flag4 = 0;
              }

              if (digitalRead(btnParar) == nivelActivo)
              {
                if (flag5 == 0)
                {
                  nivelEdicion = 1;
                  // fase = 0;
                  // eeprom_write();
                  // delay(100);
                }
              }
              else
              {
                flag5 = 0;
              }
            }

            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flag4 = 0;
        }

        if (digitalRead(btnParar) == nivelActivo)
        {
          if (flag5 == 0)
          {
            nivelEdicion = 0;
            editarValorVariable(programa, fase, numeroVariable, valorVariable);
            numeroVariable = 1;
            // fase = 1;
            // eeprom_write();
            // delay(100);
          }
        }
        else
        {
          flag5 = 0;
        }
      }
    }
    pintarVentanaEdicionMenu(programa, fase, numeroVariable);
    nivelEdicion = 1;
  }
  else
  {
    flag4 = 0;
  }
}

void pintarVentanaSeleccion(uint8_t nroPrograma)
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(nroPrograma + 22);

  lcd.setCursor(4, 0);
  lcd.print("N");
  for (size_t i = 0; i < 4; i++)
  {
    lcd.setCursor(i + 5, 0);
    lcd.print(NivelAgua[nroPrograma][i]);
  }

  lcd.setCursor(10, 0);
  lcd.print("T");
  for (size_t i = 0; i < 4; i++)
  {
    lcd.setCursor(i + 11, 0);
    lcd.print(TemporizadorLim[nroPrograma][i]);
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
  for (size_t i = 0; i < 4; i++)
  {
    lcd.setCursor(3 * i, 1);
    lcd.print(TemperaturaLim[nroPrograma][i]);
  }

  lcd.setCursor(12, 1);
  lcd.print("R");
  for (size_t i = 0; i < 3; i++)
  {
    lcd.setCursor(i + 13, 1);
    lcd.print(RotacionTam[nroPrograma][i]);
  }
}

void pintarVentanaEdicionMenu(uint8_t nroPrograma, uint8_t nroFase, uint8_t nroVariable)
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(nroPrograma + 22);

  lcd.setCursor(4, 0);
  lcd.print("Seleccione:");

  // Segunda fila
  switch (nroVariable)
  {
  case 1:
    lcd.setCursor(0, 1);
    lcd.print("Nivel:");
    lcd.setCursor(6, 1);
    lcd.print(NivelAgua[nroPrograma - 1][nroFase - 1]);
    break;

  case 2:
    lcd.setCursor(0, 1);
    lcd.print("Tiemp:");
    lcd.setCursor(6, 1);
    lcd.print(TemporizadorLim[nroPrograma - 1][nroFase - 1]);
    break;

  case 3:
    lcd.setCursor(0, 1);
    lcd.print("Tempe:");
    lcd.setCursor(6, 1);
    lcd.print(TemperaturaLim[nroPrograma - 1][nroFase - 1]);
    break;

  case 4:
    lcd.setCursor(0, 1);
    lcd.print("VRota:");
    lcd.setCursor(6, 1);
    lcd.print(RotacionTam[nroPrograma - 1][nroFase - 1]);
    break;

  default:
    break;
  }
}

void editarValorVariable(uint8_t nroPrograma, uint8_t nroFase, uint8_t nroVariable, uint8_t vlrVariable)
{
  switch (nroVariable)
  {
  case 1:
    NivelAgua[nroPrograma][nroFase] = vlrVariable;
    break;

  case 2:
    TemporizadorLim[nroPrograma][nroFase] = vlrVariable;
    break;

  case 3:
    TemperaturaLim[nroPrograma][nroFase] = vlrVariable;
    break;

  case 4:
    RotacionTam[nroPrograma][nroFase] = vlrVariable;
    break;

  default:
    break;
  }
}

uint8_t obtenerValorVariable(uint8_t nroPrograma, uint8_t nroFase, uint8_t nroVariable)
{
  uint8_t valorTemporal = 0;
  switch (nroVariable)
  {
  case 1:
    valorTemporal = NivelAgua[nroPrograma][nroFase];
    break;

  case 2:
    valorTemporal = TemporizadorLim[nroPrograma][nroFase];
    break;

  case 3:
    valorTemporal = TemperaturaLim[nroPrograma][nroFase];
    break;

  case 4:
    valorTemporal = RotacionTam[nroPrograma][nroFase];
    break;

  default:
    break;
  }
  return valorTemporal;
}

void pintarVentanaEjecucion(uint8_t nroPrograma, uint8_t nroFase)
{
  lcd.clear();
  // primera fila
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.setCursor(1, 0);
  lcd.print(nroPrograma + 22);

  lcd.setCursor(4, 0);
  lcd.print("F:");
  lcd.setCursor(6, 0);
  lcd.print(nroFase);

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
  lcd.print(minutos[1]);
  lcd.setCursor(5, 1);
  lcd.print(":");
  lcd.setCursor(6, 1);
  lcd.print(segundos[1]);

  lcd.setCursor(12, 1);
  lcd.print("R:");
  lcd.setCursor(14, 1);
  lcd.print(RotacionTam[nroPrograma][nroFase - 1]);
}

void pintarVariablesVentana2(uint8_t nroPrograma, uint8_t nroFase)
{
  // lcd.setCursor(4, 0);
  // lcd.print("F:");
  // lcd.setCursor(6, 0);
  // lcd.print(nroFase);

  // lcd.setCursor(8, 0);
  // lcd.print("N:");
  // lcd.setCursor(10, 0);
  // lcd.print(NivelAgua[nroPrograma][nroFase - 1]);

  lcd.setCursor(14, 0);
  lcd.print(" ");
  // lcd.print(valorTemperatura);
  lcd.print(TemperaturaLim[nroPrograma][0]);

  // Segunda fila
  lcd.setCursor(3, 1);
  lcd.print(" ");
  lcd.print(minutos[1]);
  lcd.setCursor(6, 1);
  lcd.print(" ");
  lcd.print(segundos[1]);
}

void controladorMotor(uint8_t minutos)
{
}

void temporizador(uint8_t min)
{
  segundosTemporizador.Update();
  horas[1] = (segundos[1] / 3600);
  minutos[1] = ((segundos[1] - horas[1] * 3600) / 60);
  segundos[1] = segundos[1] - (horas[1] * 3600 + minutos[1] * 60);
  if (minutos[1] == min)
  {
    segundosTemporizador.Stop();
    segundos[1] = 0;
  }
}
