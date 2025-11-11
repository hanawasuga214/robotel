// Pin de test pentru TCRT (nodul fototransistor + pull-up)
const int PIN_TCRT_M = 32;  // ADC1 (NU folosi ADC2 pe ESP32)
const int PIN_TCRT_L = 33;
const int PIN_TCRT_R = 34;
// --- PINI HC-SR04 ---
const int PIN_TRIG = 25;
const int PIN_ECHO = 26;  // ATENȚIE: prin divizor 5V->3.3V!
// --- PINI OLED ---
const int PIN_SDA = 21;
const int PIN_SCL = 22; 

// opțional: scale bar până la 150 cm
const int BAR_MAX_CM = 150;
const long INTERVAL_SENSOR = 100; // Update sensors/display every 100ms (~10 Hz)

unsigned long previousMillisSensor = 0;

// ------------------------------------------- FUNCTION PROTOTYPES ---------------------------------------------------

float readUltrasonicCM(uint32_t timeout_us = 30000UL);

// ------------------------------------------- FUNCTIONS ---------------------------------------------------

void setupSensors() {
  setupSH1106();
  setupTCRT();
}

void loopSensors(unsigned long currentMillis) {
  if (currentMillis - previousMillisSensor < INTERVAL_SENSOR) { return; }
  previousMillisSensor = currentMillis;
  // Read sensors
  float distance;
  int tcrt_raw_l, tcrt_raw_m, tcrt_raw_r;
  readSensors(distance, tcrt_raw_l, tcrt_raw_m, tcrt_raw_r);
  displayData(distance, tcrt_raw_l, tcrt_raw_m, tcrt_raw_r);
}

void setupSH1106() {
  // I2C + OLED
  Wire.begin(PIN_SDA, PIN_SCL);
  u8g2.begin();

  // Pini senzor
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Mesaj inițial
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 12, "HC-SR04 test");
  u8g2.drawStr(0, 26, "TRIG=25  ECHO=26");
  u8g2.drawStr(0, 40, "ECHO prin divizor!");
  u8g2.sendBuffer();
}

void setupTCRT() {
  // Configure ADC resolution and attenuation
  analogReadResolution(12); // 12-bit resolution (0-4095)
  analogSetPinAttenuation(PIN_TCRT_L, ADC_11db); // Full 0-3.3V range
  analogSetPinAttenuation(PIN_TCRT_M, ADC_11db);
  analogSetPinAttenuation(PIN_TCRT_R, ADC_11db);
  
  Serial.println("TCRT5000 sensors initialized (analogRead mode)");
}

void readSensors(float &dist, int &raw_l, int &raw_m, int &raw_r) {
  // Ultrasonic (HC-SR04)
  dist = readUltrasonicCM();

  // TCRT5000 using analogRead() - compatible with new I2S driver
  raw_l = analogRead(PIN_TCRT_L); // GPIO33
  raw_m = analogRead(PIN_TCRT_M); // GPIO32
  raw_r = analogRead(PIN_TCRT_R); // GPIO34
}float readUltrasonicCM(uint32_t timeout_us) {
  // Puls TRIG 10 µs
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Măsoară ECHO (HIGH) cu timeout (~30ms ≈ >5m)
  unsigned long dur = pulseIn(PIN_ECHO, HIGH, timeout_us);
  if (dur == 0) return NAN;  // out of range / timeout

  // v ~ 343 m/s => 0.0343 cm/µs; dus-întors => /2
  float cm = (dur * 0.0343f) / 2.0f;
  return cm;
}

void displayData(float dist, int raw_l, int raw_m, int raw_r) {
  // OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  // 1. Ultrasonic Reading
  u8g2.drawStr(0, 10, "HC-SR04 Dist:");
  if (isnan(dist)) {
    u8g2.drawStr(85, 10, "--");
  } else {
    u8g2.setCursor(85, 10);
    u8g2.print(dist, 1);
    u8g2.print("cm");
  }

  // 2. Ultrasonic Bar (Optional, keeps your original code's feature)
  u8g2.drawStr(0, 26, "Scale:");
  drawBar(35, 18, 90, 12, dist);

  // 3. TCRT5000 Readings (Raw Values)
  u8g2.drawStr(0, 42, "TCRT:");  // Label for the TCRT group

  // Left Sensor (L)
  u8g2.drawStr(0, 60, "L:");
  u8g2.setCursor(10, 60);
  u8g2.print(raw_l);  // Displaying raw reading

  // Middle Sensor (M)
  u8g2.drawStr(40, 60, "M:");
  u8g2.setCursor(50, 60);
  u8g2.print(raw_m);

  // Right Sensor (R)
  u8g2.drawStr(80, 60, "R:");
  u8g2.setCursor(90, 60);
  u8g2.print(raw_r);
  u8g2.sendBuffer();
}

void drawBar(int x, int y, int w, int h, float cm) {
  u8g2.drawFrame(x, y, w, h);
  if (isnan(cm)) return;
  if (cm < 0) cm = 0;
  if (cm > BAR_MAX_CM) cm = BAR_MAX_CM;
  int fill = (int)((w - 2) * (cm / (float)BAR_MAX_CM));
  u8g2.drawBox(x + 1, y + 1, fill, h - 2);
}