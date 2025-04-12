#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

// Pin definitions
#define THROTTLE_CONTROL_PIN 39
#define ESC_OUTPUT_PIN 3

class MotorControl {
public:
    // Constructor
    MotorControl();

    // Functions
    void setThrottleCut(bool cut);
    int setThrottle(); // Reads throttle control from pin
    int setThrottle(int percent); // Sets throttle using a parameter
    bool getThrottleCut();

private:
    bool throttleCut; // Global variable for throttle cut
};

#endif // MOTORCONTROL_H