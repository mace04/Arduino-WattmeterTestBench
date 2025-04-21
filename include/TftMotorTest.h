#ifndef TFT_MOTOR_TEST_H
#define TFT_MOTOR_TEST_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "tft_config.h"

enum TestType { MANUAL, AUTO };

class TftMotorTest {
public:
    TftMotorTest(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode));
    void init(TestType testType);
    void handleTouch();
    void updateThrottle(int throttlePercent);
    void updatePanel(const char* label, const char* value);

private:
    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;
    TestType testType;
    void (*screenChangeCallback)(TftScreenMode);
    static const int debounceDelay = 200; // Debounce delay in milliseconds
    unsigned long lastTouchTime = 0; // Timestamp for non-blocking debounce    

    enum ButtonState { ENABLE, DISABLE };
    struct Button {
        const char* label;
        int x, y, width, height;
        ButtonState state;
    };

    Button startButton;
    Button stopButton;
    Button resetButton;
    Button exitButton;

    void drawRibbon();
    void drawButton(const Button& button);
    void setButtonState(Button& button, ButtonState state);
    void drawThrottleIndicator(int throttlePercent);
    void drawPanels();
    void drawPanel(const char* label, const char* value, const char* unit, int x, int y);

    void onStartPressed();
    void onStopPressed();
    void onResetPressed();
};

#endif // TFT_MOTOR_TEST_H