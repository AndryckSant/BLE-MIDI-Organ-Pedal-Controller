/******************************************************************************
 * @file    nvs_driver.h
 * @author  Andryck Santiago
 * @brief   ESP32 NVS Driver public interface.
 *
 * @details
 * Provides the public API for initializing and accessing persistent
 * non-volatile storage.
 *
 * This driver abstracts the ESP32 Preferences library and exposes
 * generic blob-based read and write operations.
 ******************************************************************************/

#ifndef NVS_DRIVER_H
#define NVS_DRIVER_H

#include <stddef.h>



/**
 * @brief Initializes the NVS driver.
 *
 * @return true if initialization succeeds.
 * @return false otherwise.
 */
bool nvs_init();

/**
 * @brief Reads a binary data block from NVS.
 *
 * @param key Storage key.
 * @param data Destination buffer.
 * @param length Number of bytes to read.
 *
 * @return true if the requested data was read successfully.
 * @return false otherwise.
 */
bool nvs_read_blob(const char *key, void *data, size_t length);

/**
 * @brief Writes a binary data block to NVS.
 *
 * @param key Storage key.
 * @param data Source buffer.
 * @param length Number of bytes to write.
 *
 * @return true if the data was written successfully.
 * @return false otherwise.
 */
bool nvs_write_blob(const char *key, const void *data, size_t length);

/**
 * @brief Deletes a key from NVS.
 *
 * @param key Storage key.
 *
 * @return true if the key was removed successfully.
 * @return false otherwise.
 */
bool nvs_delete_key(const char *key);

/**
 * @brief Clears all keys from the application NVS namespace.
 *
 * @return true if the namespace was cleared successfully.
 * @return false otherwise.
 */
bool nvs_clear();

#endif // NVS_DRIVER_H