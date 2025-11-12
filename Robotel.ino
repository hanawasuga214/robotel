#include <Wire.h>
#include <U8g2lib.h>

// -------------------------------------------OTHER VALUES---------------------------------------------------

// --- OLED SH1106 128x64 pe I2C ---
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0, /* reset = */ U8X8_PIN_NONE);


// -------------------------------------------SETUP FUNCTIONS---------------------------------------------------
void setup() {
  Serial.begin(115200);
  setupAudio();    // Initialize I2S first (before ADC)
  setupSensors();  // Then initialize ADC continuous
  setupMotors();   // Motor control
  setupWifi();     // WiFi button
  delay(500);
}

// -------------------------------------------LOOP FUNCTIONS---------------------------------------------------
void loop() {
  unsigned long currentMillis = millis();
  // --- Task 1: Sensor Reading and Display Update ---
  loopSensors(currentMillis);
  // --- Task 2: Non-Blocking Motor Sequence ---
  loopMotorsNonBlocking(currentMillis); 
  // --- Task 3: Wifi ---
  loopWifi(currentMillis);
  // --- Task 4: Audio Input and Output ---
  loopAudio(currentMillis);  // Safe (disabled by default)
}