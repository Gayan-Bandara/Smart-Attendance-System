# ⚡ Hardware & Firmware Documentation

This page covers the hardware specifications, pinout wiring, and microcontroller firmware architecture for the **Physical Attendance Terminal**.

---

## 1. Bill of Materials (BOM)

| Component | Specification | Quantity | Purpose |
|---|---|---|---|
| **Microcontroller** | ESP32 DevKit V1 (30-pin or 38-pin, ESP-WROOM-32) | 1 | IoT Controller, Wi-Fi, QR Generation |
| **Display** | ILI9341 2.8" or 2.4" SPI TFT (240x320) | 1 | Renders dynamic QR codes for scanning |
| **Power Supply** | Micro-USB Cable (5V, 1A minimum) | 1 | Powers ESP32 and display |
| **Jumper Wires** | Female-to-Male Dupont wires | 9 | Breadboard / terminal interconnects |

---

## 2. Hardware Wiring & Pinout Table

The ILI9341 display communicates with the ESP32 using the high-speed **Hardware VSPI** bus:

```
 ESP32 DevKit                  ILI9341 TFT Display
┌─────────────┐               ┌───────────────────┐
│         3V3 ├───────────────┤ VCC               │
│         GND ├───────────────┤ GND               │
│      GPIO15 ├───────────────┤ CS (Chip Select)  │
│       GPIO4 ├───────────────┤ RESET / RST       │
│       GPIO2 ├───────────────┤ DC (Data/Command) │
│      GPIO23 ├───────────────┤ MOSI / SDI        │
│      GPIO18 ├───────────────┤ SCK / CLK         │
│         3V3 ├───────────────┤ LED / Backlight   │
│      GPIO19 ├───────────────┤ MISO / SDO        │
└─────────────┘               └───────────────────┘
```

> [!TIP]
> Connect the display `LED` pin directly to `3V3` (or via a 100Ω resistor) for constant maximum brightness so mobile cameras can easily focus on the QR code under varying room lighting conditions.

---

## 3. Firmware Architecture (`esp32_firmware.ino`)

The firmware runs an autonomous loop that does not rely on any incoming server polling:

```mermaid
flowchart TD
    A["Boot & Setup"] --> B["Connect to Wi-Fi"]
    B --> C["Connect to ThingsBoard MQTT<br>(eu.thingsboard.cloud:1883)"]
    C --> D{"Every 5000 ms"}
    D -- No --> E["client.loop() (Maintain MQTT)"]
    D -- Yes --> F["Generate random 8-digit token<br>random(10000000, 99999999)"]
    F --> G["Publish to ThingsBoard Client Attributes<br>{\"active_token\": token}"]
    G --> H["Encode QR Code Matrix using esp_qrcode_generate"]
    H --> I["Render centered QR code on ILI9341 TFT"]
    I --> E
```

### Key Highlights of the Firmware:
1. **Native ESP-IDF QR Code Generation:**
   Instead of third-party libraries that consume excessive SRAM or conflict with newer ESP32 Arduino Core versions, the firmware leverages the native `esp_qrcode_generate()` engine.
2. **Clean Display (QR Only):**
   The raw token text is completely hidden from the TFT display screen, ensuring students cannot manually copy or read the number; they *must* physically scan the QR code with their camera.
3. **Non-Blocking `millis()` Execution:**
   The firmware avoids long blocking `delay()` calls, ensuring the MQTT connection stays alive without dropping packets.
4. **Autonomous Generation:**
   The ESP32 does not wait for ThingsBoard to tell it to change the token. It rotates the token independently and pushes the update via MQTT topic `v1/devices/me/attributes`.

---

## 4. Arduino IDE Setup & Configuration

1. Install **Arduino IDE** (v2.0 or newer).
2. Add ESP32 Board URL in **Preferences**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Install **ESP32 by Espressif Systems** via Board Manager.
4. Install Required Libraries via Library Manager:
   * **`Adafruit GFX Library`**
   * **`Adafruit ILI9341`**
   * **`PubSubClient`** (by Nick O'Leary)
5. Select Board: **`ESP32 Dev Module`** (or `DOIT ESP32 DEVKIT V1`).
6. Set Flash Frequency: `80MHz`, Upload Speed: `921600`.
