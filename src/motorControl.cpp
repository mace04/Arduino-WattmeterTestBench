#include "motorControl.h"

MotorControl::MotorControl() {
    throttleCut = false; // Initialize throttle cut to false
    pinMode(THROTTLE_CONTROL_PIN, INPUT);
    pinMode(ESC_OUTPUT_PIN, OUTPUT);

    // Configure PWM for ESC output
    ledcSetup(0, 50, 8); // Channel 0, 50 Hz frequency, 8-bit resolution
    ledcAttachPin(ESC_OUTPUT_PIN, 0); // Attach ESC output pin to channel 0
}

void MotorControl::setThrottleCut(bool cut) {
    throttleCut = cut;
}

int MotorControl::setThrottle() {
    int throttleValue = analogRead(THROTTLE_CONTROL_PIN); // Read throttle control value
    int throttlePercent = map(throttleValue, 0, 4095, 0, 100); // Convert to percentage

    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        ledcWrite(0, 0); // Assuming channel 0 for PWM
        return 0;
    } else {
        // Set ESC output based on throttle percentage
        int pwmValue = map(throttlePercent, 0, 100, 0, 255); // Map to PWM range
        ledcWrite(0, pwmValue); // Assuming channel 0 for PWM
        return throttlePercent;
    }
}

int MotorControl::setThrottle(int percent) {
    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        ledcWrite(0, 0); // Assuming channel 0 for PWM
        return 0;
    } else {
        // Set ESC output based on provided percentage
        int pwmValue = map(percent, 0, 100, 0, 255); // Map to PWM range
        ledcWrite(0, pwmValue); // Assuming channel 0 for PWM
        return percent;
    }
}

bool MotorControl::getThrottleCut() {
    return throttleCut;
}