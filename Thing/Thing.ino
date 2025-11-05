#include <Arduino.h>
#include "MqttManager.h"
#include "Servo360.h"
#include "UltrasonicSensor.h"

// Wifi Connection
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// MQTT Broker
const char* mqttBrokerURL = "c877f40f18e34e858972f14b8a14d0aa.s1.eu.hivemq.cloud";
const uint16_t mqttPort = 8883;
const char* mqttUserName = "YR_USRNM";
const char* mqttUserPassword = "YR_USRPSSWRD";

// Topics
const char* sensorTopic = "sensors/ultrasonic/distance";
const char* actuatorTopic = "actuators/servo/state";

// Thing Pinout
const uint8_t pwdPin = 27;
const uint8_t trigPin = 26;
const uint8_t echoPin = 25;

// Required Classes
// Pass MQTT username and password so MqttManager can use TLS with broker auth
MqttManager mqtt(ssid, password, mqttBrokerURL, mqttPort, mqttUserName, mqttUserPassword);
Servo360 servo(pwdPin);
UltrasonicSensor sensor(trigPin, echoPin);

void setup() {
  Serial.begin(115200);
  mqtt.begin();
  mqtt.subscribe(actuatorTopic);
  mqtt.subscribe(sensorTopic);
  servo.begin();
}

void loop() {
  mqtt.loop();
  servo.update();

  double distance = sensor.readDistanceCM();
  double calibratedDistance = sensor.applySquareCalibration(distance);

  char payload[50];
  snprintf(payload, sizeof(payload), "{\"distance_cm\": %.2f}", calibratedDistance);
  mqtt.publish(sensorTopic, payload);

  delay(1000);
}
