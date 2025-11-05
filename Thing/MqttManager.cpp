#include "MqttManager.h"

MqttManager::MqttManager(const char *ssid, const char *password, const char *broker, uint16_t port,
                                                 const char* mqttUser, const char* mqttPassword)
        : wifiManager(ssid, password),
            secureClient(),
            mqttClient(secureClient),
            mqttBroker(broker),
            mqttPort(port),
            mqttUser(mqttUser),
            mqttPassword(mqttPassword),
            lastReconnectAttempt(0) {}

void MqttManager::begin()
{
    wifiManager.connect();
    // Configure TLS: broker manages certificates via username/password, so
    // we use setInsecure() (no server cert verification) to establish TLS.
    // If you later provide a CA PEM, replace this with setCACert(caPem).
    secureClient.setInsecure();

    mqttClient.setServer(mqttBroker, mqttPort);
    connectToBroker();
}

void MqttManager::connectToBroker() {
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT broker");
        String clientId = "esp32-smart-thing" + String(random(0xffff), HEX);
        bool connected = false;
        if (mqttUser != nullptr) {
            connected = mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword);
        } else {
            connected = mqttClient.connect(clientId.c_str());
        }

        if (connected) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void MqttManager::loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            connectToBroker();
        }
    } else {
        mqttClient.loop();
    }
}

void MqttManager::setCallback(MQTT_CALLBACK_SIGNATURE) {
    mqttClient.setCallback(callback);
}

bool MqttManager::publish(const char* topic, const char* payload) {
    return mqttClient.publish(topic, payload);
}

bool MqttManager::subscribe(const char* topic) {
    return mqttClient.subscribe(topic);
}
