/******************************************************************************
 * @file    nvs_driver.cpp
 * @author  Andryck Santiago
 * @brief   ESP32 NVS Driver.
 *
 * @details
 * This module encapsulates the ESP32 Preferences library and provides
 * a simple interface for persistent storage operations.
 *
 * This module is responsible only for low-level non-volatile storage.
 * Data structures, configuration logic, and application policies are
 * handled by higher layers of the application.
 ******************************************************************************/

#include "nvs_driver.h"

#include <Preferences.h>

namespace {
	//=========================================================================
    // Private Constants
    //=========================================================================

    /// NVS namespace used by the application.
    constexpr char kNamespace[] = "ble_midi";

    //=========================================================================
    // Private Variables
    //=========================================================================

    Preferences preferences;

    bool isInitialized = false;

    //=========================================================================
    // Private Function Prototypes
    //=========================================================================

    bool ensureOpen();
}

//=============================================================================
// Public Functions
//=============================================================================

bool nvs_init() {
	return ensure_open();
}

bool nvs_read_blob(const char *key, void *data, size_t length) {
	if (!ensure_open()) {
		return false;
	}

	return preferences.getBytes(key, data, length) == length;
}

bool nvs_write_blob(const char *key, const void *data, size_t length) {
	if (!ensure_open()) {
		return false;
	}

	return preferences.putBytes(key, data, length) == length;
}

bool nvs_delete_key(const char *key) {
	if (!ensure_open()) {
		return false;
	}

	return preferences.remove(key);
}

bool nvs_clear() {
	if (!ensure_open()) {
		return false;
	}

	return preferences.clear();
}

//=============================================================================
// Private Functions
//=============================================================================

/**
 * @brief Ensures that the NVS namespace is open.
 *
 * Opens the application NVS namespace if it has not been initialized yet.
 *
 * @return true if the namespace is open.
 * @return false if the namespace could not be opened.
 */
bool ensureOpen()
{
    if(isInitialized){
        return true;
    }

    isInitialized = preferences.begin(kNamespace, false);

    return isInitialized;
}