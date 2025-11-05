#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trig, uint8_t echo)
    : trigPin(trig), echoPin(echo)
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

double UltrasonicSensor::readDistanceCM()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    double duration = pulseIn(echoPin, HIGH, 30000); // >= 30 ms timeout
    if (duration == 0)
        return -1;
    return duration * 0.034 / 2;
}

double UltrasonicSensor::applySquareCalibration(double distance)
{
    const double a = 0.0005;
    const double b = 1.0191;
    const double c = -0.9427;
    return a * distance * distance + b * distance + c;
}
