#pragma once

#include <Arduino.h>
#include <HX711.h>
#include <esp_adc_cal.h>
#include "Settings.h" // Include Settings.h to use the Settings class
#include "WebServerHandler.h"

// GPIO pin definitions
#define VOLTAGE_SENSOR_PIN 35   //Voltage Divider - R1:14.98 KOhm, R2:2.14KOhm
#define CURRENT_SENSOR_PIN 34   // Voltage Divider - R1:14.86 KOhm, R2:13.59KOhm
#define HX711_DT_PIN 26
#define HX711_SCK_PIN 25
#define LOADCELL_CALIBRATION 139
#define LOADCELL_OFFSET     0

// ADC Parameters
#define DEFAULT_VREF    1100  // mV (used if no eFuse calibration)
#define ADC_WIDTH       ADC_WIDTH_BIT_12
#define ADC_ATTEN       ADC_ATTEN_DB_11
#define ADC_VOLTAGE_SENSOR_CHANNEL ADC1_CHANNEL_7  // e.g., GPIO35
#define ADC_CURRENT_SENSOR_CHANNEL ADC1_CHANNEL_6  // e.g., GPIO34
#define ADC_THTOTTLE_CHANNEL ADC1_CHANNEL_0  // e.g., GPIO36

// Sensor Scales
#define VOLTAGE_SENSOR_RATIO  (105.295) // Voltage Divider Ratio
#define CURRENT_SENSOR_Ratio  (240.350)  // Current Sensor Sensitivity x Voltage Divider Ratio


// Running average parameters
const int AVERAGE_WINDOW_SIZE = 10;

// Function prototypes
void initSensors();
void calibrateWeightSensor();
float readVoltageSensor();
float readCurrentSensor();
void resetWeightSensor();
int readWeightSensor();
uint32_t readVoltageGpio();
uint32_t readCurrentGpio();

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


