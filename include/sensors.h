#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// GPIO pin definitions
#define VOLTAGE_SENSOR_PIN 34
#define CURRENT_SENSOR_PIN 39
#define HX711_DT_PIN 32
#define HX711_SCK_PIN 33

// Running average parameters
const int AVERAGE_WINDOW_SIZE = 10;

// Function prototypes
void initSensors();
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