#ifndef TFT_MAIN_MENU_H
#define TFT_MAIN_MENU_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "tft_config.h"

extern Profile testProfile;

class TftMainMenu {
public:
    TftMainMenu(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode));
    void init();
    void handleTouch();

private:
    struct Button {
        int x, y, width, height;
        const char* label;
        TftScreenMode screenMode;
        bool isPressed;
    };

    void drawButton(const Button& button);
    void handleMenuSelection(TftScreenMode screenMode);
    void handleProfileSelection();

    TFT_eSPI& tft;
    XPT2046_Touchscreen& ts;
    String lastProfileName; // To track the last displayed profile name
    void (*screenChangeCallback)(TftScreenMode); // Callback for screen change

    // Private button properties
    Button buttons[3];
    static const int buttonWidth = 200;
    static const int buttonHeight = 50;
    static const int buttonSpacing = 20;
    static const int buttonX = 60; // Centered for a 320px wide screen
    static const int buttonYStart = 80;

    // Private colors
    static const uint16_t buttonColor = TFT_GREEN;
    static const uint16_t buttonOutlineColor = TFT_BLACK;
    static const uint16_t buttonPressedColor = TFT_RED;

    static const int debounceDelay = 200; // Debounce delay in milliseconds
    unsigned long lastTouchTime = 0; // Timestamp for non-blocking debounce
};

#endif // TFT_MAIN_MENU_H