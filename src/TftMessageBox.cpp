#include "TftMessageBox.h"

void TftMessageBox::info(TFT_eSPI& tft, const String& message){
    drawBox(tft, splitMessage(message, 28), TFT_LIGHTGREY, TFT_WHITE, TFT_BLACK);
}

void TftMessageBox::error(TFT_eSPI& tft, const String& message){
    drawBox(tft, splitMessage(message, 28), TFT_RED, TFT_WHITE, TFT_WHITE);
}
std::vector<String> TftMessageBox::splitMessage(const String& message, int maxLineLen){
    // Split error string into lines of max 28 chars (adjust as needed)
    std::vector<String> lines;
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
    return lines;
}

void TftMessageBox::drawBox(TFT_eSPI& tft, const std::vector<String>& lines, uint32_t fillColor, uint32_t borderColor, uint32_t textColor){
    setCS(PANEL);

    int lineHeight = 28;
    int boxWidth = tft.width() * 2 / 3 + 30;
    int boxHeight = lineHeight * lines.size() + 40;
    int boxX = (tft.width() - boxWidth) / 2;
    int boxY = (tft.height() - boxHeight) / 2;

    // Draw red box with border
    tft.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 12, fillColor);
    tft.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 12, borderColor);

    // Draw each line centered in box
    tft.setTextColor(textColor, fillColor);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    int textY = boxY + 20 + lineHeight / 2;
    for (const auto& line : lines) {
        tft.drawString(line, boxX + boxWidth / 2, textY);
        textY += lineHeight;
    }

    setCS(TOUCH);    
}

