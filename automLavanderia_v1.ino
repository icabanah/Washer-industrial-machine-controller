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
boolean terminarEdicion = false;
boolean nivelActivo = HIGH;

// Antirebote
int flag1 = 0, flag2 = 0;

// Variables de proceso
uint8_t programa = 0;
uint8_t horas[2] = {0, 0};
uint8_t minutos[2] = {0, 0};
uint8_t segundos[2] = {0, 0};
uint8_t valorTemperatura = 40;
// uint8_t sensorPresion = 40;

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
    pintarVentana1(programa);
}

void loop()
{
    if (digitalRead(btnAumentar) == nivelActivo)
    {
        if (flag1 == 0)
        {
            flag1 = 1;
            programa++;
            if (programa < 0) programa = 2;
            if (programa > 2) programa = 0;
            pintarVentana1(programa);

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
            if (programa < 0) programa = 2;
            if (programa > 2) programa = 0;
            pintarVentana1(programa);
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
        segundosTemporizador.Start();
        // temporizador();
        pintarVentana2(programa, 1);
        while (!tiempoCumplido)
        {
            temporizador(TemporizadorLim[programa][0]);
            pintarVariablesVentana2(programa, 1);
        }
        // while (!tiempoCumplido)
        // {
        //     pintarVentana2(programa, 1);
        //     temporizador(TemporizadorLim[programa][0])
        // }
    }
}

void pintarVentana1(uint8_t nroPrograma)
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

void pintarVariablesVentana1(uint8_t nroPrograma)
{
    // primera fila
    lcd.setCursor(1, 0);
    // lcd.print(" ");
    lcd.print(nroPrograma + 22);
}

void pintarVentana2(uint8_t nroPrograma, uint8_t nroFase)
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
    lcd.print(NivelAgua[nroPrograma][nroFase - 1]);

    lcd.setCursor(12, 0);
    lcd.print("T:");
    lcd.setCursor(14, 0);
    lcd.print(valorTemperatura);
    // lcd.print(TemperaturaLim[nroPrograma][0]);

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
    lcd.print("Ro:");
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
