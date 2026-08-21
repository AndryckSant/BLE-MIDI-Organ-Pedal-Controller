/******************************************************************************
 * @file    ssd1306_driver.cpp
 * @author  Andryck Santiago
 * @brief   SSD1306 OLED Driver.
 *
 * @details
 * This module encapsulates the Adafruit SSD1306 library and provides
 * functions for initializing the OLED display and rendering text.
 *
 * This module is responsible only for hardware-level display control.
 * Screen layout, application state, and user interface logic are
 * handled by higher layers of the application.
 ******************************************************************************/

#include "ssd1306_driver.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


namespace {
    //=========================================================================
    // Private Constants
    //=========================================================================

    /// OLED display width in pixels.
    constexpr uint8_t kScreenWidth = 128;

    /// OLED display height in pixels.
    constexpr uint8_t kScreenHeight = 32;

    /// SSD1306 I2C address.
    constexpr uint8_t kI2cAddress = 0x3C;

    //=========================================================================
    // Private Variables
    //=========================================================================

    Adafruit_SSD1306 display(
        kScreenWidth,
        kScreenHeight,
        &Wire,
        -1
    );

    bool isInitialized = false;
}

//=============================================================================
// Public Functions
//=============================================================================

SSD1306_Status_t ssd1306_init()
{
    if(isInitialized){
        return SSD1306_OK;
    }

    if(!display.begin(SSD1306_SWITCHCAPVCC, kI2cAddress)){
        return SSD1306_ERROR;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.display();

    isInitialized = true;

    return SSD1306_OK;
}

void ssd1306_clear()
{
    if(!isInitialized){
        return;
    }

    display.clearDisplay();
}

void ssd1306_set_cursor(uint8_t x, uint8_t y)
{
    if(!isInitialized){
        return;
    }

    display.setCursor(x, y);
}

void ssd1306_print(const char *text)
{
    if(!isInitialized){
        return;
    }

    display.print(text);
}

void ssd1306_display()
{
    if(!isInitialized){
        return;
    }

    display.display();
}

bool ssd1306_is_ready()
{
    return isInitialized;
}