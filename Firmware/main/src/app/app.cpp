/**
 * @file    app.cpp
 * @author  Andryck Santiago
 * @brief   Application coordinator.
 *
 * @details
 * This module coordinates the application services and finite state machine.
 *
 * The application layer is responsible for initializing the services,
 * running the finite state machine, processing application inputs, and
 * coordinating communication between the services.
 */

#include "app.h"

#include <Arduino.h>

#include "../core/fsm/fsm.h"
#include "../core/states/ble_connected.h"
#include "../core/states/ble_disconnected.h"
#include "../core/states/config.h"
#include "../core/states/state_actions.h"

#include "../drivers/adc/ads1115.h"
#include "../drivers/ble/ble_driver.h"

#include "../services/config_service/config_service.h"
#include "../services/display_service/display_service.h"
#include "../services/input/pedal_service/pedal_service.h"
#include "../services/midi_service/midi_service.h"

namespace
{

//=============================================================================
// Private Constants
//=============================================================================

/**
 * @brief GPIO used by the dedicated configuration button.
 */
constexpr uint8_t kConfigButtonPin = 34U;

/**
 * @brief Number of pedals supported by the hardware.
 */
constexpr uint8_t kPedalCount = 2U;

/**
 * @brief Pedal sampling interval in milliseconds.
 */
constexpr unsigned long kSampleIntervalMs = 5UL;

/**
 * @brief GPIO used by the status LED.
 */
constexpr uint8_t kStatusLedPin = 2U;

//=============================================================================
// Private Variables
//=============================================================================

bool lastBleConnectedState = false;

/**
 * @brief Previous state of the configuration button.
 */
bool configButtonLastState = false;

/**
 * @brief Application finite state machine context.
 */
FsmContext fsmContext;

/**
 * @brief Callbacks provided to the FSM states.
 */
StateActions stateActions;

/**
 * @brief Previous FSM state.
 */
FsmState lastState = FsmState::Boot;

/**
 * @brief Pointer to the system configuration.
 *
 * This pointer is used for read-only access to the current configuration.
 */
const SystemConfig *systemConfig = nullptr;

/**
 * @brief Pointer to the mutable pedal configuration array.
 *
 * The pedal service modifies runtime fields such as raw_value,
 * filtered_value, midi_value, and last_midi_value.
 */
PedalModel *pedals = nullptr;

/**
 * @brief Timestamp of the previous pedal processing cycle.
 */
unsigned long lastSampleTime = 0UL;

//=============================================================================
// Private Function Prototypes
//=============================================================================

/**
 * @brief Processes the dedicated configuration button.
 */
void processConfigButton();

/**
 * @brief Processes configuration commands received through Serial.
 */
void processSerialCommands();

/**
 * @brief Sets the application status LED.
 *
 * @param on LED state.
 */
void setStatusLed(bool on);

/**
 * @brief Updates the display BLE status.
 *
 * @param connected BLE connection state.
 * @param midiValue Initial MIDI value associated with the connection.
 */
void showStatus(
    bool connected,
    uint8_t midiValue);

/**
 * @brief Logs a message to the serial interface.
 *
 * @param message Message to log.
 */
void logMessage(const char *message);

/**
 * @brief Sends a MIDI Control Change message.
 *
 * @param channel MIDI channel.
 * @param cc MIDI Control Change number.
 * @param value MIDI value.
 */
void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    midi_send_control_change(channel, cc, value);
}

/**
 * @brief Handles an FSM state transition.
 *
 * @param currentState New FSM state.
 */
void handleStateChange(FsmState currentState);

/**
 * @brief Processes the configured pedals.
 */
void processPedals();

//=============================================================================
// Private Functions
//=============================================================================


    void showStatus(
        bool connected,
        uint8_t midiValue)
    {
        /*
        * The display service currently only receives the BLE
        * connection state. The initial MIDI value is handled
        * by the BLE-connected state through the MIDI callback.
        */
        (void)midiValue;
        Serial.print("showStatus(): ");
        Serial.println(connected ? "CONNECTED" : "DISCONNECTED");
        display_show_ble_status(connected);
    }

    void logMessage(const char *message)
    {
        if (message == nullptr)
        {
            return;
        }

        Serial.println(message);
    }

    void handleStateChange(FsmState currentState)
    {
        /*
        * Leave configuration mode before entering another state.
        */
        if (lastState == FsmState::Config &&
            currentState != FsmState::Config)
        {
            config_mode_exit();
        }

        switch (currentState)
        {
            case FsmState::BleDisconnected:
                fsm_post(fsmContext, FsmEvent::StopStream);
                delay(1000);
                ble_disconnected_enter(
                    stateActions);

                break;

            case FsmState::BleConnected:

                if (pedals == nullptr)
                {
                    return;
                }
                ble_connected_enter(
                    stateActions,
                    pedals[0].midi_channel,
                    pedals[0].midi_cc,
                    0);
                delay(2000);
                fsm_post(fsmContext, FsmEvent::StartStream);


                break;

            case FsmState::ReadAndSend:

                if (pedals == nullptr)
                {
                    return;
                }

                /*
                * The BLE connection is already established.
                * The pedal service takes responsibility for
                * generating subsequent MIDI values.
                */
                break;

            case FsmState::Config:
                fsm_post(fsmContext, FsmEvent::StopStream);
                delay(1000);
                config_mode_enter(stateActions);

                break;

            case FsmState::Boot:
            case FsmState::Error:
            default:

                break;
        }
    }

    void processPedals()
    {
        if (pedals == nullptr)
        {
            return;
        }

        const unsigned long currentTime = millis();

        if (currentTime - lastSampleTime < kSampleIntervalMs)
        {
            return;
        }

        lastSampleTime = currentTime;

        /*
        * The pedal service handles:
        *
        * - ADC acquisition
        * - filtering
        * - calibration
        * - normalization
        * - MIDI value generation
        */
        pedal_process();

        for (uint8_t index = 0U;
            index < kPedalCount;
            ++index)
        {
            if (!pedal_has_new_value(index))
            {
                continue;
            }

            PedalModel &pedal = pedals[index];

            /*
            * Send the newly generated MIDI value through
            * the application MIDI callback.
            */
            sendControlChange(
                pedal.midi_channel,
                pedal.midi_cc,
                pedal.midi_value);

            /*
            * Mark the value as transmitted.
            */
            pedal.last_midi_value =
                pedal.midi_value;

            /*
            * Update the display with the transmitted value.
            */
            display_show_pedal(
                index,
                pedal.midi_value);

            /*
            * Log the transmitted value for debugging.
            */
            Serial.printf(
                "Pedal %u - MIDI CC%d: %d (raw: %d, filtered: %d)\n",
                index + 1U,
                pedal.midi_cc,
                pedal.midi_value,
                pedal.raw_value,
                pedal.filtered_value);
        }
    }

    void processSerialCommands()
    {
        const FsmState currentState =
            fsm_get_state(fsmContext);

        while (Serial.available() > 0)
        {
            const int value = Serial.read();

            /*
            * 'C' enters configuration mode.
            */
            if ((value == 'c' || value == 'C'))
            {

                Serial.println("Posting EnterConfig");

                fsm_post(
                    fsmContext,
                    FsmEvent::EnterConfig);

                continue;
            }

            /*
            * 'X' requests configuration mode exit.
            */
            if (value == 'x' || value == 'X')
            {
                 Serial.println("Posting ExitConfig");

                fsm_post(
                    fsmContext,
                    FsmEvent::ExitConfig);
            }
        }
    }

    void processConfigButton()
    {
        const bool currentButtonState =
            digitalRead(kConfigButtonPin) == HIGH;

        /*
        * Detect rising edge.
        */
        if (currentButtonState && !configButtonLastState)
        {
            const FsmState currentState =
                fsm_get_state(fsmContext);

            if (currentState == FsmState::BleConnected ||
                currentState == FsmState::ReadAndSend || currentState == FsmState::BleDisconnected)
            {
                fsm_post(
                    fsmContext,
                    FsmEvent::EnterConfig);
            }
        }

        configButtonLastState = currentButtonState;
    }

    void setStatusLed(bool on)
    {
        digitalWrite(
            kStatusLedPin,
            on ? HIGH : LOW);
    }

    void observeBleConnectionState(){
        const bool currentBleConnectedState = ble_is_connected();

        if (currentBleConnectedState == lastBleConnectedState){
            return;
        }

        lastBleConnectedState = currentBleConnectedState;

        fsm_post(fsmContext, currentBleConnectedState ? FsmEvent::BleConnected : FsmEvent::BleDisconnected);
    }

    void observeConfigButtonState(){
        if (digitalRead(kConfigButtonPin) == HIGH && !configButtonLastState){
            fsm_post(fsmContext, FsmEvent::EnterConfig);
        }   
    }

} // namespace

//=============================================================================
// Public Functions
//=============================================================================

void app_init()
{
    Serial.begin(115200);

    Serial.println("Starting BLE MIDI Device");

    /*
     * Initialize the status LED.
     */
    pinMode(kStatusLedPin, OUTPUT);
    setStatusLed(false);

    /*
     * Initialize the configuration button.
     */
    pinMode(kConfigButtonPin, INPUT);

    /*
     * Initialize the ADC driver.
     */
    if (ads1115_init() != ADS1115_OK)
    {
        Serial.println("ADS1115 initialization failed.");
    }

    /*
     * Initialize the MIDI service.
     *
     * The MIDI service initializes the BLE MIDI driver.
     */
    midi_init();

    /*
     * Initialize the display service.
     */
    if (!display_init())
    {
        Serial.println("Display initialization failed.");
    }

    /*
     * Initialize and load the system configuration.
     */
    config_service_init();

    /*
     * Obtain read-only access to the configuration.
     */
    systemConfig = config_service_get();

    if (systemConfig == nullptr)
    {
        Serial.println("Configuration unavailable.");

        return;
    }

    /*
     * Obtain mutable access to the pedal configuration.
     *
     * The pedal service updates runtime fields in PedalModel,
     * therefore a mutable pointer is required here.
     */
    pedals = config_service_get_mutable().pedals;

    if (pedals == nullptr)
    {
        Serial.println("Pedal configuration unavailable.");

        return;
    }

    /*
     * Initialize the pedal service using the configured pedal models.
     */
    pedal_init(
        pedals,
        kPedalCount);

    /*
     * Configure the callbacks used by the FSM states.
     */
    stateActions.set_led = setStatusLed;
    stateActions.show_status = showStatus;
    stateActions.log_message = logMessage;
    stateActions.send_control_change = sendControlChange;
    stateActions.display_show_config = display_show_config;

    /*
     * Initialize the finite state machine.
     */
    fsm_init(fsmContext);

    lastState = fsm_get_state(fsmContext);

    /*
     * Show the initial application screen.
     */
    display_show_boot();
    
    Serial.println("Waiting for BLE connection...");
    delay(1000);
}

void app_run()
{
    observeBleConnectionState();
    processSerialCommands();
    processConfigButton();

    /*
     * Run the finite state machine.
     */
    fsm_run(fsmContext);

    const FsmState currentState =
        fsm_get_state(fsmContext);

    /*
     * Process state transitions.
     */
    if (currentState != lastState)
    {
        handleStateChange(currentState);

        lastState = currentState;
    }

    /*
     * Configuration mode owns the application while active.
     */
    if (currentState == FsmState::Config)
    {
        config_mode_run();

        if (config_mode_should_exit())
        {
            config_mode_exit();

            ESP.restart();
        }

        return;
    }

    /*
     * Pedals are processed only while the FSM is in the
     * normal MIDI transmission state.
     */
    if (currentState == FsmState::ReadAndSend)
    {
        processPedals();
    }

}