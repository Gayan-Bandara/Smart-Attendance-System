#include <Keypad.h>
#include <HX711.h>
#include <AccelStepper.h>

// ---------------------------------------------------------
// 1. Human-Machine Interface (Keypad Setup)
// ---------------------------------------------------------
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// ESP32 Pin Connections for Keypad
byte rowPins[ROWS] = {19, 18, 5, 17}; 
byte colPins[COLS] = {16, 4, 2, 15};  

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String inputString = "";
float targetWeight = 0.0;

// ---------------------------------------------------------
// 2. Precision Scale (HX711 Setup)
// ---------------------------------------------------------
const int HX711_dout = 22;
const int HX711_sck = 23;
const int tareButtonPin = 33;
HX711 scale;

// Replace this with your load cell's specific calibration factor
float calibration_factor = 2280.0; 

// ---------------------------------------------------------
// 3. Actuator (Stepper Motor Setup)
// ---------------------------------------------------------
const int dirPin = 14;
const int stepPin = 12;
// Using A4988 or DRV8825 driver (Type = 1)
AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

// Assuming 200 steps per revolution (1.8 degree step angle)
// 90 degrees = 50 steps. Adjust based on your microstepping settings.
const int STEPS_90_DEG = 50; 

// ---------------------------------------------------------
// 4. Indicator LEDs
// ---------------------------------------------------------
const int greenLED = 27;
const int redLED = 26;

// State management for package detection
bool packageOnScale = false;
const float MIN_WEIGHT_THRESHOLD = 15.0; // Minimum weight (grams) to trigger a scan

void setup() {
  Serial.begin(115200);
  
  // Initialize LEDs and Tare Button
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(tareButtonPin, INPUT_PULLUP);
  
  // Initialize Scale
  scale.begin(HX711_dout, HX711_sck);
  scale.set_scale(calibration_factor);
  scale.tare(); // Auto-tare on startup
  Serial.println("System Ready. Enter target weight and press #.");

  // Initialize Stepper Motor
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
}

void loop() {
  // --- Check Tare Button ---
  if (digitalRead(tareButtonPin) == LOW) {
    scale.tare();
    Serial.println("Scale Zeroed (Tared).");
    delay(500); // Debounce
  }

  // --- Keypad Input Logic ---
  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      inputString = ""; // Clear input
      Serial.println("Screen Cleared.");
    } 
    else if (key == '#') {
      targetWeight = inputString.toFloat(); // Save target weight
      Serial.print("Target Weight Saved: ");
      Serial.print(targetWeight);
      Serial.println(" g");
      inputString = ""; // Reset for next input
    } 
    else if (isDigit(key)) {
      inputString += key;
      Serial.print("Input: ");
      Serial.println(inputString);
    }
  }

  // --- Sorting Logic ---
  // Only process if a target weight has been set
  if (targetWeight > 0) {
    float currentWeight = scale.get_units(5); // Average of 5 readings

    // Detect if a package is placed on the scale
    if (currentWeight > MIN_WEIGHT_THRESHOLD && !packageOnScale) {
      packageOnScale = true;
      Serial.println("Package detected. Evaluating...");
      
      // Wait a moment for the scale to stabilize
      delay(1000);
      currentWeight = scale.get_units(10);
      
      Serial.print("Measured Weight: ");
      Serial.print(currentWeight);
      Serial.println(" g");

      // Calculate 5% tolerance
      float lowerBound = targetWeight * 0.95;
      float upperBound = targetWeight * 1.05;

      if (currentWeight >= lowerBound && currentWeight <= upperBound) {
        // ACCEPT
        Serial.println("Status: ACCEPT");
        flashLED(greenLED);
        kickPackage(STEPS_90_DEG); // Kick Left
      } else {
        // REJECT
        Serial.println("Status: REJECT");
        flashLED(redLED);
        kickPackage(-STEPS_90_DEG); // Kick Right
      }
    } 
    // Reset system once the package is removed from the scale
    else if (currentWeight < MIN_WEIGHT_THRESHOLD && packageOnScale) {
      packageOnScale = false;
      Serial.println("Scale cleared. Ready for next package.");
    }
  }
}

// ---------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------

// Function to smoothly rotate the stepper to a position and return
void kickPackage(int targetPosition) {
  // Move to bin
  stepper.moveTo(targetPosition);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  
  delay(500); // Pause briefly at the bin
  
  // Return to center (0 position)
  stepper.moveTo(0);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

// Function to rapidly flash an LED
void flashLED(int pin) {
  for (int i = 0; i < 5; i++) {
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
}