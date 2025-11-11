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
All motor functions take `speed` (0 or 1) as parameter:
```cpp
goForward(MOTOR_SPEED);    // Both motors forward
turnLeft(MOTOR_SPEED);     // Left backward, right forward (pivot turn)
stopMotors();              // All pins LOW
```

**Uses basic GPIO** (`digitalWrite()`) instead of PWM to avoid ADC driver conflicts in ESP32 core 3.x:
```cpp
pinMode(pin, OUTPUT);                          // Setup
digitalWrite(pin, speed > 0 ? HIGH : LOW);     // Control: on/off only (no speed control)
```
**Note:** PWM speed control (`ledcAttach`/`analogWrite`) conflicts with `adc_oneshot` in ESP32 core 3.3.3. Motors run at full speed (digital HIGH) or stopped (LOW).

State machine in `loopMotorsNonBlocking()` cycles through: Forward→Stop→Backward→Stop→Left→Stop→Right→Stop (states 0-7, wraps at 40).

### ADC Reading (ESP-IDF v5 API)
Uses new `adc_oneshot` API, not deprecated `analogRead()`:
```cpp
adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &raw_value);
```
**Critical:** Only use ADC1 channels (GPIO32-39). ADC2 conflicts with WiFi on ESP32.

### Display Updates
`MovementSensors.ino` uses U8g2 full buffer mode (`_F_` variant):
1. `u8g2.clearBuffer()` - clear RAM buffer
2. Draw operations (`drawStr`, `drawFrame`, `print`)
3. `u8g2.sendBuffer()` - send to OLED in one transaction

### Audio Processing
I2S samples are 32-bit left-aligned. Convert to 24-bit before processing:
```cpp
int32_t sample = samples[i] >> 8;  // Right-shift to extract 24-bit data
```
Calculates RMS and approximates dB SPL for serial plotter visualization.

## Build & Upload Workflow

**Platform:** Arduino IDE 2.x or PlatformIO with ESP32 board support.

**Board configuration:**
- Board: "ESP32 Dev Module" or equivalent
- Partition Scheme: Default (some I2S configs need larger app partition)
- Upload Speed: 921600 (or 115200 if unstable)
- Port: Auto-detect via USB (CP2102/CH340 driver required)

**Dependencies (install via Library Manager):**
- `U8g2` by olikraus (for SH1106 OLED)
- ESP32 core includes `driver/i2s.h` and `esp_adc/adc_oneshot.h`

**Common issues:**
- **Driver conflict crash (ESP32 core 3.x)** - Both `analogWrite()` AND `ledcAttach()` conflict with `adc_oneshot` API. Workaround: use basic `digitalWrite()` (no PWM speed control). This is a known bug in ESP32 Arduino core 3.3.3
- **I2S + ADC conflict** - `#include <driver/i2s.h>` triggers legacy ADC driver initialization even if code is commented out. `Audio.ino` uses `#ifdef ENABLE_AUDIO` guard to prevent compilation conflicts. Audio and ADC sensors cannot run simultaneously on ESP32 core 3.x
- **WiFi kills ADC2** - All TCRT sensors deliberately on ADC1 (GPIO32-34)
- **ECHO pin voltage** - HC-SR04 outputs 5V; use resistor divider to 3.3V or risk GPIO damage

## Incomplete Features

**Audio module (`Audio.ino`):** DISABLED by default due to I2S/ADC driver conflict in ESP32 core 3.x. To enable audio (breaks TCRT sensors), uncomment `#define ENABLE_AUDIO` at the top of `Audio.ino`. The I2S INMP441 microphone cannot coexist with `adc_oneshot` API on ESP32 Arduino core 3.3.3.

**WiFi module (`Wifi.ino`):** Button press detection works, but actual WiFi connection code missing. Stub prints "Pressed button" to serial. Likely intended for remote control or telemetry.

**Motor sequence auto-stop:** `loopMotorsNonBlocking()` commented out in main loop. When enabled, stops after `motorState > 40` (5 complete cycles). Remove condition for continuous operation.

## Testing Notes

- **Serial Monitor @ 115200 baud** - Audio module prints dB values for Arduino Serial Plotter
- **OLED shows:** Ultrasonic distance (cm), visual bar graph, 3x TCRT raw ADC values (0-4095)
- **Motor test:** Uncomment `loopMotorsNonBlocking()` in `Robotel.ino` to run movement demo

## Key Files

- `Robotel.ino` - Main setup/loop orchestrator
- `MovementMotors.ino` - Motion primitives (forward/backward/turn/stop)
- `MovementSensors.ino` - All sensors + OLED display logic
- `Audio.ino` - Microphone sampling and dB calculation
