/*
  Exercise 02c — The Actuator (Stepper Motor 90° rotation)
  IIoT-Batch02 | Week 02

  Components:
  - 28BYJ-48 Stepper Motor + ULN2003 Driver
  - Push Button

  Behavior:
  - Press button → motor smoothly accelerates to 90° clockwise (Accept bin)
  - Then automatically returns to 0° (centre position)

  Note:
  - 28BYJ-48 in half-step mode = 4096 steps per revolution
  - 90° = 4096 / 4 = 1024 steps
*/

#include <AccelStepper.h>

// ─── Pin Definitions ─────────────────────────────────────────────────────────
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17
#define BTN_PIN 15  // Push button (active LOW)

// 4-wire stepper in HALF4WIRE mode for smooth movement
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

bool lastBtnState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);

  stepper.setMaxSpeed(600);
  stepper.setAcceleration(300);
  stepper.setCurrentPosition(0);

  Serial.println("Stepper ready. Press button to rotate 90°.");
}

void loop() {
  // Always call run() to process motor movement
  stepper.run();

  bool btnState = digitalRead(BTN_PIN);

  // Detect button press (falling edge)
  if (btnState == LOW && lastBtnState == HIGH) {
    delay(50); // debounce

    Serial.println("Button pressed — rotating 90° clockwise...");
    stepper.moveTo(1024); // 90° clockwise

    // Block until 90° is reached
    while (stepper.distanceToGo() != 0) stepper.run();

    Serial.println("At 90°. Returning to centre...");
    delay(500); // pause at accept position

    stepper.moveTo(0); // return to centre
    while (stepper.distanceToGo() != 0) stepper.run();

    Serial.println("Back at centre.");
  }

  lastBtnState = btnState;
}
