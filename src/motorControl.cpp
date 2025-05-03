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
}

void MotorControl::init(Settings& settings){
    pinMode(THROTTLE_CONTROL_PIN, INPUT);
    pinMode(THROTTLE_CUT_PIN, INPUT_PULLUP); // Configure throttle cut pin with pull-up resistor
    pinMode(ESC_OUTPUT_PIN, OUTPUT);

    warmDuration = settings.getTestWarmDuration(); // Duration for throttle ramp-up/down (in seconds)
    phaseDuration = settings.getTestPhaseDuration(); // Duration for holding throttle levels (in seconds)


    // Configure PWM for ESC output
    ledcSetup(0, 50, 8); // Channel 0, 50 Hz frequency, 8-bit resolution
    ledcAttachPin(ESC_OUTPUT_PIN, 0); // Attach ESC output pin to channel 0

    // Attach interrupt to THROTTLE_CONTROL_PIN
    attachInterrupt(THROTTLE_CONTROL_PIN, throttleISR, CHANGE);

    // Attach interrupt to THROTTLE_CUT_PIN
    attachInterrupt(THROTTLE_CUT_PIN, throttleCutISR, FALLING); // Trigger on falling edge
}

void MotorControl::reset() {
    // Reset throttle cut
    setThrottleCut(false);

    // Reset throttle to 0
    setThrottle(0);

    // Reset running state
    setRunning(false);

    // Reset ESC output
    ledcWrite(0, 0); // Assuming channel 0 for PWM

    if (getRunningMode() == AUTO) {
        state = 0; // State machine to track the current phase
        stateStartTime = 0; // Start time for the current phase
        stateCurrentThrottle = 0; // Current throttle percentage
    }

    resetTimer(); // Assuming resetTimer() is defined elsewhere

    Serial.println("MotorControl has been reset to default state.");
}

bool MotorControl::startManual(String& error) {
    if (throttleCut) {
        error = "Cannot start motor: Throttle cut is enabled.";
        return false;
    }

    if (isThrottle()) {
        error = "Cannot start motor: Throttle is not 0.";
        return false;
    }

    // Set running mode to MANUAL
    setRunningMode(MANUAL);

    // Set running state to true
    setRunning(true);

    // Initialize throttle to 0
    setThrottle(0);

    // Start Timer
    startTimer(); // Assuming startTimer() is defined elsewhere

    Serial.println("Motor started in MANUAL mode.");

    return true;
}

bool MotorControl::startAuto(String& error) {
    if (throttleCut) {
        error = "Cannot start motor: Throttle cut is enabled.";
        return false;
    }
    // Set running mode to MANUAL
    setRunningMode(AUTO);

    warmDuration = settings.getTestWarmDuration(); // Duration for throttle ramp-up/down (in seconds)
    phaseDuration = settings.getTestPhaseDuration(); // Duration for holding throttle levels (in seconds)

    // Set running state to true
    setRunning(true);

    // Initialize throttle to 0
    setThrottle(0);

    // Start Timer
    startTimer(); // Assuming startTimer() is defined elsewhere

    state = 0; // State machine to track the current phase
    stateStartTime = 0; // Start time for the current phase
    stateCurrentThrottle = 0; // Current throttle percentage

    Serial.println("Motor started in AUTO mode.");

    return true;
}

void MotorControl::handleAutoTest() {
    unsigned long currentTime = millis(); // Get the current time

    switch (state) {
        case 0: // Ramp up from 0% to 50%
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (stateCurrentThrottle < 50) {
                int elapsed = currentTime - stateStartTime;
                stateCurrentThrottle = map(elapsed, 0, warmDuration * 1000, 0, 50); // Calculate throttle percentage
                setThrottle(stateCurrentThrottle);
            } else {
                stateCurrentThrottle = 50;
                setThrottle(stateCurrentThrottle);
                stateStartTime = 0; // Reset start time
                state = 1; // Move to the next state
            }
            break;

        case 1: // Hold at 50%
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (currentTime - stateStartTime >= phaseDuration * 1000) {
                stateStartTime = 0; // Reset start time
                state = 2; // Move to the next state
            }
            break;

        case 2: // Ramp up from 50% to 100%
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (stateCurrentThrottle < 100) {
                int elapsed = currentTime - stateStartTime;
                stateCurrentThrottle = map(elapsed, 0, warmDuration * 1000, 50, 100); // Calculate throttle percentage
                setThrottle(stateCurrentThrottle);
            } else {
                stateCurrentThrottle = 100;
                setThrottle(stateCurrentThrottle);
                stateStartTime = 0; // Reset start time
                state = 3; // Move to the next state
            }
            break;

        case 3: // Hold at 100%
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (currentTime - stateStartTime >= phaseDuration * 1000) {
                stateStartTime = 0; // Reset start time
                state = 4; // Move to the next state
            }
            break;

        case 4: // Ramp down from 100% to 0%
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (stateCurrentThrottle > 0) {
                int elapsed = currentTime - stateStartTime;
                stateCurrentThrottle = map(elapsed, 0, warmDuration * 1000, 100, 0); // Calculate throttle percentage
                setThrottle(stateCurrentThrottle);
            } else {
                stateCurrentThrottle = 0;
                setThrottle(stateCurrentThrottle);
                stateStartTime = 0; // Reset start time
                state = 5; // Move to the next state
            }
            break;

        case 5: // Stop the motor
            stop();
            state = 0; // Reset state machine for the next test
            break;
    }
}

void MotorControl::stop() {
    // Set throttle to 0
    setThrottle(0);

    // Set running state to false
    setRunning(false);
    pauseTimer(); // Assuming pauseTimer() is defined elsewhere
    if (getRunningMode() == AUTO) {
        state = 0; // State machine to track the current phase
        stateStartTime = 0; // Start time for the current phase
        stateCurrentThrottle = 0; // Current throttle percentage
    }

    // Reset ESC output
    ledcWrite(0, 0); // Assuming channel 0 for PWM

    Serial.println("Motor has been stopped.");
}

bool MotorControl::isThrottle() {
    int throttleValue = analogRead(THROTTLE_CONTROL_PIN); // Read the throttle pin value
    Serial.println("Throttle pin value: " + String(throttleValue)); // Debugging output
    return throttleValue > 0; // Return true if throttle is not 0, false otherwise
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

int MotorControl::getThrottlePercent() {
    int throttlePercent;
    if (getRunningMode() == AUTO){
        throttlePercent = map(ledcRead(0), 0, 255, 0, 100);
    }
    else{
        int throttleValue = analogRead(THROTTLE_CONTROL_PIN); // Read throttle control value
        throttlePercent = map(throttleValue, 0, 4095, 0, 100); // Convert to percentage
    }
    return throttlePercent; // Return the throttle percentage
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