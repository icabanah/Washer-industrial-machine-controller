// === VERIFICACIÓN DE COMPILACIÓN - CONFIG MODULE ===
// Archivo de prueba para verificar que todas las funciones compilen correctamente
// Proyecto: Controlador de Lavadora Industrial - ESP32 + Nextion
// Autor: Ingeniero Electrónico Senior

#include "Arduino.h"
#include "config.h"
#include <stdio.h>
#include <stdarg.h>

// ===== FUNCIÓN DE PRUEBA PARA VERIFICAR COMPILACIÓN =====

/**
 * @brief Función de prueba para verificar que todas las funciones de config.cpp compilen correctamente
 * Esta función no se ejecuta en producción, solo sirve para verificar la compilación
 */
void testConfigFunctions() {
    // Test de validación de parámetros
    int nivelValido = validarNivel(3);
    int tempValida = validarTemperatura(45);
    int tiempoValido = validarTiempo(15);
    int rotacionValida = validarRotacion(2);
    
    // Test de incremento/decremento
    int nivelIncrement = incrementarParametro(PARAM_NIVEL, 2);
    int tempDecrement = decrementarParametro(PARAM_TEMPERATURA, 50);
    
    // Test de navegación
    int siguiente = obtenerSiguienteParametro(PARAM_NIVEL);
    int anterior = obtenerAnteriorParametro(PARAM_ROTACION);
    
    // Test de información de parámetros
    const char* texto = obtenerTextoParametro(PARAM_TEMPERATURA);
    int minimo = obtenerValorMinimo(PARAM_TIEMPO);
    int maximo = obtenerValorMaximo(PARAM_ROTACION);
    int incremento = obtenerIncrementoParametro(PARAM_NIVEL);
    
    // Test de utilidades
    char bufferPrograma[10];
    generarTextoPrograma(1, bufferPrograma, sizeof(bufferPrograma));
    
    bool valido = esParametroValido(PARAM_TEMPERATURA, 45);
    
    char bufferFormato[20];
    formatearParametroConUnidad(PARAM_TEMPERATURA, 45, bufferFormato, sizeof(bufferFormato));
    
    // Evitar warnings de variables no usadas
    (void)nivelValido;
    (void)tempValida;
    (void)tiempoValido;
    (void)rotacionValida;
    (void)nivelIncrement;
    (void)tempDecrement;
    (void)siguiente;
    (void)anterior;
    (void)texto;
    (void)minimo;
    (void)maximo;
    (void)incremento;
    (void)valido;
    
    Serial.println("Test de funciones de config completado exitosamente");
    Serial.println("Programa generado: " + String(bufferPrograma));
    Serial.println("Formato con unidad: " + String(bufferFormato));
}

// ===== ALTERNATIVA PARA COMPATIBILIDAD CON SPRINTF =====

/**
 * @brief Función wrapper para manejar diferencias entre sprintf y snprintf
 * En caso de que snprintf no esté disponible en alguna versión de Arduino
 */
void safeStringFormat(char* buffer, int bufferSize, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    #ifdef ESP32
        // ESP32 tiene soporte completo para snprintf
        vsnprintf(buffer, bufferSize, format, args);
    #else
        // Para Arduino más antiguos, usar sprintf (menos seguro pero compatible)
        vsprintf(buffer, format, args);
    #endif
    
    va_end(args);
}

// ===== FUNCIÓN DE DIAGNÓSTICO =====

/**
 * @brief Imprimir información de diagnóstico sobre los parámetros del sistema
 */
void printParameterDiagnostics() {
    Serial.println("=== DIAGNÓSTICO DE PARÁMETROS DEL SISTEMA ===");
    
    // Información sobre límites de parámetros
    Serial.println("Límites de Nivel: " + String(MIN_NIVEL) + " - " + String(MAX_NIVEL));
    Serial.println("Límites de Temperatura: " + String(MIN_TEMPERATURA) + " - " + String(MAX_TEMPERATURA) + "°C");
    Serial.println("Límites de Tiempo: " + String(MIN_TIEMPO) + " - " + String(MAX_TIEMPO) + " min");
    Serial.println("Límites de Rotación: " + String(MIN_ROTACION) + " - " + String(MAX_ROTACION));
    
    // Información sobre incrementos
    Serial.println("Incrementos:");
    Serial.println("  Nivel: " + String(INCREMENT_NIVEL));
    Serial.println("  Temperatura: " + String(INCREMENT_TEMP) + "°C");
    Serial.println("  Tiempo: " + String(INCREMENT_TIEMPO) + " min");
    Serial.println("  Rotación: " + String(INCREMENT_ROTACION));
    
    // Test de navegación circular
    Serial.println("Navegación circular de parámetros:");
    int param = PARAM_NIVEL;
    for (int i = 0; i < 5; i++) {
        Serial.println("  " + String(i) + ": " + String(obtenerTextoParametro(param)));
        param = obtenerSiguienteParametro(param);
    }
    
    Serial.println("=== FIN DE DIAGNÓSTICO ===");
}