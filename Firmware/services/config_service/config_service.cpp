#include "config_service.h"

#include <Arduino.h>

#include "../../drivers/nvs_driver/nvs_driver.h"

namespace {

constexpr uint32_t kConfigMagic = 0x424D4346; // BMCF
constexpr uint16_t kConfigVersion = 1;
constexpr char kConfigKey[] = "system_config";

struct ConfigBlob {
	uint32_t magic;
	uint16_t version;
	SystemConfig config;
};

SystemConfig g_config;
bool g_config_initialized = false;

void set_factory_defaults(SystemConfig &config) {
	for (uint8_t index = 0; index < 2; ++index) {
		PedalModel &pedal = config.pedals[index];
		pedal.enabled = true;
		pedal.adc_channel = index;
		pedal.midi_channel = 1;
		pedal.midi_cc = (index == 0) ? 11 : 7;
		pedal.raw_value = 0;
		pedal.filtered_value = 0;
		pedal.calibration_min = 32767.0f;
		pedal.calibration_max = 0.0f;
		pedal.calibration_initialized = false;
		pedal.midi_value = 0;
		pedal.last_midi_value = 255;
	}

	for (uint8_t index = 0; index < 8; ++index) {
		ButtonModel &button = config.buttons[index];
		button.enabled = false;
		button.gpio_pin = 0;
		button.inverted = false;
		button.midi_channel = 1;
		button.midi_cc = 20 + index;
		button.pressed = false;
		button.last_pressed = false;
	}
}

bool load_from_nvs(SystemConfig &config) {
	ConfigBlob blob{};
	if (!nvs_read_blob(kConfigKey, &blob, sizeof(blob))) {
		return false;
	}

	if (blob.magic != kConfigMagic || blob.version != kConfigVersion) {
		return false;
	}

	config = blob.config;
	return true;
}

bool save_to_nvs(const SystemConfig &config) {
	ConfigBlob blob{};
	blob.magic = kConfigMagic;
	blob.version = kConfigVersion;
	blob.config = config;
	return nvs_write_blob(kConfigKey, &blob, sizeof(blob));
}

} // namespace

void config_service_init() {
	if (g_config_initialized) {
		return;
	}

	nvs_init();
	set_factory_defaults(g_config);
	config_service_load();
	g_config_initialized = true;
}

SystemConfig &config_service_get_mutable() {
	if (!g_config_initialized) {
		config_service_init();
	}

	return g_config;
}

const SystemConfig &config_service_get() {
	if (!g_config_initialized) {
		config_service_init();
	}

	return g_config;
}

void config_service_load() {
	if (!load_from_nvs(g_config)) {
		set_factory_defaults(g_config);
	}
}

void config_service_save() {
	if (!save_to_nvs(g_config)) {
		Serial.println("NVS save failed");
	}
}

void config_service_factory_reset() {
	set_factory_defaults(g_config);
	config_service_save();
}
