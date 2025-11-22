# Expression Pedal BLE MIDI

This project converts an analog signal from a pedal/potentiometer into MIDI messages over BLE using the sketch in [main/main.ino](main/main.ino).

Key points implemented in the code:
- Analog read from the pin [`pedal_pin`](main/main.ino) (set to 34).
- Simple smoothing (moving average) stored in [`filtered`](main/main.ino).
- Conversion to a MIDI value 0–127 stored in [`midi_value`](main/main.ino).
- Sends Control Change messages when the change exceeds a deadband, using CC 11. Main logic is in [`loop()`](main/main.ino); initialization is in [`setup()`](main/main.ino).
- Previous state tracked in [`last_value`](main/main.ino) to reduce MIDI traffic.

Hardware
- BLE-capable microcontroller (e.g., ESP32).
- Pedal/expression connected as a voltage divider to analog pin 34 (0–3.3V).
- Power and GND according to your board.

Wiring (example)
- Pedal output -> pin 34
- Pedal GND -> GND
- Pedal VCC -> 3.3V (never 5V if using ESP32 ADC)

Important code settings
- Pedal pin: variable [`pedal_pin`](main/main.ino)
- Serial baud for debug: 115200 (`Serial.begin(115200)` in [main/main.ino](main/main.ino))
- BLE device name set when starting the server (string `"Basic MIDI Device"`).
- MIDI messages are sent as Control Change with values calculated from [`filtered`](main/main.ino) to [`midi_value`](main/main.ino).

Usage / Debug
1. Open [main/main.ino](main/main.ino) in the Arduino IDE or PlatformIO.
2. Adjust the pedal pin if needed (`[`pedal_pin`](main/main.ino)`).
3. Compile and upload to your board.
4. Open the Serial Monitor at 115200 baud to view debug values.
5. Pair/connect the BLE device on the host (PC, smartphone) named "Basic MIDI Device".

Notes
- The sketch applies simple smoothing and a deadband to avoid excessive messages.
- Adjust the pedal voltage divider to ensure ADC readings stay within the ADC range (0–3.3V).
- If using a different board, verify analog pin mapping and ADC resolution (the code assumes 0–4095).

Main file
- Source: [main/main.ino](main/main.ino)
- Main symbols: [`setup()`](main/main.ino), [`loop()`](main/main.ino), [`pedal_pin`](main/main.ino), [`filtered`](main/main.ino), [`midi_value`](main/main.ino), [`last_value`](main/main.ino)