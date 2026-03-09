#include "tft_config.h"

void setCS(Device device) {
    // Disable all CS pins
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
    delay(1);

    // Enable the selected device
    switch (device) {
        case PANEL:
            digitalWrite(TFT_CS, LOW); // Enable PANEL
            break;
        case TOUCH:
            digitalWrite(TOUCH_CS, LOW); // Enable TOUCH
            break;
        case SDCARD:
            digitalWrite(SD_CS, LOW); // Enable SDCARD
            break;
    }
    delay(1);
}

bool isCSActive(Device device) {
    // Check the state of the CS pin for the given device
    switch (device) {
        case PANEL:
            return digitalRead(TFT_CS) == LOW; // Return true if PANEL CS is active
        case TOUCH:
            return digitalRead(TOUCH_CS) == LOW; // Return true if TOUCH CS is active
        case SDCARD:
            return digitalRead(SD_CS) == LOW; // Return true if SDCARD CS is active
        default:
            return false; // Invalid device
    }
}