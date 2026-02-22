# Arduino Wattmeter Test Bench

## Project Overview
The Arduino Wattmeter Test Bench project is designed to provide effective testing capabilities for various electric propulsion systems by measuring performance metrics such as power (Watts), current (Amperes), and voltage (Volts). The test bench supports both manual and automatic testing modes, allowing users to perform comprehensive assessments of the propulsion systems' efficiency and reliability.

## Main Capabilities
- **Test Modes**:
  - Manual Test Mode
  - Automatic Test Mode
- **Measurements**:
  - Current (A)
  - Voltage (V)
  - Power (W)
- **Safety Cutoffs**: Integrated safety features to prevent damage to components during operation.
- **EEPROM Settings**: Ability to store calibration settings for persistent measurements.

## Hardware and Library Dependencies
- **Hardware**:
  - Arduino Board (e.g., Arduino Uno)
  - HX711 Load Cell Amplifier
  - Electronic Speed Controller (ESC)
  - Push Buttons
- **Libraries**:
  - HX711 Library for load cell communication

## Detailed Pin/Port Mapping
- `PIN_VIN`: A3  
- `PIN_AIN`: A1  
- `HX711`: D9/D10  
- `ESC`: D11  
- `Throttle Input`: A7  
- `Button Interrupt`: D3  
- `Buttons`: D4/D5/D6/D7/D8  

## UI/Screens Description
The user interface includes various screens to navigate through test modes, display measurements in real-time, and present options for adjusting settings.

## Known Issues
- mAh integration bug leading to inaccurate readings.
- Average logic inconsistencies affecting computed averages.
- The PREVIOUS button condition incorrectly uses `||` instead of `&&`, causing unexpected behavior.
- Calibration offsets are not applied to measurements properly.
- Blocking warm-up loops may cause delays in responsiveness.
- Thrust unit label mismatch on the UI.

## TODO List
- Fix mAh integration bug.
- Review average logic implementation.
- Correct PREVIOUS button condition.
- Apply calibration offsets to measurements.
- Optimize warm-up loop logic.
- Standardize thrust unit labeling across the UI.

---
This overview provides a comprehensive foundation for further enhancements and functionality additions to the Arduino Wattmeter Test Bench project.