# Robotel - ESP32 Robot Project

## Architecture Overview

This is an **Arduino multi-tab sketch** for an ESP32-based mobile robot. The main file `Robotel.ino` orchestrates multiple subsystems split across separate `.ino` files. Arduino IDE automatically combines all `.ino` files in the same directory into a single compilation unit, making all functions globally accessible.

**Key subsystems:**
- `MovementMotors.ino` - L9110S dual motor driver control (4 GPIOs, PWM-based)
- `MovementSensors.ino` - HC-SR04 ultrasonic sensor + 3x TCRT5000 line sensors with SH1106 OLED display
- `Audio.ino` - I2S INMP441 microphone with dB level monitoring
- `Wifi.ino` - Button-triggered WiFi connection stub (incomplete)

**Critical timing pattern:** Non-blocking execution using `millis()` intervals. Each subsystem has its own `previousMillis` variable and update interval (e.g., `INTERVAL_SENSOR = 100ms`, `INTERVAL_AUDIO = 20ms`). Functions return early if insufficient time has elapsed.

## Hardware Pin Mapping

**Motors (L9110S):**
- Left Motor A: GPIO18 (A1A), GPIO19 (A1B)
- Right Motor B: GPIO16 (B1A), GPIO17 (B1B)
- Speed control via `analogWrite()` PWM (0-255)

**Sensors:**
- HC-SR04: TRIG=GPIO25, ECHO=GPIO26 (5V→3.3V through voltage divider!)
- TCRT5000 IR line sensors: GPIO32/33/34 (ADC1 channels 4/5/6)
- OLED SH1106 I2C: SDA=GPIO21, SCL=GPIO22

**Audio:**
- INMP441 I2S mic: SD=GPIO4, WS=GPIO5, SCK=GPIO14
- Buzzer: GPIO27

**WiFi button:** GPIO13 (INPUT_PULLUP, active LOW)

## Development Conventions

### Motor Control Pattern
All motor functions take `speed` (0-255) as a parameter:
```cpp
goForward(MOTOR_SPEED);    // Both motors forward
turnLeft(MOTOR_SPEED);     // Left backward, right forward (pivot turn)
stopMotors();              // All pins LOW
```
State machine in `loopMotorsNonBlocking()` cycles through: Forward→Stop→Backward→Stop→Left→Stop→Right→Stop.

### ADC Reading
The project uses the legacy `analogRead()` for the TCRT5000 sensors. This works with the new I2S driver API (`i2s_std.h`) without conflicts.
**Critical:** All ADC sensors are on ADC1 (GPIO32-39) to avoid conflicts with the WiFi module, which disables ADC2. The legacy `analogRead()` is compatible with the new I2S driver but NOT with the legacy I2S driver.

### Display Updates
`MovementSensors.ino` uses the U8g2 library in full buffer mode (`_F_` variant):
1. `u8g2.clearBuffer()` - clear RAM buffer
2. Draw operations (`drawStr`, `drawFrame`, `print`)
3. `u8g2.sendBuffer()` - send to OLED in one transaction

### Audio Processing
`Audio.ino` uses the **new I2S driver API** (`i2s_std.h`) to read 32-bit samples from the INMP441 microphone, converts them to 24-bit, and calculates RMS/dB levels.
```cpp
i2s_channel_read(rx_handle, samples, sizeof(samples), &bytes_read, 0);
int32_t sample = samples[i] >> 8;  // Right-shift to extract 24-bit data
double db = 20 * log10(rms / 2147483647.0) + 120;  // Approximate dB scale
```
The new I2S driver (`i2s_new_channel`, `i2s_channel_init_std_mode`) is compatible with `analogRead()` and avoids hardware conflicts present in the legacy driver.

## Build & Upload Workflow

**Platform:** Arduino IDE 2.x or PlatformIO with ESP32 board support.

**Board configuration:**
- Board: "ESP32 Dev Module" or equivalent
- Partition Scheme: Default
- Upload Speed: 921600 (or 115200 if unstable)
- Port: Auto-detect via USB

**Dependencies (install via Library Manager):**
- `U8g2` by olikraus (for SH1106 OLED)

## Incomplete Features & Known Issues

- **WiFi Module (`Wifi.ino`):** Button press detection works, but the actual WiFi connection code is a stub. It only prints "Pressed button" to the serial monitor.
- **Motor Sequence:** `loopMotorsNonBlocking()` is commented out in the main `loop()`. Uncomment it to run the pre-programmed movement demonstration.
- **Driver Compatibility:** The new I2S driver API (`i2s_std.h`) is used instead of the legacy driver to avoid conflicts with ADC. If you see "CONFLICT! driver_ng" errors, ensure you're using the new API throughout.

## Key Files

- `Robotel.ino` - Main setup/loop orchestrator
- `MovementMotors.ino` - Motion primitives (forward/backward/turn/stop)
- `MovementSensors.ino` - All sensors + OLED display logic
- `Audio.ino` - Microphone sampling (disabled by default)
- `Wifi.ino` - WiFi connection stub
