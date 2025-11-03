#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <algorithm>
#include "Settings.h"
#include "tft_config.h"
#include "sensors.h"
#include "TftCalibrate.h"
#include "TftMessageBox.h"

extern Settings settings;
extern TftCalibrate tftCalibrate;

class TftCalibrateScale {
public:
    TftCalibrateScale(TFT_eSPI& tft, XPT2046_Touchscreen& ts);
    void init(bool calibrateEsc);
    void handle();
    bool calibrationComplete = false;

private:
    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;

    enum StageEnum {
        STAGE_REMOVE_WEIGHT = 1,
        STAGE_PLACE_KNOWN_WEIGHT,
        STAGE_UPDATE_SCALE_FACTOR
    };

    StageEnum currentStage = STAGE_REMOVE_WEIGHT;

    struct Button {
        const char* label;
        int x, y, width, height;
        bool isVisible;
    };
    Button nextButton;


void drawScreen(int stage, long reading = 0);
void showMessageBox(const String& message);
void drawButton(const Button& button);
};