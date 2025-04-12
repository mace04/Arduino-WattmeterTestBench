#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class Settings {
public:
    // Constructor
    Settings();

    // Getter methods
    String getSSID();
    String getPassword();
    float getVoltsPerPointVoltage();
    double getVoltageOffset();
    float getVoltsPerPointCurrent();
    double getCurrentOffset();
    double getThrustOffset();
    int getMaxCurrent();
    int getMaxThrust();
    int getTestPhaseDuration();

    // Setter methods
    void setSSID(const String& ssid);
    void setPassword(const String& password);
    void setVoltsPerPointVoltage(double value);
    void setVoltageOffset(double value);
    void setVoltsPerPointCurrent(double value);
    void setCurrentOffset(double value);
    void setThrustOffset(double value);
    void setMaxCurrent(int value);
    void setMaxThrust(int value);
    void setTestPhaseDuration(int value);

    // Load and save settings
    void loadSettings();
    void saveSettings();

private:
    // Key-value pairs
    String ssid;
    String password;
    float voltsPerPointVoltage;
    double voltageOffset;
    float voltsPerPointCurrent;
    double currentOffset;
    double thrustOffset;
    int maxCurrent;
    int maxThrust;
    int testPhaseDuration;

    // Helper methods
    void readFromSPIFFS();
    void writeToSPIFFS();
};

#endif // SETTINGS_H