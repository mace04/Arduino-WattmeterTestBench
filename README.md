# ESP32 Wattmeter Test Bench

This project implements a comprehensive motor test bench on an ESP32 microcontroller, designed for measuring and analyzing the performance of electric motors, particularly in RC applications. It provides both local touchscreen interface and remote web-based control and monitoring.

## Hardware Capabilities

### Microcontroller
- ESP32 with dual-core processing for concurrent tasks
- Real-time sensor data acquisition and motor control

### Sensors and Measurement
- **Voltage Sensor**: Analog input with voltage divider (GPIO 35)
  - Measures battery voltage with configurable calibration
  - Running average filtering for stable readings
- **Current Sensor**: Analog input with voltage divider (GPIO 34)
  - Measures current draw with configurable calibration
  - Calculates power consumption in watts
  - Tracks accumulated energy consumption in mAh
- **Thrust Sensor**: HX711 load cell amplifier (GPIO 33/25)
  - Measures thrust force in grams
  - Configurable calibration and offset
- **Throttle Input**: Analog input (GPIO 36)
  - Reads manual throttle control
  - Supports percentage-based control

### Motor Control
- **ESC Control**: Servo output (GPIO 21)
  - PWM control for Electronic Speed Controllers
  - Supports throttle range 1000-2000μs
- **Throttle Cut Switch**: Digital input with interrupt (GPIO 22)
  - Emergency stop functionality
  - Triggers calibration modes

### Display and Interface
- **TFT Display**: ILI9341 or compatible with TFT_eSPI library
  - 320x240 resolution with rotation support
  - Real-time data visualization
- **Touchscreen**: XPT2046 resistive touchscreen
  - Capacitive sensing with interrupt support
  - Debounced touch handling

### Storage
- **SD Card**: SPI interface for data logging
  - File storage and retrieval
  - Test data export
- **Internal Storage**: SPIFFS filesystem
  - Web server file hosting
  - Configuration storage

### Connectivity
- **WiFi**: Station mode for network connectivity
  - Configurable SSID and password
  - Web server hosting on port 80
- **USB Serial**: 115200 baud for debugging and logging

## Software Capabilities

### Operating Modes
- **Normal Mode**: Standard operation with menu navigation
- **Calibration Mode**: Sensor calibration routines
  - Voltage/current sensor calibration
  - Load cell (thrust) calibration
  - Triggered by throttle cut switch

### Motor Testing
- **Manual Test Mode**: Direct throttle control
  - Real-time sensor feedback
  - Start/stop controls
- **Auto Test Mode**: Automated test sequences
  - Configurable warm-up duration
  - Multi-phase testing with ramp-up/ramp-down
  - Phase duration control
  - Automatic data logging

### Display Interface (TFT)
- **Main Menu**: Navigation between functions
  - Motor Test (Manual/Auto)
  - About screen
  - Profile selection
- **Motor Test Screen**: Real-time monitoring
  - Voltage, current, thrust, power panels
  - Consumption and elapsed time display
  - Throttle percentage indicator
  - Start/stop and back controls
- **Calibration Screens**: Guided sensor calibration
  - Step-by-step instructions
  - Touch-based confirmation
- **Message Boxes**: User feedback and error reporting

### Web Interface
- **Main Menu**: Web-based navigation
  - Links to all major functions
  - Live log display
- **Test Profiles**: Motor configuration
  - Motor name and type (EDF vs propeller)
  - Propeller pitch settings
  - Profile storage and retrieval
- **Settings Page**: Comprehensive configuration
  - WiFi network settings
  - Sensor calibration parameters
  - Maximum current/thrust limits
  - Test duration settings
- **File Management**:
  - SD card file browser and download
  - Internal storage (SPIFFS) access
  - File upload capabilities
- **Firmware Update**: Over-the-air updates
  - Binary file upload
  - Progress indication
- **Real-time Streaming**: Live data feed
  - WebSocket-based data transmission
  - Continuous sensor updates

### Configuration and Settings
- **Persistent Storage**: Settings saved to SPIFFS
  - WiFi credentials
  - Sensor calibration values
  - Test parameters
- **JSON-based Configuration**: ArduinoJson library
  - Structured settings management
  - Easy web interface integration

### Data Processing
- **Real-time Calculations**:
  - Power = Voltage × Current
  - Energy consumption accumulation
  - Elapsed time tracking
- **Filtering**: Running average for sensor stability
- **Timer Functions**: Start, pause, reset capabilities

### Communication Protocols
- **WebSocket Events**: Real-time notifications
  - Debug messages
  - Error reporting
  - Log updates
- **HTTP REST API**: Web server endpoints
  - GET/POST for settings and profiles
  - File upload/download
  - Firmware update

### Multi-threading
- **Dual-core Utilization**:
  - Core 0: Web server, motor control, sensor reading
  - Core 1: Touchscreen handling or calibration tasks
- **Task Management**: FreeRTOS tasks with priorities

### Safety Features
- **Throttle Cut**: Emergency stop mechanism
- **Calibration Guards**: Prevent accidental calibration
- **Error Handling**: Comprehensive error reporting
- **Debounced Inputs**: Stable touch and switch detection

### Development and Maintenance
- **Version Management**: Automatic version increment
- **Debug Logging**: Serial and web-based logging
- **Modular Architecture**: Separate classes for each subsystem
  - MotorControl, Settings, Sensors, WebServerHandler
  - TFT screens with inheritance

## Dependencies
- HX711: Load cell amplifier
- TFT_eSPI: TFT display driver
- XPT2046_Touchscreen: Touchscreen controller
- ESP32Servo: Servo motor control
- AsyncTCP/ESPAsyncWebServer: Asynchronous web server
- ArduinoJson: JSON parsing and generation
- ESP32 core libraries

## File Structure
- `src/`: Main source code
- `include/`: Header files
- `data/`: Web server files (HTML, CSS, JS)
- `lib/`: Custom libraries (if any)
- `test/`: Test files
- `schematics/`: Hardware schematics

This system provides a complete solution for motor testing with both local and remote interfaces, suitable for RC hobbyists and engineers needing precise motor performance measurements.</content>
<parameter name="filePath">d:\dev\Microcontrollers\PlatformIO\ESP32\ESP32-WattmeterTestBench\README.md