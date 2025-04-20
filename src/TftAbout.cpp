#include "TftAbout.h"

TftAbout::TftAbout(TFT_eSPI& tft, XPT2046_Touchscreen& ts, void (*screenChangeCallback)(TftScreenMode))
    : tft(tft), ts(ts), screenChangeCallback(screenChangeCallback) {}

void TftAbout::init() {
    // Chip select PANEL
    setCS(PANEL);

    // Set background color
    tft.fillScreen(TFT_NAVY);

    // Set text color
    tft.setTextColor(TFT_WHITE);

    // Display title
    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM); // Center text horizontally
    tft.drawString("Motor Testbench", tft.width() / 2, 20);

    // Display details in two columns
    tft.setTextSize(2);
    drawText("Firmware:", "v1.0.0", 80);
    drawText("WiFi Name:", WiFi.SSID().c_str(), 120);
    drawText("IP Address:", WiFi.localIP().toString().c_str(), 160);

    // Chip select TOUCH
    setCS(TOUCH);
}

void TftAbout::drawText(const char* label, const char* value, int y) {
    // Create a buffer to hold the concatenated string
    // Calculate column positions
    int columnWidth = tft.width() / 6; // Divide screen width into 4 equal columns
    int column2X = columnWidth * 1;   // Column 2 (for labels)
    int column3X = columnWidth * 5;   // Column 3 (for values)

    // Draw label in column 2
    tft.setTextDatum(TL_DATUM); // Align text to the right
    tft.drawString(label, column2X, y);

    // Draw value in column 3
    tft.setTextDatum(TR_DATUM); // Align text to the left
    tft.drawString(value, column3X, y);
}

void TftAbout::handleTouch() {
    if (digitalRead(TOUCH_IRQ) == LOW && ts.touched()) {
        if (screenChangeCallback) {
            screenChangeCallback(MAIN_MENU); // Switch back to the main menu
        }
    }
}