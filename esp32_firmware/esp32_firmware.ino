#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <qrcode.h>
#include <ArduinoJson.h>

// --- Network & ThingsBoard Configuration ---
const char* WIFI_SSID = "GAYAN 4464";
const char* WIFI_PASSWORD = "gggggggg";
const char* TB_SERVER = "eu.thingsboard.cloud"; // Changed to EU cloud server
const int   TB_PORT = 1883;
const char* TB_TOKEN = "KQYXoDYomNDMJ29qdH0k"; // Your ThingsBoard Access Token

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI(); 

unsigned long lastTokenUpdate = 0;
const unsigned long TOKEN_INTERVAL = 5000; // 5 seconds
String currentToken = "";

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

// Custom display function called automatically by ESP-IDF QR generator
void renderQR(esp_qrcode_handle_t qrcode) {
    int size = esp_qrcode_get_size(qrcode);
    int scale = min(tft.width() / size, tft.height() / size);
    int offsetX = (tft.width() - (size * scale)) / 2;
    int offsetY = (tft.height() - (size * scale)) / 2;

    // Render the matrix
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (esp_qrcode_get_module(qrcode, x, y)) {
                tft.fillRect(offsetX + (x * scale), offsetY + (y * scale), scale, scale, TFT_BLACK);
            }
        }
    }
}

void displayDynamicQR(String token) {
    tft.fillScreen(TFT_WHITE);

    // Use ESP32's built-in QR generator!
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    cfg.display_func = renderQR;
    cfg.max_qrcode_version = 4;
    
    // Only encode the raw 8-character token, NO URL!
    esp_qrcode_generate(&cfg, token.c_str());
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect("ESP32_Attendance_Node", TB_TOKEN, NULL)) {
            Serial.println("Connected to ThingsBoard");
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(0));

    // Initialize Display
    tft.init();
    tft.setRotation(1); // Landscape mode

    setup_wifi();

    // Initialize MQTT
    client.setServer(TB_SERVER, TB_PORT);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Generate new token every 5 seconds
    if (millis() - lastTokenUpdate > TOKEN_INTERVAL) {
        lastTokenUpdate = millis();
        
        // Generate random 8 digit token
        long randomToken = random(10000000, 99999999);
        currentToken = String(randomToken);
        
        Serial.print("New Token: ");
        Serial.println(currentToken);
        
        // Update physical screen
        displayDynamicQR(currentToken);
        
        // Push the new active token to ThingsBoard device attributes!
        String payload = "{\"active_token\":\"" + currentToken + "\"}";
        client.publish("v1/devices/me/attributes", payload.c_str());
    }
}
