#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <qrcode.h>

// --- Network & ThingsBoard Configuration ---
const char* WIFI_SSID = "Wokwi-GUEST"; // Standard Wokwi WiFi
const char* WIFI_PASSWORD = "";
const char* TB_SERVER = "eu.thingsboard.cloud";
const int   TB_PORT = 1883;
const char* TB_TOKEN = "KQYXoDYomNDMJ29qdH0k"; // Terminal 1 Access Token

// --- Global Objects ---
WiFiClient espClient;
PubSubClient client(espClient);
TFT_eSPI tft = TFT_eSPI(); 
QRCode qrcode;

unsigned long lastTokenUpdate = 0;
const unsigned long TOKEN_INTERVAL = 5000; // 5 seconds
String currentToken = "";

void setup_wifi() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.println("Connecting WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }
    tft.println("\nWiFi Connected!");
}

void displayDynamicQR(String token) {
    tft.fillScreen(TFT_WHITE);
    
    // Generate QR Code data
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, 0, token.c_str());

    // Calculate scale and offset to center the QR code on ILI9341 (320x240)
    int scale = 8;
    int offsetX = (tft.width() - (qrcode.size * scale)) / 2;
    int offsetY = ((tft.height() - (qrcode.size * scale)) / 2) - 10;

    // Draw the QR Code
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(offsetX + (x * scale), offsetY + (y * scale), scale, scale, TFT_BLACK);
            }
        }
    }

    // Draw the text token below the QR code for debug
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(offsetX, offsetY + (qrcode.size * scale) + 10);
    tft.print("Token: ");
    tft.print(token);
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

    tft.init();
    tft.setRotation(1); // Landscape

    setup_wifi();
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
        
        // Display on screen
        displayDynamicQR(currentToken);
        
        // Sync the active token to ThingsBoard Client Attributes!
        String payload = "{\"active_token\":\"" + currentToken + "\"}";
        client.publish("v1/devices/me/attributes", payload.c_str());
    }
}
