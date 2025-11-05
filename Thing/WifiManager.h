#pragma once
#include <WiFi.h>

class WifiManager
{
public:
    WifiManager(const char *ssid, const char *password);
    void connect();

    WiFiClient &getClient();

private:
    const char *ssid;
    const char *password;
    WiFiClient wifiClient;
};
