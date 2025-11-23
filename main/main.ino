#include <BLEMidi.h>
#include <Arduino.h>


const int SWELL_PEDAL_PIN = 34;
const uint16_t ADC_MAX = 4095;

const uint8_t SWELL_MIDI_CC = 11;
const uint8_t SWELL_MIDI_CHANNEL = 0;

constexpr char BLE_NAME[] = "Organ Pedal";

constexpr uint8_t DEAD_BAND = 2;
constexpr uint8_t SMOOTH_DIV = 12;
constexpr unsigned long SAMPLE_INTERVAL_MS = 5;

uint8_t midi_value = 0;
bool isConnected = false;
int last_value = -1; 
int filtered = 0;

float cal_min = ADC_MAX;
float cal_max = 0.0f;

float organSwellCurve(float x){
  const float gamma = 0.45f;
  return powf(x, gamma);
}

void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value){
  BLEMidiServer.controlChange(channel, cc, value);
}

int readAndFilterSwellPedal(){
  int raw = analogRead(SWELL_PEDAL_PIN);
  filtered = filtered + (raw - filtered) / SMOOTH_DIV;
  return filtered;
}

void updateCalibration(int value){
  if(value < cal_min) cal_min = cal_min * 0.99f + value * 0.01f;
  if(value > cal_max) cal_max = cal_max * 0.99f + value * 0.01f;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE MIDI Device");
  BLEMidiServer.begin(BLE_NAME);
  Serial.println("waiting for connections...");
  BLEMidiServer.enableDebugging();
  filtered = analogRead(SWELL_PEDAL_PIN);

}

void loop() {
  isConnected = BLEMidiServer.isConnected();
  if(isConnected){
    int SwellFilteredValue = readAndFilterSwellPedal();
    updateCalibration(SwellFilteredValue);
    
    float span = cal_max - cal_min;
    if(span < 10.0f) span = 10.0f;

    float x = (filtered - cal_min) / span;
    if(x < 0.0f) x = 0.0f;
    if(x > 1.0f) x = 1.0f;

    float curved = organSwellCurve(x);

    midi_value = (uint8_t)(curved*127.0f);

    if(abs(midi_value - last_value) > DEAD_BAND){
      Serial.println(midi_value);
      sendControlChange(SWELL_MIDI_CHANNEL, SWELL_MIDI_CC, midi_value);
      last_value = midi_value;
    }
    delay(SAMPLE_INTERVAL_MS);
  }
}
