# Sprint 02 - Configuration Portal

## Goal
Bring the `config` state of the FSM to life with a WiFi AP and a web interface for editing the MIDI configuration.

## What changed
- The `config` state now starts an ESP32 WiFi AP named `BLE MIDI Controller Setup`.
- A web page is served for the 2 pedals and 8 buttons.
- Pedal configuration includes enable/disable, ADS channel, MIDI channel, MIDI CC and calibration min/max.
- Button configuration includes enable/disable, GPIO pin, inverted logic, MIDI channel and MIDI CC.
- A save action persists the current config to NVS.
- A factory reset action restores default values and persists them.

## Entry path
- Temporary manual trigger: serial command `c` or `C` enters config mode while the device is in `BLE_connected` or `ReadAndSend`.
- Temporary exit command: serial command `x` or `X` exits config mode.

## Notes
- The FSM now returns from config mode to `BLE_disconnected` after a valid save, matching the state diagram direction.
- The portal is intentionally simple and self-hosted so it can run without external dependencies.
- Next improvement is to replace the temporary serial entry with a proper hardware or BLE-driven trigger.