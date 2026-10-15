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

#include "../../board_pins.h"
#include "../../drivers/nvs_driver/nvs_driver.h"

namespace {
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

struct ConfigBlob {
    uint32_t magic;
    uint16_t version;
    SystemConfig config;
};

struct ConfigHeader {
    uint32_t magic;
    uint16_t version;
};

struct LegacyV1CalibrationData {
    float minimum;
    float maximum;
    bool initialized;
};

struct LegacyV1PedalModel {
    bool enabled;
    uint8_t adcChannel;
    uint8_t midiChannel;
    uint8_t midiCc;
    int rawValue;
    int filteredValue;
    LegacyV1CalibrationData calibration;
    uint8_t midiValue;
    uint8_t lastMidiValue;
};

struct LegacyV1ButtonModel {
    bool enabled;
    uint8_t gpioPin;
    bool inverted;
    uint8_t midiChannel;
    uint8_t midiCc;
    bool pressed;
    bool lastPressed;
};

struct LegacyV1SystemConfig {
    LegacyV1PedalModel pedals[kPedalCount];
    LegacyV1ButtonModel buttons[kButtonCount];
};

struct LegacyV1ConfigBlob {
    uint32_t magic;
    uint16_t version;
    LegacyV1SystemConfig config;
};

struct LegacyV2CalibrationData {
    float minimum;
    float maximum;
    bool initialized;
    bool autoRange;
};

struct LegacyV2PedalModel {
    bool enabled;
    uint8_t adcChannel;
    uint8_t midiChannel;
    uint8_t midiCc;
    int rawValue;
    int filteredValue;
    LegacyV2CalibrationData calibration;
    uint8_t midiValue;
    uint8_t lastMidiValue;
};

struct LegacyV2ButtonModel {
    bool enabled;
    uint8_t gpioPin;
    bool inverted;
    uint8_t midiChannel;
    uint8_t midiCc;
    bool pressed;
    bool lastPressed;
};

struct LegacyV2SystemConfig {
    LegacyV2PedalModel pedals[kPedalCount];
    LegacyV2ButtonModel buttons[kButtonCount];
};

struct LegacyV2ConfigBlob {
    uint32_t magic;
    uint16_t version;
    LegacyV2SystemConfig config;
};

static_assert(sizeof(LegacyV1SystemConfig) == sizeof(LegacyV2SystemConfig),
              "Legacy configuration layout changed; update migration.");

//=============================================================================
// Private Variables
//=============================================================================

SystemConfig config;

bool configInitialized = false;

//=============================================================================
// Private Functions
//=============================================================================

void setFactoryDefaults(SystemConfig &config);
bool loadFromNvs(SystemConfig &config);
bool saveToNvs(const SystemConfig &config);
bool validatePedal(const PedalModel &pedal);
bool validateButton(const ButtonModel &button);

void setFactoryDefaults(SystemConfig &config) {
    for (uint8_t index = 0U; index < kPedalCount; ++index) {
        PedalModel &pedal = config.pedals[index];

        pedal.enabled = true;
        pedal.adcChannel = index;
        pedal.midiChannel = 1U;
        pedal.midiCc = (index == 0U) ? 11U : 7U;

        pedal.rawValue = 0;
        pedal.filteredValue = 0;

        pedal.calibration.minimum = kAdcMaximum;
        pedal.calibration.maximum = 0.0f;
        pedal.calibration.initialized = false;
        pedal.calibration.autoRange = true;

        pedal.midiValue = 0U;
        pedal.lastMidiValue = 255U;
    }

    for (uint8_t index = 0U; index < kButtonCount; ++index) {
        ButtonModel &button = config.buttons[index];

        button.enabled = false;
        button.gpioPin = BoardPins::kMidiButtons[index];
        button.inverted = false;
        button.midiChannel = 1U;
        button.midiCc = 20U + index;
        button.latch = true;
        button.latched = false;
        button.pressed = false;
        button.lastPressed = false;
    }
}

bool loadFromNvs(SystemConfig &config) {
    const size_t blobLength = nvs_get_blob_length(kConfigKey);

    if (blobLength < sizeof(ConfigHeader) || blobLength > sizeof(ConfigBlob)) {
        return false;
    }

    uint8_t blobBuffer[sizeof(ConfigBlob)] = {};
    if (!nvs_read_blob(kConfigKey, blobBuffer, blobLength)) {
        return false;
    }

    ConfigHeader header{};
    memcpy(&header, blobBuffer, sizeof(header));

    if (header.magic != kConfigMagic) {
        return false;
    }

    if (header.version == kConfigVersion) {
        if (blobLength != sizeof(ConfigBlob)) {
            return false;
        }

        ConfigBlob blob{};
        memcpy(&blob, blobBuffer, sizeof(blob));
        config = blob.config;
        return true;
    }

    if (header.version == kAutoRangeConfigVersion && blobLength == sizeof(LegacyV2ConfigBlob)) {
        LegacyV2ConfigBlob legacyBlob{};
        memcpy(&legacyBlob, blobBuffer, sizeof(legacyBlob));

        for (uint8_t index = 0U; index < kPedalCount; ++index) {
            const LegacyV2PedalModel &legacyPedal = legacyBlob.config.pedals[index];
            PedalModel &pedal = config.pedals[index];

            pedal.enabled = legacyPedal.enabled;
            pedal.adcChannel = legacyPedal.adcChannel;
            pedal.midiChannel = legacyPedal.midiChannel;
            pedal.midiCc = legacyPedal.midiCc;
            pedal.rawValue = legacyPedal.rawValue;
            pedal.filteredValue = legacyPedal.filteredValue;
            pedal.calibration.minimum = legacyPedal.calibration.minimum;
            pedal.calibration.maximum = legacyPedal.calibration.maximum;
            pedal.calibration.initialized = legacyPedal.calibration.initialized;
            pedal.calibration.autoRange = legacyPedal.calibration.autoRange;
            pedal.midiValue = legacyPedal.midiValue;
            pedal.lastMidiValue = legacyPedal.lastMidiValue;
        }

        for (uint8_t index = 0U; index < kButtonCount; ++index) {
            const LegacyV2ButtonModel &legacyButton = legacyBlob.config.buttons[index];
            ButtonModel &button = config.buttons[index];

            button.enabled = legacyButton.enabled;
            button.gpioPin = legacyButton.gpioPin;
            button.inverted = legacyButton.inverted;
            button.midiChannel = legacyButton.midiChannel;
            button.midiCc = legacyButton.midiCc;
            button.latch = true;
            button.latched = false;
            button.pressed = legacyButton.pressed;
            button.lastPressed = legacyButton.lastPressed;
        }

        return true;
    }

    if (header.version == kLegacyConfigVersion && blobLength == sizeof(LegacyV1ConfigBlob)) {
        LegacyV1ConfigBlob legacyBlob{};
        memcpy(&legacyBlob, blobBuffer, sizeof(legacyBlob));

        for (uint8_t index = 0U; index < kPedalCount; ++index) {
            const LegacyV1PedalModel &legacyPedal = legacyBlob.config.pedals[index];
            PedalModel &pedal = config.pedals[index];

            pedal.enabled = legacyPedal.enabled;
            pedal.adcChannel = legacyPedal.adcChannel;
            pedal.midiChannel = legacyPedal.midiChannel;
            pedal.midiCc = legacyPedal.midiCc;
            pedal.rawValue = legacyPedal.rawValue;
            pedal.filteredValue = legacyPedal.filteredValue;
            pedal.calibration.minimum = legacyPedal.calibration.minimum;
            pedal.calibration.maximum = legacyPedal.calibration.maximum;
            pedal.calibration.initialized = legacyPedal.calibration.initialized;
            pedal.calibration.autoRange = true;
            pedal.midiValue = legacyPedal.midiValue;
            pedal.lastMidiValue = legacyPedal.lastMidiValue;
        }

        for (uint8_t index = 0U; index < kButtonCount; ++index) {
            const LegacyV1ButtonModel &legacyButton = legacyBlob.config.buttons[index];
            ButtonModel &button = config.buttons[index];

            button.enabled = legacyButton.enabled;
            button.gpioPin = legacyButton.gpioPin;
            button.inverted = legacyButton.inverted;
            button.midiChannel = legacyButton.midiChannel;
            button.midiCc = legacyButton.midiCc;
            button.latch = true;
            button.latched = false;
            button.pressed = legacyButton.pressed;
            button.lastPressed = legacyButton.lastPressed;
        }

        return true;
    }

    return false;
}

bool saveToNvs(const SystemConfig &config) {
    ConfigBlob blob{};

    blob.magic = kConfigMagic;
    blob.version = kConfigVersion;
    blob.config = config;

    return nvs_write_blob(kConfigKey, &blob, sizeof(blob));
}

bool validatePedal(const PedalModel &pedal) {
    if (pedal.adcChannel >= 4U) {
        return false;
    }

    if (pedal.midiChannel < kMidiChannelMinimum || pedal.midiChannel > kMidiChannelMaximum) {
        return false;
    }

    if (pedal.midiCc < kMidiCcMinimum || pedal.midiCc > kMidiCcMaximum) {
        return false;
    }

    if (pedal.calibration.minimum < 0.0f || pedal.calibration.minimum > kAdcMaximum) {
        return false;
    }

    if (pedal.calibration.maximum < 0.0f || pedal.calibration.maximum > kAdcMaximum) {
        return false;
    }

    if (pedal.calibration.initialized && pedal.calibration.maximum < pedal.calibration.minimum) {
        return false;
    }

    if (!pedal.calibration.autoRange && (!pedal.calibration.initialized ||
                                         pedal.calibration.maximum <= pedal.calibration.minimum)) {
        return false;
    }

    return true;
}

bool validateButton(const ButtonModel &button) {
    if (button.enabled && !BoardPins::is_midi_button(button.gpioPin)) {
        return false;
    }

    if (button.midiChannel < kMidiChannelMinimum || button.midiChannel > kMidiChannelMaximum) {
        return false;
    }

    if (button.midiCc < kMidiCcMinimum || button.midiCc > kMidiCcMaximum) {
        return false;
    }

    return true;
}

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void config_service_init() {
    if (configInitialized) {
        return;
    }

    nvs_init();

    if (!loadFromNvs(config)) {
        setFactoryDefaults(config);
    }

    configInitialized = true;
}

void config_service_process() {
    /*
     * Reserved for future configuration processing.
     */
}

const SystemConfig *config_service_get() {
    if (!configInitialized) {
        config_service_init();
    }

    return &config;
}

SystemConfig &config_service_get_mutable() {
    if (!configInitialized) {
        config_service_init();
    }

    return config;
}

bool config_service_save() {
    if (!configInitialized) {
        config_service_init();
    }

    if (!config_service_validate()) {
        return false;
    }

    if (!saveToNvs(config)) {
        Serial.println("Configuration save failed.");

        return false;
    }

    return true;
}

bool config_service_factory_reset() {
    if (!configInitialized) {
        config_service_init();
    }

    setFactoryDefaults(config);

    if (!saveToNvs(config)) {
        Serial.println("Configuration reset save failed.");

        return false;
    }

    return true;
}

bool config_service_validate() {
    if (!configInitialized) {
        config_service_init();
    }

    for (uint8_t index = 0U; index < kPedalCount; ++index) {
        if (!validatePedal(config.pedals[index])) {
            return false;
        }
    }

    for (uint8_t index = 0U; index < kButtonCount; ++index) {
        if (!validateButton(config.buttons[index])) {
            return false;
        }
    }

    return true;
}