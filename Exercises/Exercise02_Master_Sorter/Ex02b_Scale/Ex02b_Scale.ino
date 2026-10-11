/*
  Exercise 02b — Precision Scale (HX711 Load Cell)
  IIoT-Batch02 | Week 02

  Components:
  - HX711 Load Cell Amplifier
  - Load Cell
  - Push Button (Tare)

  Behavior:
  - Continuously reads weight in grams
  - Press button to Tare (zero) the scale
  - Calibration factor must be adjusted to your specific load cell
*/

#include <HX711.h>

// ─── Pin Definitions ─────────────────────────────────────────────────────────
const int DOUT_PIN  = 4;   // HX711 Data Out
const int CLK_PIN   = 5;   // HX711 Clock
const int TARE_BTN  = 15;  // Tare button (active LOW with pull-up)

// ─── Calibration ─────────────────────────────────────────────────────────────
// HOW TO CALIBRATE:
// 1. Upload with CALIBRATION_FACTOR = 1.0
// 2. Read raw value from Serial Monitor with a known weight (e.g. 500g)
// 3. CALIBRATION_FACTOR = raw_value / known_weight_in_grams
const float CALIBRATION_FACTOR = 420.0;

HX711 scale;

void setup() {
  Serial.begin(115200);
  pinMode(TARE_BTN, INPUT_PULLUP);

  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare(); // reset to 0 at startup

  Serial.println("Precision Scale Ready.");
  Serial.println("Place items on scale. Press button to Tare.");
}

void loop() {
  // Tare if button is pressed
  if (digitalRead(TARE_BTN) == LOW) {
    scale.tare();
    Serial.println("Scale tared (zeroed).");
    delay(500); // simple debounce
  }

  // Read average of 5 samples for stability
  float weight = scale.get_units(5);
  if (weight < 0) weight = 0; // clamp negatives

  Serial.print("Weight: ");
  Serial.print(weight, 1);
  Serial.println(" g");

  delay(200);
}
