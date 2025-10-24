#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <User_Setup.h> // Include the User_Setup.h file for TFT_eSPI

#define ILI9488_DRIVER      // Select the correct display driver. Change this in TFT_eSPI/User_Setup.h

// ESP32 SPI Pin Configuration
// Change these in TFT_eSPI/User/Setups/Setup21_ILI9488.h as well
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5   // Chip Select
#define TFT_DC   2   // Data/Command
#define TFT_RST  4   // Reset

// Touch Controller (XPT2046)
#define USE_TOUCH
#define TOUCH_CS 15
#define TOUCH_IRQ 13  // Optional

// SD Card Configuration
#define SD_CS   17
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCLK 18

// Rotation settings
#define TFT_ROTATION 1  // Adjust for landscape/portrait mode



enum TftScreenMode {
    MAIN_MENU,
    MANUAL_TEST,
    AUTO_TEST,
    ABOUT,
    TEST_PROFILE,
    // LOGGING,
    ERROR
};

// Enum for devices
enum Device {
    PANEL,
    TOUCH,
    SDCARD
};

void setCS(Device device);
bool isCSActive(Device device);

struct Profile{
    String name;
    bool isEDF;
    int diameter;
    int pitch;
    int cells;
};


#endif