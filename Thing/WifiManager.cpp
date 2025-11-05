#include "WifiManager.h"

WifiManager::WifiManager(const char* ssid, const char* password)
    : ssid(ssid), password(password) {}

void WifiManager::connect() {
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);

    const unsigned long timeout = 10000;
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < timeout) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected with IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nConnection timed out. Could not connect to Wi-Fi.");
    }
}

WiFiClient& WifiManager::getClient() {
    return wifiClient;
}
