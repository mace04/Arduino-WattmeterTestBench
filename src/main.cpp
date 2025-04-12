#include <Arduino.h>
#include "sensors.h"
#include "motorControl.h"
#include "Settings.h"
#include "WebServerHandler.h"

Settings settings; // Create an instance of the Settings class
MotorControl motorControl; // Create an instance of the MotorControl class


void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.printf("Flash size: %i MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    // Serial.printf("Flash chip ID: 0x%s", String(ESP.getFlashChipId(), HEX).c_str());
    // Add your setup code here

    pinMode(VOLTAGE_SENSOR_PIN, INPUT); // Set voltage sensor pin as input  
    pinMode(CURRENT_SENSOR_PIN, INPUT); // Set current sensor pin as input
    // resetWeightSensor(); // Initialize weight sensor
    settings.loadSettings(); // Initialize settings
    initWiFi(settings.getSSID().c_str(), settings.getPassword().c_str()); // Initialize WiFi
    initWebServer(settings); // Initialize web server    
    Serial.println("Setup complete. Ready to read sensors and control motor.");

}

void loop() {
    // Add your main code here
    // This will run repeatedly
    handleWebServer();
    delay(50); // Wait for 1 second
}