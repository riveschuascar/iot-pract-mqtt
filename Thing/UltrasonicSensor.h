#pragma once
#include <Arduino.h>

class UltrasonicSensor
{
public:
    UltrasonicSensor(uint8_t trig, uint8_t echo);

    double readDistanceCM();
    double applySquareCalibration(double distance);

private:
    uint8_t trigPin;
    uint8_t echoPin;
};
