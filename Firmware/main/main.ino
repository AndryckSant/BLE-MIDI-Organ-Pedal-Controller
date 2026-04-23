#include <BLEMidi.h>
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ===== OLED =====
#define SCREEN_W  128
#define SCREEN_H  32
#define OLED_ADDR 0x3C
Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);
// running bare-metal (no RTOS mutex)

// ===== HARDWARE CONFIGURATION =====
Adafruit_ADS1115 ads;
const int i2c_sda = 21;
const int i2c_scl = 22;
const int i2c_dready = 23;

const int LED_STATUS_PIN = 2;  // LED embutido no ESP32
const uint16_t ADC_MAX = 32767; // ADS1115 single-ended maximum

// ===== MIDI CONFIGURATION =====
const uint8_t SWELL_MIDI_CC = 11;
const uint8_t SWELL_MIDI_CHANNEL = 1;
// Pedal 2 (conectado em A1 do ADS1115)
const uint8_t PEDAL2_ADS_CHANNEL = 1;
const uint8_t PEDAL2_MIDI_CC = 11;
const uint8_t PEDAL2_MIDI_CHANNEL =1;

// ===== BLE CONFIGURATION =====
constexpr char BLE_NAME[] = "Organ Pedal";

// ===== FILTER PARAMETERS =====
constexpr uint8_t DEAD_BAND = 2;
constexpr uint8_t SMOOTH_DIV = 12;
constexpr unsigned long SAMPLE_INTERVAL_MS = 5;
constexpr float CALIBRATION_RATE = 0.001f;  // Taxa de adaptação da calibração

// ===== STATE VARIABLES =====
volatile bool isConnected = false;
unsigned long lastSampleTime = 0;

// Pedal 2 state (ADS channel A1)
int filtered2 = 0;
uint8_t midi_value2 = 0;
uint8_t last_value2 = 255; // impossible initial value to force first send

// Pedal 2 calibration
float cal_min2 = ADC_MAX;
float cal_max2 = 0.0f;
bool calibrationInitialized2 = false;
constexpr float CALIBRATION_MARGIN = 0.02f; // 2% margin when mapping

// ===== CALLBACKS BLE =====
// Ajuste: callbacks sem parâmetros (compatível com BLEMidiServer.setOnConnectCallback)
void onConnect() {
  isConnected = true;
  digitalWrite(LED_STATUS_PIN, HIGH);
  Serial.println("BLE Client connected!");
  // Envia valores atuais para sincronizar o cliente
  //sendControlChange(SWELL_MIDI_CHANNEL, SWELL_MIDI_CC, midi_value);
  sendControlChange(PEDAL2_MIDI_CHANNEL, PEDAL2_MIDI_CC, midi_value2);
  oledShowStatus(true, midi_value2);
}

void onDisconnect() {
  isConnected = false;
  digitalWrite(LED_STATUS_PIN, LOW);
  Serial.println("BLE Client disconnected.");
  oledShowStatus(false, 0);
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

// Read+filter a pedal on given ADS channel (bare-metal, no RTOS mutex)
int readAndFilterPedal(uint8_t adsChannel, int &filteredState){
  int raw = ads.readADC_SingleEnded(adsChannel);
  filteredState = filteredState + (raw - filteredState) / SMOOTH_DIV;
  return filteredState;
}

// Improved calibration updater for pedal 2 with spike/disconnect protection
void updateCalibrationFor(int value, float &cmin, float &cmax, bool &initialized) {
  const float topThreshold = ADC_MAX * 0.98f; // readings above this likely indicate open/floating to VCC
  const float lowThreshold = ADC_MAX * 0.02f;

  if (!initialized) {
    cmin = value;
    cmax = value;
    initialized = true;
    return;
  }

  // If reading is near top/full-scale, treat cautiously: update max slowly, don't lower min
  if (value >= topThreshold) {
    cmax = cmax * 0.98f + value * 0.02f; // very slow move towards top
    return;
  }
  // If reading is near very low, update min cautiously
  if (value <= lowThreshold) {
    cmin = cmin * 0.98f + value * 0.02f;
    return;
  }

  // Normal adaptive update with small smoothing
  if (value < cmin) {
    cmin = cmin * 0.9f + value * 0.1f;
  } else {
    cmin = cmin * (1.0f - CALIBRATION_RATE) + value * CALIBRATION_RATE;
  }
  if (value > cmax) {
    cmax = cmax * 0.9f + value * 0.1f;
  } else {
    cmax = cmax * (1.0f - CALIBRATION_RATE) + value * CALIBRATION_RATE;
  }
}

// Simple OLED update (bare-metal). Shows name and connection state; when connected shows pedal2 CC
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

  // Inicializa OLED e mostra estado inicial
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.clearDisplay();
  oledShowStatus(false, 0);
  
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
    // Read pedal 2 (ADS A1)
    int pedal2FilteredValue = readAndFilterPedal(PEDAL2_ADS_CHANNEL, filtered2);
    updateCalibrationFor(pedal2FilteredValue, cal_min2, cal_max2, calibrationInitialized2);

    float span2 = cal_max2 - cal_min2;
    if (span2 < 10.0f) span2 = 10.0f;

    // Apply a small margin beyond observed extrema to allow headroom
    float eff_min = cal_min2 - span2 * CALIBRATION_MARGIN;
    float eff_max = cal_max2 + span2 * CALIBRATION_MARGIN;
    if (eff_min < 0.0f) eff_min = 0.0f;
    if (eff_max > (float)ADC_MAX) eff_max = (float)ADC_MAX;

    float normalized2 = constrain((pedal2FilteredValue - eff_min) / (eff_max - eff_min), 0.0f, 1.0f);
    float curved2 = organSwellCurve(normalized2);
    midi_value2 = (uint8_t)(curved2 * 127.0f);

    if (abs((int)midi_value2 - (int)last_value2) > DEAD_BAND) {
      Serial.printf("MIDI CC%d: %d (raw: %d, cal: %.0f-%.0f)\n",
                    PEDAL2_MIDI_CC, midi_value2, pedal2FilteredValue, cal_min2, cal_max2);
      sendControlChange(PEDAL2_MIDI_CHANNEL, PEDAL2_MIDI_CC, midi_value2);
      last_value2 = midi_value2;
      oledShowStatus(true, midi_value2);
    }
  }
  
  
}
