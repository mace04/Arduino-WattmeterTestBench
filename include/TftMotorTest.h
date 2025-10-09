#ifndef TFT_MOTOR_TEST_H
#define TFT_MOTOR_TEST_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <esp_timer.h> // Include ESP32 timer library
#include <vector>
#include <algorithm>
#include "tft_config.h"
#include "motorControl.h"

enum TestType { MANUAL, AUTO };
extern MotorControl motorControl; // Forward declaration of motorControl

class TftMotorTest {
public:
    float Voltage = 0.0; // Variable to store voltage reading
    float Current = 0.0; // Variable to store current reading
    int Thrust = 0; // Variable to store thrust reading
    int Power = 0; // Variable to store power reading
    int Consumption = 0; // Variable to store accumulated mAh
    String Time = "00:00"; // Variable to store elapsed time
    int ThrottlePercent = 0; // Variable to store throttle percentage
    
    TftMotorTest(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode));
    void init(TestType testType);
    void handleTouch();

private:
    enum PanelIndex { PANEL_VOLTAGE = 0, PANEL_CURRENT, PANEL_THRUST, PANEL_POWER, PANEL_CONSUMPTION, PANEL_TIME }; // Define panel modes
    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;
    TestType testType;
    bool errorBoxDisplayed = false; // Flag to indicate if an error box is currently displayed
    void (*screenChangeCallback)(TftScreenMode);

    static const int debounceDelay = 200; // Debounce delay in milliseconds
    unsigned long lastTouchTime = 0; // Timestamp for non-blocking debounce    

    enum ButtonState { ENABLE, DISABLE };
    struct Button {
        const char* label;
        int x, y, width, height;
        ButtonState state;
    };

    struct Panel {
        const char* label;  // Panel label (e.g., "Volts")
        const char* unit;   // Unit of the value (e.g., "Volts")
        int x, y;           // Top-left corner position of the panel
        int width, height;  // Dimensions of the panel
        char value[10];     // Current value displayed in the panel
    
        // Constructor for easy initialization
        Panel(const char* lbl, const char* unt, int posX, int posY, int w, int h, const char* val) 
        : label(lbl), unit(unt), x(posX), y(posY), width(w), height(h) {
        strncpy(value, val, sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0'; // Ensure null termination
    }
};

    Button startButton;
    Button stopButton;
    Button resetButton;
    Button exitButton;

    esp_timer_handle_t updateTimer; // Timer handle for periodic updates

    static const int NUM_PANELS = 6; // Number of panels
    Panel panels[NUM_PANELS];       // Array to store panel details

    enum ScreenState{IDLE, TESTING};
    ScreenState currentScreenState = IDLE; // Current state of the screen

    static void IRAM_ATTR onTimer(void* arg); // Timer interrupt handler
    void updatePanelValue(int panelIndex, const char* value); // Update a specific panel's value
    void drawRibbon();
    void drawButton(const Button& button);
    void setButtonState(Button& button, ButtonState state);
    void drawThrottleIndicator(int throttlePercent);
    void updateThrottle(int throttlePercent);
    void updatePanelValues(); // Update only the values within the panels
    void drawPanels();
    void drawPanel(const Panel& panel);
    void onStartPressed();
    void onStopPressed();
    void onResetPressed();
    void showErrorBox(const String& error);
};

#endif // TFT_MOTOR_TEST_H