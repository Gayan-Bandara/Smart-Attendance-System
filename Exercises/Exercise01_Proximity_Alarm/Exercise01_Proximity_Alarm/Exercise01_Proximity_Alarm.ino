/*
  Exercise 01 — Smart Proximity Alarm System
  IIoT-Batch02 | Week 01

  Components:
  - Ultrasonic Sensor (HC-SR04) → detects obstacles
  - LED (Red) → blinks as visual alert
  - Buzzer → sounds audio alert
  - Servo → simulates barrier gate

  Behavior:
  - < 10cm : DANGER — fast blink + fast beep + gate closes
  - 10–30cm: WARNING — slow blink + slow beep + gate neutral
  - > 30cm  : SAFE   — all off + gate open
*/

#include <ESP32Servo.h>

// ─── Pin Definitions ─────────────────────────────────────────────────────────
#define TRIG_PIN  13
#define ECHO_PIN  12
#define LED_PIN   14
#define BUZZER    27
#define SERVO_PIN 26

Servo myServo;

// ─── Measure distance in cm ───────────────────────────────────────────────────
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(180); // gate open by default
  Serial.println("Smart Proximity Alarm Ready.");
}

void loop() {
  long dist = getDistance();
  Serial.print("Distance: ");
  Serial.print(dist);
  Serial.println(" cm");

  if (dist > 0 && dist < 10) {
    // ── DANGER zone ──────────────────────────────────────────────────────────
    myServo.write(0); // gate closed
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER, LOW);
    delay(100);

  } else if (dist < 30) {
    // ── WARNING zone ─────────────────────────────────────────────────────────
    myServo.write(90); // gate half
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER, LOW);
    delay(300);

  } else {
    // ── SAFE zone ────────────────────────────────────────────────────────────
    myServo.write(180); // gate fully open
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }
}
