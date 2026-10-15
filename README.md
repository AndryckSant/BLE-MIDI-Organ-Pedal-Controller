# BLE MIDI Organ Pedal Controller

Firmware for an ESP32-based expression-pedal controller that sends MIDI Control Change messages over BLE MIDI. The firmware is organized into application, FSM, service, model, and driver layers.

Firmware version: **1.2.0**

## Features

- BLE MIDI output for two expression pedals and up to eight physical buttons.
- ADS1115 input sampling, pedal smoothing, calibration, normalization, and MIDI mapping.
- Per-pedal adaptive auto-range or manually configured calibration limits.
- Configurable button input polarity and momentary or latch MIDI behavior.
- OLED status display and a Wi-Fi access-point configuration portal.
- System configuration persisted in ESP32 non-volatile storage (NVS), including migration from earlier configuration formats.

## Hardware

- ESP32 with BLE support.
- ADS1115 I2C analog-to-digital converter (default address `0x48`).
- SSD1306 I2C OLED display (default address `0x3C`).
- Up to two expression pedals and eight MIDI buttons.

### Default pin assignments

| Function | ESP32 pin / input |
| --- | --- |
| I2C SDA | GPIO 21 |
| I2C SCL | GPIO 22 |
| Configuration button | GPIO 15 |
| Status LED | GPIO 2 |
| MIDI buttons 1-8 | GPIO 13, 12, 14, 27, 26, 25, 33, 32 |
| Pedal inputs | ADS1115 channels A0 and A1 by default |

Connect the buttons with external pull-down resistors. The dedicated configuration button is also read as a digital input. Connect each pedal output to its assigned ADS1115 input and share ground; use a supply voltage appropriate for the pedal and board.

## Configuration portal

Enter configuration mode by pressing the button on GPIO 15 or sending `c`/`C` through Serial at **115200 baud**. BLE MIDI is stopped while the portal is active.

The ESP32 starts an open Wi-Fi access point named **BLE MIDI Controller Setup**. Connect to it and open the IP address shown on the OLED and in the Serial output. The portal lets you configure:

- Each pedal's enabled state, ADS1115 channel, MIDI channel, MIDI CC, calibration minimum/maximum, and auto-range setting.
- Each button's enabled state, GPIO, input polarity, MIDI channel, MIDI CC, and behavior.
- MIDI channels from 1 to 16 and Control Change numbers from 0 to 127.

Button behavior:

- **Momentary:** sends value 127 when pressed and 0 when released.
- **Latch:** alternates between 127 and 0 on successive presses; release sends nothing. This is the default.

Each button input has a 40 ms debounce interval. Use the assigned MIDI-button GPIOs (13, 12, 14, 27, 26, 25, 33, or 32); GPIO 15 is reserved for configuration mode.

Saving settings or restoring factory defaults persists the configuration to NVS and restarts the device. Sending `x`/`X` exits the portal without saving and returns to normal BLE MIDI operation. If the access point cannot start, the OLED displays an error for five seconds before the device restarts.

Factory defaults enable both pedals on ADS1115 channels A0 and A1, MIDI channel 1, with CC 11 and CC 7 respectively. Adaptive auto-range is enabled. All eight buttons default to disabled, use the GPIO assignments above, and have MIDI channel 1, CCs 20-27, and latch behavior.

## Pedal calibration

Auto-range is enabled by default and adapts the calibration range as new pedal extremes are detected. Disable auto-range to use fixed minimum and maximum values configured in the portal. For a useful fixed calibration, set the minimum below the maximum and exercise the pedal across its intended range.

## Build and upload

1. Install the ESP32 board support and the Arduino libraries required by the source, including Adafruit ADS1X15, Adafruit GFX, Adafruit SSD1306, and the BLE MIDI library.
2. Open `Firmware/main/main.ino` in the Arduino IDE, select the target ESP32 board and port, then compile and upload.
3. Open Serial Monitor at **115200 baud** for startup messages and configuration-mode IP details.
4. Pair the BLE MIDI device named **BLE MIDI Controller** with the host and connect to receive MIDI CC messages.

The web-page source is `Firmware/main/src/core/states/config_page.html`; its firmware-embedded header is `Firmware/main/src/core/states/config_page.h`. After editing the HTML, regenerate the header by running `.\Firmware\tools\embed_config_page.ps1` from the repository root.

## Architecture

- `Firmware/main/main.ino` — Arduino `setup()`/`loop()` wrapper.
- `Firmware/main/src/app/` — application initialization, coordination, and runtime processing.
- `Firmware/main/src/core/` — FSM, event queue, and application states, including BLE, streaming, and configuration.
- `Firmware/main/src/drivers/` — ADS1115, BLE, OLED, and NVS hardware access.
- `Firmware/main/src/models/` — pedal, button, calibration, MIDI, and system configuration data.
- `Firmware/main/src/services/` — configuration persistence, input processing, calibration, display, and MIDI services.
- `Firmware/tools/` — utility for embedding the configuration page in the firmware.
- `Hardware/` — KiCad board design and related hardware files.
- `Docs/` — architecture and configuration-portal notes.

## License

- Firmware and original software in `Firmware/` are licensed under the [MIT License](Firmware/LICENSE).
- Hardware design files in `Hardware/` are licensed under [CERN-OHL-S-2.0](Hardware/LICENSE) (Strongly Reciprocal).
- These licenses apply to the original project work in their respective directories. Third-party libraries and other third-party materials remain under their own licenses; see their respective sources for terms.
