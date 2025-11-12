# Robotel 🤖

**An ESP32-based autonomous mobile robot with voice recognition and AI-driven navigation capabilities**

![Status](https://img.shields.io/badge/Status-Hardware%20Testing%20Phase-yellow)
![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Arduino](https://img.shields.io/badge/Arduino-3.3.3%20Espressif-teal)

---

## 🎯 Project Vision

Robotel is an ambitious self-learning robot platform designed to explore autonomous navigation and voice-directed control through AI integration. The ultimate goal is to create a robot that can:

- **Navigate autonomously** using ultrasonic and infrared sensors for obstacle avoidance and edge detection
- **Respond to voice commands** through custom-trained speech recognition
- **Communicate with GPT-based AI** via WiFi for intelligent decision-making and feedback
- **Learn and adapt** its behavior over time through sensor data analysis

This project represents a complete hardware-software integration journey, from component-level testing to AI-powered autonomous behavior.

---

## 📍 Current Status: Hardware Integration & Testing

✅ **Completed:**
- All hardware components individually tested and validated
- Motor control system with PWM speed control
- Ultrasonic distance measurement (HC-SR04)
- IR line/edge detection sensors (3x TCRT5000)
- Real-time OLED display for sensor feedback
- I2S microphone audio capture with dB level monitoring
- Buzzer melody playback system
- Non-blocking multi-subsystem architecture
- Autonomous movement demonstration sequence

🚧 **In Progress:**
- Autonomous navigation algorithms
- WiFi connectivity for GPT API integration
- Voice recognition AI development

📋 **Planned:**
- GPT-based decision-making system
- Voice command training and recognition
- Sensor fusion for intelligent navigation
- Web-based telemetry dashboard (fallback)
- Over-the-air (OTA) firmware updates

---

## 🔧 Hardware Architecture

### Core Components

| Component | Model | Purpose | Pins |
|-----------|-------|---------|------|
| **Microcontroller** | ESP32 Dev Module | Main processing unit | - |
| **Motor Driver** | L9110S Dual H-Bridge | Controls 2x DC motors | GPIO 16-19 |
| **Distance Sensor** | HC-SR04 Ultrasonic | Obstacle detection (2-400cm) | GPIO 25 (TRIG), 26 (ECHO)* |
| **Line Sensors** | 3x TCRT5000 IR | Edge/hole detection | GPIO 32, 33, 34 (ADC1) |
| **Display** | SH1106 OLED 128x64 | Real-time sensor visualization | GPIO 21 (SDA), 22 (SCL) |
| **Microphone** | INMP441 I2S | Voice input for recognition | GPIO 4 (SD), 5 (WS), 14 (SCK) |
| **Buzzer** | Passive Buzzer | Audio feedback/notifications | GPIO 27 |
| **WiFi Button** | Push Button | Trigger WiFi connection | GPIO 13 (INPUT_PULLUP) |

**\* Important:** HC-SR04 ECHO pin uses 5V→3.3V voltage divider to protect ESP32

### Power System

- **Motor Power:** 4x AA Alkaline batteries (1.5V each) = 6V for dual 500mAh DC motors
- **ESP32 Power:** 3.7V LiPo battery → USB charger module → MT3608 boost converter (set to 5V)
- **Design Rationale:** Separate power rails prevent motor noise from affecting ESP32 logic

### Physical Structure

- **Chassis:** Cardboard prototype (lightweight, easy to modify)
- **Drive System:** 2-wheel differential drive with rear caster wheel for stability
  - Front: 2x motorized wheels (L9110S control)
  - Rear: 1x passive caster wheel
  - Configuration: Stable 3-point base for precise turning
- **Sensor Placement:** 
  - HC-SR04 front-facing for forward obstacle detection
  - TCRT5000 sensors underneath for edge/hole detection (prevents falls)

---

## 💻 Software Architecture

### Development Environment

- **IDE:** Arduino IDE with Espressif ESP32 board support v3.3.3
- **Language:** C++ (Arduino framework)
- **Key Libraries:**
  - `U8g2` (OLED display driver)
  - `driver/i2s_std.h` (new I2S API for microphone)
  - `Wire.h` (I2C communication)

### Multi-Tab Sketch Structure

The project uses Arduino's multi-tab architecture where each `.ino` file represents a subsystem:

```
Robotel/
├── Robotel.ino              # Main orchestrator (setup/loop)
├── MovementMotors.ino       # L9110S motor control primitives
├── MovementSensors.ino      # HC-SR04 + TCRT5000 + OLED display
├── Audio.ino                # INMP441 microphone + buzzer
└── Wifi.ino                 # WiFi connection handler (stub)
```

### Non-Blocking Execution Model

All subsystems use **millis()-based timing** instead of `delay()` to enable true multitasking:

```cpp
// Each subsystem has its own update interval
INTERVAL_SENSOR  = 100ms  // 10 Hz sensor readings
INTERVAL_AUDIO   = 20ms   // 50 Hz audio sampling
INTERVAL_MOTOR   = 1500ms // State machine transitions
```

This architecture allows simultaneous sensor monitoring, motor control, and audio processing without blocking.

### Motor Control API

Simple, speed-based motion primitives:

```cpp
goForward(speed);   // Both motors forward (0-255 PWM)
goBackward(speed);  // Both motors backward
turnLeft(speed);    // Pivot turn (left backward, right forward)
turnRight(speed);   // Pivot turn (right backward, left forward)
stopMotors();       // Emergency stop
```

### Sensor Data Flow

1. **HC-SR04:** Pulses TRIG (10µs) → measures ECHO → calculates distance in cm
2. **TCRT5000:** ADC reads IR reflectance (0-4095) → low values = surface detected, high = hole/edge
3. **OLED Display:** U8g2 full-buffer mode updates sensor readings + visual bar graphs every 100ms

### Audio Processing

- **Microphone:** 16kHz I2S sampling → 32-bit samples → 24-bit conversion → RMS/dB calculation
- **Buzzer:** Non-blocking melody playback using state machine
- **Critical:** Uses **new I2S driver API** (`i2s_std.h`) to avoid conflicts with ADC

---

## 🚀 Getting Started

### Prerequisites

1. **Arduino IDE** with ESP32 board support (Espressif v3.3.3)
2. **U8g2 library** (install via Library Manager)
3. USB cable for ESP32 programming
4. All hardware components assembled

### Installation

```bash
# Clone the repository
git clone https://github.com/BogdanDumbravean/robotel.git
cd robotel/Robotel

# Open in Arduino IDE
# File → Open → Robotel.ino
```

### Configuration

**Board Settings (Tools menu):**
- Board: "ESP32 Dev Module"
- Upload Speed: 921600 (or 115200 if unstable)
- Partition Scheme: Default
- Port: [Auto-detect your ESP32]

### Upload & Test

1. Connect ESP32 via USB
2. Click **Upload** button
3. Open **Serial Monitor** (115200 baud)
4. Observe sensor readings on OLED display

**Test Sequence:**
- Place hand in front of HC-SR04 → distance should update
- Move TCRT sensors over black/white surfaces → values change
- Press GPIO13 button → "Pressed button" appears in Serial Monitor
- Hear startup melody from buzzer

---

## 📊 Current Features Demo

### 1. Autonomous Movement Sequence

Pre-programmed demonstration of motor control capabilities:

```
Forward (1.5s) → Stop (1s) → Backward (1.5s) → Stop (1s) → 
Turn Left (1.5s) → Stop (1s) → Turn Right (1.5s) → Stop (1s) → Repeat
```

### 2. Real-Time OLED Display

Displays:
- **Line 1:** HC-SR04 distance (cm) with visual bar graph
- **Line 2-4:** TCRT5000 raw readings (L/M/R: 0-4095)

### 3. Audio Monitoring

Serial Plotter shows real-time dB levels from INMP441 microphone (20ms update rate)

---

## 🔮 Roadmap

### Phase 1: Autonomous Navigation *(Next)*
- [ ] Sensor fusion algorithm (ultrasonic + IR)
- [ ] Obstacle avoidance state machine
- [ ] Edge detection safety system
- [ ] Path planning with waypoint navigation

### Phase 2: Voice Recognition AI
- [ ] Collect training data from INMP441
- [ ] Train lightweight speech recognition model
- [ ] Implement wake word detection
- [ ] Command vocabulary (forward, stop, turn, etc.)

### Phase 3: WiFi & GPT Integration
- [ ] Complete WiFi connection module
- [ ] Implement HTTPS client for OpenAI API
- [ ] Design prompt engineering for robot control commands
- [ ] Create feedback loop (sensors → GPT → motors)

### Phase 4: Advanced Features
- [ ] Telemetry web dashboard
- [ ] OTA firmware updates
- [ ] OLED menu system for configuration
- [ ] Battery voltage monitoring

---

## 🐛 Known Issues

- **WiFi Module:** Button press detection works, but connection logic is a stub
- **ADC Constraint:** All analog sensors must use ADC1 (GPIO32-39) to avoid WiFi conflicts

---

## 📚 Technical Notes

### Why New I2S Driver API?

The legacy I2S driver conflicts with `analogRead()` on ESP32. This project uses the **new driver API** (`driver/i2s_std.h`) introduced in ESP-IDF 5.0+, which coexists peacefully with ADC operations.

```cpp
// Old (conflicting): #include <driver/i2s.h>
// New (compatible):  #include <driver/i2s_std.h>
```

### HC-SR04 Voltage Level Shifting

The HC-SR04 operates at 5V logic, but ESP32 GPIO is 3.3V tolerant. **Always use a voltage divider** on the ECHO pin:

```
HC-SR04 ECHO → [R1: 1kΩ] → ESP32 GPIO26
                    ↓
                [R2: 2kΩ] → GND
```

This creates a 2:3 ratio (5V → 3.3V).

### TCRT5000 Edge Detection Logic

- **Surface detected (white/reflective):** ADC reads **LOW** (0-1500)
- **Hole/edge detected (no reflection):** ADC reads **HIGH** (2500-4095)
- Threshold tuning required based on surface materials

---

## 🤝 Contributing

This is a personal learning project, but suggestions and ideas are welcome! Feel free to:

- Open issues for bugs or improvement ideas
- Share similar projects or resources
- Provide feedback on the AI integration approach

---

## 📄 License

This project is open-source and available for educational purposes. Feel free to replicate, modify, and learn from it.

---

## 👤 Author

**Bogdan Dumbravean**
- GitHub: [@BogdanDumbravean](https://github.com/BogdanDumbravean)

---

## 🙏 Acknowledgments

- ESP32 community for extensive hardware documentation
- Arduino framework for accessible embedded development
- OpenAI for GPT API (planned integration)

---

*Last Updated: November 2025*
*Project Status: Hardware Testing & Integration Phase*