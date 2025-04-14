Got it! Here’s your **updated ESP32 pinout mapping** with the **Throttle Cut** now using **GPIO0** and the **HX711 Weight Sensor Data (DT)** now using **GPIO22**:

### **Pinout Mapping**
| **Component**              | **ESP32 Pin**       | **Function**              | **Purpose** |
|----------------------------|---------------------|--------------------------|-------------|
| **TFT LCD - VCC**          | 3.3V               | Power                   | Powers the display |
| **TFT LCD - GND**          | GND                | Ground                  | Common ground connection |
| **TFT LCD - CS**           | GPIO5              | Chip Select             | Activates the TFT module |
| **TFT LCD - DC**           | GPIO4              | Data/Command            | Selects between command and data transmission |
| **TFT LCD - RESET**        | GPIO16             | Reset                   | Resets the TFT display |
| **TFT LCD - WR**           | GPIO17             | Write Strobe            | Signals a write operation to the TFT |
| **TFT LCD - RD**           | GPIO21             | Read Strobe             | Signals a read operation from the TFT |
| **TFT LCD - D0 to D7**     | GPIO12, GPIO13, GPIO26, GPIO25, GPIO27, GPIO14, GPIO33, GPIO32 | Parallel Data Bus | Sends image data to the TFT |
| **Touchscreen - T_CLK**    | GPIO18             | SPI Clock (Shared with SD) | Synchronizes touchscreen data transfer |
| **Touchscreen - T_CS**     | GPIO15             | Touch Chip Select       | Activates touchscreen module |
| **Touchscreen - T_DIN**    | GPIO23             | SPI Data In (Shared with SD) | Receives data from the touchscreen |
| **Touchscreen - T_DO**     | GPIO19             | SPI Data Out (Shared with SD) | Sends data to the touchscreen |
| **Touchscreen - T_IRQ**    | GPIO36             | Touch Interrupt         | Detects touch events |
| **SD Card - SD_SCK**       | GPIO18             | SPI Clock (Shared with Touchscreen) | Synchronizes SD card data transfer |
| **SD Card - SD_MISO**      | GPIO19             | SPI MISO (Shared with Touchscreen) | Sends data from SD card to ESP32 |
| **SD Card - SD_MOSI**      | GPIO23             | SPI MOSI (Shared with Touchscreen) | Sends data from ESP32 to SD card |
| **SD Card - SD_CS**        | GPIO2              | Chip Select (Unique)    | Activates SD card module |
| **Voltage Sensor**         | GPIO34             | Analog Input (ADC1)     | Reads voltage measurements |
| **Current Sensor**         | GPIO35             | Analog Input (ADC1)     | Reads current measurements |
| **Throttle Control**       | GPIO39             | Analog Input (ADC1)     | Reads throttle position |
| **Throttle Cut**           | GPIO0              | Digital Input (Unique)  | Emergency stop signal |
| **ESC Output (PWM)**       | GPIO3              | PWM Output (Unique)     | Controls motor speed via ESC |
| **HX711 - DT**             | GPIO22             | Weight Sensor Data      | Weight measurement input |
| **HX711 - SCK**            | GPIO1              | Weight Sensor Clock     | Weight sensor clock |



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

Create a main menu screen file for a Starbun Parallel 3.5" TFT LCD Colour Touch Panel with the follwing requirements
    * Use LVGL library
    * initalise TFT panel using pinout table from "ESP32 pinout mapping.md" file
    * Background colour set to NAVY
    * A menu option for Start Manual Test
    * A menu option for Start Auto Test
    * A menu option for About
    * Menu options should be buttons that can be selected by touch
    * Determining menu selection should be in separate function 
    * Button colour should be green with an black outline
    * Buttons colour should changed to red when pressed
    * Remove button handle event after one of the buttons is pressed







