#include "nvs_driver.h"

#include <Preferences.h>

namespace {

constexpr char kNamespace[] = "ble_midi";

Preferences g_preferences;
bool g_initialized = false;

bool ensure_open() {
	if (g_initialized) {
		return true;
	}

	g_initialized = g_preferences.begin(kNamespace, false);
	return g_initialized;
}

} // namespace

bool nvs_init() {
	return ensure_open();
}

bool nvs_read_blob(const char *key, void *data, size_t length) {
	if (!ensure_open()) {
		return false;
	}

	return g_preferences.getBytes(key, data, length) == length;
}

bool nvs_write_blob(const char *key, const void *data, size_t length) {
	if (!ensure_open()) {
		return false;
	}

	return g_preferences.putBytes(key, data, length) == length;
}

bool nvs_delete_key(const char *key) {
	if (!ensure_open()) {
		return false;
	}

	return g_preferences.remove(key);
}

bool nvs_clear() {
	if (!ensure_open()) {
		return false;
	}

	return g_preferences.clear();
}
