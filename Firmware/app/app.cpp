#include "app.h"

#include <Arduino.h>
#include <BLEMidi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../core/fsm/fsm.h"
#include "../core/states/ble_connected.h"
#include "../core/states/ble_disconnected.h"
#include "../core/states/config.h"
#include "../core/states/state_actions.h"
#include "../drivers/adc/ads1115.h"
#include "../services/config_service/config_service.h"
#include "../services/input/pedal/pedal_service.h"

namespace {

constexpr char BLE_NAME[] = "BLE MIDI Controller";
constexpr int SCREEN_W = 128;
constexpr int SCREEN_H = 32;
constexpr int OLED_ADDR = 0x3C;
constexpr int LED_STATUS_PIN = 2;
constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;
constexpr unsigned long SAMPLE_INTERVAL_MS = 5;

Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);
PedalModel pedal1;
unsigned long lastSampleTime = 0;
FsmContext fsmContext;
FsmState lastState = FsmState::Boot;
StateActions stateActions{};
uint8_t displayMdiCc = 11;

void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    if (fsm_get_state(fsmContext) == FsmState::ReadAndSend || fsm_get_state(fsmContext) == FsmState::BleConnected) {
        BLEMidiServer.controlChange(channel, cc, value);
    }
}

void setLed(bool on) {
    digitalWrite(LED_STATUS_PIN, on ? HIGH : LOW);
}

void showStatus(bool connected, uint8_t ccValue) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);

    oled.setCursor(0, 0);
    oled.print(BLE_NAME);

    oled.setCursor(0, 12);
    oled.print("BLE: ");
    oled.print(connected ? "Conectado" : "Aguardando...");

    if (connected) {
        oled.setCursor(0, 24);
        oled.printf("P1 CC%d: %d", displayMdiCc, ccValue);
    }

    oled.display();
}

void logMessage(const char *message) {
    Serial.println(message);
}

void onConnect() {
    fsm_post(fsmContext, FsmEvent::BleConnected);
    fsm_post(fsmContext, FsmEvent::StartStream);
}

void onDisconnect() {
    fsm_post(fsmContext, FsmEvent::StopStream);
    fsm_post(fsmContext, FsmEvent::BleDisconnected);
}

void processSerialCommands() {
    const FsmState currentState = fsm_get_state(fsmContext);
    while (Serial.available() > 0) {
        const int value = Serial.read();
        if ((value == 'c' || value == 'C') && (currentState == FsmState::BleConnected || currentState == FsmState::ReadAndSend)) {
            fsm_post(fsmContext, FsmEvent::EnterConfig);
        } else if (value == 'x' || value == 'X') {
            fsm_post(fsmContext, FsmEvent::ExitConfig);
        }
    }
}

} // namespace

void app_init() {
    Serial.begin(115200);
    Serial.println("Starting BLE MIDI Device");

    BLEMidiServer.begin(BLE_NAME);
    BLEMidiServer.enableDebugging();
    BLEMidiServer.setOnConnectCallback(onConnect);
    BLEMidiServer.setOnDisconnectCallback(onDisconnect);

    stateActions.set_led = setLed;
    stateActions.show_status = showStatus;
    stateActions.log_message = logMessage;
    stateActions.send_control_change = sendControlChange;

    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, LOW);

    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    adc_init();

    config_service_init();

    const SystemConfig &config = config_service_get();
    const PedalModel &pedalConfig = config.pedals[0];

    pedal_service_init(pedal1, pedalConfig.adc_channel, pedalConfig.midi_channel, pedalConfig.midi_cc);
    pedal1.enabled = pedalConfig.enabled;
    pedal1.calibration_min = pedalConfig.calibration_min;
    pedal1.calibration_max = pedalConfig.calibration_max;
    pedal1.calibration_initialized = pedalConfig.calibration_initialized;
    displayMdiCc = pedal1.midi_cc;

    fsm_init(fsmContext);
    lastState = fsm_get_state(fsmContext);

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(250);
        digitalWrite(LED_STATUS_PIN, LOW);
        delay(250);
    }

    oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    oled.clearDisplay();
    showStatus(false, 0);

    Serial.println("waiting for connections...");
}

void app_run() {
    processSerialCommands();
    fsm_run(fsmContext);

    const FsmState currentState = fsm_get_state(fsmContext);
    if (currentState != lastState) {
        if (lastState == FsmState::Config && currentState != FsmState::Config) {
            config_mode_exit();
        }

        if (currentState == FsmState::BleDisconnected) {
            ble_disconnected_enter(stateActions);
        } else if (currentState == FsmState::BleConnected || currentState == FsmState::ReadAndSend) {
            ble_connected_enter(stateActions, pedal1.midi_channel, pedal1.midi_cc, pedal_service_get_midi_value(pedal1));
        } else if (currentState == FsmState::Config) {
            config_mode_enter();
        }
        lastState = currentState;
    }

    if (currentState == FsmState::Config) {
        config_mode_run();
        if (config_mode_should_exit()) {
            fsm_post(fsmContext, FsmEvent::ExitConfig);
        }
        return;
    }

    if (currentState != FsmState::ReadAndSend) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
        delay(1);
        return;
    }
    lastSampleTime = currentTime;

    int pedal1RawValue = adc_read(pedal1.adc_channel);
    if (pedal_service_process(pedal1, pedal1RawValue, true, sendControlChange)) {
        Serial.printf("MIDI CC%d: %d (raw: %d, filtered: %d)\n",
                      pedal1.midi_cc,
                      pedal_service_get_midi_value(pedal1),
                      pedal1.raw_value,
                      pedal1.filtered_value);
        showStatus(true, pedal_service_get_midi_value(pedal1));
    }
}