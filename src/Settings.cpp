#include "Settings.h"

Settings::Settings() {
    // Initialize default values
    ssid = "";
    password = "";
    voltsPerPointVoltage = 0.12790;
    voltageOffset = 0.00;
    voltsPerPointCurrent = 0.02;
    currentOffset = -2.5;
    thrustScale = 139.00;
    thrustOffset = 0.00;
    maxCurrent = 100;
    maxThrust = 4900;
    testPhaseDuration = 15;
    testWarmDuration = 5; // Assuming testWarmDuration is defined
}

void Settings::loadSettings() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }
    readFromSPIFFS();
}
  
void Settings::saveSettings() {
    writeToSPIFFS();
    readFromSPIFFS(); // Read back to verify
}

void Settings::readFromSPIFFS() {
    File file = SPIFFS.open("/settings.json", "r");
    if (!file) {
        Serial.println("Failed to open settings file for reading");
        return;
    }

    size_t size = file.size();
    if (size > 1024) {
        Serial.println("Settings file size is too large");
        return;
    }

    std::unique_ptr<char[]> buf(new char[size]);
    file.readBytes(buf.get(), size);

    JsonDocument doc;
    auto error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse settings file");
        return;
    }

    ssid = doc["ssid"] | "";
    password = doc["password"] | "";
    voltsPerPointVoltage = doc["voltsPerPointVoltage"] | 0.12790;
    voltageOffset = doc["voltageOffset"] | 0.00;
    voltsPerPointCurrent = doc["voltsPerPointCurrent"] | 0.02;
    currentOffset = doc["currentOffset"] | -2.5;
    thrustScale = doc["thrustScale"] | 139.00;
    thrustOffset = doc["thrustOffset"] | 0.00;
    maxCurrent = doc["maxCurrent"] | 100;
    maxThrust = doc["maxThrust"] | 4900;
    testPhaseDuration = doc["testPhaseDuration"] | 45; 
    testWarmDuration = doc["testWarmDuration"] | 15; // Assuming testWarmDuration is defined

    file.close();
}

void Settings::writeToSPIFFS() {
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["voltsPerPointVoltage"] = voltsPerPointVoltage;
    doc["voltageOffset"] = voltageOffset;
    doc["voltsPerPointCurrent"] = voltsPerPointCurrent;
    doc["currentOffset"] = currentOffset;
    doc["thrustScale"] = thrustScale;
    doc["thrustOffset"] = thrustOffset;
    doc["maxCurrent"] = maxCurrent;
    doc["maxThrust"] = maxThrust;
    doc["testPhaseDuration"] = testPhaseDuration;
    doc["testWarmDuration"] = testWarmDuration; // Assuming testWarmDuration is defined
    File file = SPIFFS.open("/settings.json", "w");
    if (!file) {
        Serial.println("Failed to open settings file for writing");
        return;
    }

    serializeJson(doc, file);
    file.close();
}

// Getter methods
String Settings::getSSID() { return ssid; }
String Settings::getPassword() { return password; }
float Settings::getVoltsPerPointVoltage() { return voltsPerPointVoltage; }
double Settings::getVoltageOffset() { return voltageOffset; }
float Settings::getVoltsPerPointCurrent() { return voltsPerPointCurrent; }
double Settings::getCurrentOffset() { return currentOffset; }
float Settings::getThrustScale() { return thrustScale; }
double Settings::getThrustOffset() { return thrustOffset; }
int Settings::getMaxCurrent() { return maxCurrent; }
int Settings::getMaxThrust() { return maxThrust; }
int Settings::getTestPhaseDuration() { return testPhaseDuration; }
int Settings::getTestWarmDuration() { return testWarmDuration; } // Assuming testWarmDuration is defined

// Setter methods
void Settings::setSSID(const String& newSSID) { ssid = newSSID; }
void Settings::setPassword(const String& newPassword) { password = newPassword; }
void Settings::setVoltsPerPointVoltage(double value) { voltsPerPointVoltage = value; }
void Settings::setVoltageOffset(double value) { voltageOffset = value; }
void Settings::setVoltsPerPointCurrent(double value) { voltsPerPointCurrent = value; }
void Settings::setCurrentOffset(double value) { currentOffset = value; }
void Settings::setThrustScale(double value) { thrustOffset = value; }
void Settings::setThrustOffset(double value) { thrustOffset = value; }
void Settings::setMaxCurrent(int value) { maxCurrent = value; }
void Settings::setMaxThrust(int value) { maxThrust = value; }
void Settings::setTestPhaseDuration(int value) { testPhaseDuration = value; }
void Settings::setTestWarmDuration(int value) { testWarmDuration = value; }