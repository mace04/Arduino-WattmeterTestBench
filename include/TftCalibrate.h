#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ESP32Servo.h>
#include "motorControl.h"
#include "Settings.h"
#include "tft_config.h"

extern Settings settings;
extern MotorControl motorControl;

class TftCalibrate {
public:
    TftCalibrate(TFT_eSPI& tft, XPT2046_Touchscreen& ts);
    void init(bool calibrateEsc);
    void handle();

private:
    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;
    Servo servo;

    struct Panel {
        const char* label;
        const char* unit;
        int x, y, width, height;
        char value[10];

        Panel(const char* lbl, const char* unt, int posX, int posY, int w, int h, const char* val)
            : label(lbl), unit(unt), x(posX), y(posY), width(w), height(h) {
            strncpy(value, val, sizeof(value) - 1);
            value[sizeof(value) - 1] = '\0';
        }
    };

    struct TextBox {
        String label;
        int x, y, width, height;
        String value;
    };

    Panel panelVoltage;
    Panel panelCurrent;
    Panel panelThrust;
    
    TextBox voltsPerPointVoltageBox;
    TextBox voltageOffsetBox;
    TextBox voltsPerPointCurrentBox;
    TextBox currentOffsetBox;
    TextBox thrustOffsetBox;
    TextBox voltsGPIOBox;
    TextBox currentGPIOBox;

    struct Button {
        const char* label;
        int x, y, width, height;
    };

    Button resetButton;

    void drawPanel(const Panel& panel);
    void updatePanel(const Panel& panel);
    void drawTextBox(const TextBox& box);
    void drawButton(const Button& button);
    void rebootESP();
};
