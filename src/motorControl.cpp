#include "motorControl.h"

// Global instance of MotorControl
extern MotorControl motorControl;


MotorControl::MotorControl() {
    throttleCut = false; // Initialize throttle cut to false
    autoTestEnabled = false; // Initialize auto test enabled to false
    running = false; // Initialize running to false
    runningMode = MANUAL; // Default running mode is MANUAL
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
}

void MotorControl::init(Settings& settings){
    pinMode(THROTTLE_CUT_PIN, INPUT_PULLUP); // Configure throttle cut pin with pull-up resistor

    warmDuration = settings.getTestWarmDuration(); // Duration for throttle ramp-up/down (in seconds)
    phaseDuration = settings.getTestPhaseDuration(); // Duration for holding throttle levels (in seconds)

    escControl.setPeriodHertz(50);    // standard 50 hz servo
    escControl.attach(ESC_OUTPUT_PIN, MIN_THROTTLE, MAX_THROTTLE); // attaches the servo on pin 18 to the servo object
    escControl.writeMicroseconds(MIN_THROTTLE); // Set initial ESC signal to minimum throttle
    Serial.println("ESC Microseconds: " + String(escControl.readMicroseconds())); // Debugging output
    sendDebugEvent("ESC Microseconds: " + String(escControl.readMicroseconds()));
}

void MotorControl::reset() {
    // Reset throttle cut
    setThrottleCut(false);

    // Reset throttle to 0
    setThrottle(0);

    // Reset running state
    setRunning(false);

    // Reset ESC output
    escControl.writeMicroseconds(MIN_THROTTLE); // Set initial ESC signal to minimum throttle

    if (getRunningMode() == AUTO) {
        state = 0; // State machine to track the current phase
        stateStartTime = 0; // Start time for the current phase
        stateCurrentThrottle = 0; // Current throttle percentage
    }

    resetTimer(); // Assuming resetTimer() is defined elsewhere

    Serial.println("MotorControl has been reset to default state.");
    sendDebugEvent("MotorControl has been reset to default state.");
}

bool MotorControl::startManual(String& error, bool calibrate) {
    if (throttleCut) {
        error = "Cannot start motor. Throttle cut is enabled.";
        return false;
    }

    if (isThrottle()) {
        error = "Cannot start motor. Throttle is not 0.";
        return false;
    }

    // Set running mode to MANUAL
    setRunningMode(MANUAL);

    // Set running state to true
    setRunning(!calibrate);

    // Initialize throttle to 0
    setThrottle(0);

    // Start Timer
    startTimer(); // Assuming startTimer() is defined elsewhere

    Serial.println("Motor started in MANUAL mode.");
    sendDebugEvent("Motor started in MANUAL mode.");

    return true;
}

bool MotorControl::startAuto(String& error) {
    if (throttleCut) {
        error = "Cannot start motor. Throttle cut is enabled.";
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
    sendDebugEvent("Motor started in AUTO mode.");

    return true;
}

void MotorControl::handleControls(){
    if(digitalRead(THROTTLE_CUT_PIN) == LOW){
        setThrottle(0);       // Set throttle to 0
        setThrottleCut(true); // Set throttleCut to true
        setRunning(false);    // Set running to false    
        Serial.println("Throttle cut activated! Motor stopped.");
        sendDebugEvent("Throttle cut activated! Motor stopped.");
    }
    static int lastThrottle = 0;
    if (!getThrottleCut() && getRunningMode() == MotorControl::MANUAL) {
        setThrottle();
        // motorControl.setRunning(true); // Set running to true
        if (lastThrottle != getThrottlePercent() && isRunning()) {
            lastThrottle = getThrottlePercent();
            Serial.println("Throttle change: " + String(lastThrottle) + "%");
            sendDebugEvent("Throttle change: " + String(lastThrottle) + "%");
        }
    }
}

void MotorControl::handleAutoTest() {
    unsigned long currentTime = millis(); // Get the current time

    switch (state) {
        case 0: // Ramp up from 0% to 50%
            static bool stage0Logged = false;
            if (!stage0Logged) {
                Serial.println("Auto Test State 0: Ramping up to 50%");
                sendDebugEvent("Auto Test State 0: Ramping up to 50%");
                stage0Logged = true;
            }
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
            static bool stage1Logged = false;
            if (!stage1Logged) {
                Serial.println("Auto Test State 1: Holding at 50%");
                sendDebugEvent("Auto Test State 1: Holding at 50%");
                stage1Logged = true;
            }
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (currentTime - stateStartTime >= phaseDuration * 1000) {
                stateStartTime = 0; // Reset start time
                state = 2; // Move to the next state
            }
            break;

        case 2: // Ramp up from 50% to 100%
            static bool stage2Logged = false;
            if (!stage2Logged) {    
                Serial.println("Auto Test State 2: Ramping up to 100%");
                sendDebugEvent("Auto Test State 2: Ramping up to 100%");
                stage2Logged = true;
            }
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
            static bool stage3Logged = false;
            if (!stage3Logged) {    
                Serial.println("Auto Test State 3: Holding at 100%");
                sendDebugEvent("Auto Test State 3: Holding at 100%");
                stage3Logged = true;
            }
            if (stateStartTime == 0) stateStartTime = currentTime; // Initialize start time
            if (currentTime - stateStartTime >= phaseDuration * 1000) {
                stateStartTime = 0; // Reset start time
                state = 4; // Move to the next state
            }
            break;

        case 4: // Ramp down from 100% to 0%
            static bool stage4Logged = false;
            if (!stage4Logged) {
                Serial.println("Auto Test State 4: Ramping down to 0%");
                sendDebugEvent("Auto Test State 4: Ramping down to 0%");
                stage4Logged = true;
            }
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
    escControl.writeMicroseconds(MIN_THROTTLE); // Set initial ESC signal to minimum throttle

    Serial.println("Motor has been stopped.");
    sendDebugEvent("Motor has been stopped.");
}

bool MotorControl::isThrottle() {
    int throttleValue = adc1_get_raw(ADC_THROTTLE_SENSOR_CHANNEL); // Read the throttle pin value
    Serial.println("Throttle pin value: " + String(throttleValue)); // Debugging output
    sendDebugEvent("Throttle pin value: " + String(throttleValue));
    return throttleValue > 0; // Return true if throttle is not 0, false otherwise
}

void MotorControl::setThrottleCut(bool cut) {
    throttleCut = cut;
    if (cut) {
        setRunning(false); // Stop running if throttle cut is enabled
    }
}

int MotorControl::setThrottle() {
    int throttleValue = adc1_get_raw(ADC_THROTTLE_SENSOR_CHANNEL); // Read throttle control value
    int throttlePercent = map(throttleValue, 0, 4095, 0, 100); // Convert to percentage

    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        escControl.writeMicroseconds(MIN_THROTTLE); // Set ESC to minimum throttle
        setRunning(false); // Stop running
    } else if (isRunning() && getRunningMode() == MANUAL) {
        // Set ESC output based on throttle percentage
        int pwmValue = map(throttleValue, 0, 4095, MIN_THROTTLE, MAX_THROTTLE); // Map to PWM range
        escControl.writeMicroseconds(pwmValue); 
        return throttlePercent;
    }
    return 0; // Return 0 if not running or throttle cut is enabled
}

int MotorControl::getThrottlePercent() {
    // Serial.println("ESC Microseconds: " + String(escControl.readMicroseconds())); // Debugging output
    return map(escControl.readMicroseconds(), MIN_THROTTLE, MAX_THROTTLE, 0, 100); // Return the throttle percentage
}

int MotorControl::setThrottle(int percent) {

    if (throttleCut) {
        // If throttle cut is set, set ESC to no throttle
        escControl.writeMicroseconds(MIN_THROTTLE); // Set ESC to minimum throttle
        setRunning(false); // Stop running
    } else if (isRunning() && getRunningMode() == AUTO) {
        // Set ESC output based on throttle percentage
        int pwmValue = map(percent, 0, 100, MIN_THROTTLE, MAX_THROTTLE); // Map to PWM range
        escControl.writeMicroseconds(pwmValue); 
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