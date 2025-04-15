#ifndef TFT_CONFIG_H
#define TFT_CONFIG_H

// Define the driver for the TFT
// #define ILI9486_DRIVER

// Define the width and height of the display
#define LCD_WIDTH  320 
#define LCD_HEIGHT 480 

// Define the pins for the parallel interface
#define LCD_CS   5   // Chip select control pin
#define LCD_DC   4   // Data/command control pin
#define LCD_RST  16  // Reset pin (could connect to ESP32 EN pin)
#define LCD_WR   17  // Write strobe control pin
#define LCD_RD   21  // Read strobe control pin (if used)

#define LCD_D0   12  // Data bus bit 0
#define LCD_D1   13  // Data bus bit 1
#define LCD_D2   26  // Data bus bit 2
#define LCD_D3   25  // Data bus bit 3
#define LCD_D4   27  // Data bus bit 4
#define LCD_D5   14  // Data bus bit 5
#define LCD_D6   33  // Data bus bit 6
#define LCD_D7   32  // Data bus bit 7

// Optional: Define the backlight pin
// #define TFT_BL   22  // Backlight control pin
// #define TFT_BACKLIGHT_ON HIGH  // Backlight active state

// SD Card screen settings
#define TFT_MISO 19  // Master In Slave Out pin
#define TFT_MOSI 23  // Master Out Slave In pin
#define TFT_SCLK 18  // Clock pin
#define TFT_CS   2   // Chip select pin for the SD Card

// Touch screen pin mapping
#define YP 5   // Must be an analog pin, connected to LCD_CS
#define XM 4   // Must be an analog pin, connected to LCD_RS
#define YM 13  // Can be a digital pin, connected to LCD_D1
#define XP 12  // Can be a digital pin, connected to LCD_D0

// Touch screen boundaries
#define TS_LEFT 105
#define TS_RT   985
#define TS_TOP  990
#define TS_BOT  175

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