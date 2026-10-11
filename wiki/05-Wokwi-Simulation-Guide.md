# 💻 Wokwi Simulation Guide

This guide explains how to simulate all hardware circuits and firmware directly in the browser using the **Wokwi IoT Simulator** without needing physical hardware.

---

## 1. How Wokwi Works

Every simulated exercise in this repository includes three standalone files:
1. **`sketch.ino`**: The microcontroller source code.
2. **`diagram.json`**: The Wokwi circuit diagram, component placements, and pin wiring.
3. **`libraries.txt`**: Third-party Arduino libraries installed in Wokwi.

---

## 2. General 4-Step Simulation Procedure

1. Open **[wokwi.com](https://wokwi.com)** in your web browser.
2. Click **Start from Scratch** → Select **ESP32**.
3. Copy and paste the contents into their corresponding tabs in Wokwi:
   * Paste the code from `sketch.ino` into the **`sketch.ino`** tab.
   * Paste the JSON from `diagram.json` into the **`diagram.json`** tab.
   * Go to the **Library Manager** tab (book icon) → Add the libraries listed in **`libraries.txt`**.
4. Click the green ▶️ **Start Simulation** button.

---

## 3. Simulating the Smart Attendance Terminal

* **Directory:** `wokwi_simulation/`
* **Components:** ESP32 DevKit V4, ILI9341 SPI TFT (`wokwi-ili9341`).
* **Libraries:**
  ```
  Adafruit GFX Library
  Adafruit ILI9341
  PubSubClient
  qrcode
  ```

> [!NOTE]
> In real hardware, we use the ESP-IDF native `esp_qrcode_generate()`. In Wokwi simulation, Wokwi uses Richard Moore's `qrcode.h` library to render the matrix onto `Adafruit_ILI9341`.

### Verifying MQTT Connection in Wokwi
Wokwi provides a simulated public Wi-Fi gateway (`Wokwi-GUEST` with no password). The simulated ESP32 automatically connects to `eu.thingsboard.cloud:1883`, publishes the rotating `active_token`, and renders the QR code on the virtual screen!

---

## 4. HTML Browser Simulator (`simulate_esp32.html`)

For instant testing without opening Wokwi or wiring an ESP32:
1. Open `simulate_esp32.html` directly in your web browser.
2. It generates a new 8-digit token every 5 seconds, renders an SVG QR code on your computer monitor, and pushes the `active_token` attribute directly to ThingsBoard Cloud via HTTP.
3. Point your smartphone camera at your computer screen using the Firebase Web App to verify the entire attendance flow!
