# BLE-MIDI-Organ-Pedal-Controller

Small BLE MIDI expression pedal project for ESP32-like boards. Reads an analog pedal, applies smoothing and adaptive calibration, maps the result to a MIDI CC value (0–127) and sends it over BLE using BLEMidi.

Current state
- Working prototype inside `main/main.ino`.
- Reads analog pin 34, smooths with a simple IIR-like filter, performs slow adaptive calibration (min/max), applies a curvature function, maps to MIDI 0–127 and sends Control Change messages over BLE.
- Uses a blocking sampling loop with delay(SAMPLE_INTERVAL_MS). Plans to replace with non-blocking timing later.

Key features implemented
- Analog read and smoothing (SMOOTH_DIV).
- Adaptive calibration of min/max values (cal_min, cal_max).
- Non-linear response curve (organSwellCurve with gamma 0.45).
- Deadband to reduce MIDI traffic (DEAD_BAND = 2).
- Sends MIDI Control Change CC 11 on channel 0 via BLE.
- Serial debug output at 115200 baud.
- BLE device name: "Organ Pedal" (BLE_NAME in code).
- BLEMidiServer debugging enabled in setup().

Hardware
- BLE-capable microcontroller (ESP32 recommended).
- Expression pedal wired as a voltage divider to ADC pin (0–3.3V).

Example wiring
- Pedal output -> GPIO34 (analog input)
- Pedal GND -> GND
- Pedal VCC -> 3.3V

Important code/configuration (see main/main.ino)
- Pedal pin: SWELL_PEDAL_PIN = 34
- ADC range assumed: 0–4095 (ADC_MAX = 4095)
- MIDI CC: SWELL_MIDI_CC = 11
- MIDI channel: SWELL_MIDI_CHANNEL = 0
- BLE device name: BLE_NAME = "Organ Pedal"
- Serial baud: 115200
- Sample interval: SAMPLE_INTERVAL_MS = 5 (uses delay)

Usage
1. Open `main/main.ino` in Arduino IDE or PlatformIO.
2. Verify pin wiring and constants if your board differs.
3. Compile and upload to the board.
4. Open Serial Monitor at 115200 for debug output.
5. Pair and connect the BLE MIDI device (named "Organ Pedal") from your host (PC/mobile) and route CC 11 as desired.

Notes & future improvements
- Current loop uses delay(); replace with millis() for concurrent tasks and lower latency.
- Consider rounding and constrain when converting floats to uint8_t for MIDI values.
- Calibrations use asymmetric update rates; you may tune the adaptation constants.
- Add configurable BLE device name and CC/channel via EEPROM or settings UI.
- Add persistent calibration storage if desired.

Repository layout
- main/main.ino — sketch containing implementation
- README.md — this file
- .gitignore — local backup exclusions

License
- No license file included; add one if you plan to publish.