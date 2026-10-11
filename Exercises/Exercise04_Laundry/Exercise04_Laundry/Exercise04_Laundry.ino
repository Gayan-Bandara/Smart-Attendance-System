/*
  Exercise 04 — Industrial Laundry Service (Edge Node)
  IIoT-Batch02 | Week 04

  Components:
  - DHT22 → motor temperature
  - Vibration sensor (SW-420) → detects machine running/stopped
  - LED → machine ON/OFF indicator (controlled locally + via ThingsBoard RPC)
  - Potentiometer → simulates RPM sensor (0–4095 mapped to 0–1800 RPM)

  Edge Node Responsibilities:
  - Read all sensors
  - Publish telemetry to ThingsBoard every 5s
  - Receive RPC command from ThingsBoard dashboard "Power Button" to toggle LED

  ThingsBoard Dashboard Tasks (do in ThingsBoard UI):
  ✅ Power Button widget  → setLEDState RPC → controls LED remotely
  ✅ HTML Value Card      → styled temperature indicator (red if > 60°C)
  ✅ Background image     → laundry machine image
  ✅ Timeseries Table     → view historical telemetry
  ✅ Explore Advanced tab on each widget for custom colors/thresholds

  GPIO Pin Map:
  ┌──────────────────┬──────┐
  │ DHT22 SDA        │ 15   │
  │ Vibration (SW420)│ 34   │
  │ RPM Pot (AO)     │ 35   │
  │ LED (Machine)    │ 26   │
  └──────────────────┴──────┘
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ─── Configuration — EDIT THESE ──────────────────────────────────────────────
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* TB_SERVER     = "eu.thingsboard.cloud";
const int   TB_PORT       = 1883;
const char* TB_TOKEN      = "YOUR_ACCESS_TOKEN_HERE";

// ─── Pin Definitions ─────────────────────────────────────────────────────────
#define DHT_PIN       15
#define VIBRATION_PIN 34  // Digital: HIGH = vibrating (running)
#define RPM_PIN       35  // Analog 0-4095 → mapped to 0-1800 RPM
#define LED_PIN       26  // Machine power indicator

// ─── Globals ─────────────────────────────────────────────────────────────────
DHT dht(DHT_PIN, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);

bool machine_on     = false;
unsigned long lastSend = 0;
const long SEND_INTERVAL = 5000;

// ─── RPC Callback: Power Button from ThingsBoard toggles the LED ──────────────
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("RPC: " + msg);

  if (msg.indexOf("setLEDState") != -1) {
    machine_on = (msg.indexOf("true") != -1);
    digitalWrite(LED_PIN, machine_on ? HIGH : LOW);
    Serial.println(machine_on ? "Machine ON (remote)" : "Machine OFF (remote)");
  }
}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    Serial.println("WiFi connected.");
  }
  while (!client.connected()) {
    if (client.connect("LaundryNode", TB_TOKEN, NULL)) {
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("ThingsBoard connected.");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, INPUT);

  client.setServer(TB_SERVER, TB_PORT);
  client.setCallback(mqttCallback);
  reconnect();

  Serial.println("Industrial Laundry Node Ready.");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    bool  vibrating = digitalRead(VIBRATION_PIN);
    int   rawRPM    = analogRead(RPM_PIN);
    int   rpm       = map(rawRPM, 0, 4095, 0, 1800);

    if (isnan(temp)) temp = 0;
    if (isnan(hum))  hum  = 0;

    char payload[256];
    snprintf(payload, sizeof(payload),
      "{\"temperature\":%.1f,\"humidity\":%.1f,\"rpm\":%d,\"vibration\":%s,\"machine_on\":%s}",
      temp, hum, rpm,
      vibrating   ? "true" : "false",
      machine_on  ? "true" : "false");

    client.publish("v1/devices/me/telemetry", payload);
    Serial.println("Sent: " + String(payload));
  }
}
