
const int BUTTON_PIN = 13; 

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool lastState = HIGH;

void setupWifi() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loopWifi(unsigned long currentMillis) {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastState) lastDebounceTime = currentMillis;
  if ((currentMillis - lastDebounceTime) > debounceDelay)
  {
    if (reading == LOW)
    {
      Serial.println("Pressed button");
      // Start Wi-Fi connection
    }
  }
  lastState = reading;
}