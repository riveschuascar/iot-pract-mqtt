#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WifiManager.h"

class MqttManager {
public:
    // This project uses broker-side certificate handling via username/password.
    // No explicit CA is provided by default; the client will use setInsecure().
    MqttManager(const char* ssid, const char* password , const char* broker, uint16_t port,
                const char* mqttUser = nullptr, const char* mqttPassword = nullptr);
    void begin();
    void loop();
    void setCallback(MQTT_CALLBACK_SIGNATURE);
    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);

private:
    void connectToBroker();

    WifiManager wifiManager;
    // secure client must be declared before PubSubClient so it's initialized first
    WiFiClientSecure secureClient;
    PubSubClient mqttClient;
    const char* mqttBroker;
    uint16_t mqttPort;
    const char* mqttUser;
    const char* mqttPassword;
    unsigned long lastReconnectAttempt;
};
