#include "TftCalibrateScale.h"

TftCalibrateScale::TftCalibrateScale(TFT_eSPI& tft, XPT2046_Touchscreen& ts)
    : tft(tft), ts(ts),
    nextButton{"Next", tft.height() / 2 - 40, tft.width() - 50, 110, 40}
{
}

void TftCalibrateScale::init(bool calibrateEsc) {
    drawScreen(1);
}

void TftCalibrateScale::handle() {
    if (digitalRead(TOUCH_IRQ) == LOW && ts.touched()) {
        TS_Point p = ts.getPoint();
        // Map touch coordinates to screen coordinates
        int16_t x = map(p.y, 0, 4095, 0, tft.width());
        int16_t y = map(p.x, 0, 4095, 0, tft.height());

        // Check if Next button is pressed
        if (x >= nextButton.x && x <= nextButton.x + nextButton.width &&
            y >= nextButton.y && y <= nextButton.y + nextButton.height) {
            currentStage = static_cast<StageEnum>(static_cast<int>(currentStage) + 1);
            if (currentStage > STAGE_UPDATE_SCALE_FACTOR) {
                tft.fillScreen(TFT_NAVY);
                // Calibration complete
                TftMessageBox::info(this->tft, "Scale calibration completed.");
                delay(3000);
                calibrationComplete = true;
                tftCalibrate.init(false); // Switch to regular calibration screen
                return;
            }
            
            if (currentStage == STAGE_PLACE_KNOWN_WEIGHT) {
                // Wait a moment for user to place weight
                calibrateWeightSensor();
                drawScreen(static_cast<int>(currentStage));
            }
            else if (currentStage == STAGE_UPDATE_SCALE_FACTOR) {
                // Read weight from scale
                long reading = static_cast<long>(readWeightSensor());
                drawScreen(static_cast<int>(currentStage), reading);
            }
        }
        // Simple debounce
        delay(300);
    }
}

void TftCalibrateScale::drawScreen(int stage, long reading) {
    tft.fillScreen(TFT_NAVY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("SCALE CALIBRATION", this->tft.width() / 2, 7);
    switch (stage) {
        case 1:
            TftMessageBox::info(this->tft, "Remove any weight from the scale and press Next.");
            break;
        case 2:
            TftMessageBox::info(this->tft, "Place a known weight on the scale and press Next.");
            break;
        case 3:
            TftMessageBox::info(this->tft, "Update scale factor setting to be " + String(reading) + "/(known weight) and press Next.");
            break;
        default:
            break;
    }
    drawButton(nextButton);
}

void TftCalibrateScale::drawButton(const Button& button) {
    tft.fillRoundRect(button.x, button.y, button.width, button.height, 5, TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(button.label, button.x + button.width / 2, button.y + button.height / 2 - 7);
}

void TftCalibrateScale::showMessageBox(const String& message) {
    setCS(PANEL);

    // Split error string into lines of max 28 chars (adjust as needed)
    std::vector<String> lines;
    int maxLineLen = 28;
    int start = 0;
    while (start < message.length()) {
        int len = min((unsigned int) maxLineLen, message.length() - start);
        // Try to break at space if possible
        int end = start + len;
        if (end < message.length() && message[end] != ' ') {
            int spacePos = message.lastIndexOf(' ', end);
            if (spacePos > start) {
                len = spacePos - start;
            }
        }
        lines.push_back(message.substring(start, start + len));
        start += len;
        while (start < message.length() && message[start] == ' ') start++; // Skip spaces
    }

    int lineHeight = 28;
    int boxWidth = tft.width() * 2 / 3 + 30;
    int boxHeight = lineHeight * lines.size() + 40;
    int boxX = (tft.width() - boxWidth) / 2;
    int boxY = (tft.height() - boxHeight) / 2;

    // Draw red box with border
    this->tft.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 12, TFT_LIGHTGREY);
    this->tft.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 12, TFT_WHITE);

    // Draw each line centered in box
    this->tft.setTextColor(TFT_BLACK, TFT_LIGHTGREY);
    this->tft.setTextSize(2);
    this->tft.setTextDatum(MC_DATUM);
    int textY = boxY + 20 + lineHeight / 2;
    for (const auto& line : lines) {
        this->tft.drawString(line, boxX + boxWidth / 2, textY);
        textY += lineHeight;
    }

    setCS(TOUCH);
}