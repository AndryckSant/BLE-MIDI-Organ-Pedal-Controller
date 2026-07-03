#include "app.h"

#include <Arduino.h>
#include <BLEMidi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "../drivers/adc/ads1115.h"
#include "../services/input/pedal/pedal_service.h"

namespace {

constexpr char BLE_NAME[] = "BLE MIDI Controller";
constexpr int SCREEN_W = 128;
constexpr int SCREEN_H = 32;
constexpr int OLED_ADDR = 0x3C;
constexpr int LED_STATUS_PIN = 2;
constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;
constexpr uint8_t PEDAL2_ADS_CHANNEL = 1;
constexpr uint8_t PEDAL2_MIDI_CC = 11;
constexpr uint8_t PEDAL2_MIDI_CHANNEL = 1;
constexpr unsigned long SAMPLE_INTERVAL_MS = 5;

Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);
PedalModel pedal2;
volatile bool isConnected = false;
unsigned long lastSampleTime = 0;

void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
    if (isConnected) {
        BLEMidiServer.controlChange(channel, cc, value);
    }
}

void oledShowStatus(bool connected, uint8_t ccValue) {
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
        oled.printf("CC%d: %d", PEDAL2_MIDI_CC, ccValue);
    }

    oled.display();
}

void onConnect() {
    isConnected = true;
    digitalWrite(LED_STATUS_PIN, HIGH);
    Serial.println("BLE Client connected!");
    sendControlChange(pedal2.midi_channel, pedal2.midi_cc, pedal_service_get_midi_value(pedal2));
    oledShowStatus(true, pedal_service_get_midi_value(pedal2));
}

void onDisconnect() {
    isConnected = false;
    digitalWrite(LED_STATUS_PIN, LOW);
    Serial.println("BLE Client disconnected.");
    oledShowStatus(false, 0);
}

} // namespace

void app_init() {
    Serial.begin(115200);
    Serial.println("Starting BLE MIDI Device");

    BLEMidiServer.begin(BLE_NAME);
    BLEMidiServer.enableDebugging();
    BLEMidiServer.setOnConnectCallback(onConnect);
    BLEMidiServer.setOnDisconnectCallback(onDisconnect);

    pinMode(LED_STATUS_PIN, OUTPUT);
    digitalWrite(LED_STATUS_PIN, LOW);

    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    adc_init();

    pedal_service_init(pedal2, PEDAL2_ADS_CHANNEL, PEDAL2_MIDI_CHANNEL, PEDAL2_MIDI_CC);

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_STATUS_PIN, HIGH);
        delay(250);
        digitalWrite(LED_STATUS_PIN, LOW);
        delay(250);
    }

    oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    oled.clearDisplay();
    oledShowStatus(false, 0);

    Serial.println("waiting for connections...");
}

void app_run() {
    unsigned long currentTime = millis();
    if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
        delay(1);
        return;
    }
    lastSampleTime = currentTime;

    int pedal2RawValue = adc_read(PEDAL2_ADS_CHANNEL);
    if (pedal_service_process(pedal2, pedal2RawValue, isConnected, sendControlChange)) {
        Serial.printf("MIDI CC%d: %d (raw: %d, filtered: %d)\n",
                      pedal2.midi_cc,
                      pedal_service_get_midi_value(pedal2),
                      pedal2.raw_value,
                      pedal2.filtered_value);
        if (isConnected) {
            oledShowStatus(true, pedal_service_get_midi_value(pedal2));
        }
    }
}