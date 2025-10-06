#include "TftUpdate.h"

TftUpdate::TftUpdate(TFT_eSPI& tft) : tft(tft) {}

void TftUpdate::init() {
    // Set the background color
    setCS(PANEL); // Set CS for TFT panel
    tft.fillScreen(backgroundColor);

    // Calculate the position of the box
    int boxX = (tft.width() - boxWidth) / 2;
    int boxY = (tft.height() - boxHeight) / 2;

    // Draw the rounded red box
    tft.fillRoundRect(boxX, boxY, boxWidth, boxHeight, boxCornerRadius, boxColor);

    // Draw the text
    tft.setTextColor(textColor, boxColor); // White text with red background
    tft.setTextSize(2);
    int16_t textWidth = tft.textWidth("Updating Firmware", 2);
    int16_t textHeight = 16; // Approximate height for font size 2
    int textX = boxX + 5 + (boxWidth - textWidth) / 2;
    int textY = boxY + (boxHeight - textHeight) / 2;
    tft.setCursor(textX, textY);
    tft.print("Updating Firmware");
    setCS(TOUCH); // Set CS for touch controller
}