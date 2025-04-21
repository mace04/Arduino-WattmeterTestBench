#include "TftMotorTest.h"

TftMotorTest::TftMotorTest(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback),
      startButton{"Start", 10, 5, 80, 30, ENABLE},
      stopButton{"Stop", 100, 5, 80, 30, DISABLE},
      resetButton{"Reset", 190, 5, 80, 30, ENABLE},
      exitButton{"Exit", tft.height() - 90, 5, 80, 30, ENABLE} {}

void TftMotorTest::init(TestType testType) {
    // Chip select PANEL
    setCS(PANEL);
    this->testType = testType; // Set the test type

    // Set background color
    tft.fillScreen(TFT_NAVY);

    // Draw the ribbon
    drawRibbon();

    // Draw the throttle indicator
    drawThrottleIndicator(0); // Start with 0% throttle

    // Draw the panels
    drawPanels();

    // Chip select TOUCH
    setCS(TOUCH);
}

void TftMotorTest::drawRibbon() {
    tft.fillRect(0, 0, tft.width(), 40, TFT_DARKCYAN);

    // Draw buttons
    drawButton(startButton);
    drawButton(stopButton);
    drawButton(resetButton);
    drawButton(exitButton);
}

void TftMotorTest::drawButton(const Button& button) {
    uint16_t bgColor = (button.state == ENABLE) ? TFT_GREEN : TFT_LIGHTGREY;
    uint16_t textColor = (button.state == ENABLE) ? TFT_BLACK : TFT_WHITE;

    tft.fillRoundRect(button.x, button.y, button.width, button.height, 5, bgColor);
    tft.setTextColor(textColor);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(button.label, button.x + button.width / 2, button.y + button.height / 2 - 7);
}

void TftMotorTest::setButtonState(Button& button, ButtonState state) {
    button.state = state;
    drawButton(button);
}

void TftMotorTest::drawThrottleIndicator(int throttlePercent) {
    // Draw the throttle background
    tft.drawRect(0, tft.height() - 30, tft.width(), 30, TFT_WHITE);

    // Determine the throttle bar color
    uint16_t barColor = TFT_GREEN;
    if (throttlePercent >= 50 && throttlePercent < 85) {
        barColor = TFT_ORANGE;
    } else if (throttlePercent >= 85) {
        barColor = TFT_RED;
    }

    // Draw the throttle bar
    int barWidth = (tft.width() * throttlePercent) / 100;
    tft.fillRect(0, tft.height() - 30, barWidth, 30, barColor);

    // Display the throttle percentage
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    String throttleValue = String(throttlePercent) + '%';
    tft.drawString(throttleValue.c_str(), tft.width() / 2, tft.height() - 22);
}

void TftMotorTest::drawPanels() {
    int panelWidth = tft.width() / 3 - 10; // 3 columns with 10px spacing
    int panelHeight = (tft.height() - 75) / 2 - 10; // 2 rows with 10px spacing
    int xOffsets[] = {5, panelWidth + 15, 2 * panelWidth + 25};
    int yOffsets[] = {50, 50 + panelHeight + 10};

    drawPanel("Volts", "12.5", "Vols", xOffsets[0], yOffsets[0]);
    drawPanel("Current", "3.2", "Amps", xOffsets[1], yOffsets[0]);
    drawPanel("Thrust", "500", "Grams", xOffsets[2], yOffsets[0]);
    drawPanel("Power", "40", "Watts", xOffsets[0], yOffsets[1]);
    drawPanel("Consumption", "120", "mAh", xOffsets[1], yOffsets[1]);
    drawPanel("Time", "00:45", "", xOffsets[2], yOffsets[1]);
}

void TftMotorTest::drawPanel(const char* label, const char* value, const char* unit, int x, int y) {
    int panelWidth = tft.width() / 3 - 10;
    int panelHeight = (tft.height() - 70) / 2 - 10;

    // Draw panel background
    tft.fillRoundRect(x, y, panelWidth, panelHeight, 5, TFT_YELLOW);

    // Draw panel border
    tft.drawRoundRect(x, y, panelWidth, panelHeight, 5, TFT_BLACK);

    // Display label at the top-left
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(label, x + 5, y + 5);

    // Display value in the middle
    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(value, x + panelWidth / 2, y + panelHeight / 2);

    // Display unit at the bottom
    tft.setTextSize(2);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(unit, x + panelWidth / 2, y + panelHeight - 5);
}

void TftMotorTest::handleTouch() {
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

        if (x > 0 && y > 0) {
            setCS(PANEL); // Set CS for TFT panel
            // Check if Exit button is pressed
            if (x >= exitButton.x && x <= exitButton.x + exitButton.width &&
                y >= exitButton.y && y <= exitButton.y + exitButton.height) {
                if (screenChangeCallback) {
                    Serial.println("Exit button pressed, changing screen to MAIN_MENU");
                    screenChangeCallback(MAIN_MENU);
                }
            }

            // Handle Start button
            if (x >= startButton.x && x <= startButton.x + startButton.width &&
                y >= startButton.y && y <= startButton.y + startButton.height &&
                startButton.state == ENABLE) {
                    Serial.println("Start button pressed");
                    onStartPressed();
                }

            // Handle Stop button
            if (x >= stopButton.x && x <= stopButton.x + stopButton.width &&
                y >= stopButton.y && y <= stopButton.y + stopButton.height &&
                stopButton.state == ENABLE) {
                    Serial.println("Stop button pressed");
                    onStopPressed();
            }

            // Handle Reset button
            if (x >= resetButton.x && x <= resetButton.x + resetButton.width &&
                y >= resetButton.y && y <= resetButton.y + resetButton.height &&
                resetButton.state == ENABLE) {
                    Serial.println("Reset button pressed");
                    onResetPressed();
            }
        }
        setCS(TOUCH); // Set CS for touch controller
    }
}

void TftMotorTest::onStartPressed() {
    setButtonState(startButton, DISABLE);
    setButtonState(stopButton, ENABLE);
    // Add custom logic for Start button
}

void TftMotorTest::onStopPressed() {
    setButtonState(stopButton, DISABLE);
    setButtonState(startButton, ENABLE);
    // Add custom logic for Stop button
}

void TftMotorTest::onResetPressed() {
    setButtonState(startButton, ENABLE);
    setButtonState(stopButton, DISABLE);
    // Add custom logic for Reset button
}