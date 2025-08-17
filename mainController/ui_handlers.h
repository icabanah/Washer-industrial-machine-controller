#ifndef UI_HANDLERS_H
#define UI_HANDLERS_H

#include <Arduino.h>

// Forward declarations para evitar dependencias circulares
class UIControllerClass;

/**
 * @file ui_handlers.h
 * @brief Handlers de eventos de UI extraídos para optimizar ui_controller.cpp
 * 
 * Este archivo contiene las declaraciones de funciones que manejan eventos
 * específicos de cada página de la interfaz Nextion. Extraídas del archivo
 * ui_controller.cpp para reducir su tamaño y mejorar la organización.
 */

// === HANDLERS DE EVENTOS POR PÁGINA ===

/**
 * @brief Maneja eventos de la página de selección de programas
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleSelectionEvents(uint8_t componentId, UIControllerClass& uiController);

/**
 * @brief Maneja eventos de la página de ejecución de programas
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleExecutionEvents(uint8_t componentId, UIControllerClass& uiController);

/**
 * @brief Maneja eventos de la página de edición de parámetros
 * @param componentId ID del componente tocado en Nextion
 * @param uiController Referencia al controlador de UI principal
 */
void handleEditEvents(uint8_t componentId, UIControllerClass& uiController);

#endif // UI_HANDLERS_H