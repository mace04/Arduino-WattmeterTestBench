#include "TftMainMenu.h"
#include <Arduino.h>

TftMainMenu::TftMainMenu(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback) {
    buttons[0] = {buttonX, buttonYStart, buttonWidth, buttonHeight, "Start Manual Test", MANUAL_TEST, false};
    buttons[1] = {buttonX, buttonYStart + buttonHeight + buttonSpacing, buttonWidth, buttonHeight, "Start Auto Test", AUTO_TEST, false};
    buttons[2] = {buttonX, buttonYStart + 2 * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight, "About", ABOUT, false};
}

void TftMainMenu::init() {
    tft.fillScreen(TFT_NAVY);

    // Draw all buttons
    for (int i = 0; i < 3; i++) {
        drawButton(buttons[i]);
    }
}

void TftMainMenu::drawButton(const Button& button) {
    tft.fillRect(button.x, button.y, button.width, button.height, button.isPressed ? buttonPressedColor : buttonColor);
    tft.drawRect(button.x, button.y, button.width, button.height, buttonOutlineColor);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(button.x + 10, button.y + 15);
    tft.print(button.label);
}

void TftMainMenu::handleTouch() {
    if (ts.touched()) {
        unsigned long currentTime = millis();
        if (currentTime - lastTouchTime < debounceDelay) {
            return; // Ignore touch if within debounce delay
        }
        lastTouchTime = currentTime;

        TS_Point p = ts.getPoint();
        int x = map(p.x, 0, 4095, 0, 320); // Map touch X to screen X
        int y = map(p.y, 0, 4095, 0, 480); // Map touch Y to screen Y

        // Check if a button was pressed
        for (int i = 0; i < 3; i++) {
            Button& button = buttons[i];
            if (x >= button.x && x <= button.x + button.width && y >= button.y && y <= button.y + button.height) {
                button.isPressed = true;
                drawButton(button);

                // Handle menu selection
                handleMenuSelection(button.screenMode);

                button.isPressed = false;
                drawButton(button);
            }
        }
    }
}

void TftMainMenu::handleMenuSelection(TftScreenMode screenMode) {
    if (screenChangeCallback) {
        screenChangeCallback(screenMode); // Notify the callback about the screen change
    }
}