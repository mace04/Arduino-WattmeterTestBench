#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <SD.H>
#include "sensors.h"
#include "motorControl.h"
#include "Settings.h"
#include "WebServerHandler.h"
#include "tft_config.h"
#include "TftMainMenu.h"

void changeScreen(TftScreenMode newScreen);

Settings settings; // Create an instance of the Settings class
MotorControl motorControl; // Create an instance of the MotorControl class
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TftMainMenu mainMenu(tft, ts, changeScreen); // Create an instance of the TftMainMenu class

float voltage = 0.0; // Variable to store voltage reading
float current = 0.0; // Variable to store current reading
int thrust = 0; // Variable to store thrust reading
float mAh = 0.0; // Variable to store accumulated mAh

// Task handles
TaskHandle_t core0TaskHandle = NULL;
TaskHandle_t core1TaskHandle = NULL;

// Callback function to handle screen changes
void changeScreen(TftScreenMode newScreen) {

    switch (newScreen) {
        case MAIN_MENU:
            Serial.println("Switching to Main Menu Screen");
            // Call the init method of the Manual Test class
            mainMenu.init();
            break;

        case MANUAL_TEST:
            Serial.println("Switching to Manual Test Screen");
            // Call the init method of the Manual Test class
            // manualTest.init();
            break;

        case AUTO_TEST:
            Serial.println("Switching to Auto Test Screen");
            // Call the init method of the Auto Test class
            // autoTest.init();
            break;

        case ABOUT:
            Serial.println("Switching to About Screen");
            // Call the init method of the About class
            // aboutScreen.init();
            break;

        default:
            Serial.println("Unknown screen mode");
            break;
    }
}

// Task running on Core 0
void core0Task(void *parameter) {
    unsigned long lastTime = millis(); // Track the last time the mAh calculation was updated

    while (true) {
        handleWebServer(); // Handle web server requests

        if (motorControl.isRunning()) {
            voltage = readVoltageSensor(); // Read voltage sensor
            current = readCurrentSensor(); // Read current sensor
            thrust = readWeightSensor(); // Read thrust sensor

            // Calculate mAh
            unsigned long currentTime = millis();
            float elapsedTimeHours = (currentTime - lastTime) / 3600000.0; // Convert elapsed time to hours
            mAh += current * elapsedTimeHours; // Accumulate mAh
            lastTime = currentTime; // Update the last time
        }

        delay(10); // Delay to prevent task starvation
    }
}

// Task running on Core 1
void core1Task(void *parameter) {
    while (true) {
        // Add your sensor reading or motor control logic here
        // Example: motorControl.update();
        delay(50); // Delay to prevent task starvation
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.printf("Flash size: %i MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.println();

    tft.reset();
    uint16_t id = tft.readID();
    Serial.print("TFT Device ID: 0x");
    Serial.println(id, HEX);
    tft.begin(id); // Initialize the TFT display with the correct ID
    tft.setRotation(1);     //Landscape
    tft.invertDisplay(true);
    Serial.println("TFT Device Initialised");

    pinMode(17, OUTPUT); // WR
    pinMode(21, OUTPUT); // RD
    digitalWrite(17, LOW); // Force WR signal
tft.fillScreen(TFT_RED);
delay(1000);

    // Initialize settings, weight sensor, WiFi, and web server
    // initWeightSensor(HX711_DT_PIN, HX711_SCK_PIN); // Initialize weight sensor
    settings.loadSettings();
    motorControl.init(); // Initialize motor control
    initSensors(); // Initialize sensors
    initWiFi(settings.getSSID().c_str(), settings.getPassword().c_str());
    initWebServer(settings);

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

    Serial.println("Setup complete. Tasks started on both cores.");

    mainMenu.init(); // Initialize the TFT main menu
}

void loop() {
    // The loop function can remain empty as tasks are running on both cores
}