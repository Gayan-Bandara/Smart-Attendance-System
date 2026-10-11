# 🧪 IIoT Batch 02 Lab Exercises

This document contains complete documentation, wiring diagrams, and source code breakdowns for all lab exercises from **IIoT Batch 02**.

---

## Exercise 01: Smart Proximity Alarm System

* **Folder:** `Exercises/Exercise01_Proximity_Alarm/`
* **Objective:** Industrial safety alarm with multi-tier distance alerts and automated barrier gate.
* **Hardware:** HC-SR04 Ultrasonic Sensor, Red LED, Active Buzzer, SG90 Servo Motor.

### Zone Logic & Thresholds:
| Distance Zone | Distance Range | LED Indicator | Buzzer Alert | Servo Barrier Gate |
|---|---|---|---|---|
| **DANGER** | `< 10 cm` | Rapid flashing (100ms) | Fast continuous beeping | Gate closed (`0°`) |
| **WARNING** | `10 cm – 30 cm` | Slow flashing (300ms) | Intermittent beeping | Gate half open (`90°`) |
| **SAFE** | `> 30 cm` | OFF | Silent | Gate fully open (`180°`) |

### Pinout:
* `Trig`: GPIO13 | `Echo`: GPIO12
* `LED`: GPIO14 | `Buzzer`: GPIO27 | `Servo PWM`: GPIO26

---

## Exercise 02: The Master Sorter System

* **Folder:** `Exercises/Exercise02_Master_Sorter/`
* **Objective:** Automated industrial quality control system that weighs incoming items, evaluates tolerance, and sorts them using an actuator.

### Sub-Modules:
1. **Ex02a (Keypad HMI):** 4x4 Membrane Keypad + LCD I2C 16x2.
   * `*` Clears input.
   * `#` Confirms and locks target weight.
2. **Ex02b (Precision Scale):** HX711 Load Cell Amplifier + Tare Pushbutton.
   * Calibrated with `scale.set_scale(CALIBRATION_FACTOR)`.
   * Auto-tare on boot and manual tare on GPIO33 button press.
3. **Ex02c (The Actuator):** A4988 Driver + Bipolar Stepper Motor.
   * Uses `AccelStepper::DRIVER` on `STEP: GPIO12`, `DIR: GPIO14`.
   * Accelerates smoothly to `+90°` or `-90°` and returns to home (`0°`).
4. **Ex02d (Combined Master Sorter):**
   * Keypad sets target weight.
   * Scale detects package (`weight > 15g`), pauses 1 second to stabilize, and takes a 10-sample average.
   * **Within 5% Tolerance:** Green LED (GPIO27) flashes 5 times; stepper kicks 90° clockwise into **Accept Bin**.
   * **Outside Tolerance:** Red LED (GPIO26) flashes 5 times; stepper kicks 90° counter-clockwise into **Reject Bin**.

---

## Exercise 03: Smart Room Edge Node

* **Folder:** `Exercises/Exercise03_Smart_Room/`
* **Objective:** Smart industrial room monitoring with bi-directional IoT control.
* **Hardware:** DHT22 (Temp & Humidity), Photoresistor / LDR (Ambient light), 2 LEDs, 2 Pushbuttons, ThingsBoard MQTT.

### Features:
1. **Non-blocking `millis()` Telemetry:** Publishes temperature, humidity, light level, and actuator statuses to ThingsBoard every 5 seconds without blocking physical switch inputs.
2. **Software Debouncing:** Clean button state transitions without bouncing glitches.
3. **Remote RPC Control:** Subscribes to `v1/devices/me/rpc/request/+` on ThingsBoard:
   * `setLightState`: Remotely toggles Room Light LED (GPIO26).
   * `setACState`: Remotely toggles AC Indicator LED (GPIO27).

---

## Exercise 04: Industrial Laundry Monitoring Node

* **Folder:** `Exercises/Exercise04_Laundry/`
* **Objective:** Condition monitoring and predictive maintenance for industrial washing machines.
* **Hardware:** DHT22 (Motor temperature), SW-420 (Vibration sensor), Potentiometer (Motor RPM simulator), Indicator LED, ThingsBoard.

### Telemetry Keys:
```json
{
  "temperature": 58.4,
  "humidity": 65.0,
  "rpm": 1200,
  "vibration": true,
  "machine_on": true
}
```

### Edge vs. Cloud Architecture:
* **Edge Responsibilities:** Sensor sampling, signal filtering, RPM mapping (`map(analogRead(35), 0, 4095, 0, 1800)`), telemetry publishing.
* **ThingsBoard Cloud Responsibilities:** Threshold alert rules, historical trend analysis, Power Button RPC widget to remotely shut down equipment during overheating.
