#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// Pin definitions
#define THROTTLE_CONTROL_PIN 39 // Pin for throttle control input
#define ESC_OUTPUT_PIN 3 // Pin for ESC output
#define THROTTLE_CUT_PIN 0 // Pin for throttle cut switch

class MotorControl {
public:
    enum RunningMode { MANUAL, AUTO }; // Define running modes

    // Constructor
    MotorControl();

    // Functions
    void setThrottleCut(bool cut);
    int setThrottle(); // Reads throttle control from pin
    int setThrottle(int percent); // Sets throttle using a parameter
    bool getThrottleCut();
    bool isAutoTestEnabled(); // Check if auto test is enabled
    void setAutoTestEnabled(bool enabled); // Set auto test enabled or disabled

    // Methods for running state
    bool isRunning(); // Check if the motor is running
    void setRunning(bool running); // Set the running state

    // Methods for running mode
    RunningMode getRunningMode(); // Get the current running mode
    void setRunningMode(RunningMode mode); // Set the running mode

private:
    bool throttleCut; // Global variable for throttle cut
    bool autoTestEnabled; // Global variable for auto test enabled
    bool running; // Member variable to track running state
    RunningMode runningMode; // Variable to store the current running mode
};

#endif // MOTORCONTROL_H