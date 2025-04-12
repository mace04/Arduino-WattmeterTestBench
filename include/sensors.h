#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// GPIO pin definitions
#define VOLTAGE_SENSOR_PIN 34
#define CURRENT_SENSOR_PIN 35
#define HX711_DT_PIN 22
#define HX711_SCK_PIN 01

// Voltage divider constants
const float R1 = 15000.0; // 15K ohms
const float R2 = 2200.0;  // 2.2K ohms
const float VOLTAGE_DIVIDER_RATIO = R2 / (R1 + R2);

// Running average parameters
const int AVERAGE_WINDOW_SIZE = 10;

// Function prototypes
float readVoltageSensor();
float readCurrentSensor();
void resetWeightSensor();
float readWeightSensor();

#endif // SENSORS_H