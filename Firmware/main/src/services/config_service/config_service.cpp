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
#include <string.h>

#include "../../drivers/nvs_driver/nvs_driver.h"
#include "../../board_pins.h"

namespace
{
    //=============================================================================
    // Private Constants
    //=============================================================================

    constexpr uint32_t kConfigMagic = 0x424D4346UL;
    constexpr uint16_t kConfigVersion = 3U;
    constexpr uint16_t kAutoRangeConfigVersion = 2U;
    constexpr uint16_t kLegacyConfigVersion = 1U;

    constexpr char kConfigKey[] = "system_config";

    constexpr uint8_t kPedalCount = 2U;
    constexpr uint8_t kButtonCount = BoardPins::kMidiButtonCount;

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

    struct ConfigHeader
    {
        uint32_t magic;
        uint16_t version;
    };

    struct LegacyV1CalibrationData
    {
        float minimum;
        float maximum;
        bool initialized;
    };

    struct LegacyV1PedalModel
    {
        bool enabled;
        uint8_t adc_channel;
        uint8_t midi_channel;
        uint8_t midi_cc;
        int raw_value;
        int filtered_value;
        LegacyV1CalibrationData calibration;
        uint8_t midi_value;
        uint8_t last_midi_value;
    };

    struct LegacyV1ButtonModel
    {
        bool enabled;
        uint8_t gpio_pin;
        bool inverted;
        uint8_t midi_channel;
        uint8_t midi_cc;
        bool pressed;
        bool last_pressed;
    };

    struct LegacyV1SystemConfig
    {
        LegacyV1PedalModel pedals[kPedalCount];
        LegacyV1ButtonModel buttons[kButtonCount];
    };

    struct LegacyV1ConfigBlob
    {
        uint32_t magic;
        uint16_t version;
        LegacyV1SystemConfig config;
    };

    struct LegacyV2CalibrationData
    {
        float minimum;
        float maximum;
        bool initialized;
        bool auto_range;
    };

    struct LegacyV2PedalModel
    {
        bool enabled;
        uint8_t adc_channel;
        uint8_t midi_channel;
        uint8_t midi_cc;
        int raw_value;
        int filtered_value;
        LegacyV2CalibrationData calibration;
        uint8_t midi_value;
        uint8_t last_midi_value;
    };

    struct LegacyV2ButtonModel
    {
        bool enabled;
        uint8_t gpio_pin;
        bool inverted;
        uint8_t midi_channel;
        uint8_t midi_cc;
        bool pressed;
        bool last_pressed;
    };

    struct LegacyV2SystemConfig
    {
        LegacyV2PedalModel pedals[kPedalCount];
        LegacyV2ButtonModel buttons[kButtonCount];
    };

    struct LegacyV2ConfigBlob
    {
        uint32_t magic;
        uint16_t version;
        LegacyV2SystemConfig config;
    };

    static_assert(sizeof(LegacyV1SystemConfig) ==
                      sizeof(LegacyV2SystemConfig),
                  "Legacy configuration layout changed; update migration.");

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
            pedal.calibration.auto_range = true;

            pedal.midi_value = 0U;
            pedal.last_midi_value = 255U;
        }

        for(uint8_t index = 0U; index < kButtonCount; ++index){
            ButtonModel &button = config.buttons[index];

            button.enabled = false;
            button.gpio_pin = BoardPins::kMidiButtons[index];
            button.inverted = false;
            button.midi_channel = 1U;
            button.midi_cc = 20U + index;
            button.latch = true;
            button.latched = false;
            button.pressed = false;
            button.last_pressed = false;
        }
    }

    bool loadFromNvs(SystemConfig &config)
    {
        const size_t blobLength =
            nvs_get_blob_length(kConfigKey);

        if(blobLength < sizeof(ConfigHeader) ||
           blobLength > sizeof(ConfigBlob)){
            return false;
        }

        uint8_t blobBuffer[sizeof(ConfigBlob)] = {};
        if(!nvs_read_blob(kConfigKey,
                          blobBuffer,
                          blobLength)){
            return false;
        }

        ConfigHeader header{};
        memcpy(&header, blobBuffer, sizeof(header));

        if(header.magic != kConfigMagic){
            return false;
        }

        if(header.version == kConfigVersion){
            if(blobLength != sizeof(ConfigBlob)){
                return false;
            }

            ConfigBlob blob{};
            memcpy(&blob, blobBuffer, sizeof(blob));
            config = blob.config;
            return true;
        }

        if(header.version == kAutoRangeConfigVersion &&
           blobLength == sizeof(LegacyV2ConfigBlob)){
            LegacyV2ConfigBlob legacyBlob{};
            memcpy(&legacyBlob, blobBuffer, sizeof(legacyBlob));

            for(uint8_t index = 0U; index < kPedalCount; ++index){
                const LegacyV2PedalModel &legacyPedal =
                    legacyBlob.config.pedals[index];
                PedalModel &pedal = config.pedals[index];

                pedal.enabled = legacyPedal.enabled;
                pedal.adc_channel = legacyPedal.adc_channel;
                pedal.midi_channel = legacyPedal.midi_channel;
                pedal.midi_cc = legacyPedal.midi_cc;
                pedal.raw_value = legacyPedal.raw_value;
                pedal.filtered_value = legacyPedal.filtered_value;
                pedal.calibration.minimum =
                    legacyPedal.calibration.minimum;
                pedal.calibration.maximum =
                    legacyPedal.calibration.maximum;
                pedal.calibration.initialized =
                    legacyPedal.calibration.initialized;
                pedal.calibration.auto_range =
                    legacyPedal.calibration.auto_range;
                pedal.midi_value = legacyPedal.midi_value;
                pedal.last_midi_value = legacyPedal.last_midi_value;
            }

            for(uint8_t index = 0U; index < kButtonCount; ++index){
                const LegacyV2ButtonModel &legacyButton =
                    legacyBlob.config.buttons[index];
                ButtonModel &button = config.buttons[index];

                button.enabled = legacyButton.enabled;
                button.gpio_pin = legacyButton.gpio_pin;
                button.inverted = legacyButton.inverted;
                button.midi_channel = legacyButton.midi_channel;
                button.midi_cc = legacyButton.midi_cc;
                button.latch = true;
                button.latched = false;
                button.pressed = legacyButton.pressed;
                button.last_pressed = legacyButton.last_pressed;
            }

            return true;
        }

        if(header.version == kLegacyConfigVersion &&
           blobLength == sizeof(LegacyV1ConfigBlob)){
            LegacyV1ConfigBlob legacyBlob{};
            memcpy(&legacyBlob, blobBuffer, sizeof(legacyBlob));

            for(uint8_t index = 0U; index < kPedalCount; ++index){
                const LegacyV1PedalModel &legacyPedal =
                    legacyBlob.config.pedals[index];
                PedalModel &pedal = config.pedals[index];

                pedal.enabled = legacyPedal.enabled;
                pedal.adc_channel = legacyPedal.adc_channel;
                pedal.midi_channel = legacyPedal.midi_channel;
                pedal.midi_cc = legacyPedal.midi_cc;
                pedal.raw_value = legacyPedal.raw_value;
                pedal.filtered_value = legacyPedal.filtered_value;
                pedal.calibration.minimum =
                    legacyPedal.calibration.minimum;
                pedal.calibration.maximum =
                    legacyPedal.calibration.maximum;
                pedal.calibration.initialized =
                    legacyPedal.calibration.initialized;
                pedal.calibration.auto_range = true;
                pedal.midi_value = legacyPedal.midi_value;
                pedal.last_midi_value = legacyPedal.last_midi_value;
            }

            for(uint8_t index = 0U; index < kButtonCount; ++index){
                const LegacyV1ButtonModel &legacyButton =
                    legacyBlob.config.buttons[index];
                ButtonModel &button = config.buttons[index];

                button.enabled = legacyButton.enabled;
                button.gpio_pin = legacyButton.gpio_pin;
                button.inverted = legacyButton.inverted;
                button.midi_channel = legacyButton.midi_channel;
                button.midi_cc = legacyButton.midi_cc;
                button.latch = true;
                button.latched = false;
                button.pressed = legacyButton.pressed;
                button.last_pressed = legacyButton.last_pressed;
            }

            return true;
        }

        return false;
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

        if(!pedal.calibration.auto_range &&
           (!pedal.calibration.initialized ||
            pedal.calibration.maximum <= pedal.calibration.minimum)){
            return false;
        }

        return true;
    }

    bool validateButton(const ButtonModel &button)
    {
        if(button.enabled &&
           !BoardPins::isMidiButton(button.gpio_pin)){
            return false;
        }

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