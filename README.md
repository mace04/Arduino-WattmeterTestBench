# ESP32 Wattmeter Test Bench

A comprehensive motor test bench system built on ESP32, designed for measuring and analyzing the performance of electric motors in RC applications (propellers and EDF units). The system provides both local touchscreen interface and web-based remote control with real-time monitoring capabilities.

## Project Overview

This test bench measures voltage, current, power consumption, thrust, and monitors motor performance over time. It supports both manual throttle control and automated test sequences with configurable parameters. The dual-core ESP32 handles sensor acquisition and web services concurrently, making it ideal for precision motor testing.

**Key Features:**
- Real-time voltage, current, thrust, and power measurement
- Manual and automated testing modes
- TFT touchscreen interface with multiple screens
- Web-based configuration and monitoring
- Over-the-air firmware updates
- Dual-core processing for responsive operation
- Persistent storage for settings and test profiles

## Main Capabilities

### 1. Motor Testing Modes

#### Manual Mode
- Direct throttle control from analog input or TFT interface
- Real-time sensor data display (voltage, current, thrust, power)
- Energy consumption tracking (mAh)
- Start/stop control with safety interlocks
- Emergency throttle cut via hardware switch

#### Auto Test Mode
- Automated test sequences with 5 phases:
  - Phase 0: Ramp up from 0% to 50% throttle (configurable duration)
  - Phase 1: Hold at 50% throttle (configurable duration)
  - Phase 2: Ramp up from 50% to 100% throttle
  - Phase 3: Hold at 100% throttle
  - Phase 4: Ramp down from 100% to 0%
- Configurable warm-up and phase durations
- Automatic data logging throughout test

### 2. Sensor Measurements

- **Voltage**: 12-bit ADC with calibration and running average filter
- **Current**: 12-bit ADC with calibration and running average filter  
- **Thrust**: HX711 load cell amplifier with configurable scale and offset
- **Power**: Calculated from voltage × current
- **Energy Consumption**: Accumulated mAh over time
- **Elapsed Time**: Test duration timer (mm:ss format)

### 3. Display Interface (TFT Touchscreen)

- **Main Menu**: Access manual test, auto test, about screen, and profile selection
- **Motor Test Screen**: Real-time data panels showing all sensor readings
- **Calibration Mode**: Guided calibration for voltage, current, and thrust sensors
- **Scale Calibration**: Separate mode for calibrating the load cell
- **Message Boxes**: User notifications and error messages

### 4. Web Interface

Accessible via WiFi network or ESP32 hotspot (fallback mode)

#### Pages:
- **Home (`/`)**: Main menu with navigation links and live event log
- **Settings (`/settings`)**: Configure all system parameters
- **Test Profile (`/testprofile`)**: Define motor specifications
- **Internal Storage (`/spiffs`)**: Browse and view SPIFFS files
- **SD Card (`/sdcard`)**: Access SD card files (if available)
- **Firmware Update (`/update`)**: OTA firmware and filesystem updates

#### Real-time Features:
- **Server-Sent Events (`/events`)**: Live debug, error, and log messages
- **Streaming (`/stream`)**: Real-time sensor data feed
- **File Download**: Download files from SPIFFS or SD card

### 5. Configuration & Settings

All settings stored in `/settings.json` on SPIFFS:

- **WiFi**: SSID and password
- **Voltage Sensor**: Sensitivity (volts per point) and offset
- **Current Sensor**: Sensitivity (volts per point) and offset
- **Thrust Sensor**: Scale and offset values
- **Safety Limits**: Maximum current and thrust thresholds
- **Test Parameters**: Phase duration and warm-up duration

### 6. Safety Features

- **Throttle Cut Switch**: Hardware emergency stop (GPIO 22)
- **Pre-start Checks**: Validates throttle at zero before starting
- **Error Handling**: Comprehensive error messages via web and display
- **HX711 Timeout Protection**: Prevents hanging on sensor failures

## ESP32 GPIO Pin Mapping

### Display & Touch (SPI)
| Function | GPIO | Description |
|----------|------|-------------|
| TFT_MISO | 19 | SPI MISO |
| TFT_MOSI | 23 | SPI MOSI |
| TFT_SCLK | 18 | SPI Clock |
| TFT_CS | 5 | TFT Chip Select |
| TFT_DC | 2 | Data/Command Select |
| TFT_RST | 4 | TFT Reset |
| TOUCH_CS | 15 | Touch Chip Select |
| TOUCH_IRQ | 13 | Touch Interrupt (optional) |

### SD Card (Shared SPI)
| Function | GPIO | Description |
|----------|------|-------------|
| SD_CS | 17 | SD Card Chip Select |
| SD_MOSI | 23 | SPI MOSI (shared) |
| SD_MISO | 19 | SPI MISO (shared) |
| SD_SCLK | 18 | SPI Clock (shared) |

### Sensors (ADC)
| Function | GPIO | ADC Channel | Description |
|----------|------|-------------|-------------|
| VOLTAGE_SENSOR_PIN | 35 | ADC1_CH7 | Battery voltage measurement |
| CURRENT_SENSOR_PIN | 34 | ADC1_CH6 | Current sensor input |
| THROTTLE_CONTROL_PIN | 36 | ADC1_CH0 | Manual throttle input |

### Motor Control & Sensors
| Function | GPIO | Description |
|----------|------|-------------|
| ESC_OUTPUT_PIN | 21 | PWM output to ESC (1000-2000μs) |
| THROTTLE_CUT_PIN | 22 | Emergency stop switch (INPUT_PULLUP) |
| HX711_DT_PIN | 26 | Load cell data pin |
| HX711_SCK_PIN | 25 | Load cell clock pin |

### Voltage Divider Details
- **Voltage Sensor**: R1=14.98kΩ, R2=2.14kΩ (ratio ~105.295)
- **Current Sensor**: R1=14.86kΩ, R2=13.59kΩ (ratio ~240.350)

## Web Interface Details

### Main Menu (`/` or `/index`)
- Navigation buttons to all major functions
- Real-time event log at bottom (debug/error/info messages)
- Clean grid layout with CSS styling

### Settings Page (`/settings`)
Organized into sections:
- **WiFi Settings**: SSID and password
- **Voltage Sensor**: Sensitivity and offset calibration
- **Current Sensor**: Sensitivity, offset, and maximum current
- **Thrust Sensor**: Scale, offset, and maximum thrust
- **Test Settings**: Phase duration and warm-up time

Settings are saved to SPIFFS and persist across reboots. Success banner shows after saving.

### Test Profile Page (`/testprofile`)
Configure motor under test:
- **Motor Name**: Up to 50 characters (alphanumeric + spaces)
- **Motor Type**: Propeller or EDF (radio button)
- **Propeller Diameter**: 0-150 inches
- **Propeller Pitch**: 1-20 inches (hidden if EDF selected)
- **Battery Cells**: 1-9 cells

Profile saved to `/testprofile.json` on SPIFFS.

### Internal Storage (`/spiffs`)
- Lists all files in SPIFFS filesystem
- Radio buttons for file selection
- View button displays file content (JSON pretty-printed)
- Text area shows file contents

### Firmware Update (`/update`)
- Upload firmware binary (`.bin`)
- Upload filesystem image (SPIFFS)
- Upload type selector: `firmware` or `filesystem`
- Settings automatically backed up and restored during filesystem updates
- Automatic reboot after successful update

### File Download (`/getfile`)
Query parameters:
- `filename`: File to download
- `from`: Either `spiffs` or `sdcard`

### Real-time Streaming (`/stream`)
Returns JSON with current sensor readings:
```json
{
  "voltage": 12.34,
  "current": 5.67,
  "thrust": 890,
  "power": 70,
  "consumption": 123,
  "time": "02:15",
  "throttle": 75
}
```

### Server-Sent Events (`/events`)
Three event types:
- `debug`: Debug messages from system
- `error`: Error notifications
- `log`: General log messages

JavaScript on web pages subscribes to these for live updates.

## Known Issues

### 1. Running Average Not Fully Implemented
**Location**: [sensors.cpp](src/sensors.cpp#L85-L112)

The `readVoltageGpio()` and `readCurrentGpio()` functions have running average code that is commented out with an early return statement. The averaging arrays are allocated but not used.

```cpp
// Current code returns immediately:
return vOut;
// Dead code below calculates average
```

**Impact**: Sensor readings may be noisier than intended.

**Fix**: Remove early returns and enable the averaging logic.

### 2. HX711 Conditional Compilation
**Location**: Multiple files

The HX711 load cell code is wrapped in `#ifdef HX711_h` checks, but the header is always included. This creates unnecessary conditional compilation.

**Impact**: Code complexity without benefit; HX711 is always compiled in.

**Fix**: Either remove conditionals or make HX711 truly optional via build flags.

### 3. Static Variables in Auto Test State Machine
**Location**: [motorControl.cpp](src/motorControl.cpp#L143-L211)

Each case in the auto test state machine uses `static bool stageXLogged` variables that are never reset. If auto test is run multiple times, debug messages won't appear after the first run.

**Impact**: Reduced debugging visibility on subsequent auto tests.

**Fix**: Reset static flags in `reset()` or `startAuto()` methods.

### 4. Unreachable Code After Return Statements
**Location**: [sensors.cpp](src/sensors.cpp#L90-L98)

Several functions have unreachable code after return statements (running average calculations).

**Impact**: Compiler warnings and misleading code.

**Fix**: Remove early returns or delete unreachable code.

### 5. Settings Restoration During Filesystem Update
**Location**: [WebServerHandler.cpp](src/WebServerHandler.cpp#L318-L360)

The settings backup/restore during filesystem updates loads settings into RAM but doesn't properly handle the case where SPIFFS is reformatted during update.

**Impact**: Settings may be lost during filesystem updates.

**Fix**: Backup to a file outside SPIFFS or implement more robust restoration.

### 6. SD Card Not Fully Integrated
**Location**: Multiple files

SD card is initialized in pins and main.cpp includes SD.h, but actual SD card functionality appears incomplete. No route handler for `/sdcard` endpoint exists.

**Impact**: SD card file browsing doesn't work from web interface.

**Fix**: Implement `handleFileAccess()` function for SD card access.

## TODO List

### High Priority
- [ ] Fix running average implementation for voltage/current sensors
- [ ] Implement SD card file browser web endpoint (`/sdcard`)
- [ ] Add data logging to SD card during tests (CSV format)
- [ ] Reset static flags in auto test state machine
- [ ] Remove unreachable code after early returns

### Medium Priority
- [ ] Add graphing capabilities to web interface (Chart.js or similar)
- [ ] Implement test result export functionality
- [ ] Add WiFi configuration portal (captive portal on first boot)
- [ ] Create calibration wizard with step-by-step instructions
- [ ] Add authentication to web interface (username/password)
- [ ] Implement motor efficiency calculations (thrust/watt)

### Low Priority
- [ ] Add support for multiple test profiles (save/load by name)
- [ ] Create comparison view for multiple test runs
- [ ] Add temperature sensor monitoring (motor and ESC)
- [ ] Implement RPM measurement (optical sensor)
- [ ] Add battery level estimation (LiPo cell count detection)
- [ ] Create mobile-friendly responsive web design
- [ ] Add WebSocket for bidirectional communication (replace SSE)

### Code Quality
- [ ] Refactor HX711 conditional compilation (make truly optional)
- [ ] Add unit tests for calculation functions
- [ ] Improve error handling and user feedback
- [ ] Document web API endpoints (OpenAPI/Swagger)
- [ ] Add comments to complex functions
- [ ] Implement proper settings validation

### Documentation
- [ ] Create wiring diagram with pin connections
- [ ] Add calibration procedure documentation
- [ ] Write user manual for web interface
- [ ] Document auto test sequence in detail
- [ ] Add troubleshooting guide
- [ ] Create video tutorials

## Dependencies

### PlatformIO Libraries
```ini
lib_deps = 
    bogde/HX711 @ ^0.7.5
    ArduinoJson
    bodmer/TFT_eSPI @ ^2.5.43
    paulstoffregen/XPT2046_Touchscreen
    madhephaestus/ESP32Servo
    esp32async/AsyncTCP
    esp32async/ESPAsyncWebServer
```

### Platform
- Framework: Arduino
- Platform: Espressif32
- Board: ESP32 DOIT DevKit v1
- Monitor Speed: 115200 baud

### Partition Scheme
Custom partition: `default_1.5MBapp_spiffs768KB.csv`
- App: ~1.5 MB
- SPIFFS: 768 KB

## File Structure
```
ESP32-WattmeterTestBench/
├── src/                          # Source files
│   ├── main.cpp                  # Main program entry
│   ├── motorControl.cpp          # ESC and throttle control
│   ├── sensors.cpp               # Sensor reading functions
│   ├── Settings.cpp              # Settings management
│   ├── WebServerHandler.cpp      # Web server and API
│   ├── TftMainMenu.cpp           # Main menu screen
│   ├── TftMotorTest.cpp          # Test screen UI
│   ├── TftCalibrate.cpp          # Calibration UI
│   └── [other TFT screens]
├── include/                      # Header files
│   ├── motorControl.h
│   ├── sensors.h
│   ├── Settings.h
│   ├── WebServerHandler.h
│   ├── tft_config.h
│   └── [other headers]
├── data/                         # Web server files (SPIFFS)
│   ├── index.html                # Main menu page
│   ├── settings.html             # Settings page
│   ├── testprofile.html          # Test profile page
│   ├── spiffs.html               # File browser
│   ├── update.html               # OTA update page
│   ├── style.css                 # Shared CSS
│   ├── common.js                 # Shared JavaScript
│   └── index.js
├── schematics/                   # Hardware design files
│   └── WattThrustMeter.fzz       # Fritzing schematic
├── platformio.ini                # PlatformIO configuration
├── increment_version.py          # Auto version script
├── version.txt                   # Current version
├── ESP32 pinout mapping.md       # Pin documentation
└── README.md                     # This file
```

## Getting Started

1. **Hardware Setup**: Wire sensors and display according to pin mapping
2. **Flash Firmware**: Use PlatformIO to build and upload
3. **Upload Filesystem**: Upload SPIFFS image with web files
4. **Configure WiFi**: Edit settings via web interface or serial
5. **Calibrate Sensors**: Use calibration mode (hold throttle cut during boot)
6. **Run Tests**: Select manual or auto mode from TFT or web interface

## License

[Add your license here]

## Author

[Add your name/contact here]