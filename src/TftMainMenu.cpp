#include "TftMainMenu.h"
#include <Arduino.h>

TftMainMenu::TftMainMenu(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback) {
    // Calculate the position of the middle button to center it vertically and horizontally
    int centerX = (tft.height() / 2) - (buttonWidth / 2); // Center horizontally
    int centerY = (tft.width() / 2) - (buttonHeight / 2); // Center vertically

    // Calculate positions for the top and bottom buttons relative to the middle button
    int topY = centerY - buttonHeight - buttonSpacing; // Top button above the middle button
    int bottomY = centerY + buttonHeight + buttonSpacing; // Bottom button below the middle button

    // Assign button positions
    buttons[0] = {centerX, topY, buttonWidth, buttonHeight, "Manual Test", MANUAL_TEST, false};
    buttons[1] = {centerX, centerY, buttonWidth, buttonHeight, "Auto Test", AUTO_TEST, false};
    buttons[2] = {centerX, bottomY, buttonWidth, buttonHeight, "About", ABOUT, false};
}

void TftMainMenu::init() {
    setCS(PANEL); // Set CS for TFT panel
    tft.fillScreen(TFT_NAVY);

    // Draw all buttons
    for (int i = 0; i < 3; i++) {
        drawButton(buttons[i]);
    }
    setCS(TOUCH); // Set CS for touch controller
}

void TftMainMenu::drawButton(const Button& button) {
    int cornerRadius = 10; // Radius for rounded corners

    // Draw button background with rounded corners
    tft.fillRoundRect(button.x, button.y, button.width, button.height, cornerRadius, button.isPressed ? buttonPressedColor : buttonColor);

    // Draw button outline with rounded corners
    tft.drawRoundRect(button.x, button.y, button.width, button.height, cornerRadius, buttonOutlineColor);

    // Center the text within the button
    int16_t textWidth = tft.textWidth(button.label, 2); // Get text width with font size 2
    int16_t textHeight = 16; // Approximate height for font size 2
    int16_t textX = button.x + (button.width - textWidth) / 2;
    int16_t textY = button.y + (button.height - textHeight) / 2;

    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(textX, textY);
    tft.print(button.label);
}

void TftMainMenu::handleTouch() {
    if (digitalRead(TOUCH_IRQ) == LOW && ts.touched()) {
        unsigned long currentTime = millis();
        if (currentTime - lastTouchTime < debounceDelay) {
            setCS(TOUCH); // Set CS for touch controller
            return; // Ignore touch if within debounce delay
        }
        lastTouchTime = currentTime;

        TS_Point p = ts.getPoint();
        int x = map(p.y, 4095, 0, 0, tft.width()); // Map touch X to screen X
        int y = map(p.x, 4095, 0, 0, tft.height()); // Map touch Y to screen Y
        Serial.println("Touch coordinates: " + String(x) + ", " + String(y));

        // Check if a button was pressed
        for (int i = 0; i < 3; i++) {
            Button& button = buttons[i];
            if (x >= button.x && x <= button.x + button.width && y >= button.y && y <= button.y + button.height) {
                Serial.println("Button pressed: " + String(button.label));
                setCS(PANEL); // Set CS for TFT panel
                button.isPressed = true;
                drawButton(button);

                // Handle menu selection
                handleMenuSelection(button.screenMode);

                button.isPressed = false;
                // drawButton(button);
            }
        }
        setCS(TOUCH); // Set CS for touch controller
    }
}

void TftMainMenu::handleMenuSelection(TftScreenMode screenMode) {
    if (screenChangeCallback) {
        screenChangeCallback(screenMode); // Notify the callback about the screen change
    }
}