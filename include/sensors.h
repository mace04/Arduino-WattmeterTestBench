#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <HX711.h>

// GPIO pin definitions
#define VOLTAGE_SENSOR_PIN 35   //Voltage Divider - R1:14.98 KOhm, R2:2.14KOhm
#define CURRENT_SENSOR_PIN 34   // Voltage Divider - R1:14.86 KOhm, R2:13.59KOhm
#define HX711_DT_PIN 33
#define HX711_SCK_PIN 25
#define LOADCELL_CALIBRATION 139
#define LOADCELL_OFFSET     0


// Running average parameters
const int AVERAGE_WINDOW_SIZE = 10;

// Function prototypes
void initSensors();
void calibrateWeightSensor();
float readVoltageSensor();
float readCurrentSensor();
void resetWeightSensor();
float readWeightSensor();

// Inline function to calculate power in watts

// Inline function to calculate power in watts
inline float calculatePower(float voltage, float current) {
    return voltage * current; // Power (P) = Voltage (V) * Current (I)
}

// Inline function to calculate and update running consumption in mAh
float calculateConsumption(float current);
void resetConsumption();

void startTimer();
void pauseTimer();
void resetTimer();
String getElapsedTime();


#endif // SENSORS_H