#ifndef TFT_ABOUT_H
#define TFT_ABOUT_H

#include <WiFi.h>
#include "tft_config.h"

class TftAbout {
public:
    TftAbout(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode));
    void init();
    void handleTouch();

private:
    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;
    void (*screenChangeCallback)(TftScreenMode);

    void drawText(const char* label, const char* value, int y);
};

#endif // TFT_ABOUT_H