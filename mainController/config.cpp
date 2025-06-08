// config.cpp
// Funciones de utilidad para gestión de configuración del controlador de lavadora industrial
// Autor: Ingeniero Electrónico Senior
// Proyecto: Controlador de Lavadora Industrial - ESP32 + Nextion
// Ubicación: Arequipa, Perú

#include "Arduino.h"
#include "config.h"
#include <stdio.h>

// ===== FUNCIONES DE VALIDACIÓN DE PARÁMETROS =====

/**
 * @brief Valida y limita el valor de nivel de agua dentro de los rangos permitidos
 * @param valor Valor de nivel a validar
 * @return int Valor limitado dentro del rango MIN_NIVEL - MAX_NIVEL
 */
int validarNivel(int valor) {
    if (valor < MIN_NIVEL) return MIN_NIVEL;
    if (valor > MAX_NIVEL) return MAX_NIVEL;
    return valor;
}

/**
 * @brief Valida y limita el valor de temperatura dentro de los rangos permitidos
 * @param valor Valor de temperatura a validar
 * @return int Valor limitado dentro del rango MIN_TEMPERATURA - MAX_TEMPERATURA
 */
int validarTemperatura(int valor) {
    if (valor < MIN_TEMPERATURA) return MIN_TEMPERATURA;
    if (valor > MAX_TEMPERATURA) return MAX_TEMPERATURA;
    return valor;
}

/**
 * @brief Valida y limita el valor de tiempo dentro de los rangos permitidos
 * @param valor Valor de tiempo a validar
 * @return int Valor limitado dentro del rango MIN_TIEMPO - MAX_TIEMPO
 */
int validarTiempo(int valor) {
    if (valor < MIN_TIEMPO) return MIN_TIEMPO;
    if (valor > MAX_TIEMPO) return MAX_TIEMPO;
    return valor;
}

/**
 * @brief Valida y limita el valor de rotación dentro de los rangos permitidos
 * @param valor Valor de rotación a validar
 * @return int Valor limitado dentro del rango MIN_ROTACION - MAX_ROTACION
 */
int validarRotacion(int valor) {
    if (valor < MIN_ROTACION) return MIN_ROTACION;
    if (valor > MAX_ROTACION) return MAX_ROTACION;
    return valor;
}

// ===== FUNCIONES DE INCREMENTO DE PARÁMETROS =====

/**
 * @brief Incrementa el valor de un parámetro según su tipo
 * @param tipoParam Tipo de parámetro (PARAM_NIVEL, PARAM_TEMPERATURA, etc.)
 * @param valorActual Valor actual del parámetro
 * @return int Nuevo valor incrementado y validado
 */
int incrementarParametro(int tipoParam, int valorActual) {
    int nuevoValor = valorActual;
    
    switch (tipoParam) {
        case PARAM_NIVEL:
            nuevoValor += INCREMENT_NIVEL;
            return validarNivel(nuevoValor);
            
        case PARAM_TEMPERATURA:
            nuevoValor += INCREMENT_TEMP;
            return validarTemperatura(nuevoValor);
            
        case PARAM_TIEMPO:
            nuevoValor += INCREMENT_TIEMPO;
            return validarTiempo(nuevoValor);
            
        case PARAM_ROTACION:
            nuevoValor += INCREMENT_ROTACION;
            return validarRotacion(nuevoValor);
            
        default:
            return valorActual; // No se modifica si el tipo no es válido
    }
}

/**
 * @brief Decrementa el valor de un parámetro según su tipo
 * @param tipoParam Tipo de parámetro (PARAM_NIVEL, PARAM_TEMPERATURA, etc.)
 * @param valorActual Valor actual del parámetro
 * @return int Nuevo valor decrementado y validado
 */int decrementarParametro(int tipoParam, int valorActual) {
    int nuevoValor = valorActual;
    
    switch (tipoParam) {
        case PARAM_NIVEL:
            nuevoValor -= INCREMENT_NIVEL;
            return validarNivel(nuevoValor);
            
        case PARAM_TEMPERATURA:
            nuevoValor -= INCREMENT_TEMP;
            return validarTemperatura(nuevoValor);
            
        case PARAM_TIEMPO:
            nuevoValor -= INCREMENT_TIEMPO;
            return validarTiempo(nuevoValor);
            
        case PARAM_ROTACION:
            nuevoValor -= INCREMENT_ROTACION;
            return validarRotacion(nuevoValor);
            
        default:
            return valorActual; // No se modifica si el tipo no es válido
    }
}

// ===== FUNCIONES DE INFORMACIÓN DE PARÁMETROS =====

/**
 * @brief Obtiene el texto descriptivo de un parámetro
 * @param tipoParam Tipo de parámetro
 * @return const char* Texto descriptivo del parámetro
 */
const char* obtenerTextoParametro(int tipoParam) {
    switch (tipoParam) {
        case PARAM_NIVEL:
            return TEXT_PARAM_NIVEL;
        case PARAM_TEMPERATURA:
            return TEXT_PARAM_TEMPERATURA;
        case PARAM_TIEMPO:
            return TEXT_PARAM_TIEMPO;
        case PARAM_ROTACION:
            return TEXT_PARAM_ROTACION;
        default:
            return "Desconocido";
    }
}
/**
 * @brief Obtiene el valor mínimo permitido para un tipo de parámetro
 * @param tipoParam Tipo de parámetro
 * @return int Valor mínimo permitido
 */
int obtenerValorMinimo(int tipoParam) {
    switch (tipoParam) {
        case PARAM_NIVEL:
            return MIN_NIVEL;
        case PARAM_TEMPERATURA:
            return MIN_TEMPERATURA;
        case PARAM_TIEMPO:
            return MIN_TIEMPO;
        case PARAM_ROTACION:
            return MIN_ROTACION;
        default:
            return 0;
    }
}

/**
 * @brief Obtiene el valor máximo permitido para un tipo de parámetro
 * @param tipoParam Tipo de parámetro
 * @return int Valor máximo permitido
 */
int obtenerValorMaximo(int tipoParam) {
    switch (tipoParam) {
        case PARAM_NIVEL:
            return MAX_NIVEL;
        case PARAM_TEMPERATURA:
            return MAX_TEMPERATURA;
        case PARAM_TIEMPO:
            return MAX_TIEMPO;
        case PARAM_ROTACION:
            return MAX_ROTACION;
        default:
            return 100;
    }
}

/**
 * @brief Obtiene el incremento/decremento para un tipo de parámetro
 * @param tipoParam Tipo de parámetro
 * @return int Valor de incremento
 */
int obtenerIncrementoParametro(int tipoParam) {
    switch (tipoParam) {
        case PARAM_NIVEL:
            return INCREMENT_NIVEL;
        case PARAM_TEMPERATURA:
            return INCREMENT_TEMP;
        case PARAM_TIEMPO:
            return INCREMENT_TIEMPO;
        case PARAM_ROTACION:
            return INCREMENT_ROTACION;
        default:
            return 1;
    }
}
// ===== FUNCIONES DE UTILIDAD PARA PROGRAMAS =====

/**
 * @brief Genera el texto de programa en formato "P22", "P23", "P24"
 * @param numeroPrograma Número del programa (1, 2, 3)
 * @param buffer Buffer donde se almacenará el texto generado
 * @param tamanioBuffer Tamaño del buffer
 */
void generarTextoPrograma(int numeroPrograma, char* buffer, int tamanioBuffer) {
    switch (numeroPrograma) {
        case 1:
            snprintf(buffer, tamanioBuffer, "P22");
            break;
        case 2:
            snprintf(buffer, tamanioBuffer, "P23");
            break;
        case 3:
            snprintf(buffer, tamanioBuffer, "P24");
            break;
        default:
            snprintf(buffer, tamanioBuffer, "P--");
            break;
    }
}

/**
 * @brief Verifica si un parámetro está dentro de sus límites válidos
 * @param tipoParam Tipo de parámetro
 * @param valor Valor a verificar
 * @return bool true si está dentro de los límites, false si no
 */
bool esParametroValido(int tipoParam, int valor) {
    int minimo = obtenerValorMinimo(tipoParam);
    int maximo = obtenerValorMaximo(tipoParam);
    return (valor >= minimo && valor <= maximo);
}

/**
 * @brief Obtiene el siguiente tipo de parámetro en el ciclo de edición
 * @param tipoActual Tipo de parámetro actual
 * @return int Siguiente tipo de parámetro (PARAM_NIVEL -> PARAM_TEMPERATURA -> etc.)
 */
int obtenerSiguienteParametro(int tipoActual) {
    switch (tipoActual) {
        case PARAM_NIVEL:
            return PARAM_TEMPERATURA;
        case PARAM_TEMPERATURA:
            return PARAM_TIEMPO;
        case PARAM_TIEMPO:
            return PARAM_ROTACION;
        case PARAM_ROTACION:
            return PARAM_NIVEL; // Vuelve al inicio
        default:
            return PARAM_NIVEL;
    }
}
/**
 * @brief Obtiene el anterior tipo de parámetro en el ciclo de edición
 * @param tipoActual Tipo de parámetro actual
 * @return int Anterior tipo de parámetro (PARAM_NIVEL <- PARAM_TEMPERATURA <- etc.)
 */
int obtenerAnteriorParametro(int tipoActual) {
    switch (tipoActual) {
        case PARAM_NIVEL:
            return PARAM_ROTACION; // Va al final
        case PARAM_TEMPERATURA:
            return PARAM_NIVEL;
        case PARAM_TIEMPO:
            return PARAM_TEMPERATURA;
        case PARAM_ROTACION:
            return PARAM_TIEMPO;
        default:
            return PARAM_NIVEL;
    }
}

// ===== FUNCIONES DE FORMATEO PARA DISPLAY =====

/**
 * @brief Formatea un valor de parámetro con su unidad correspondiente
 * @param tipoParam Tipo de parámetro
 * @param valor Valor del parámetro
 * @param buffer Buffer donde se almacenará el texto formateado
 * @param tamanioBuffer Tamaño del buffer
 */
void formatearParametroConUnidad(int tipoParam, int valor, char* buffer, int tamanioBuffer) {
    switch (tipoParam) {
        case PARAM_NIVEL:
            snprintf(buffer, tamanioBuffer, "%d", valor);
            break;
        case PARAM_TEMPERATURA:
            snprintf(buffer, tamanioBuffer, "%d°C", valor);
            break;
        case PARAM_TIEMPO:
            snprintf(buffer, tamanioBuffer, "%d min", valor);
            break;
        case PARAM_ROTACION:
            snprintf(buffer, tamanioBuffer, "Vel %d", valor);
            break;
        default:
            snprintf(buffer, tamanioBuffer, "%d", valor);
            break;
    }
}