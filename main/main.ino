#include <BLEMidi.h>
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>


// ===== HARDWARE CONFIGURATION =====
Adafruit_ADS1115 ads;
const int pedal1_signal = 4;
const int pedal2_signal = 15;
const int i2c_sda = 21;
const int i2c_scl = 22;
const int i2c_dready = 23;

const int SWELL_PEDAL_PIN = 34;
const int LED_STATUS_PIN = 2;  // LED embutido no ESP32
const uint16_t ADC_MAX = 65535;

// ===== MIDI CONFIGURATION =====
const uint8_t SWELL_MIDI_CC = 11;
const uint8_t SWELL_MIDI_CHANNEL = 0;

// ===== BLE CONFIGURATION =====
constexpr char BLE_NAME[] = "Organ Pedal";

// ===== FILTER PARAMETERS =====
constexpr uint8_t DEAD_BAND = 2;
constexpr uint8_t SMOOTH_DIV = 12;
constexpr unsigned long SAMPLE_INTERVAL_MS = 5;
constexpr float CALIBRATION_RATE = 0.001f;  // Taxa de adaptação da calibração

// ===== STATE VARIABLES =====
uint8_t midi_value = 0;
volatile bool isConnected = false;
uint8_t last_value = 255;  // Valor impossível para forçar primeiro envio
int filtered = 0;
unsigned long lastSampleTime = 0;

// ===== CALIBRATION =====
float cal_min = ADC_MAX;
float cal_max = 0.0f;
bool calibrationInitialized = false;

// ===== CALLBACKS BLE =====
// Ajuste: callbacks sem parâmetros (compatível com BLEMidiServer.setOnConnectCallback)
void onConnect() {
  isConnected = true;
  digitalWrite(LED_STATUS_PIN, HIGH);
  Serial.println("BLE Client connected!");
}

void onDisconnect() {
  isConnected = false;
  digitalWrite(LED_STATUS_PIN, LOW);
  Serial.println("BLE Client disconnected.");
}

// ===== PROCESSING FUNCTIONS =====
float organSwellCurve(float x) {
  const float gamma = 0.45f;
  return powf(x, gamma);
}

void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
  if (isConnected) {
    BLEMidiServer.controlChange(channel, cc, value);
  }
}

int readAndFilterSwellPedal(){
  int raw = ads.readADC_SingleEnded(0);
  filtered = filtered + (raw - filtered) / SMOOTH_DIV;
  return filtered;
}

void updateCalibration(int value) {
  if (!calibrationInitialized) {
    cal_min = value;
    cal_max = value;
    calibrationInitialized = true;
    return;
  }
  
  if (value < cal_min) {
    cal_min = cal_min * 0.9f + value * 0.1f; 
  } else {
    cal_min = cal_min * (1.0f - CALIBRATION_RATE) + value * CALIBRATION_RATE;  
  }
  
  if (value > cal_max) {
    cal_max = cal_max * 0.9f + value * 0.1f; 
  } else {
    cal_max = cal_max * (1.0f - CALIBRATION_RATE) + value * CALIBRATION_RATE;  
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE MIDI Device");
  BLEMidiServer.begin(BLE_NAME);
  BLEMidiServer.enableDebugging();
  BLEMidiServer.setOnConnectCallback(onConnect);
  BLEMidiServer.setOnDisconnectCallback(onDisconnect);
  
  // Configurações do hardware
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);

  Wire.begin(i2c_sda, i2c_scl);
  delay(100);
  // Configura ADC
  ads.setGain(GAIN_ONE);

  //Diagnóstico de localização de dispositivos...
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
  if(!ads.begin()){
    Serial.println("ADS1115 not found!");
    while (1);
  }
  delay(100);
  
  // Pisca LED para indicar que está pronto
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(250);
    digitalWrite(LED_STATUS_PIN, LOW);
    delay(250);
  }
  
  Serial.println("waiting for connections...");
}

void loop() {

  unsigned long currentTime = millis();
  if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
    delay(1); // permite tarefas do sistema/BLE
    return;
  }
  lastSampleTime = currentTime;
  
  
  if (isConnected) {
    int swellFilteredValue = readAndFilterSwellPedal();
    updateCalibration(swellFilteredValue);
    
    
    float span = cal_max - cal_min;
    if (span < 10.0f) span = 10.0f;  
    
    
    float normalized = constrain((swellFilteredValue - cal_min) / span, 0.0f, 1.0f);
    
    
    float curved = organSwellCurve(normalized);
    

    midi_value = (uint8_t)(curved * 127.0f);
    
    if (abs((int)midi_value - (int)last_value) > DEAD_BAND) {
      Serial.printf("MIDI CC%d: %d (raw: %d, cal: %.0f-%.0f)\n", 
                    SWELL_MIDI_CC, midi_value, swellFilteredValue, cal_min, cal_max);
      sendControlChange(SWELL_MIDI_CHANNEL, SWELL_MIDI_CC, midi_value);
      last_value = midi_value;
    }
  }
  
  
}
