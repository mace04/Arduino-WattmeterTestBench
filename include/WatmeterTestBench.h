#include <Arduino.h>

struct Settings {
    int maxCurrent;
    int maxThrust;
    int midTestDuration;
    int maxTestDuration;
//    int testCycles;
    int warmUptime;
    float currentOffset;
    float voltageOffset;
    float thrustOffset;
};

struct WattmeterValues {
    int throttle;
    float voltage;
    float current;
    long power;
    int consumption;
    int thrust;
};

struct AverageValues {
    int samples;
    WattmeterValues values;
};

void pressInterrupt();
void configureCommon();
void configureDistinct();
void buttonPressed(int button);
void toggleScreenMode();
String fixedLength(String str, int len);
void  processMaxValues();
void processAverageValues();
void displayValues(String header, WattmeterValues readings);
void displayAverageValues(String header, AverageValues val);
void settingsValues();
void calibrationValues();
void displayCurrentCutoffError();
void displayThrustCutoffError();
Settings readEepromSettings();
void writeEepromSettings(Settings values);
bool settingsDiff(Settings values);
void displayAutoTestResultMenu();
void displayAutoTestStart();
void displayAutoTestPage1();
void displayAutoTestPage2();
void displayAutoTestEnd();
void printDebug(String string);
void printDebugNewLine();
