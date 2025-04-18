#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <User_Setup.h> // Include the User_Setup.h file for TFT_eSPI

#define ILI9488_DRIVER      // Select the correct display driver

// ESP32 SPI Pin Configuration
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5   // Chip Select
#define TFT_DC   2   // Data/Command
#define TFT_RST  4   // Reset

// Touch Controller (XPT2046)
#define TOUCH_CS 15
#define TOUCH_IRQ 36  // Optional

// SD Card Configuration
#define SD_CS   13
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCLK 18

// Rotation settings
#define TFT_ROTATION 1  // Adjust for landscape/portrait mode

// Enable touch support
#define USE_TOUCH
#define TOUCH_CS 15


enum TftScreenMode {
    MAIN_MENU,
    MANUAL_TEST,
    AUTO_TEST,
    ABOUT,
    // SETTINGS,
    // LOGGING,
    ERROR
};

#endif