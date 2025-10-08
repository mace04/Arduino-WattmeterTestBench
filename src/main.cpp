#include <Arduino.h>
#include <SD.H>
#include "sensors.h"
#include "motorControl.h"
#include "Settings.h"
#include "WebServerHandler.h"
#include "tft_config.h"
#include "TftMainMenu.h"
#include "TftAbout.h"
#include "TftMotorTest.h"
#include <WiFi.h>

void screenChangeCallback(TftScreenMode screenMode);

Settings settings; // Create an instance of the Settings class
MotorControl motorControl; // Create an instance of the MotorControl class
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ); // Touchscreen instance
TftScreenMode currentScreen = MAIN_MENU;
struct Profile testProfile = {"",0,0,0};

float voltage = 0.0; // Variable to store voltage reading
float current = 0.0; // Variable to store current reading
int thrust = 0; // Variable to store thrust reading
int power=0; // Variable to store power reading
float mAh = 0.0; // Variable to store accumulated mAh

// Task handles
TaskHandle_t core0TaskHandle = NULL;
TaskHandle_t core1TaskHandle = NULL;

TftMainMenu tftMainMenu(tft, ts, screenChangeCallback);
TftAbout tftAbout(tft, ts, screenChangeCallback);
TftMotorTest tftMotorTest(tft, ts, screenChangeCallback);

// Callback function to handle screen changes
void screenChangeCallback(TftScreenMode screenMode) {
    currentScreen = screenMode;
    if (screenMode == MAIN_MENU) {
        tftMainMenu.init();
    } else if (screenMode == ABOUT) {
        tftAbout.init();
    } else if (screenMode == MANUAL_TEST){
        tftMotorTest.init(MANUAL);
    }
    else if (screenMode == AUTO_TEST){
        tftMotorTest.init(AUTO);
    }
}

// Task running on Core 0
void core0Task(void *parameter) {
    while (true) {
        handleWebServer(); // Handle web server requests

        if (motorControl.isRunning()) {
            if (motorControl.getRunningMode() == MotorControl::RunningMode::AUTO) {
                motorControl.handleAutoTest(); // Handle auto test
            }
            tftMotorTest.Voltage = readVoltageSensor(); // Read voltage sensor
            tftMotorTest.Current = readCurrentSensor(); // Read current sensor
            tftMotorTest.Thrust = readWeightSensor(); // Read thrust sensor
            tftMotorTest.Power = calculatePower(voltage, current); // Calculate power in watts
            tftMotorTest.Consumption = calculateConsumption(current); // Calculate mAh
            tftMotorTest.Time = getElapsedTime(); // Get elapsed time
            tftMotorTest.ThrottlePercent = motorControl.getThrottlePercent(); // Get throttle percentage
       } else{
                tftMotorTest.Voltage = readVoltageSensor(); // Read voltage sensor
                tftMotorTest.Current = readCurrentSensor(); // Read current sensor
                tftMotorTest.Thrust = readWeightSensor(); // Read thrust sensor
                tftMotorTest.Power = 0;
                tftMotorTest.Consumption = 0;
                tftMotorTest.Time = getElapsedTime(); // Get elapsed time
                tftMotorTest.ThrottlePercent = motorControl.getThrottlePercent(); // Get throttle percentage
                delay(100); // Delay to prevent task starvation
        }
        delay(10); // Delay to prevent task starvation
    }
}

// Task running on Core 1
void core1Task(void *parameter) {
    while (true) {
        if (isCSActive(TOUCH)) {
            if (currentScreen == MAIN_MENU) {
                tftMainMenu.handleTouch();
            } else if (currentScreen == ABOUT) {
                tftAbout.handleTouch();
            } else if (currentScreen == MANUAL_TEST || currentScreen == AUTO_TEST) {
                tftMotorTest.handleTouch();
            } 
        }
        delay(10); // Delay to prevent task starvation
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.printf("Flash size: %i MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println();

    // Initialize CS pins
    pinMode(TFT_CS, OUTPUT);
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    // pinMode(TOUCH_IRQ, INPUT_PULLUP); // Set touch IRQ pin as input with pull-up resistor

    // Set all CS pins to HIGH (disabled)
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);

    // Initialise TFT touch screen
    tft.init();
    tft.setRotation(3); // Adjust as needed

    // Initialize touchscreen
    setCS(PANEL); // Set CS for TFT panel

    tft.fillScreen(TFT_BLACK);
    // Test display colors
    tft.fillScreen(TFT_RED);
    delay(1000);
    tft.fillScreen(TFT_GREEN);
    delay(1000);
    tft.fillScreen(TFT_BLUE);
    delay(1000);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    Serial.println("TFT Device Initialised");

    ts.begin();
    tft.println("Touch Device Initialised");
    Serial.println("Touch Device Initialised");
    ts.setRotation(1);    

    // Initialize settings, weight sensor, WiFi, and web server
    // initWeightSensor(HX711_DT_PIN, HX711_SCK_PIN); // Initialize weight sensor
    settings.loadSettings();
    tft.println("Settings Loaded");
    Serial.println("Settings Loaded");

    motorControl.init(settings); // Initialize motor control
    tft.println("Motor Control Initialised");
    Serial.println("Motor Control Initialised");

    initSensors(); // Initialize sensors
    tft.println("Sensors Initialised");
    Serial.println("Sensors Initialised");

    initWiFi(settings.getSSID().c_str(), settings.getPassword().c_str());
    tft.println("WiFi Initialised");
    Serial.println("WiFi Initialised");

    initWebServer(settings);
    tft.println("Web Server Initialised");
    Serial.println("Web Server Initialised");

    Serial.println("Setup complete. Tasks started on both cores.");
    delay(5000);
    screenChangeCallback(MAIN_MENU); 

    // Create tasks for each core
    xTaskCreatePinnedToCore(
        core0Task,          // Task function
        "Core0Task",        // Name of the task
        10000,              // Stack size (in bytes)
        NULL,               // Task input parameter
        1,                  // Priority of the task
        &core0TaskHandle,   // Task handle
        0                   // Core to run the task on (Core 0)
    );

    xTaskCreatePinnedToCore(
        core1Task,          // Task function
        "Core1Task",        // Name of the task
        10000,              // Stack size (in bytes)
        NULL,               // Task input parameter
        1,                  // Priority of the task
        &core1TaskHandle,   // Task handle
        1                   // Core to run the task on (Core 1)
    );
}

void loop() {
    // The loop function can remain empty as tasks are running on both cores
}