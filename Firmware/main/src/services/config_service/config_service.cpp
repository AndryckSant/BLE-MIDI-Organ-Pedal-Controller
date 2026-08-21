/******************************************************************************
 *
 * @file    config_service.cpp
 * @author  Andryck Santiago
 * @brief   System configuration service implementation.
 *
 * @details
 * This module manages the system configuration and its persistence
 * in non-volatile memory.
 *
 ******************************************************************************/

#include "config_service.h"

#include <Arduino.h>

#include "../../drivers/nvs_driver/nvs_driver.h"

namespace
{
    //=============================================================================
    // Private Constants
    //=============================================================================

    constexpr uint32_t kConfigMagic = 0x424D4346UL;
    constexpr uint16_t kConfigVersion = 1U;

    constexpr char kConfigKey[] = "system_config";

    constexpr uint8_t kPedalCount = 2U;
    constexpr uint8_t kButtonCount = 8U;

    constexpr uint8_t kMidiChannelMinimum = 1U;
    constexpr uint8_t kMidiChannelMaximum = 16U;

    constexpr uint8_t kMidiCcMinimum = 0U;
    constexpr uint8_t kMidiCcMaximum = 127U;

    constexpr float kAdcMaximum = 32767.0f;

    //=============================================================================
    // Private Types
    //=============================================================================

    struct ConfigBlob
    {
        uint32_t magic;
        uint16_t version;
        SystemConfig config;
    };

    //=============================================================================
    // Private Variables
    //=============================================================================

    SystemConfig g_config;

    bool g_configInitialized = false;

    //=============================================================================
    // Private Functions
    //=============================================================================

    void setFactoryDefaults(SystemConfig &config)
    {
        for(uint8_t index = 0U; index < kPedalCount; ++index){
            PedalModel &pedal = config.pedals[index];

            pedal.enabled = true;
            pedal.adc_channel = index;
            pedal.midi_channel = 1U;
            pedal.midi_cc = (index == 0U) ? 11U : 7U;

            pedal.raw_value = 0;
            pedal.filtered_value = 0;

            pedal.calibration.minimum = kAdcMaximum;
            pedal.calibration.maximum = 0.0f;
            pedal.calibration.initialized = false;

            pedal.midi_value = 0U;
            pedal.last_midi_value = 255U;
        }

        for(uint8_t index = 0U; index < kButtonCount; ++index){
            ButtonModel &button = config.buttons[index];

            button.enabled = false;
            button.gpio_pin = 0U;
            button.inverted = false;
            button.midi_channel = 1U;
            button.midi_cc = 20U + index;
            button.pressed = false;
            button.last_pressed = false;
        }
    }

    bool loadFromNvs(SystemConfig &config)
    {
        ConfigBlob blob{};

        if(!nvs_read_blob(kConfigKey,
                        &blob,
                        sizeof(blob))){
            return false;
        }

        if(blob.magic != kConfigMagic){
            return false;
        }

        if(blob.version != kConfigVersion){
            return false;
        }

        config = blob.config;

        return true;
    }

    bool saveToNvs(const SystemConfig &config)
    {
        ConfigBlob blob{};

        blob.magic = kConfigMagic;
        blob.version = kConfigVersion;
        blob.config = config;

        return nvs_write_blob(kConfigKey,
                            &blob,
                            sizeof(blob));
    }

    bool validatePedal(const PedalModel &pedal)
    {
        if(pedal.adc_channel >= 4U){
            return false;
        }

        if(pedal.midi_channel < kMidiChannelMinimum ||
        pedal.midi_channel > kMidiChannelMaximum){
            return false;
        }

        if(pedal.midi_cc < kMidiCcMinimum ||
        pedal.midi_cc > kMidiCcMaximum){
            return false;
        }

        if(pedal.calibration.minimum < 0.0f ||
        pedal.calibration.minimum > kAdcMaximum){
            return false;
        }

        if(pedal.calibration.maximum < 0.0f ||
        pedal.calibration.maximum > kAdcMaximum){
            return false;
        }

        if(pedal.calibration.initialized &&
        pedal.calibration.maximum < pedal.calibration.minimum){
            return false;
        }

        return true;
    }

    bool validateButton(const ButtonModel &button)
    {
        if(button.midi_channel < kMidiChannelMinimum ||
        button.midi_channel > kMidiChannelMaximum){
            return false;
        }

        if(button.midi_cc < kMidiCcMinimum ||
        button.midi_cc > kMidiCcMaximum){
            return false;
        }

        return true;
    }

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void config_service_init()
{
    if(g_configInitialized){
        return;
    }

    nvs_init();

    if(!loadFromNvs(g_config)){
        setFactoryDefaults(g_config);
    }

    g_configInitialized = true;
}

void config_service_process()
{
    /*
     * Reserved for future configuration processing.
     */
}

const SystemConfig *config_service_get()
{
    if(!g_configInitialized){
        config_service_init();
    }

    return &g_config;
}

SystemConfig &config_service_get_mutable()
{
    if(!g_configInitialized){
        config_service_init();
    }

    return g_config;
}

bool config_service_save()
{
    if(!g_configInitialized){
        config_service_init();
    }

    if(!config_service_validate()){
        return false;
    }

    if(!saveToNvs(g_config)){
        Serial.println("Configuration save failed.");

        return false;
    }

    return true;
}

bool config_service_factory_reset()
{
    if(!g_configInitialized){
        config_service_init();
    }

    setFactoryDefaults(g_config);

    if(!saveToNvs(g_config)){
        Serial.println("Configuration reset save failed.");

        return false;
    }

    return true;
}

bool config_service_validate()
{
    if(!g_configInitialized){
        config_service_init();
    }

    for(uint8_t index = 0U; index < kPedalCount; ++index){
        if(!validatePedal(g_config.pedals[index])){
            return false;
        }
    }

    for(uint8_t index = 0U; index < kButtonCount; ++index){
        if(!validateButton(g_config.buttons[index])){
            return false;
        }
    }

    return true;
}