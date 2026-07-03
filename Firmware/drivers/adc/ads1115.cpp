#include "ads1115.h"

#include <Wire.h>
#include <Adafruit_ADS1X15.h>

namespace {

constexpr uint8_t kI2cSda = 21;
constexpr uint8_t kI2cScl = 22;

Adafruit_ADS1115 g_ads;
bool g_initialized = false;

} // namespace

void adc_init() {
	if (g_initialized) {
		return;
	}

	Wire.begin(kI2cSda, kI2cScl);
	g_ads.setGain(GAIN_ONE);
	g_ads.begin();
	g_initialized = true;
}

int16_t adc_read(uint8_t channel) {
	if (!g_initialized) {
		adc_init();
	}

	return g_ads.readADC_SingleEnded(channel);
}
