/******************************************************************************
 *
 * @file    config.h
 * @author  Andryck Santiago
 * @brief   Configuration mode state interface.
 *
 * @details
 * Provides the public interface for the configuration mode.
 *
 * Configuration mode disables BLE MIDI, starts the ESP32 as a Wi-Fi
 * Access Point, and provides the WebUI used to modify the system
 * configuration.
 *
 ******************************************************************************/

#ifndef CONFIG_STATE_H
#define CONFIG_STATE_H

#include <stdbool.h>

#include "state_actions.h"

/**
 * @brief Enters configuration mode.
 */
void config_mode_enter(const StateActions &actions);

/**
 * @brief Processes configuration mode.
 */
void config_mode_run();

/**
 * @brief Exits configuration mode.
 */
void config_mode_exit();

/**
 * @brief Returns whether configuration mode requested an exit.
 *
 * @return true if configuration mode should be exited.
 * @return false otherwise.
 */
bool config_mode_should_exit();

/**
 * @brief Returns whether configuration mode is active.
 *
 * @return true if configuration mode is active.
 * @return false otherwise.
 */
bool config_mode_is_active();

#endif // CONFIG_STATE_H