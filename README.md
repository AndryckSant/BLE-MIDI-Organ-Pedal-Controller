# BLE-MIDI-Organ-Pedal-Controller

Modular BLE MIDI firmware for ESP32-based expression pedals. The project is being organized around drivers, models, services, an application layer and a future FSM so the code stays maintainable as features grow.

Current state
- The sketch entry point in `Firmware/main/main.ino` is now a thin wrapper that calls `app_init()` and `app_run()`.
- Application orchestration lives in `Firmware/app/app.cpp`.
- ADS1115 access is isolated in `Firmware/drivers/adc/ads1115.cpp`.
- Pedal processing logic is isolated in `Firmware/services/input/pedal/pedal_service.cpp`.
- `Firmware/models/pedal_model.h` stores pedal state and MIDI output data.
- The FSM now has a minimal event queue and state dispatch path.
- The `config` state brings up a WiFi AP and web UI to edit pedal and button parameters.

Key features implemented
- ADS1115 reading behind a driver API.
- Pedal smoothing, adaptive calibration and MIDI mapping in a service.
- `PedalModel`-based state for the pedal slice.
- BLE MIDI control change output guarded by connection state.
- OLED status display handled by the application layer.
- Arduino IDE compatibility preserved through sketch-local wrapper translation units.
- WiFi AP configuration portal with save and factory reset actions.
- Editable MIDI parameters for 2 pedals and 8 buttons.
- Pedal calibration fields exposed in the configuration UI.

Hardware
- BLE-capable microcontroller (ESP32 recommended).
- Expression pedal wired to ADS1115 channel A1.
- OLED SSD1306 on I2C.
- ESP32 I2C pins currently configured as SDA 21 and SCL 22.

Example wiring
- Pedal output -> ADS1115 A1
- Pedal GND -> GND
- Pedal VCC -> 3.3V

Important code/configuration (see main/main.ino)
- Entry point: `Firmware/main/main.ino`
- App orchestration: `Firmware/app/app.cpp`
- ADS1115 driver: `Firmware/drivers/adc/ads1115.cpp`
- Pedal service: `Firmware/services/input/pedal/pedal_service.cpp`
- Pedal model: `Firmware/models/pedal_model.h`
- MIDI CC: `11`
- MIDI channel: `1`
- BLE device name: `BLE MIDI Controller`
- Serial baud: `115200`
- Sample interval: `5 ms`

Usage
1. Open `Firmware/main/main.ino` in Arduino IDE.
2. Compile and upload to the board.
3. Open Serial Monitor at 115200 for debug output.
4. Pair and connect the BLE MIDI device (named `BLE MIDI Controller`) from your host and route CC 11 as desired.

Notes & future improvements
- The FSM should keep taking ownership of state transitions and runtime decisions.
- The configuration portal currently uses serial `c`/`C` as a temporary entry trigger from connected/runtime states; a dedicated input path should replace that later.
- Consider moving BLE, display and event coordination further out of `app.cpp` once the FSM grows.
- Calibration constants and MIDI routing are now configurable, but validation and richer UX can still improve.

Repository layout
- Firmware/main/main.ino — thin sketch entry point
- Firmware/app/ — application orchestration
- Firmware/core/ — FSM, events and states
- Firmware/drivers/ — hardware access layers
- Firmware/services/ — algorithmic services
- Firmware/models/ — domain data structures
- README.md — this file

License
- No license file included; add one if you plan to publish.