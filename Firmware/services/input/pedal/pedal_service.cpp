#include "pedal_service.h"

#include <Arduino.h>

namespace {

constexpr int kAdcMax = 32767;
constexpr uint8_t kSmoothDiv = 12;
constexpr uint8_t kDeadBand = 2;
constexpr float kCalibrationRate = 0.001f;
constexpr float kCalibrationMargin = 0.02f;
constexpr float kMinSpan = 10.0f;

float organSwellCurve(float normalized) {
	const float gamma = 0.45f;
	return powf(normalized, gamma);
}

void updateCalibration(int value, float &minimum, float &maximum, bool &initialized) {
	const float topThreshold = static_cast<float>(kAdcMax) * 0.98f;
	const float lowThreshold = static_cast<float>(kAdcMax) * 0.02f;

	if (!initialized) {
		minimum = static_cast<float>(value);
		maximum = static_cast<float>(value);
		initialized = true;
		return;
	}

	if (value >= topThreshold) {
		maximum = maximum * 0.98f + static_cast<float>(value) * 0.02f;
		return;
	}

	if (value <= lowThreshold) {
		minimum = minimum * 0.98f + static_cast<float>(value) * 0.02f;
		return;
	}

	if (value < minimum) {
		minimum = minimum * 0.9f + static_cast<float>(value) * 0.1f;
	} else {
		minimum = minimum * (1.0f - kCalibrationRate) + static_cast<float>(value) * kCalibrationRate;
	}

	if (value > maximum) {
		maximum = maximum * 0.9f + static_cast<float>(value) * 0.1f;
	} else {
		maximum = maximum * (1.0f - kCalibrationRate) + static_cast<float>(value) * kCalibrationRate;
	}
}

uint8_t calculateMidiValue(PedalModel &pedal, int raw_value) {
	if (!pedal.calibration_initialized) {
		pedal.calibration_min = static_cast<float>(raw_value);
		pedal.calibration_max = static_cast<float>(raw_value);
		pedal.calibration_initialized = true;
	}

	updateCalibration(raw_value, pedal.calibration_min, pedal.calibration_max, pedal.calibration_initialized);

	float span = pedal.calibration_max - pedal.calibration_min;
	if (span < kMinSpan) {
		span = kMinSpan;
	}

	float effective_min = pedal.calibration_min - span * kCalibrationMargin;
	float effective_max = pedal.calibration_max + span * kCalibrationMargin;
	if (effective_min < 0.0f) {
		effective_min = 0.0f;
	}
	if (effective_max > static_cast<float>(kAdcMax)) {
		effective_max = static_cast<float>(kAdcMax);
	}

	const float normalized = constrain((static_cast<float>(raw_value) - effective_min) / (effective_max - effective_min), 0.0f, 1.0f);
	const float curved = organSwellCurve(normalized);
	return static_cast<uint8_t>(curved * 127.0f);
}

} // namespace

void pedal_service_init(PedalModel &pedal, uint8_t ads_channel, uint8_t midi_channel, uint8_t midi_cc) {
	pedal.enabled = true;
	pedal.adc_channel = ads_channel;
	pedal.midi_channel = midi_channel;
	pedal.midi_cc = midi_cc;
	pedal.raw_value = 0;
	pedal.filtered_value = 0;
	pedal.calibration_min = static_cast<float>(kAdcMax);
	pedal.calibration_max = 0.0f;
	pedal.calibration_initialized = false;
	pedal.midi_value = 0;
	pedal.last_midi_value = 255;
}

bool pedal_service_process(PedalModel &pedal, int raw_value, bool is_connected, PedalMidiSendFn send_fn) {
	if (!pedal.enabled) {
		return false;
	}

	pedal.raw_value = raw_value;
	pedal.filtered_value = pedal.filtered_value + (raw_value - pedal.filtered_value) / kSmoothDiv;
	pedal.midi_value = calculateMidiValue(pedal, pedal.filtered_value);

	const bool has_changed = abs(static_cast<int>(pedal.midi_value) - static_cast<int>(pedal.last_midi_value)) > kDeadBand;
	if (has_changed) {
		pedal.last_midi_value = pedal.midi_value;
		if (is_connected && send_fn != nullptr) {
			send_fn(pedal.midi_channel, pedal.midi_cc, pedal.midi_value);
		}
	}

	return has_changed;
}

uint8_t pedal_service_get_midi_value(const PedalModel &pedal) {
	return pedal.midi_value;
}
