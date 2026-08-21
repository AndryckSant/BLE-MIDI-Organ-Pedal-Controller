/******************************************************************************
 *
 * @file    config_service.h
 * @author  Andryck Santiago
 * @brief   System configuration service public interface.
 *
 * @details
 * Provides functions for initializing, accessing, validating, saving,
 * and restoring the system configuration.
 *
 * The service is responsible for managing system configuration and
 * persisting it in non-volatile memory.
 *
 ******************************************************************************/

#ifndef CONFIG_SERVICE_H
#define CONFIG_SERVICE_H

#include "../../models/system_config.h"

/**
 * @brief Initializes the configuration service.
 *
 * Loads the stored configuration from non-volatile memory.
 * Factory defaults are used if no valid configuration is found.
 */
void config_service_init();

/**
 * @brief Processes configuration-related tasks.
 */
void config_service_process();

/**
 * @brief Returns the current system configuration.
 *
 * @return Pointer to the current configuration.
 */
const SystemConfig *config_service_get();

/**
 * @brief Returns a mutable reference to the current configuration.
 *
 * Used by configuration interfaces such as the WebUI.
 *
 * @return Reference to the current configuration.
 */
SystemConfig &config_service_get_mutable();

/**
 * @brief Saves the current configuration to non-volatile memory.
 *
 * @return true if the configuration was saved successfully.
 * @return false otherwise.
 */
bool config_service_save();

/**
 * @brief Restores and saves the factory-default configuration.
 *
 * @return true if the reset was successful.
 * @return false otherwise.
 */
bool config_service_factory_reset();

/**
 * @brief Validates the current system configuration.
 *
 * @return true if the configuration is valid.
 * @return false otherwise.
 */
bool config_service_validate();

#endif // CONFIG_SERVICE_H