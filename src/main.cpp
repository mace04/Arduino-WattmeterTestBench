#include <Arduino.h>
#include "WatmeterTestBench.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HX711.h>
#include <Servo.h>
#include <EEPROM.h>

/* This sketch describes how to connect a ACS715 Current Sense Carrier 
(http://www.pololu.com/catalog/product/1186) to the Arduino, 
and read current flowing through the sensor.

*/

LiquidCrystal_I2C lcd(0x27, 20, 4);
HX711 loadcell;
Servo esc;

/*

Vcc on carrier board to Arduino +5v
GND on carrier board to Arduino GND
OUT on carrier board to Arduino A0



Insert the power lugs into the loads positive lead circuit, 
arrow on carrier board points to load, other lug connects to 
power supply positive

Voltage Divider

11.66 from + to A4
4.62k from A4 to Gnd
Ratio 2.5238


*/
#define MAX_SAMPLES         10
#define LOADCELL_TIMEOUT    10
#define LOADCELL_CALIBRATION 139
#define LOADCELL_OFFSET     0
#define CURRSENSOR_OFFSET   124.00F             // Reading value of Current sensor at 0A - measuriung arouund 0.5V
#define CURRSENSOR_VPP      0.1220703125F    // or 0.1221896383186706 Current sensor sensitivity amps per point
                                            // Calculation: (Total Port Read in Volts/sensor sensitivity V/A)/Total Points
#define VOLTSENSOR_OFFSET   15              // Reading value from voltage Divider at 0V
#define VOLTSENSOR_VPP      0.1741   // Voltage Divider output voltage per point
#undef  EXPORT_VALUES

#undef _DEBUG_

#define PIN_VIN                     A3
#define PIN_AIN                     A1 // originally A0
#define PIN_LOADCELL_DOUT           9
#define PIN_LOADCELL_SCK            10
#define PIN_THROTTLE_IN             A7
#define PIN_THROTTLE_OUT            11 // Was originally 6

#define PIN_BUTTON_ISR              3 // Was originally 2

#define PIN_BUTTON_SCREEN_MODE      4
#define PIN_BUTTON_TEST_MODE        7
#define PIN_BUTTON_THROTTLE_CUT     8
#define PIN_BUTTON_PREVIOUS         6 // Was originally 11
#define PIN_BUTTON_OK               5

#define DEFAULT_SETTING_CURRENT     50
#define DEFAULT_SETTING_TRUST       5000
#define DEFAULT_SETTING_TEST1       15
#define DEFAULT_SETTING_TEST2       15
#define DEFAULT_SETTING_CYCLES      1
#define DEFAULT_SETTING_WARMUP      2

#define PWM_MIN                     1000
#define PWM_MAX                     2000

// Current sensor constrants
const float VCC = 3.3; // Supply voltage
const float QOV = 0.12 * VCC; // quiescent voltage of sensor
const float current_offset = 0.007; //Value to set sensor voltage value to 0 when no current
const float sensor_sensitivity = 40.0 / 1000.00; //Current sensor sensitivity 

struct TestCollection {
    AverageValues averageValues;
    WattmeterValues maximumValues;
} midThrottleTest, maxThrottleTest;

//struct CycleTestValues {
//    int cycles;
//    TestCollection midThrottleTests[3];
//    TestCollection maxThrottleTests[3];
//} automaticTestCycles;

const int buttonPins[] = { PIN_BUTTON_SCREEN_MODE, PIN_BUTTON_TEST_MODE, PIN_BUTTON_THROTTLE_CUT, PIN_BUTTON_OK, PIN_BUTTON_PREVIOUS };

enum ScreenMode { RUNNING_VALUES, AVERAGE_VALUES, MAXIMUM_VALUES, AUTO_TEST1, AUTO_TEST2, SETTINGS, CALIBRATION, CURRENT_CUTOFF, THRUST_CUTOFF, AUTO_START, AUTO_RESULTS, AUTO_END } screenMode;
enum TestMode { MANUAL, AUTOMATIC }  testMode;
enum TestCycle { OFF, TEST1, TEST2 } testCycle;

WattmeterValues runningValues;
AverageValues averageValues = { 0, {0,0,0,0,0,0 } };
WattmeterValues maximumValues = { 0,0,0,0,0,0 };

Settings settings;

bool enableThrottle = false;
bool cutoffChecking = false;
long cutoffTimer;
bool saveSettings = false;
bool collectData;

unsigned long ahTimer;

void setup() {
    // initialize serial communications at 9600 bps:
    char line1[] = "********************";
    char line2[] = "*Thrust&Watt Meter *";
    char line3[] = "*  For Prop & EDF  *";
    char line4[] = "********************";
    char* welcomeScreen[] = { line1, line2, line3, line4 };
    Serial.begin(115200);

    lcd.init();                      // initialize the lcd 
    lcd.backlight();
    for (int x = 0; x < 4;x++) {
        for (int y = 0;y < 20;y++) {
            lcd.setCursor(y, x);
            lcd.print(welcomeScreen[x][y]);
        }
        delay(50);
    }

    loadcell.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);
    loadcell.set_scale(LOADCELL_CALIBRATION);
    loadcell.set_offset(LOADCELL_OFFSET);
    loadcell.tare();

    esc.attach(PIN_THROTTLE_OUT, PWM_MIN, PWM_MAX);
    esc.writeMicroseconds(PWM_MIN);
    pinMode(PIN_THROTTLE_IN, INPUT);

    lcd.setCursor(0, 0);
    delay(2500);
    lcd.clear();
    screenMode = ScreenMode::RUNNING_VALUES;
    testMode = TestMode::MANUAL;

    settings = readEepromSettings();

    configureCommon(); // Setup pins for interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_ISR), pressInterrupt, FALLING);

    esc.writeMicroseconds(PWM_MIN);

    ahTimer = millis();
}

void loop() {

    //int batMonPin = A4;    // input pin for the voltage divider
    int batVal = 0;       // variable for the A/D value
    float pinVoltage = 0; // variable to hold the calculated voltage
    float batteryVoltage = 0;

    //int analogInPin = A0;  // Analog input pin that the carrier board OUT is connected to
    int sensorValue = 0;        // value read from the carrier board
    long outputValue = 0;        // output in milliamps
    unsigned long msec = 0;
    float time = 0.0;
    int sample = 0;
    float totalCharge = 0.0;
    float averageAmps = 0.0;
    float ampSeconds = 0.0;
    float ampHours = 0.0;
    float wattHours = 0.0;
    float amps = 0.0;

    float R1 = 47000.00; // 11660; // Resistance of R1 in ohms
    float R2 = 10000.00; // 4620; // Resistance of R2 in ohms

    float ratio = 0;  // Calculated from R1 / R2

    int sampleBVal = 0;
    int avgBVal = 0;
    long sampleAmpVal = 0;
    long avgSAV = 0;
    long thrust = 0;
 
    if (saveSettings) {
        writeEepromSettings(settings);
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("   Settings Saved   ");
        delay(1500);
        saveSettings = false;
    }
    //Get measurements only when in screens required
    if (screenMode < ScreenMode::SETTINGS) {
        printDebugNewLine();

        // Get input measurements for current and voltage
        for (int x = 0; x < MAX_SAMPLES; x++) { // run through loop 10x

         // read the analog in value:
            sensorValue = analogRead(PIN_AIN);
            sampleAmpVal = sampleAmpVal + sensorValue; // add samples together

            batVal = analogRead(PIN_VIN);    // read the voltage on the divider 
            sampleBVal = sampleBVal + batVal; // add samples together

            if (MAX_SAMPLES > 1)
                delayMicroseconds(2); // let ADC settle before next sample

        }



        int throttle = -1;
        int val;
        if (!enableThrottle) { // Disable throttle control
            esc.writeMicroseconds(PWM_MIN);
        }
        else if (testMode == TestMode::MANUAL && enableThrottle) { // Get throttle measurment for manual tests and map to % value
            val = analogRead(PIN_THROTTLE_IN);
            throttle = map(val, 0, 1023, 0, 100);
            val = map(val, 0, 1023, PWM_MIN, PWM_MAX);
            esc.writeMicroseconds(val);
        }
        else if (testMode == TestMode::AUTOMATIC) { //Set Throttle to the value set by the autmatic testing at the time
            throttle = runningValues.throttle;
            val = map(throttle, 0, 100, PWM_MIN, PWM_MAX);
            esc.writeMicroseconds(val);
        }

#ifdef _DEBUG_
        printDebug("I:" + String(avgSAV));
        printDebug("THR:" + String(analogRead(PIN_THROTTLE_IN)));
        printDebug("ESC:" + String(esc.readMicroseconds()));
#endif

        //Calculate reading for Voltage, Amps, Power and consumption
        long avgSAV = sampleAmpVal / MAX_SAMPLES;
        long avgBVal = sampleBVal / MAX_SAMPLES;

        float amps = (float)((avgSAV - CURRSENSOR_OFFSET) * CURRSENSOR_VPP); // Calculate amps on A/D pin
        //float amps = (((5.0 / 1023) * (float)avgSAV) - (QOV + current_offset)) / sensor_sensitivity; // Alternative way to calculate current
        amps = amps > 0 ? amps : 0;
        float batteryVoltage = (avgBVal + VOLTSENSOR_OFFSET) * 0.00459 * (float)(R1 / R2);       //  Calculate the voltage on the A/D pin
        float watts = amps * batteryVoltage;

        float time = (float)(millis() - ahTimer) / 1000.0;
        float ampHours = amps * 1000.00 * time / 3600.00;

        long weightRead = -1;
        if (loadcell.wait_ready_timeout(LOADCELL_TIMEOUT)) {    // Get Thrust measurement
            weightRead = loadcell.get_units();
        }
        printDebug("W:" + String(weightRead));

        // Store value measurements
        runningValues = { throttle, batteryVoltage , amps, (int)watts, ampHours > 0.00 ? (int)ampHours : maximumValues.consumption, (int)weightRead };
        if (((screenMode == ScreenMode::RUNNING_VALUES || screenMode == ScreenMode::AVERAGE_VALUES || screenMode == ScreenMode::MAXIMUM_VALUES)&& enableThrottle) 
            || (testMode == TestMode::AUTOMATIC && collectData)) {
            processAverageValues();
        }
        processMaxValues();


        // Make sure that the Current or thrust is not above the cuttof value
        if (runningValues.current > settings.maxCurrent) {  // Current is over the cutoff setting
            if (!cutoffChecking) {  // First time here so record the time
                cutoffChecking = !cutoffChecking;
                cutoffTimer = millis();
            }
            else {
                if (millis() - cutoffTimer > 500 && screenMode != ScreenMode::CURRENT_CUTOFF && screenMode != ScreenMode::SETTINGS) { // Disable throttle control if cuttof persit for over 500ms
                    esc.writeMicroseconds(PWM_MIN);
                    enableThrottle = !enableThrottle;
                    screenMode = ScreenMode::CURRENT_CUTOFF;
                    lcd.clear();
                    enableThrottle = false;
                    esc.writeMicroseconds(PWM_MIN);
                    cutoffChecking = !cutoffChecking;
                }
            }
        }
        else if (runningValues.thrust > settings.maxThrust) {
            if (!cutoffChecking) {  // First time here so record the time
                cutoffChecking = !cutoffChecking;
                cutoffTimer = millis();
            }
            else {
                if (millis() - cutoffTimer > 500 && screenMode != ScreenMode::THRUST_CUTOFF && screenMode != ScreenMode::SETTINGS) { // Disable throttle control if cuttof persit for over 500ms
                    esc.writeMicroseconds(PWM_MIN);
                    enableThrottle = !enableThrottle;
                    screenMode = ScreenMode::THRUST_CUTOFF;
                    lcd.clear();
                    enableThrottle = false;
                    esc.writeMicroseconds(PWM_MIN);
                    cutoffChecking = !cutoffChecking;
                }
            }
        }
    } // if(screenMode < ScreenMode::SETTINGS)

    switch (screenMode) {
    case ScreenMode::RUNNING_VALUES:
        displayValues("***RUNNING VALUES***", runningValues);
        break;
    case ScreenMode::AVERAGE_VALUES:
        displayAverageValues("***AVERAGE VALUES***", averageValues);
        break;
    case ScreenMode::MAXIMUM_VALUES:
        displayValues("***MAXIMUM VALUES***", maximumValues);
        break;
    case ScreenMode::SETTINGS:
        settingsValues();
        break;
    case ScreenMode::CALIBRATION:
        calibrationValues();
        break;
    case ScreenMode::CURRENT_CUTOFF:
        displayCurrentCutoffError();
        break;
    case ScreenMode::THRUST_CUTOFF:
        displayThrustCutoffError();
        break;
    case ScreenMode::AUTO_START:
        displayAutoTestStart();
        break;
    case ScreenMode::AUTO_TEST1:
        displayAutoTestPage1();
        break;
    case ScreenMode::AUTO_TEST2:
        displayAutoTestPage2();
        break;
    case ScreenMode::AUTO_RESULTS:
        displayAutoTestResultMenu();
        break;
    case ScreenMode::AUTO_END:
        displayAutoTestEnd();
        break;
    //default:
    //    displayValues("***RUNNING VALUES***", runningValues);
    }

#ifdef EXPORT_VALUES
    String dataLine = String(runningValues.throttle) + "," + String(runningValues.voltage) + ',' + 
        String(runningValues.current) + ',' + String(runningValues.power) + ',' + 
        String(runningValues.consumption) + ',' + String(runningValues.thrust);
    Serial.println(dataLine);
#endif
}

unsigned long lastFire = 0;
bool settingEditMode = false;
bool blink;
void pressInterrupt() { // ISR
    if (millis() - lastFire < 200) { // Debounce
        return;
    }

    lastFire = millis();

    configureDistinct(); // Setup pins for testing individual buttons

    for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) { // Test each button for press
        if (!digitalRead(buttonPins[i])) {
            buttonPressed(buttonPins[i]);
        }
    }

    configureCommon(); // Return to original state
}

void configureCommon() {
    pinMode(PIN_BUTTON_ISR, INPUT_PULLUP);

    for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
        pinMode(buttonPins[i], OUTPUT);
        digitalWrite(buttonPins[i], LOW);
    }
}

void configureDistinct() {
    pinMode(PIN_BUTTON_ISR, OUTPUT);
    digitalWrite(PIN_BUTTON_ISR, LOW);

    for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }
}

long autoTestTimer;
long settingEditBlinkTimer;
bool settingSelectNext = false;
bool settingSelectPrevious = false;
bool isAborted = false;
bool viewResult = false;
int autoTestResultsPage = 1;
bool clearScreen;
void buttonPressed(int button) { // Our handler
    switch (button) {
    case PIN_BUTTON_SCREEN_MODE:  // Used to toggle between screens in manual mode
        if (testMode == TestMode::MANUAL && !settingEditMode){// && !enableThrottle) { // Toggle screens when in Manual Mode
            clearScreen = true;
            toggleScreenMode();
        }
        else if (screenMode == ScreenMode::AUTO_RESULTS) {
            clearScreen = true;
            autoTestResultsPage++;
            if (autoTestResultsPage > 4) {
                autoTestResultsPage = 1;
            }
        }
        break;
    case PIN_BUTTON_TEST_MODE:  // Used to toggle between Manual and Auto modes
                                // Also used to decrease values in settings when in edit mode
        if (testMode == TestMode::MANUAL && (screenMode != ScreenMode::SETTINGS && screenMode!=ScreenMode::CALIBRATION)) { // If in Manual test Toggle to first page for Autotmatic Test and start timer
            testMode = TestMode::AUTOMATIC;
            screenMode = ScreenMode::AUTO_START;
            autoTestTimer = micros()/1000 + 6000;
        }
        else if ((screenMode == ScreenMode::SETTINGS || screenMode == ScreenMode::CALIBRATION) && !settingEditMode) {
            settingSelectNext = true;
        }
        else if (screenMode==ScreenMode::AUTO_RESULTS) { //otherwise toggle to manual test 
            testMode = TestMode::MANUAL;
            screenMode = ScreenMode::RUNNING_VALUES;
        }
        break;
    case PIN_BUTTON_PREVIOUS:
        if (screenMode == ScreenMode::AVERAGE_VALUES || screenMode == ScreenMode::MAXIMUM_VALUES) {
            // Reset average and maximum values for new manual tests
            averageValues = { 0,{0,0,0,0,0,0} };
            maximumValues = { 0,0,0,0,0,0 };
            // Reset scale back to zero
            loadcell.tare();
            //Reset AH timer
            ahTimer = millis();
        }
        if ((screenMode != ScreenMode::SETTINGS || screenMode != ScreenMode::CALIBRATION) && !settingEditMode) {
            settingSelectPrevious = true;
        }
        break;
    case PIN_BUTTON_THROTTLE_CUT:   // Button to control throttle cut
                                    // Also used to decrease values in settings when in edit mode
        if (screenMode == ScreenMode::RUNNING_VALUES) { // Controll throttle engagement when in Manual
            if (!enableThrottle && analogRead(PIN_THROTTLE_IN)==0) { // Enable throttle only if throttle value is 0%
                enableThrottle = !enableThrottle;
            }
            else if (enableThrottle)
                enableThrottle = !enableThrottle;
        }
        else if (testMode == TestMode::AUTOMATIC && (screenMode == ScreenMode::AUTO_TEST1 || screenMode == ScreenMode::AUTO_TEST2)) {
            // if throttle cut is pressed during autot testing, stop the test
            enableThrottle = false;
            screenMode = ScreenMode::AUTO_END;
            isAborted = true;
        }
        break;
    case PIN_BUTTON_OK:
        if (testMode == TestMode::MANUAL && (screenMode == ScreenMode::CURRENT_CUTOFF || screenMode == ScreenMode::THRUST_CUTOFF)) {
            // Goto to first page in manual test when pressed ok in error screens
            screenMode = ScreenMode::RUNNING_VALUES;
            isAborted = true;
        }
        else if (testMode == TestMode::AUTOMATIC && (screenMode == ScreenMode::CURRENT_CUTOFF || screenMode == ScreenMode::THRUST_CUTOFF)) {
            // Goto to last page in auto test when pressed ok in error screens
            screenMode = ScreenMode::AUTO_END;
            isAborted = true;
        }
        else if (screenMode == ScreenMode::SETTINGS || screenMode == ScreenMode::CALIBRATION) {
            // Toggle edit mode when pressed ok in Settings screen
            settingEditMode = !settingEditMode;
            settingEditBlinkTimer = micros() / 1000;
        }
        else if (screenMode == ScreenMode::AUTO_RESULTS) {
            clearScreen = true;
                 autoTestResultsPage++;
                if (autoTestResultsPage > 4) {
                    autoTestResultsPage = 1;
                }
        }
        break;
    }
}

void toggleScreenMode() {
    if (testMode == TestMode::MANUAL) {
        switch (screenMode) {
        case ScreenMode::RUNNING_VALUES:
            screenMode = ScreenMode::AVERAGE_VALUES;
            break;
        case ScreenMode::AVERAGE_VALUES:
            screenMode = ScreenMode::MAXIMUM_VALUES;
            break;
        case ScreenMode::MAXIMUM_VALUES:
            if (enableThrottle) {
                screenMode = ScreenMode::RUNNING_VALUES;
            }
            else {
                screenMode = ScreenMode::SETTINGS;
            }
            break;
        case ScreenMode::SETTINGS:
            screenMode = ScreenMode::CALIBRATION;
            //saveSettings = settingsDiff(settings);
            break;
        case ScreenMode::CALIBRATION:
            screenMode = ScreenMode::RUNNING_VALUES;
            saveSettings = settingsDiff(settings);
            break;
        case ScreenMode::CURRENT_CUTOFF:
            screenMode = ScreenMode::RUNNING_VALUES;
            break;
        case ScreenMode::THRUST_CUTOFF:
            screenMode = ScreenMode::RUNNING_VALUES;
            break;
        default:
            screenMode = ScreenMode::RUNNING_VALUES;
        }

    }
    else if (testMode == TestMode::AUTOMATIC) {
        switch (screenMode) {
         default:
            testMode = TestMode::MANUAL;
            screenMode = ScreenMode::RUNNING_VALUES;
            isAborted = false;
        }
    }
}


String fixedLength(String str, int len) {
    String spaces = "";

    if (len > str.length())
        for (int i = 0; i < len - str.length(); i++)
            spaces += " ";
    return str + spaces;
}

void  processMaxValues() {
    if (runningValues.throttle > maximumValues.throttle)
        maximumValues.throttle = runningValues.throttle;
    if (runningValues.voltage > maximumValues.voltage)
        maximumValues.voltage = runningValues.voltage;
    if (runningValues.current > maximumValues.current)
        maximumValues.current = runningValues.current;
    if (runningValues.power > maximumValues.power)
        maximumValues.power = runningValues.power;
    if (runningValues.consumption > maximumValues.consumption)
        maximumValues.consumption = runningValues.consumption;
    if (runningValues.thrust > maximumValues.thrust)
        maximumValues.thrust = runningValues.thrust;
}

void processAverageValues() {
    if (averageValues.samples == 100) {
        averageValues.values.throttle = averageValues.values.throttle / averageValues.samples;
        averageValues.values.voltage = averageValues.values.voltage / averageValues.samples;
        averageValues.values.current = averageValues.values.current / averageValues.samples;
        averageValues.values.power = averageValues.values.power / averageValues.samples;
        averageValues.values.thrust = averageValues.values.thrust / averageValues.samples;
        averageValues.values.consumption = runningValues.consumption;
        averageValues.samples = 1;
    }
    averageValues.samples += 1;
    averageValues.values.throttle += runningValues.throttle;
    averageValues.values.voltage += runningValues.voltage;
    averageValues.values.current += runningValues.current;
    averageValues.values.power += runningValues.power;
    averageValues.values.consumption = runningValues.consumption;
    averageValues.values.thrust += runningValues.thrust;
}

long seconds;
void displayValues(String header, WattmeterValues readings) {
    if (clearScreen) {
        lcd.clear();
        clearScreen = false;
    }
    lcd.setCursor(0, 0);
    lcd.print(header);

    lcd.setCursor(0, 1);
    lcd.print(fixedLength("V=" + String(readings.voltage) + "V", 10));

    lcd.setCursor(10, 1);
    lcd.print(fixedLength("I=" + String(readings.current) + "A", 10));

    lcd.setCursor(0, 2);
    lcd.print(fixedLength("P=" + String(readings.power) + "W", 10));
    lcd.setCursor(10, 2);
    if (screenMode == ScreenMode::RUNNING_VALUES) {
        lcd.print(fixedLength("Q=" + String(readings.consumption) + "mAh", 10));
    }
    else if(testMode==TestMode::AUTOMATIC){
        lcd.print(fixedLength("t=" + String(seconds) + "s", 10));
    }
    else {
        lcd.print("          ");
    }

    lcd.setCursor(0, 3);
    if (readings.thrust > 0) {
        lcd.print(fixedLength("T=" + String(readings.thrust) + "g", 10));
    }
    else {
        lcd.print(fixedLength("T=" + String(0) + "g", 10));
    }
    lcd.setCursor(10, 3);
    if (readings.throttle >= 0) {
        lcd.print(fixedLength("THR=" + String(readings.throttle) + "%", 10));
    }
    else {
        lcd.print(fixedLength("THR=(IDLE)", 10));
    }

}

void displayAverageValues(String header, AverageValues val) {
    WattmeterValues newAverage = { 0,0,0,0,0,0 };

    if (val.samples > 0) {
        newAverage.throttle = val.values.throttle / val.samples;
        newAverage.voltage = val.values.voltage / val.samples;
        newAverage.current = val.values.current / val.samples;
        newAverage.power = val.values.power / val.samples;
        newAverage.thrust = val.values.thrust / val.samples;
        newAverage.consumption = val.values.consumption;
    }
    displayValues(header, newAverage);
}

int cursor = 1;
void settingsValues() {

    lcd.setCursor(0, 0);
    lcd.print("******SETTINGS******");
    
    if (settingEditMode) {
        if (millis() - settingEditBlinkTimer > 500) {
            blink = !blink;
            settingEditBlinkTimer = millis();
        }
    }
    else{
        blink = false;
    }

    int selected;
    if (!settingEditMode) {
        if (settingSelectNext && cursor < 6) {
            cursor++;
            settingSelectNext = false;
        }
        if (settingSelectPrevious && cursor > 1) {
            cursor--;
            settingSelectPrevious = false;
        }

    }
    selected = cursor % 3;
    if (cursor / 3.00 <= 1) {
        lcd.setCursor(1, 1);
        if (settingEditMode && blink && selected == 1) {
            lcd.print(fixedLength("Max Current=", 18));
        }
        else {
            lcd.print(fixedLength("Max Current=" + String(settings.maxCurrent) + "A", 18));
        }
        lcd.setCursor(1, 2);
        if (settingEditMode && blink && selected == 2) {
            lcd.print(fixedLength("Max Thrust=", 18));
        }
        else {
            lcd.print(fixedLength("Max Thrust=" + String(settings.maxThrust) + "gr", 18));
        }
        lcd.setCursor(1, 3);
        if (settingEditMode && blink && selected == 0) {
            lcd.print(fixedLength("Half THR Test=", 18));
        }
        else {
            lcd.print(fixedLength("Half THR Test=" + String(settings.midTestDuration) + "s", 18));
        }
        if (settingEditMode) {
           switch (selected) {
            case 1:
                settings.maxCurrent = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 10, 120);
                break;
            case 2:
                settings.maxThrust = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 500, 10000);
                break;
            case 0:
                settings.midTestDuration = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 1, 60);
                break;
            }

        }
    }

    if (cursor / 3.00 > 1 && cursor / 3.00 <= 2) {
        lcd.setCursor(1, 1);
        if (settingEditMode && blink && selected == 1) {
            lcd.print(fixedLength("Full THR Test=", 18));
        }
        else {
            lcd.print(fixedLength("Full THR Test=" + String(settings.maxTestDuration) + "s", 18));
        }
        lcd.setCursor(1, 2);
        if (settingEditMode && blink && selected == 2) {
            lcd.print(fixedLength("Warm Up Time=", 18));
        }
        else {
            lcd.print(fixedLength("Warm Up Time=" + String(settings.warmUptime) + "s", 18));
        }
        lcd.setCursor(1, 3);
        lcd.print(fixedLength(" ", 18));

        if (settingEditMode) {
            switch (selected) {
            case 1:
                settings.maxTestDuration = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 1, 60);
                break;
            case 2:
                settings.warmUptime = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 1, 6);
                break;
            case 0:
                break;
            }

        }
        else if (selected == 0) {
            // Last line is not a setting so don't select it
            selected = 2;
            cursor = 5;
        }
    }

    switch (selected) {
    case 1:
        lcd.setCursor(0, 1);
        lcd.print(">");
        lcd.setCursor(19, 1);
        lcd.print("<");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(0,3);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");
        break;
    case 2:
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(">");
        lcd.setCursor(19, 2);
        lcd.print("<");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");
        break;
    case 0:
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(">");
        lcd.setCursor(19, 3);
        lcd.print("<");
        break;
    default:
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");
    }
}
bool throttleCheck = true;
void calibrationValues() {
    bool okToContinue = false;
    if (settingEditMode) {
        if (millis() - settingEditBlinkTimer > 500) {
            blink = !blink;
            settingEditBlinkTimer = millis();
        }
        if (cursor == 1 && throttleCheck) {
            if (analogRead(PIN_THROTTLE_IN) > 0) {
                lcd.setCursor(0, 0);
                lcd.print("********************");
                lcd.setCursor(0, 1);
                lcd.print("* THROTTLE IS NOT  *");
                lcd.setCursor(0, 2);
                lcd.print("*       IDLE       *");
                lcd.setCursor(0, 3);
                lcd.print("********************");
                settingEditMode = false;
                delay(1500);
                return;
            }
            else {
                throttleCheck = false;
            }
        }
    }
    else {
        blink = false;
        if (settingSelectNext && cursor < 3) {
            cursor++;
            settingSelectNext = false;
        }
        if (settingSelectPrevious && cursor > 1) {
            cursor--;
            settingSelectPrevious = false;
        }
        throttleCheck = true;

    }

    lcd.setCursor(0, 0);
    lcd.print("****CALIBRATION*****");

    int selected;
    selected = cursor % 3;
    if (cursor / 3.00 <= 1) {
        lcd.setCursor(1, 1);
        lcd.print(fixedLength("I=" + String(runningValues.current) + "A", 9));
        lcd.setCursor(10, 1);
        if (settingEditMode && blink && selected == 1) {

            lcd.print(fixedLength("f=", 8));
        }
        else {
            lcd.print(fixedLength("f=" + String(settings.currentOffset), 9));
        }
        lcd.setCursor(1, 2);
        lcd.print(fixedLength("V=" + String(runningValues.voltage) + "V", 9));
        if (settingEditMode && blink && selected == 2) {
            lcd.print(fixedLength("f=", 8));
        }
        else {
            lcd.print(fixedLength("f=" + String(settings.voltageOffset), 9));
        }
        lcd.setCursor(1, 3);
        lcd.print(fixedLength("W=" + String(runningValues.thrust) + "Kg", 9));
        if (settingEditMode && blink && selected == 0) {
            lcd.print(fixedLength("f=", 8));
        }
        else {
            lcd.print(fixedLength("f=" + String(settings.thrustOffset), 9));
        }
        if (settingEditMode) {
            switch (selected) {
            case 1:
                settings.currentOffset = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 0, 1000) / 10.00;
                break;
            case 2:
                settings.voltageOffset = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 0, 1000) / 10.00;
                break;
            case 0:
                settings.thrustOffset = map(analogRead(PIN_THROTTLE_IN), 0, 1023, 0, 1000) / 10.00;
                break;
            }

        }
    }
    switch (selected) {
    case 1:
        lcd.setCursor(0, 1);
        lcd.print(">");
        lcd.setCursor(19, 1);
        lcd.print("<");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");
        break;
    case 2:
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(">");
        lcd.setCursor(19, 2);
        lcd.print("<");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        lcd.setCursor(19, 3);
        lcd.print(" ");
        break;
    case 0:
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.setCursor(19, 1);
        lcd.print(" ");
        lcd.setCursor(0, 2);
        lcd.print(" ");
        lcd.setCursor(19, 2);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(">");
        lcd.setCursor(19, 3);
        lcd.print("<");
        break;
    }

}

void displayCurrentCutoffError() {
    lcd.setCursor(0, 0);
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("* CURRENT OVERLOAD *");
    lcd.setCursor(0, 2);
    lcd.print("* PRESS OK BUTTON  *");
    lcd.setCursor(0, 3);
    lcd.print("********************");
}

void displayThrustCutoffError() {
    lcd.setCursor(0, 0);
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("* THRUST OVERLOAD  *");
    lcd.setCursor(0, 2);
    lcd.print("* PRESS OK BUTTON  *");
    lcd.setCursor(0, 3);
    lcd.print("********************");
}

Settings readEepromSettings() {
    if (EEPROM.read(0x00) == 0xFF) {
        settings = { DEFAULT_SETTING_CURRENT, DEFAULT_SETTING_TRUST, DEFAULT_SETTING_TEST1, DEFAULT_SETTING_TEST2, DEFAULT_SETTING_WARMUP,0, 0, 0 };
    }
    else {
        EEPROM.get(0x00, settings);
    }
    return settings;
}

void writeEepromSettings(Settings values) {
        EEPROM.put(0x00, values);
        saveSettings = false;
}

bool settingsDiff(Settings values) {
    bool retval = false;
    Settings val2;
    if (EEPROM.read(0x00) == 0xFF) {
        retval = true;
    }
    else{
        EEPROM.get(0x00, val2);
    }
    if (!retval)
        retval = values.maxCurrent != val2.maxCurrent;
    if (!retval)
        retval = values.maxThrust != val2.maxThrust;
    if (!retval)
        retval = values.midTestDuration != val2.midTestDuration;
    if (!retval)
        retval = values.maxTestDuration != val2.maxTestDuration;
    //if (!retval)
    //    retval = values.testCycles != val2.testCycles;
    if (!retval)
        retval = values.warmUptime != val2.warmUptime;
    if (!retval)
        retval = values.currentOffset != val2.currentOffset;
    if (!retval)
        retval = values.voltageOffset != val2.voltageOffset;
    if (!retval)
        retval = values.thrustOffset != val2.thrustOffset;
    return retval;
}

void displayAutoTestResultMenu() {

    switch (autoTestResultsPage) {
    case 1:
        displayAverageValues("MID THROTTLE AVERAGE", midThrottleTest.averageValues);
        break;
    case 2:
        displayValues("MID THROTTLE MAXIMUM", midThrottleTest.maximumValues);
        break;
    case 3:
        displayAverageValues(" FULL THROTTLE AVG ", maxThrottleTest.averageValues);
        break;
    case 4:
        displayValues(" FULL THROTTLE MAX  ", maxThrottleTest.maximumValues);
        break;
    }
}
void displayAutoTestStart() {
    int seconds = (int)(autoTestTimer - millis()) / 1000;

    lcd.setCursor(0, 0);
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("* Automatic test   *");
    lcd.setCursor(0, 2);
    lcd.print("* will start in " + String(seconds) + "s *");
    lcd.setCursor(0, 3);
    lcd.print("********************");

    if (seconds == 0) {
        screenMode = ScreenMode::AUTO_TEST1;
        enableThrottle = true;
        averageValues = { 0,{0,0,0,0,0,0} };
        maximumValues = { 0,0,0,0,0,0 };
    }
}

bool freeze = false;
long autoTestFreezeTimer;
bool warmup = true;
WattmeterValues freezeValues;
void displayAutoTestPage1() {
    collectData = !warmup && !freeze;
    if (warmup) {
        runningValues.throttle = 0;
        displayValues(" HALF THROTTLE TEST ", runningValues);
        long warmupTime = millis();
        int pwmThrottle;
        while (millis() - warmupTime < settings.warmUptime * 1000) {
            runningValues.throttle = map(millis(), warmupTime, warmupTime + (settings.warmUptime * 1000), 0, 50);
            pwmThrottle = map(runningValues.throttle, 0, 100, PWM_MIN, PWM_MAX);
            esc.writeMicroseconds(pwmThrottle);
            lcd.setCursor(10, 3);
            lcd.print(fixedLength("THR=" + String(runningValues.throttle) + "%", 10));
            delayMicroseconds(10);
        }
        warmup = false;
        autoTestTimer = ((settings.midTestDuration + 1) * 1000) + millis();
    }
    if (freeze) {
        freezeValues.throttle = -1;
        displayValues(" HALF THROTTLE TEST ", freezeValues);
    }
    else {
        runningValues.throttle = 50;
        displayValues(" HALF THROTTLE TEST ", runningValues);
    }

    seconds = (long)(autoTestTimer - millis()) / 1000;
    if (seconds == 0) {
        freeze = true;
        autoTestFreezeTimer = millis() + 6000;
        freezeValues = runningValues;
        enableThrottle = false;
    } else if(freeze){
        seconds = (long)(autoTestFreezeTimer - millis())/1000;
        if (seconds <= 0) {
            screenMode = ScreenMode::AUTO_TEST2;
            enableThrottle = true;
            freeze = false;
            warmup = true;
            midThrottleTest.averageValues = averageValues;
            midThrottleTest.maximumValues = maximumValues;
            averageValues = { 0,{0,0,0,0,0,0} };
            maximumValues = { 0,0,0,0,0,0 };
        }
    }
}

void displayAutoTestPage2() {
    collectData = !warmup && !freeze;
    if (warmup) {
        runningValues.throttle = 0;
        displayValues(" HALF THROTTLE TEST ", runningValues);
        long warmupTime = millis();
        int pwmThrottle;
        while (millis() - warmupTime < settings.warmUptime * 1000) {
            runningValues.throttle = map(millis(), warmupTime, warmupTime + (settings.warmUptime * 1000), 0, 100);
            pwmThrottle = map(runningValues.throttle, 0, 100, PWM_MIN, PWM_MAX);
            esc.writeMicroseconds(pwmThrottle);
            lcd.setCursor(10, 3);
            lcd.print(fixedLength("THR=" + String(runningValues.throttle) + "%", 10));
            delayMicroseconds(10);
        }
        warmup = false;
        autoTestTimer = ((settings.maxTestDuration + 1) * 1000) + millis();
    }    if (freeze) {
        freezeValues.throttle = -1;
        displayValues(" FULL THROTTLE TEST ", freezeValues);
    }
    else {
        runningValues.throttle = 100;
        displayValues(" FULL THROTTLE TEST ", runningValues);
    }

    seconds = (long)(autoTestTimer - millis()) / 1000;
    if (seconds == 0) {
        freeze = true;
        autoTestFreezeTimer = millis() + 6000;
        freezeValues = runningValues;
        enableThrottle = false;
    }
    else if (freeze) {
        seconds = (long)(autoTestFreezeTimer - millis()) / 1000;
            screenMode = ScreenMode::AUTO_END;
            autoTestTimer = (settings.midTestDuration * 1000) + millis();
            enableThrottle = false;
            freeze = false;
            warmup = true;
            cursor = 1;
            maxThrottleTest.averageValues = averageValues;
            maxThrottleTest.maximumValues = maximumValues;
            averageValues = { 0,{0,0,0,0,0,0} };
            maximumValues = { 0,0,0,0,0,0 };
        //}
    }
}


void displayAutoTestEnd() {
    lcd.setCursor(0, 0);
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("*  Automatic test  *");
    lcd.setCursor(0, 2);
    if (isAborted) {
        lcd.print("*     aborted      *");
    }
    else {
        lcd.print("*    successful    *");
    }
    lcd.setCursor(0, 3);
    lcd.print("********************");
    freeze = false;
    warmup = true;
    enableThrottle = false;
    delay(1500);
    if (!isAborted) {
        screenMode = ScreenMode::AUTO_RESULTS;
        lcd.clear();
    }
    else {
        testMode = TestMode::MANUAL;
        screenMode = ScreenMode::RUNNING_VALUES;
    }
}

bool newLine = false;
void printDebug(String string) {
#ifdef _DEBUG_
    if (!newLine)
        Serial.print(",");
    Serial.print(string);
    newLine = false;
#endif
}
void printDebugNewLine() {
#ifdef _DEBUG_
    Serial.println("");
    newLine = true;
#endif
}
