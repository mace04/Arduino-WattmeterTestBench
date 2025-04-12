#include "sensors.h"
#include "HX711.h"

// HX711 instance
HX711 scale;

// Variables for running averages
float voltageReadings[AVERAGE_WINDOW_SIZE] = {0};
float currentReadings[AVERAGE_WINDOW_SIZE] = {0};
int voltageIndex = 0;
int currentIndex = 0;

// Function to read and smooth voltage sensor readings
float readVoltageSensor() {
    int rawADC = analogRead(VOLTAGE_SENSOR_PIN);
    float voltage = rawADC * (3.3 / 4095.0); // Convert ADC value to voltage
    voltage /= VOLTAGE_DIVIDER_RATIO;        // Adjust for voltage divider

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

    // ACS758KCB-150B-PFF-T sensitivity: 20mV/A, zero current at 2.5V
    float current = (voltage - 2.5) / 0.02;

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
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.set_scale(); // Set scale to default
    scale.tare();      // Reset the scale to zero
}

// Function to read weight in grams from the HX711 sensor
float readWeightSensor() {
    if (!scale.is_ready()) {
        Serial.println("HX711 not ready");
        return 0.0;
    }
    return scale.get_units(10); // Average over 10 readings
}