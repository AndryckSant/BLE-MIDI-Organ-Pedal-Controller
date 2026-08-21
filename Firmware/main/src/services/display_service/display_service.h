/******************************************************************************
 * @file    display_service.h
 * @author  Andryck Santiago
 * @brief   Display service public interface.
 *
 * @details
 * Provides high-level functions for updating the device user interface.
 *
 * This service handles screen content and application-level display logic,
 * while the SSD1306 driver handles low-level hardware communication.
 ******************************************************************************/

#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initializes the display service.
 *
 * @return true if initialization succeeds.
 * @return false otherwise.
 */
bool display_init();

/**
 * @brief Displays the initial device screen.
 */
void display_show_boot();

/**
 * @brief Displays the BLE connection status.
 *
 * @param connected BLE connection state.
 */
void display_show_ble_status(bool connected);

void display_show_config(const char *message);

/**
 * @brief Displays a pedal MIDI value.
 *
 * @param pedal Pedal index.
 * @param value MIDI value (0-127).
 */
void display_show_pedal(uint8_t pedal,
                        uint8_t value);

/**
 * @brief Displays an error message.
 *
 * @param message Error message to display.
 */
void display_show_error(const char *message);

#endif // DISPLAY_SERVICE_H