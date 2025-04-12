#include <Arduino.h>
#include "sensors.h"
#include "motorControl.h"
#include "Settings.h"
#include "WebServerHandler.h"

Settings settings; // Create an instance of the Settings class
MotorControl motorControl; // Create an instance of the MotorControl class

float voltage = 0.0; // Variable to store voltage reading
float current = 0.0; // Variable to store current reading
int thrust = 0; // Variable to store thrust reading
float mAh = 0.0; // Variable to store accumulated mAh

// Task handles
TaskHandle_t core0TaskHandle = NULL;
TaskHandle_t core1TaskHandle = NULL;

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

    // Initialize pins
    pinMode(VOLTAGE_SENSOR_PIN, INPUT); // Set voltage sensor pin as input  
    pinMode(CURRENT_SENSOR_PIN, INPUT); // Set current sensor pin as input

    // Initialize settings, weight sensor, WiFi, and web server
    // initWeightSensor(HX711_DT_PIN, HX711_SCK_PIN); // Initialize weight sensor
    settings.loadSettings();
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
}

void loop() {
    // The loop function can remain empty as tasks are running on both cores
}