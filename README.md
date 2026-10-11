# Smart Attendance System

A highly secure, IoT-based Smart Attendance System utilizing an ESP32, ThingsBoard, and Firebase. This system completely replaces traditional attendance sheets with a dynamic, time-rotating QR code system that is immune to cheating (e.g., sharing screenshots of old QR codes).

## System Architecture

The system operates using three core components that communicate in real-time:

### 1. Hardware Terminal (ESP32)
The physical terminal is powered by an ESP32 microcontroller and a TFT display (e.g., ILI9341).
* **Autonomous Token Generation:** Every 5 seconds, the ESP32 generates a completely new 8-digit random token offline.
* **Dynamic QR Code:** It uses the native ESP-IDF QR code library to render the token into a QR code on the screen.
* **ThingsBoard Sync:** Instantly publishes the new token to ThingsBoard over MQTT as a Client Attribute (`active_token`), keeping the cloud perfectly synchronized with the physical screen.

*(Note: A software simulator is also available in the `wokwi_simulation` and `simulate_esp32.html` files for testing without physical hardware).*

### 2. Student Web App (Firebase Hosting)
A mobile-friendly web application used by students to check into class.
* **Google Authentication:** Students log in seamlessly using their Google accounts via Firebase Auth, eliminating typos and ensuring verified identities.
* **CORS-Free Architecture:** The app natively pulls available courses directly from Firebase Realtime Database.
* **No-CORS Telemetry:** To bypass strict mobile browser security tracking restrictions, the app uses a clever `text/plain` no-cors trick to blind-POST the scanned `qr_token`, `course`, `student_email`, and `student_name` directly to the ThingsBoard Telemetry API.
* **Real-time Verification:** After scanning, the app enters a polling loop, watching the Firebase Realtime Database for a confirmation ticket from ThingsBoard before showing the student a green success screen.

### 3. Cloud Backend (ThingsBoard & Firebase)
* **ThingsBoard Rule Engine:** The brain of the operation. When a telemetry scan arrives from a student's phone, the rule engine fetches the live `active_token` from the terminal's device attributes. A strict JavaScript transformation script compares the scanned token to the live token.
* **Anti-Cheating:** If the tokens mismatch (meaning the student scanned an old screenshot), ThingsBoard silently drops the request.
* **Data Logging & Confirmation:** If the tokens match, ThingsBoard saves the attendance to the Timeseries database (viewable in the Admin Dashboard) and triggers a REST API Call node. This node pushes a `{"status":"success"}` ticket to the Firebase Realtime Database to instantly notify the student's waiting web app.

---

## Setup & Deployment Guide

### 1. Firebase Configuration
1. Go to your Firebase Console.
2. Enable **Authentication** and turn on the **Google** Sign-in provider.
3. Enable **Realtime Database** and set the rules to allow public read/write (for course syncing and confirmations).
4. Run `firebase deploy` in the root of this project to push the `public/index.html` web app live to Firebase Hosting.

### 2. ThingsBoard Configuration
1. **Device Provisioning:** Create a device named `Attendance Terminal 1` and copy its Access Token.
2. **Rule Engine:** 
   * Configure an `Originator Attributes` node to fetch `active_token` from Client Attributes into metadata.
   * Add a `Transformation - script` node to strictly compare `msg.qr_token === metadata.cs_active_token`.
   * On success, route to a `Save Timeseries` node.
   * Route the success output of that to a `REST API Call` node targeting `https://<YOUR-FIREBASE-PROJECT>-default-rtdb.firebaseio.com/confirmations/$[qr_token].json` with a `PUT` request containing `{"status":"success"}`.
3. **Admin Dashboard:** Create a dashboard featuring a "Timeseries table" pointing to the terminal device to view the raw log of attendees over time.

### 3. ESP32 Hardware Setup
1. Open the `esp32_firmware/esp32_firmware.ino` file in the Arduino IDE.
2. Update the `WIFI_SSID` and `WIFI_PASSWORD`.
3. Update the `TB_TOKEN` with your ThingsBoard terminal's access token.
4. Flash the code to your ESP32 connected to an SPI TFT screen.

## Project Structure
* `/public` - Firebase Hosting files (Student Web App).
* `/esp32_firmware` - The physical C++ firmware for the ESP32 hardware terminal.
* `/wokwi_simulation` - Configuration files to run the hardware terminal virtually in a browser at wokwi.com.
* `simulate_esp32.html` - A lightweight HTML/JS version of the hardware terminal for fast local testing.
* `simulate_esp32.py` - A lightweight Python version of the hardware terminal.
* `*.json` - Various exported widget and dashboard configurations for ThingsBoard.
