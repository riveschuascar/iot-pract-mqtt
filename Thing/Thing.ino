#include <Arduino.h>
#include "MqttManager.h"
#include "Servo360.h"
#include "UltrasonicSensor.h"

// Wifi Connection
const char *ssid = "TU_SSID";
const char *password = "TU_PASSWORD";

// MQTT Broker
const char *mqttBrokerURL = "c877f40f18e34e858972f14b8a14d0aa.s1.eu.hivemq.cloud";
const uint16_t mqttPort = 8883;
const char *mqttUserName = "YR_USRNM";
const char *mqttUserPassword = "YR_USRPSSWRD";

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
Servo360 servo(pwdPin);
UltrasonicSensor sensor(trigPin, echoPin);

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
  mqtt.subscribe(sensorTopic);
  mqtt.subscribe(actuatorTopic);
  servo.begin();
}

void loop()
{
  // Handle MQTT communication and servo updates
  mqtt.loop();
  servo.update();

  // Check if servo should auto-stop due to inactivity
  if (servoActive && (millis() - lastServoCommandMillis >= servoTimeoutMs)) {
    servo.setInterval(0); // Stop servo
    servoActive = false;
    
    // Publish stop state to notify other clients
    mqtt.publish(actuatorTopic, "0");
    Serial.println("Servo auto-stopped due to inactivity");
  }

  // Handle periodic sensor reading and publishing
  unsigned long now = millis();
  if (now - lastPublishMillis >= publishIntervalMs)
  {
    lastPublishMillis = now;

    double distance = sensor.readDistanceCM();
    double calibratedDistance = sensor.applySquareCalibration(distance);

    // Publish raw distance value (no JSON)
    char payload[32];
    snprintf(payload, sizeof(payload), "%.4f", calibratedDistance);
    mqtt.publish(sensorTopic, payload);
  }

  // Update last command time and active state
  lastServoCommandMillis = millis();
  servoActive = (val != 0); // Active if not stopped
}

// PubSubClient-style callback: convert payload to null-terminated string and act
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

  // Interpret payload as int command for servo interval (0..2)
  int val = atoi(msgBuf);
  val = constrain(val, 0, 2);
  servo.setInterval((uint8_t)val);
  
  // Update last command time and active state
  lastServoCommandMillis = millis();
  servoActive = (val != 2); // Active if not stopped
}
