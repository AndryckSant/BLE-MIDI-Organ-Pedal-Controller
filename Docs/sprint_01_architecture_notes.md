# Sprint 01 - Architecture Notes

## Goal
Move the pedal slice away from `main.ino` and into the intended layered architecture without changing the functional behavior of the firmware.

## What changed
- `Firmware/drivers/adc/ads1115.cpp` now owns the ADS1115 access layer.
- `Firmware/services/input/pedal/pedal_service.cpp` now owns pedal smoothing, adaptive calibration, mapping and MIDI value generation.
- `Firmware/models/pedal_model.h` holds the pedal state as data only.
- `Firmware/app/app.cpp` now coordinates setup and runtime execution.
- `Firmware/main/main.ino` was reduced to a thin wrapper that calls `app_init()` and `app_run()`.

## Notes
- The current build still uses sketch-local wrapper translation units so the Arduino IDE can link the code from nested folders.
- The FSM layer remains a future sprint item; it is not yet the application coordinator.
- The current focus was architecture separation, not feature expansion.

## Next sprint
1. Move higher-level application decisions out of `app.cpp` and into the FSM layer.
2. Introduce explicit state/event transitions for BLE connection and streaming.
3. Start replacing hardcoded pedal constants with persisted configuration.