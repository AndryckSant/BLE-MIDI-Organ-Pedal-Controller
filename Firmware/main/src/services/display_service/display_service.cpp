/******************************************************************************
 *
 * @file    display_service.cpp
 * @author  Andryck Santiago
 * @brief   Display service implementation.
 *
 * @details
 * This module implements the high-level display logic for the device.
 *
 * The service is responsible for displaying boot information, BLE
 * connection status, pedal MIDI values, and error messages.
 *
 * Low-level SSD1306 communication is handled by the SSD1306 driver.
 *
 ******************************************************************************/

#include "display_service.h"

#include <stdio.h>

#include "../../drivers/display_driver/ssd1306_driver.h"

namespace
{
    //=============================================================================
    // Private Constants
    //=============================================================================

    /**
     * @brief Horizontal cursor position used for display text.
     */
    constexpr uint8_t kDisplayStartX = 0U;

    /**
     * @brief Vertical position of the first display line.
     */
    constexpr uint8_t kDisplayLine0 = 0U;

    /**
     * @brief Vertical position of the second display line.
     */
    constexpr uint8_t kDisplayLine1 = 10U;

    /**
     * @brief Vertical position of the third display line.
     */
    constexpr uint8_t kDisplayLine2 = 20U;

    /**
     * @brief Maximum message buffer size.
     */
    constexpr uint8_t kMessageBufferSize = 32U;

    /**
     * @brief Number of supported pedals.
     */
    constexpr uint8_t kPedalCount = 2U;

    //=============================================================================
    // Private Types
    //=============================================================================

    //=============================================================================
    // Private Variables
    //=============================================================================

    /**
     * @brief Indicates whether the display service has been initialized.
     */
    bool initialized = false;

    //=============================================================================
    // Private Functions
    //=============================================================================

    void showLines(const char *line0,
                const char *line1,
                const char *line2)
    {
        ssd1306_clear();

        ssd1306_set_cursor(kDisplayStartX, kDisplayLine0);
        ssd1306_print(line0);

        ssd1306_set_cursor(kDisplayStartX, kDisplayLine1);
        ssd1306_print(line1);

        ssd1306_set_cursor(kDisplayStartX, kDisplayLine2);
        ssd1306_print(line2);

        ssd1306_display();
    }
} // namespace

//=============================================================================
// Public Functions
//=============================================================================

bool display_init()
{
    if(initialized){
        return true;
    }

    if(ssd1306_init() != SSD1306_OK){
        initialized = false;

        return false;
    }

    initialized = true;

    ssd1306_clear();
    ssd1306_display();

    return true;
}

void display_show_boot()
{
    if(!initialized){
        return;
    }

    showLines("BLE MIDI",
              "Controller",
              "V1.1.0");
}

void display_show_ble_status(bool connected)
{
    if(!initialized){
        return;
    }

    if(connected){
        showLines("BLE MIDI",
                  "Status:",
                  "Connected");

        return;
    }

    showLines("BLE MIDI",
              "Status:",
              "Disconnected");
}

void display_show_config(const char *message){
    if(!initialized){
        return;
    }

    if(message == nullptr){
        return;
    }

    showLines("BLE MIDI",
              "Config Mode IP:",
                message);

}

void display_show_pedal(uint8_t pedal,
                        uint8_t value)
{
    if(!initialized){
        return;
    }

    if(pedal >= kPedalCount){
        return;
    }

    char valueText[kMessageBufferSize];
    char pedalText[kMessageBufferSize];

    snprintf(pedalText,
             sizeof(pedalText),
             "PEDAL %u",
             static_cast<unsigned int>(pedal + 1U));

    snprintf(valueText,
             sizeof(valueText),
             "MIDI: %u",
             static_cast<unsigned int>(value));

    showLines("BLE MIDI",
              pedalText,
              valueText);
}

void display_show_error(const char *message)
{
    if(!initialized){
        return;
    }

    if(message == nullptr){
        return;
    }

    showLines("BLE MIDI",
              "ERROR:",
              message);
}