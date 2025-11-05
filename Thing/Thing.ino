#include <Arduino.h>
#include "MqttManager.h"
#include "Servo360.h" 
#include"UltrasonicSensor.h"

// Wifi Connection
const char *ssid = "TIGO RIVERO";
const char *password = "a36bb1e335c28";
// MQTT Broker
const char *mqttBrokerURL = "c877f40f18e34e858972f14b8a14d0aa.s1.eu.hivemq.cloud";
const uint16_t mqttPort = 8883;
const char *mqttUserName = "mqtt-thing";
const char *mqttUserPassword = "N9'9qQr_;f9TEr*";
// Topics
const char *sensorTopic = "sensors/ultrasonic/distance";
const char *actuatorTopic = "actuators/servo/state";
// Thing Pinout 
const uint8_t pwdPin = 27;
const uint8_t trigPin = 26;
const uint8_t echoPin = 25;
// Forward-declare MQTT callback and pass it to MqttManager so the class registers it
void mqttCallback(char *topic, byte *payload, unsigned int length);
// Required Classes
// Pass MQTT username and password so MqttManager can use TLS with broker auth
MqttManager mqtt(ssid, password, mqttBrokerURL, mqttPort, mqttUserName, mqttUserPassword, mqttCallback);
Servo360 servo(pwdPin); UltrasonicSensor sensor(trigPin, echoPin);
// Publish rate interval
unsigned long lastPublishMillis = 0;
const unsigned long publishIntervalMs = 5000; // 5 seconds
// Auto-stop servo after no commands received
unsigned long lastServoCommandMillis = 0;
const unsigned long servoTimeoutMs = 5000;
bool servoActive = false; // Track if servo is running (not stopped)

void setup()
{
  Serial.begin(115200);
  mqtt.begin();
  mqtt.subscribe(actuatorTopic);
  servo.begin();
}

void loop()
{
  mqtt.loop();
  servo.update();

  // Autostop based on innactivity
  if (servoActive && (millis() - lastServoCommandMillis >= servoTimeoutMs))
  {
    servo.setInterval(0);
    servoActive = false;
    mqtt.publish(actuatorTopic, "0");
    Serial.println("Servo auto-stopped after inactivity");
  }

  // Publish read distance every 5 sec
  unsigned long now = millis();
  if (now - lastPublishMillis >= publishIntervalMs)
  {
    lastPublishMillis = now;
    double distance = sensor.readDistanceCM();
    double calibratedDistance = sensor.applySquareCalibration(distance);

    char payload[32];
    snprintf(payload, sizeof(payload), "%.4f", calibratedDistance);
    mqtt.publish(sensorTopic, payload);
  }
}

// Callback method
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  char msgBuf[32];
  if (length >= sizeof(msgBuf))
    length = sizeof(msgBuf) - 1;
  memcpy(msgBuf, payload, length);
  msgBuf[length] = '\0';

  Serial.print("MQTT msg on ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msgBuf);

  int val = atoi(msgBuf);
  val = constrain(val, 0, 2);

  servo.setInterval((uint8_t)val);

  if (val == 0)
  {
    servoActive = false;
  }
  else
  {
    servoActive = true;
    lastServoCommandMillis = millis();
  }
}
