# Sprint 02 - Configuration Portal

## Goal
Bring the `config` state of the FSM to life with a WiFi AP and a web interface for editing the MIDI configuration.

## What changed
- The `config` state now starts an ESP32 WiFi AP named `BLE MIDI Controller Setup`.
- A web page is served for the 2 pedals and 8 buttons.
- Pedal configuration includes enable/disable, ADS channel, MIDI channel, MIDI CC and calibration min/max.
- Each pedal can enable or disable adaptive auto-range independently. When disabled, valid configured calibration limits remain fixed during operation; fixed mode requires minimum to be lower than maximum.
- Button configuration includes enable/disable, GPIO pin, inverted logic, MIDI channel and MIDI CC.
- Each button can be configured as momentary (send 127 on press and 0 on release) or latch (toggle between 127 and 0 on each press). Latch is the default, including for buttons migrated from previous configuration versions.
- NVS configuration version 3 stores button modes and migrates existing version 1 and 2 configurations without discarding their pedal, button or auto-range settings.
- A save action persists the current config to NVS.
- A factory reset action restores default values and persists them.
- The web page markup and styles live in `Firmware/main/src/core/states/config_page.html`.
- `Firmware/main/src/core/states/config_page.h` embeds the page in firmware; regenerate it after editing the HTML by running `.\Firmware\tools\embed_config_page.ps1` from the repository root.

## Entry path
- Press the hardware configuration button on GPIO15 (D15), or use serial command `c` or `C`.
- BLE MIDI is stopped before the Wi-Fi portal starts. Serial command `x` or `X` exits the portal and restarts BLE MIDI.
- If the Wi-Fi AP cannot start, the OLED displays an error for five seconds and the device restarts.
- Configurable MIDI buttons use GPIO13, GPIO12, GPIO14, GPIO27, GPIO26, GPIO25, GPIO33 and GPIO32. Each button has a 40 ms debounce. Momentary sends 127 on press and 0 on release; latch sends alternating 127/0 values on presses only.

## Notes
- Saving or restoring factory defaults persists the configuration and restarts the device so all configured inputs are initialized from the saved values.
- The portal is intentionally simple and self-hosted so it can run without external dependencies.

## Firmware release
- Firmware version: `1.2.0` (shown on the OLED boot screen).
- Release includes configurable momentary/latch button actions, per-pedal adaptive/fixed calibration ranges, and the Wi-Fi configuration portal hardware/lifecycle behavior.