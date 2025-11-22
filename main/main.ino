#include <BLEMidi.h>
#include <Arduino.h>

uint8_t midi_value = 127;
int pedal_pin = 34;
int last_value = -1, filtered;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE MIDI Device");
  BLEMidiServer.begin("Basic MIDI Device");
  Serial.println("waiting for connections...");
  BLEMidiServer.enableDebugging();
  filtered = analogRead(pedal_pin);
  // put your setup code here, to run once:

}

void loop() {
  bool isConneceted;
  isConneceted = BLEMidiServer.isConnected();
  if(isConneceted){
    int raw = analogRead(pedal_pin);
    filtered = filtered + (raw - filtered) / 16;
    midi_value = (filtered*127)/4095;

    // deadband
    if(abs(midi_value - last_value) > 2){

    Serial.println(midi_value);
    BLEMidiServer.controlChange(0, 11, midi_value);
    last_value = midi_value;
    }
    // Serial.println(midi_value);
    delay(5);
  }
}
