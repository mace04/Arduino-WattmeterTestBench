#ifndef TFT_UPDATE_H
#define TFT_UPDATE_H

#include <TFT_eSPI.h>

class TftUpdate {
public:
    TftUpdate(TFT_eSPI& tft);
    void init();

private:
    TFT_eSPI& tft;

    // Private constants for UI
    static const uint16_t backgroundColor = TFT_NAVY;
    static const uint16_t boxColor = TFT_RED;
    static const uint16_t shadowColor = TFT_LIGHTGREY;
    static const uint16_t textColor = TFT_WHITE;

    static const int boxWidth = 240;
    static const int boxHeight = 100;
    static const int boxCornerRadius = 10;
    static const int shadowOffset = 5;
};

#endif // TFT_UPDATE_H