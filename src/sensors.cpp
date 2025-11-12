#include "sensors.h"
// #include "HX711.h"

// Declare the global settings object from main.cpp
extern Settings settings;
esp_adc_cal_characteristics_t adc_characteristics;


// HX711 instance
#ifdef HX711_h
HX711 scale;
#endif

// Variables for running averages
float voltageReadings[AVERAGE_WINDOW_SIZE] = {0};
uint32_t voltageGpioReadings[AVERAGE_WINDOW_SIZE] = {0};
float currentReadings[AVERAGE_WINDOW_SIZE] = {0};
uint32_t currentGpioReadings[AVERAGE_WINDOW_SIZE] = {0};
int voltageIndex = 0;
int voltageGpioIndex = 0;
int currentIndex = 0;
int currentGpioIndex = 0;

// Global variables to track running consumption and elapsed time
float totalConsumption; // Total consumption in mAh
unsigned long lastConsumptionUpdateTime; // Last time consumption was updated

// Global variables for the timer
bool timerRunning = false;               // Tracks whether the timer is running
unsigned long timerStartTime = 0;        // Stores the start time of the timer
unsigned long timerElapsedTime = 0;      // Stores the elapsed time when paused

void initSensors() {
    // Initialize ADC
    pinMode(CURRENT_SENSOR_PIN, INPUT); // Set current sensor pin as input    
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_VOLTAGE_SENSOR_CHANNEL, ADC_ATTEN);
    adc1_config_channel_atten(ADC_CURRENT_SENSOR_CHANNEL, ADC_ATTEN);
    adc1_config_channel_atten(ADC_THTOTTLE_CHANNEL, ADC_ATTEN);
    // Characterize the ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, &adc_characteristics);


    // Initialize the HX711 weight sensor
    #ifdef HX711_h
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.set_scale(settings.getThrustScale()); // Set scale from settings
    scale.set_offset(settings.getThrustOffset());
    scale.tare(); // Reset the scale to zero
    #endif
}

void calibrateWeightSensor() {
    // It is assumed the initSensors have been called before this function and the scale is initialized    
    #ifdef HX711_h
    scale.set_scale(); // Set scale to default
    scale.set_offset();
    scale.tare();      // Reset the scale to zero
    #endif
}

uint32_t readVoltageGpio(){
    uint32_t adc_reading = adc1_get_raw(ADC_VOLTAGE_SENSOR_CHANNEL);
    uint32_t vOut = esp_adc_cal_raw_to_voltage(adc_reading, &adc_characteristics);
    // Update running average
    voltageGpioReadings[voltageIndex] = vOut;
    voltageGpioIndex = (voltageIndex + 1) % AVERAGE_WINDOW_SIZE;

    float sum = 0;
    for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++) {
        sum += voltageGpioReadings[i];
    }
    return sum / AVERAGE_WINDOW_SIZE;
}

uint32_t readCurrentGpio(){
    uint32_t adc_reading = adc1_get_raw(ADC_CURRENT_SENSOR_CHANNEL);
    uint32_t vOut = esp_adc_cal_raw_to_voltage(adc_reading, &adc_characteristics);
    // Update running average
    currentGpioReadings[currentIndex] = vOut;
    currentGpioIndex = (currentIndex + 1) % AVERAGE_WINDOW_SIZE;

    float sum = 0;
    for (int i = 0; i < AVERAGE_WINDOW_SIZE; i++) {
        sum += currentGpioReadings[i];
    }
    return sum / AVERAGE_WINDOW_SIZE;
}

// Function to read and smooth voltage sensor readings
float readVoltageSensor() {

    uint32_t adc_reading = adc1_get_raw(ADC_VOLTAGE_SENSOR_CHANNEL);
    float vOut = esp_adc_cal_raw_to_voltage(adc_reading, &adc_characteristics) / 1000.00;
    float voltage = vOut * settings.getVoltsPerPointVoltage() + settings.getVoltageOffset(); // Convert ADC value to voltage

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
    
    uint32_t adc_reading = adc1_get_raw(ADC_CURRENT_SENSOR_CHANNEL);
    float vOut = esp_adc_cal_raw_to_voltage(adc_reading, &adc_characteristics) / 1000.00;
    float current = vOut * settings.getVoltsPerPointCurrent() + settings.getCurrentOffset(); // Convert ADC value to voltage

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
    scale.tare();      // Reset the scale to zero
    #endif
}

// Function to read weight in grams from the HX711 sensor
int readWeightSensor() {
    int adjustedWeight = 0; // Initialize adjusted weight
    #ifdef HX711_h
    if (!scale.is_ready()) {
        Serial.println("HX711 not ready");
        return 0.0;
    }

    // Read raw weight and adjust using the weight offset from settings
    float rawWeight = scale.get_units(10); // Average over 10 readings
    // adjustedWeight = rawWeight + settings.getThrustOffset();
    #endif
    return adjustedWeight;
}

void resetConsumption() {
    totalConsumption = 0.0; // Reset total consumption to zero
    lastConsumptionUpdateTime = millis(); // Update the last update time
}

// Inline function to calculate and update running consumption in mAh
float calculateConsumption(float current) {
    unsigned long currentTime = millis();
    float elapsedTimeInSeconds = (currentTime - lastConsumptionUpdateTime) / 1000.0; // Convert milliseconds to seconds
    totalConsumption += (current * elapsedTimeInSeconds) / 3600.0; // Update running consumption
    lastConsumptionUpdateTime = currentTime; // Update the last update time
    return totalConsumption; // Return the updated total consumption
}

void startTimer() {
    if (!timerRunning) {
        timerStartTime = millis() - timerElapsedTime; // Resume from the paused time
        timerRunning = true; // Set the timer to running
        Serial.println("Timer started.");
    }
}

void pauseTimer() {
    if (timerRunning) {
        timerElapsedTime = millis() - timerStartTime; // Calculate elapsed time
        timerRunning = false; // Pause the timer
        Serial.println("Timer paused.");
    }
}

void resetTimer() {
    timerRunning = false; // Stop the timer
    timerStartTime = 0;   // Reset the start time
    timerElapsedTime = 0; // Reset the elapsed time
    Serial.println("Timer reset.");
}

String getElapsedTime() {
    unsigned long elapsedTime;
    if (timerRunning) {
        elapsedTime = millis() - timerStartTime; // Calculate elapsed time while running
    } else {
        elapsedTime = timerElapsedTime; // Return the paused elapsed time
    }

    // Convert elapsed time to minutes and seconds
    unsigned long totalSeconds = elapsedTime / 1000; // Convert milliseconds to seconds
    unsigned int minutes = totalSeconds / 60;       // Calculate minutes
    unsigned int seconds = totalSeconds % 60;       // Calculate remaining seconds

    // Format the result as mm:ss
    char buffer[6]; // Buffer to hold the formatted string
    snprintf(buffer, sizeof(buffer), "%02u:%02u", minutes, seconds);

    return String(buffer); // Return the formatted string
}