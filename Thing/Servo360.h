#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class Servo360 {
public:
    Servo360(uint8_t pin);

    void begin();
    void update();
    void setInterval(uint8_t newInterval);
    Servo servo;
private:
    bool pausing;             // true if is paused
    unsigned long pauseTime;  // ms betheew direction changes
    uint8_t pin;
    uint8_t interval;       // [0, 1, 2]
    int dir;                // 1 = Clockwise, -1 = CounterClockwise
    unsigned long lastOsc;
    unsigned long oscTime;  // time for ~(180 degress) or ~(-180 degress)
    int pulse;              // pulse for speed and direction

    // Pulses 
    const int stopPulse = 1500;
    const int slowPulseClock = 1000;
    const int fastPulseClock = 500;
    const int slowPulseCClock = 2000;
    const int fastPulseCClock = 2500;

    void applyPulse();
};
