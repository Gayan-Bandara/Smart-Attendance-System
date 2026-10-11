#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <qrcode.h>
#include <ArduinoJson.h>

// --- Network & ThingsBoard Configuration ---
// Wokwi provides a simulated WiFi network named "Wokwi-GUEST" with no password
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* TB_SERVER = "demo.thingsboard.io";
const int   TB_PORT = 1883;
const char* TB_TOKEN = "KQYXoDYomNDMJ29qdH0k"; // Your ThingsBoard Access Token

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI(); 
QRCode qrcode;

// --- Web Portal Base URL ---
const String BASE_URL = "https://smart-attendance-system-c3110.web.app/?token=";

void setup_wifi() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.println("Connecting WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }
    tft.println("\nWiFi Connected!");
}

void displayDynamicQR(String token) {
    String full_url = BASE_URL + token;
    tft.fillScreen(TFT_WHITE);

    // Version 3 QR code supports up to 55 alphanumeric characters
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, 0, full_url.c_str());

    // Calculate dynamic scaling and centering
    int scale = min(tft.width() / qrcode.size, tft.height() / qrcode.size);
    int offsetX = (tft.width() - (qrcode.size * scale)) / 2;
    int offsetY = (tft.height() - (qrcode.size * scale)) / 2;

    // Render the matrix
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(offsetX + (x * scale), offsetY + (y * scale), scale, scale, TFT_BLACK);
            }
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to String
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    // Parse ThingsBoard JSON (assumes payload is {"token":"YOUR_DYNAMIC_STRING"})
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (!error) {
        if (doc.containsKey("token")) {
            String newToken = doc["token"].as<String>();
            displayDynamicQR(newToken);
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect("ESP32_Attendance_Node", TB_TOKEN, NULL)) {
            // Subscribe to ThingsBoard Shared Attributes to receive rotating tokens
            client.subscribe("v1/devices/me/attributes");
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Initialize Display
    tft.init();
    tft.setRotation(1); // Landscape mode

    setup_wifi();

    // Initialize MQTT
    client.setServer(TB_SERVER, TB_PORT);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
