/******************************************************************************
 * @file    ssd1306_driver.h
 * @author  Andryck Santiago
 * @brief   SSD1306 OLED Driver public interface.
 *
 * @details
 * Provides the public API for initializing and controlling the SSD1306
 * OLED display.
 *
 * This driver encapsulates the SSD1306 hardware interface and provides
 * basic functions for rendering text to the display.
 ******************************************************************************/

#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdint.h>

/**
 * @brief SSD1306 initialization status.
 */
typedef enum
{
    SSD1306_OK = 0,
    SSD1306_ERROR

} SSD1306_Status_t;


/**
 * @brief Initializes the SSD1306 OLED display.
 *
 * @return SSD1306_OK if initialization succeeds.
 * @return SSD1306_ERROR otherwise.
 */
SSD1306_Status_t ssd1306_init();

/**
 * @brief Clears the display buffer.
 */
void ssd1306_clear();

/**
 * @brief Sets the text cursor position.
 *
 * @param x Horizontal cursor position in pixels.
 * @param y Vertical cursor position in pixels.
 */
void ssd1306_set_cursor(uint8_t x, uint8_t y);

/**
 * @brief Prints text to the display buffer.
 *
 * @param text Null-terminated string to be displayed.
 */
void ssd1306_print(const char *text);

/**
 * @brief Sends the display buffer to the OLED.
 */
void ssd1306_display();

/**
 * @brief Returns the SSD1306 initialization status.
 *
 * @return true if the display is initialized and ready.
 * @return false otherwise.
 */
bool ssd1306_is_ready();

#endif // SSD1306_DRIVER_H