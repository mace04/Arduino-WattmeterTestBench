#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "sensors.h"
#include "Settings.h"
#include "WebServerHandler.h"   

// Pin definitions
#define THROTTLE_CONTROL_PIN 36 // Pin for throttle control input
#define ESC_OUTPUT_PIN 21 // Pin for ESC output
#define THROTTLE_CUT_PIN 22 // Pin for throttle cut switch
#define MIN_THROTTLE 1000 // Minimum throttle value for ESC
#define MAX_THROTTLE 2000 // Maximum throttle value for ESC
#define ADC_THROTTLE_SENSOR_CHANNEL ADC1_CHANNEL_0  // e.g., GPIO36

class MotorControl {
public:
    enum RunningMode { MANUAL, AUTO }; // Define running modes

    // Constructor
    MotorControl();

    // Functions
    void init(Settings& settings); // Initialize the motor control
    void reset(); // Method to reset the motor control state
    bool startManual(String& error, bool calibrate = false); // Method to start the motor in manual mode
    bool startAuto(String& error);
    void handleControls(); // Method to handle manual controls
    void handleAutoTest(); // Method to handle auto test
    void stop(); // Method to stop the motor
    
    void setThrottleCut(bool cut);
    int setThrottle(); // Reads throttle control from pin
    int setThrottle(int percent); // Sets throttle using a parameter
    int getThrottlePercent(); // Get the throttle percentage
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
    Settings settings;
    int state; // State machine to track the current phase
    unsigned long stateStartTime; // Start time for the current phase
    int stateCurrentThrottle; // Current throttle percentage
    int warmDuration; // Duration for throttle ramp-up/down (in seconds)
    int phaseDuration; // Duration for holding throttle levels (in seconds)
    Servo escControl; // Servo object for ESC control


    bool isThrottle(); // Method to determine if the throttle pin is at 0 or not
};

#endif // MOTORCONTROL_H