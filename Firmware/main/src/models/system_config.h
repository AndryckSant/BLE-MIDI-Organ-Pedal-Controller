/******************************************************************************
 * @file    system_config.h
 * @author  Andryck Santiago
 * @brief   System Configuration Model.
 *
 * @details
 * Defines the complete configuration and runtime data model of the
 * BLE MIDI controller.
 *
 * The system configuration contains the configuration and state of
 * all supported pedals and buttons.
 ******************************************************************************/

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "button_model.h"
#include "pedal_model.h"

/**
 * @brief Maximum number of configurable pedals.
 */
constexpr uint8_t kMaxPedals = 2;

/**
 * @brief Maximum number of configurable buttons.
 */
constexpr uint8_t kMaxButtons = 8;

/**
 * @brief Represents the complete system configuration.
 */

struct SystemConfig {
    PedalModel pedals[kMaxPedals]; // Configuration for Pedal 1 and Pedal 2
    ButtonModel buttons[kMaxButtons]; // Configuration for up to 8 buttons
};

#endif // SYSTEM_CONFIG_H