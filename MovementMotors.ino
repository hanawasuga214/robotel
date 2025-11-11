
// L9110S connections to ESP32:
// Motor B (Right Motor)
const int B1A_PIN = 16; // Motor B, Pin 1 (D16)
const int B1B_PIN = 17; // Motor B, Pin 2 (D17)
// Motor A (Left Motor)
const int A1A_PIN = 18; // Motor A, Pin 1 (D18)
const int A1B_PIN = 19; // Motor A, Pin 2 (D19)
// Motor Speed (PWM value 0-255)
const int MOTOR_SPEED = 150; 

// Timing
const int MOVE_DURATION = 1500; // Time in milliseconds to run the motors
const int STOP_DURATION = 1000; // Time in milliseconds to pause

unsigned long previousMillisMotor = 0;
int motorState = 0; // 0: Forward, 1: Stop, 2: Backward, 3: Stop, etc.

void setupMotors() {
  // Initialize all motor control pins as OUTPUT
  pinMode(B1A_PIN, OUTPUT);
  pinMode(B1B_PIN, OUTPUT);
  pinMode(A1A_PIN, OUTPUT);
  pinMode(A1B_PIN, OUTPUT);

  // Ensure motors are stopped at start
  stopMotors();
}

void loopMotorsNonBlocking(unsigned long currentMillis) {
  // The 'loopMotors' logic is now executed here constantly, but actions only change
  // when the required time (MOVE/STOP_DURATION) has passed.
  if (currentMillis - previousMillisMotor >= (motorState % 2 == 0 ? MOVE_DURATION : STOP_DURATION)) {
    previousMillisMotor = currentMillis; 
    
    // Increment state, and wrap back to 0 after the last step
    motorState = (motorState + 1); // 8 total states: F, S, B, S, L, S, R, S
    if (motorState > 40) { return; }

    // Set the action based on the new state
    switch (motorState % 8) {
      case 0: goForward(MOTOR_SPEED); break;
      case 2: goBackward(MOTOR_SPEED); break;
      case 4: turnLeft(MOTOR_SPEED); break;
      case 6: turnRight(MOTOR_SPEED); break;
      default: stopMotors(); break; // States 1, 3, 5, 7 are all STOP
    }
    Serial.print("Motor State: ");
    Serial.println(motorState);
  }
}

// --- MOTOR CONTROL FUNCTIONS ---

/**
 * Moves both motors forward by setting one pin HIGH/PWM and the other LOW.
 * A1A/B1A high, A1B/B1B low.
 */
void goForward(int speed) {
  // Left Motor (A)
  analogWrite(A1A_PIN, speed); // Forward
  analogWrite(A1B_PIN, 0);     // Stop
  
  // Right Motor (B)
  analogWrite(B1A_PIN, speed); // Forward
  analogWrite(B1B_PIN, 0);     // Stop
}

/**
 * Moves both motors backward (reverse of forward).
 * A1A/B1A low, A1B/B1B high.
 */
void goBackward(int speed) {
  // Left Motor (A)
  analogWrite(A1A_PIN, 0);     // Stop
  analogWrite(A1B_PIN, speed); // Backward
  
  // Right Motor (B)
  analogWrite(B1A_PIN, 0);     // Stop
  analogWrite(B1B_PIN, speed); // Backward
}

/**
 * Turns Left by running the Left Motor backward and the Right Motor forward (Pivot).
 */
void turnLeft(int speed) {
  // Left Motor (A) - Backward
  analogWrite(A1A_PIN, 0);     // Stop
  analogWrite(A1B_PIN, speed); // Backward
  
  // Right Motor (B) - Forward
  analogWrite(B1A_PIN, speed); // Forward
  analogWrite(B1B_PIN, 0);     // Stop
}

/**
 * Turns Right by running the Left Motor forward and the Right Motor backward (Pivot).
 */
void turnRight(int speed) {
  // Left Motor (A) - Forward
  analogWrite(A1A_PIN, speed); // Forward
  analogWrite(A1B_PIN, 0);     // Stop
  
  // Right Motor (B) - Backward
  analogWrite(B1A_PIN, 0);     // Stop
  analogWrite(B1B_PIN, speed); // Backward
}

/**
 * Stops both motors by setting all control pins LOW (or both to the same state).
 */
void stopMotors() {
  // Stop Left Motor (A)
  analogWrite(A1A_PIN, 0);
  analogWrite(A1B_PIN, 0);
  
  // Stop Right Motor (B)
  analogWrite(B1A_PIN, 0);
  analogWrite(B1B_PIN, 0);
}