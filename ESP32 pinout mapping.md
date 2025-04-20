### **ESP32 Pin Mapping**
| **Panel Pin**        | **ESP32 Pin**  | **Function**            | **Purpose** |
|----------------------|---------------|------------------------|-------------|
| **LCD_RST**         | GPIO16         | Reset                  | Resets the TFT display |
| **LCD_CS (YP)**     | GPIO5          | Chip Select / Y+ (ADC) | Activates TFT module / Y-axis touch input |
| **LCD_RS (XM)**     | GPIO4          | Data/Command / X-      | Selects between command/data / X-axis touch control |
| **LCD_WR**          | GPIO17         | Write Strobe           | Signals a write operation to the TFT |
| **LCD_RD**          | GPIO21         | Read Strobe            | Signals a read operation from the TFT |
| **5V**              | 5V             | Power                  | Powers the display |
| **GND**             | GND            | Ground                 | Common ground connection |
| **3V3**             | 3.3V           | Power                  | Alternative power supply if required |
| **LCD_D0 (XP)**     | GPIO12         | Parallel Data Bus / X+ | Pixel data transfer / X-axis touch input |
| **LCD_D1 (YM)**     | GPIO13         | Parallel Data Bus / Y- | Pixel data transfer / Y-axis touch control |
| **LCD_D2**          | GPIO26         | Parallel Data Bus      | Pixel data transfer |
| **LCD_D3**          | GPIO25         | Parallel Data Bus      | Pixel data transfer |
| **LCD_D4**          | GPIO27         | Parallel Data Bus      | Pixel data transfer |
| **LCD_D5**          | GPIO14         | Parallel Data Bus      | Pixel data transfer |
| **LCD_D6**          | GPIO33         | Parallel Data Bus      | Pixel data transfer |
| **LCD_D7**          | GPIO32         | Parallel Data Bus      | Pixel data transfer |
| **SD_SS**           | GPIO2          | SD Card Chip Select    | Activates SD card module |
| **SD_DI (MOSI)**    | GPIO23         | SPI MOSI               | Sends data to SD card |
| **SD_DO (MISO)**    | GPIO19         | SPI MISO               | Receives data from SD card |
| **SD_SCK**          | GPIO18         | SPI Clock              | Synchronizes SD card data transfer |
| **Voltage Sensor**  | GPIO34         | Analog Input (ADC1)    | Reads voltage measurements |
| **Current Sensor**  | GPIO35         | Analog Input (ADC1)    | Reads current measurements |
| **Throttle Control**| GPIO39         | Analog Input (ADC1)    | Reads throttle position |
| **Throttle Cut**    | GPIO15         | Digital Input (Unique) | Emergency stop signal |
| **ESC Output (PWM)**| GPIO3          | PWM Output (Unique)    | Controls motor speed via ESC |
| **HX711 - DT**      | GPIO22         | Weight Sensor Data     | Weight measurement input |
| **HX711 - SCK**     | GPIO1          | Weight Sensor Clock    | Weight sensor clock |


Define mapping to ESP32 devkit v1 for the Starbun TFT LCD Color Touch Panel 3.5" TFT LCD Module 480x320 with the following pins on the panel
    * LCD_RST
    * LCD_CS
    * LCD_RS
    * LCD_WR
    * LCD_RD
    * 5V
    * GND
    * 3V3
    * LCD_D0
    * LCD_D1
    * LCD_D2
    * LCD_D3
    * LCD_D4
    * LCD_D5
    * LCD_D6
    * LCD_D7
    * SD_SS
    * SD_DI
    * SD_DO
    * SD_SCK
Also, the touchscreen uses the below pins from the above defined pins
    * LCD_CS as YP
    * LCD_RS as XM
    * LCD_D1 as YM
    * LCD0 as XP
Finally add the following requirements in the mapping
    * Analog input for voltage sensor
    * Analog input for current sensor
    * Analog input for throttle control
    * Digital input for throttle cut
    * PWM output for ESC control
    * DT for HX711 sensor
    * SCK for HX711 sensor

Define mapping to ESP32 devkit v1 for the 3.5 inch LCD TFT touch display with ili9488 chip Binghe 3.5 inch LCD display touch module with the following pins on the panel
    * VCC
    * GND 
    * CS
    * RESET
    * C/C
    * SDI
    * SCK
    * LED 
    * SDO(MISO)
    * T_CLK
    * TOUCH_CS
    * T_DIN
    * T_OUT
    * TOUCH_IRQ
    * SD_CS
    * SD_MOSI
    * SD_MISO
    * SD_SCK
Also add the following requirements in the mapping
    * Analog input for voltage sensor
    * Analog input for current sensor
    * Analog input for throttle control
    * Digital input for throttle cut
    * PWM output for ESC control
    * DT for HX711 sensor
    * SCK for HX711 sensor



# Sensors 
what is the best board_build.partitions to use for the following
    * Factory app 2MB
    * OTA app 2MB
    * SPIFFS 1.5MB

Create a sensors include file with the following

    Create a function readVoltageSensor to read voltage from the voltage sensor in GPIO34 with the following requirements
        * the pin is connected to a voltage divivder with R1 of 15K and R2 of 2.2K
        * use a running average to smooth readings

    Create a function readCurrentSensor to read voltage from the voltage sensor in GPIO35 with the following requirements
        * the sensor is connected to a ACS758KCB-150B-PFF-T
        * use a running average to smooth readings

    Create a function resetWeightSensor that resets the weight sensor usinh HX711

    Create a function readWeightSensor that reads weight in grams from the weight sensor HX711

Create an motorControl class with the following
    * A function setThrottleCut that sets a global variable for the throttle cut based on a boolean parameter
    * A function setThrottle with the following
        * read the value of throttle control from pin 39
        * if the throttle cut is not set sets the corresponding PWM setting to the esc output in pin 3
        * if the throttle cut is set esc must be set to no throttle
        * returns the throttle control value from pin 39 as a percent
    * create a second function setThottle to set the thottle using a parameter percent value and not from a reading from the THROTTLE_CONTROL_PIN with the rest of requirments the same as the original function
    * a function to get the value fo the throttle cut

Create a new include file to handle settings with the following:
    * The following key value pairs should be defined:
        * SSID name and password as string
        * volts per point for votlage sensor as double with initial value of 0.12790
        * voltage offset as double with initial value of 0.00
        * volts per point for current sensor as double with initial value of 0.02
        * current offset as double with a initial value of - 2.5
        * thrust offset as double with initial value of 0.00
        * maximum current as int with initial value of 100
        * maximum thrust as int with initial value of 4900
        * test phase duration in seconds as int with initial value of 45
    * ablity to read key value pair settings from SPIFFS and return with correct data type for the setting
    * ability to write key value pair settings to SPIFFS usng a correct data type for the setting
    * implement all getters and setters
    * the SPIFF settings files not be opened everytime a read is happening
    * the SPIFF settings file should be refreshed everytime a setter is called
    * include any changes required to platforio.ini for lib_deps

Create a new include file with the following:
    * A function to initialise WiFi connection. If wifi is not connected a hotspot should be created to direct to settings endpoint
    * a function to initialise web server endpoints using the WebServer class for settings, ota, file access to sd card files and realtime streaming of sensor readings. 
    * a function to handle GET settings that will respond with the settings.html from SPIFFS to the connected client
    * a function to handle POST settings. all setting values should be saved to the settings variable in main.cpp
    * a function to hanlde OTA firmware update
    * a function get all files from the SD Card and respond with a full list to the connected client
    * a function to stream sensor readings from sensors.h for voltage, current, weight and cacluated watts in csv to the connected client
    * include any changes required to platforio.ini for lib_deps

create a settings.html with the following requirements
    * use style.css
    * use index.js
    * Title heading on the top of the page of Motor Test Settings 
    * Display settings label and input for each setting defined to be replaced by the handleGetSettings function
    * All string input should be of type text
    * Password input should be of type password
    * All numeric inputs should be of type number
    * All labels should be displayed above the inputs in separate row
    * Settings should be displayed in 2 columns

create an update.html file with the following requirements
    * use style.css
    * Title heading on the top of the page of update firmware
    * Request client to select and input a firmware BIN file 
    * submit with value update

change WebServerHandler files to add capabilitie for OTA firmware update with the following
    * Handle for a GET update endpoint to display the update.htm to connected client
    * Handle for as POST update endpoint that will 
        * be able to and httpupload of the file 
        * when upload is finished perform an update of firmware 
        * determine if the update was a success or failure
        * restart the ESP 


Create a class TftMainMenu for a Starbun Parallel 3.5" TFT LCD Colour Touch Panel with the follwing requirements 
    * Use TFT_eSPI and XPT2046_Touchscreen libraries
    * For UX
        * Background colour set to NAVY
        * Menu options should be buttons that can be selected by touch
        * A menu option for Start Manual Test
        * A menu option for Start Auto Test
        * A menu option for About
        * All buttons should be centered vertically and horizontally within the screen
        * Button text should be centered within the boundaries of the button
        * Button colour should be green with an black outline
        * Buttons colour should changed to red when pressed
        * Button corners should be rounded
    * The class should have the following capabilities
        * accept tft panel and touchscreen as parameters in contructor
        * Determining menu selection should be in separate function 
        * Ability  to change to a different screen using the TftScreenMode enum when a button is pressed. This should instruct the core0Task to call the init method of a different class
        * Introduce a debounce capability to detect continuous press on buttons

Create a class TftUpdate class for a 3.5" TFT LCD Colour Touch Panel with the follwing requirements 
    * Use TFT_eSPI and XPT2046_Touchscreen libraries
    * Accept tft panel as parameter in contructor
    * For UX
        * Background colour set to NAVY
        * A rounded red box appears in the center of the screen 
        * The text "Updating Firmware" is displayed 
        * The text displayed in the box should be centered withing the boundaries of the box

Using TftMainMenu as a template create a class TftAbout for a 3.5" TFT LCD Colour Touch Panel with the following requirements 
    * Use TFT_eSPI and XPT2046_Touchscreen libraries
    * For UX
        * Background colour set to NAVY
        * All text set to white
        * Display the title "Motor Testbench" in bold
        * Display Firmware version
        * Display WiFi name
        * Display IP Address
        * All texts should be centered horizontally into two column one for descriptions and one for the values
    * The class should have the following capabilities
        * accept tft panel and touchscreen as parameters in contructor
        * The init method should chip select PANEL at the start and TOUCH at the end using the setCS?() function in tft_config files
        * the handleTouch method should check both the TOUCH_IRQ interrupt pin and ts.touch() for a touch  
        * TftAbout should be displayed when the About button in TftMainMenu is pressed
        * TftAbout should display TftMainMenu when any touch is detected anywhere inh the screen        