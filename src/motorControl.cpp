#include "motorControl.h"

// Global instance of MotorControl
extern MotorControl motorControl;

// Interrupt Service Routine (ISR) for throttle cut pin
void IRAM_ATTR throttleCutISR() {
    motorControl.setThrottle(0);       // Set throttle to 0
    motorControl.setThrottleCut(true); // Set throttleCut to true
    motorControl.setRunning(false);    // Set running to false
}

// Interrupt Service Routine (ISR) for throttle control pin
void IRAM_ATTR throttleISR() {
    if (!motorControl.getThrottleCut() && motorControl.getRunningMode() == MotorControl::MANUAL) {
        motorControl.setThrottle();
        motorControl.setRunning(true); // Set running to true
    }
}

MotorControl::MotorControl() {
    throttleCut = false; // Initialize throttle cut to false
    autoTestEnabled = false; // Initialize auto test enabled to false
    running = false; // Initialize running to false
    runningMode = MANUAL; // Default running mode is MANUAL
    pinMode(THROTTLE_CONTROL_PIN, INPUT);
    pinMode(THROTTLE_CUT_PIN, INPUT_PULLUP); // Configure throttle cut pin with pull-up resistor
    pinMode(ESC_OUTPUT_PIN, OUTPUT);

    // Configure PWM for ESC output
    ledcSetup(0, 50, 8); // Channel 0, 50 Hz frequency, 8-bit resolution
    ledcAttachPin(ESC_OUTPUT_PIN, 0); // Attach ESC output pin to channel 0

    // Attach interrupt to THROTTLE_CONTROL_PIN
    attachInterrupt(THROTTLE_CONTROL_PIN, throttleISR, CHANGE);

    // Attach interrupt to THROTTLE_CUT_PIN
    attachInterrupt(THROTTLE_CUT_PIN, throttleCutISR, FALLING); // Trigger on falling edge
}

void MotorControl::setThrottleCut(bool cut) {
    throttleCut = cut;
    if (cut) {
        setRunning(false); // Stop running if throttle cut is enabled
    }
}

int MotorControl::setThrottle() {
    int throttleValue = analogRead(THROTTLE_CONTROL_PIN); // Read throttle control value
    int throttlePercent = map(throttleValue, 0, 4095, 0, 100); // Convert to percentage

    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        ledcWrite(0, 0); // Assuming channel 0 for PWM
        setRunning(false); // Stop running
    } else if (isRunning() && getRunningMode() == MANUAL) {
        // Set ESC output based on throttle percentage
        int pwmValue = map(throttlePercent, 0, 100, 0, 255); // Map to PWM range
        ledcWrite(0, pwmValue); // Assuming channel 0 for PWM
        return throttlePercent;
    }
    return 0; // Return 0 if not running or throttle cut is enabled
}

int MotorControl::setThrottle(int percent) {

    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        ledcWrite(0, 0); // Assuming channel 0 for PWM
        setRunning(false); // Stop running
    } else if (isRunning() && getRunningMode() == AUTO) {
        // Set ESC output based on throttle percentage
        int pwmValue = map(percent, 0, 100, 0, 255); // Map to PWM range
        ledcWrite(0, pwmValue); // Assuming channel 0 for PWM
        return percent;
    }
    return 0; // Return 0 if not running or throttle cut is enabled
}


bool MotorControl::getThrottleCut() {
    return throttleCut;
}

bool MotorControl::isAutoTestEnabled() {
    return autoTestEnabled;
}

void MotorControl::setAutoTestEnabled(bool enabled) {
    autoTestEnabled = enabled;
}

bool MotorControl::isRunning() {
    return running;
}

void MotorControl::setRunning(bool runningState) {
    running = runningState;
}

MotorControl::RunningMode MotorControl::getRunningMode() {
    return runningMode;
}

void MotorControl::setRunningMode(RunningMode mode) {
    runningMode = mode;
}