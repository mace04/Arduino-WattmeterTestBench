#include "sensors.h"
// #include "HX711.h"
#include "Settings.h" // Include Settings.h to use the Settings class

// Declare the global settings object from main.cpp
extern Settings settings;

// HX711 instance
#ifdef HX711_h
HX711 scale;
#endif

// Variables for running averages
float voltageReadings[AVERAGE_WINDOW_SIZE] = {0};
float currentReadings[AVERAGE_WINDOW_SIZE] = {0};
int voltageIndex = 0;
int currentIndex = 0;

void initSensors() {
    // Initialize pins
    pinMode(VOLTAGE_SENSOR_PIN, INPUT); // Set voltage sensor pin as input  
    pinMode(CURRENT_SENSOR_PIN, INPUT); // Set current sensor pin as input    
    // Initialize the HX711 weight sensor
    #ifdef HX711_h
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.set_scale(); // Set scale to default
    scale.tare();      // Reset the scale to zero
    #endif
}

// Function to read and smooth voltage sensor readings
float readVoltageSensor() {
    int rawADC = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = rawADC * (3.3 / 4095.0); // Convert ADC value to voltage

    // Calculate adjusted voltage using voltsPerPointVoltage and voltageOffset from settings
    voltage = (voltage / settings.getVoltsPerPointVoltage()) + settings.getVoltageOffset();

    // Update running average
    voltageReadings[voltageIndex] = voltage;
    voltageIndex = (voltageIndex + 1) % AVERAGE_WINDOW_SIZE;

    float sum = 0;
    for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++) {
        sum += voltageReadings[i];
    }
    return sum / AVERAGE_WINDOW_SIZE;
}

// Function to read and smooth current sensor readings
float readCurrentSensor() {
    int rawADC = analogRead(CURRENT_SENSOR_PIN);
    float voltage = rawADC * (3.3 / 4095.0); // Convert ADC value to voltage

    // Calculate current using voltsPerPointCurrent and currentOffset from settings
    float current = (voltage / settings.getVoltsPerPointCurrent()) + settings.getCurrentOffset();

    // Update running average
    currentReadings[currentIndex] = current;
    currentIndex = (currentIndex + 1) % AVERAGE_WINDOW_SIZE;

    float sum = 0;
    for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++) {
        sum += currentReadings[i];
    }
    return sum / AVERAGE_WINDOW_SIZE;
}

// Function to reset the HX711 weight sensor
void resetWeightSensor() {
    #ifdef HX711_h
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.set_scale(); // Set scale to default
    scale.tare();      // Reset the scale to zero
    #endif
}

// Function to read weight in grams from the HX711 sensor
float readWeightSensor() {
    float adjustedWeight = 0.0; // Initialize adjusted weight
    #ifdef HX711_h
    if (!scale.is_ready()) {
        Serial.println("HX711 not ready");
        return 0.0;
    }

    // Read raw weight and adjust using the weight offset from settings
    float rawWeight = scale.get_units(10); // Average over 10 readings
    adjustedWeight = rawWeight + settings.getThrustOffset();
    #endif
    return adjustedWeight;
}