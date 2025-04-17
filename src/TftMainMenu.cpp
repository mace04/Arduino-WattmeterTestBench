#include "TftMainMenu.h"

TftMainMenu::TftMainMenu(MCUFRIEND_kbv& tft, TouchScreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback) {
    buttons[0] = {buttonX, buttonYStart, buttonWidth, buttonHeight, "Start Manual Test", MANUAL_TEST, false};
    buttons[1] = {buttonX, buttonYStart + buttonHeight + buttonSpacing, buttonWidth, buttonHeight, "Start Auto Test", AUTO_TEST, false};
    buttons[2] = {buttonX, buttonYStart + 2 * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight, "About", ABOUT, false};
}

void TftMainMenu::init() {
    tft.fillScreen(TFT_NAVY);

    Serial.println("Initializing TFT Main Menu...");
    
    // Draw all buttons
    for (int i = 0; i < 3; i++) {
        drawButton(buttons[i]);
    }
}

void TftMainMenu::drawButton(const Button& button) {
    tft.fillRect(button.x, button.y, button.width, button.height, button.isPressed ? buttonPressedColor : buttonColor);
    tft.drawRect(button.x, button.y, button.width, button.height, buttonOutlineColor);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(button.x + 10, button.y + 15);
    tft.print(button.label);
}

void TftMainMenu::handleTouch() {
    TSPoint p = ts.getPoint();
    if (p.z > ts.pressureThreshhold) {
        unsigned long currentTime = millis();
        if (currentTime - lastTouchTime < debounceDelay) {
            return; // Ignore touch if within debounce delay
        }
        lastTouchTime = currentTime;

        // Map touch coordinates to screen coordinates
        // int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
        // int y = map(p.y, TS_MINY, TS_MAXY, 0, 480);
        int x = map(p.y, TS_TOP, TS_BOT, 0, tft.width()); //.kbv makes sense to me
        int y = map(p.x, TS_RT, TS_LEFT, 0, tft.height()); 

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