#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <vector>
#include <algorithm>
#include "tft_config.h"

class TftMessageBox{
    public:
        static void info(TFT_eSPI& tft, const String& message);
        static void error(TFT_eSPI& tft, const String& message);
    private:
        static std::vector<String> splitMessage(const String& message, int maxLineLen);
        static void drawBox(TFT_eSPI& tft, const std::vector<String>& lines, uint32_t fillColor, uint32_t borderColor, uint32_t textColor);
};