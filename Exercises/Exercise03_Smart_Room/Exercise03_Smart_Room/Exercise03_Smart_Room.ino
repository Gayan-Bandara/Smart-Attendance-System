/*
  Exercise 03 — Smart Room Edge Node
  IIoT-Batch02 | Week 03

  Components:
  - DHT22 (Temperature & Humidity)
  - Photoresistor / LDR (Light level)
  - LED 1 on GPIO26 (Room Light)
  - LED 2 on GPIO27 (AC/Fan Indicator)
  - Switch 1 on GPIO32 (Toggle Room Light)
  - Switch 2 on GPIO33 (Toggle AC)

  Features:
  - Physical switch toggling with software debounce
  - Sends telemetry to ThingsBoard every 5s (non-blocking millis)
  - Receives ThingsBoard RPC commands to control LEDs remotely

  GPIO Pin Map:
  ┌─────────────┬──────┐
  │ DHT22 SDA   │ 15   │
  │ LDR AO      │ 34   │
  │ LED 1 (Light│ 26   │
  │ LED 2 (AC)  │ 27   │
  │ Switch 1    │ 32   │
  │ Switch 2    │ 33   │
  └─────────────┴──────┘
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ─── Configuration — EDIT THESE ──────────────────────────────────────────────
const char* WIFI_SSID     = "Wokwi-GUEST";   // Use your WiFi SSID
const char* WIFI_PASSWORD = "";               // Use your WiFi password
const char* TB_SERVER     = "eu.thingsboard.cloud";
const int   TB_PORT       = 1883;
const char* TB_TOKEN      = "YOUR_ACCESS_TOKEN_HERE"; // Paste from ThingsBoard

// ─── Pin Definitions ─────────────────────────────────────────────────────────
#define DHT_PIN   15
#define LDR_PIN   34
#define LED1_PIN  26  // Room Light
#define LED2_PIN  27  // AC Indicator
#define BTN1_PIN  32  // Switch 1
#define BTN2_PIN  33  // Switch 2

// ─── Globals ─────────────────────────────────────────────────────────────────
DHT dht(DHT_PIN, DHT22);
WiFiClient espClient;
PubSubClient client(espClient);

bool light_state = false;
bool ac_state    = false;
bool lastBtn1    = HIGH;
bool lastBtn2    = HIGH;
unsigned long lastSend = 0;
const long SEND_INTERVAL = 5000; // 5 seconds

// ─── MQTT Callback: handle remote control commands from ThingsBoard ───────────
// ThingsBoard sends RPC commands like:
// {"method":"setLightState","params":{"value":true}}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("RPC received: " + msg);

  if (msg.indexOf("setLightState") != -1) {
    light_state = (msg.indexOf("true") != -1);
    digitalWrite(LED1_PIN, light_state ? HIGH : LOW);
    Serial.println(light_state ? "Light ON (remote)" : "Light OFF (remote)");
  }
  if (msg.indexOf("setACState") != -1) {
    ac_state = (msg.indexOf("true") != -1);
    digitalWrite(LED2_PIN, ac_state ? HIGH : LOW);
    Serial.println(ac_state ? "AC ON (remote)" : "AC OFF (remote)");
  }
}

// ─── Connect to WiFi and ThingsBoard MQTT ────────────────────────────────────
void reconnect() {
  // Reconnect WiFi if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println(" Connected!");
  }

  // Reconnect MQTT
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard...");
    if (client.connect("SmartRoom", TB_TOKEN, NULL)) {
      Serial.println(" Connected!");
      // Subscribe to ThingsBoard RPC topic to receive remote commands
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print(" Failed. Retry in 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  // INPUT_PULLUP: button reads HIGH normally, LOW when pressed
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);

  client.setServer(TB_SERVER, TB_PORT);
  client.setCallback(mqttCallback);
  reconnect();

  Serial.println("Smart Room Node Ready.");
}

void loop() {
  // Keep MQTT alive and process incoming messages
  if (!client.connected()) reconnect();
  client.loop();

  // ── Physical Switch Polling (with software debounce) ─────────────────────
  bool btn1 = digitalRead(BTN1_PIN);
  bool btn2 = digitalRead(BTN2_PIN);

  if (btn1 == LOW && lastBtn1 == HIGH) {
    delay(50); // debounce delay
    light_state = !light_state;
    digitalWrite(LED1_PIN, light_state ? HIGH : LOW);
    Serial.println(light_state ? "Light ON (button)" : "Light OFF (button)");
  }
  if (btn2 == LOW && lastBtn2 == HIGH) {
    delay(50);
    ac_state = !ac_state;
    digitalWrite(LED2_PIN, ac_state ? HIGH : LOW);
    Serial.println(ac_state ? "AC ON (button)" : "AC OFF (button)");
  }
  lastBtn1 = btn1;
  lastBtn2 = btn2;

  // ── Non-blocking 5-second telemetry using millis() ───────────────────────
  // Using millis() instead of delay() keeps the switches instantly responsive
  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    int   ldr  = analogRead(LDR_PIN);

    if (isnan(temp) || isnan(hum)) {
      Serial.println("DHT22 read failed — skipping telemetry.");
      return;
    }

    // Build JSON using snprintf (no library needed, keeps code simple)
    char payload[220];
    snprintf(payload, sizeof(payload),
      "{\"temperature\":%.1f,\"humidity\":%.1f,\"light_level\":%d,\"light_status\":%s,\"ac_status\":%s}",
      temp, hum, ldr,
      light_state ? "true" : "false",
      ac_state    ? "true" : "false");

    // Publish to ThingsBoard telemetry topic
    client.publish("v1/devices/me/telemetry", payload);
    Serial.println("Telemetry sent: " + String(payload));
  }
}
