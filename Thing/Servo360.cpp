#include "Servo360.h"

Servo360::Servo360(uint8_t pin)
  : pin(pin),
    interval(2),
    dir(1),
    lastOsc(0),
    oscTime(2000), // 2 seconds pause when boot
    pauseTime(100),    // 100 ms default pause
    pausing(false),
    pulse(stopPulse) {}

void Servo360::begin() {
    servo.attach(pin, 500, 2500);
    servo.writeMicroseconds(stopPulse);
    lastOsc = millis();
}

void Servo360::setInterval(uint8_t newInterval) {
    if (newInterval > 2) newInterval = 2;
    interval = newInterval;

    switch (interval) {
        case 0: oscTime = 200; break; // rápido
        case 1: oscTime = 320; break; // lento
        case 2: oscTime = 0;   break; // detener
    }

    // Reiniciar estado para aplicar el cambio inmediatamente
    pausing = false;
    lastOsc = millis();
    applyPulse(); // aplica el pulso correcto inmediatamente
}

void Servo360::applyPulse() {
    switch (interval) {
        case 0:
            pulse = (dir == 1) ? fastPulseClock : fastPulseCClock;
            break;
        case 1:
            pulse = (dir == 1) ? slowPulseClock : slowPulseCClock;
            break;
        case 2:
        default:
            pulse = stopPulse;
            break;
    }
    servo.writeMicroseconds(pulse);
}

void Servo360::update() {
    unsigned long now = millis();

    if (interval == 2) {
        // Detenido
        servo.writeMicroseconds(stopPulse);
        pausing = false;
        return;
    }

    // Si está en pausa, espera a que termine antes de continuar
    if (pausing) {
        if (now - lastOsc >= pauseTime) {
            pausing = false;
            lastOsc = now;
        } else {
            servo.writeMicroseconds(stopPulse); // mantener detenido
            return;
        }
    }

    // Oscilación normal
    if (oscTime > 0 && now - lastOsc >= oscTime) {
        dir *= -1;          // cambia dirección
        pausing = true;     // activa pausa
        lastOsc = now;
    }

    applyPulse();
}
