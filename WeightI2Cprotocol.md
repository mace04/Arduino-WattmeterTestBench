# Weight Sensor I2C Protocol

This document defines the I2C protocol used between the ESP32 firmware and the external weight sensor bridge device.

## Overview

- **Bus mode**: ESP32 is I2C **master**
- **Device address**: `0x11` (`WEIGHT_I2C_ADDRESS`)
- **ESP32 pins**:
  - `SDA = GPIO26` (`WEIGHT_I2C_SDA_PIN`)
  - `SCL = GPIO25` (`WEIGHT_I2C_SCL_PIN`)
- **I2C clock**: `100 kHz`

## Data Types and Endianness

- Floating point values are transmitted as **32-bit IEEE-754 float**.
- Integer offset is transmitted as **signed 32-bit integer** (`int32_t`).
- Multi-byte values are sent as raw in-memory bytes from the ESP32 side.
- Both ends must use compatible byte order and type sizes.

## Message Types

## 1) Weight Read (ESP32 -> Device, then Device -> ESP32)

### Request
ESP32 requests 4 bytes:

- `Wire.requestFrom(0x11, 4)`

### Response payload (4 bytes)

```text
Byte 0..3: float weight_grams
```

### Firmware behavior on receive

- If exactly 4 bytes are returned and value is finite: cache and return new weight.
- If byte count is wrong, stream underruns, or float is non-finite:
  - generate error event (throttled by warning flag)
  - return last known valid weight
  - if no valid value has ever been received, return `0`

## 2) Calibration Write (ESP32 -> Device)

### Purpose
Set calibration parameters used by the weight bridge.

### Payload (8 bytes)

```text
Byte 0..3: float scale
Byte 4..7: int32_t offset
```

### Used by firmware

- `calibrateWeightSensor()` sends default calibration:
  - `scale = 1.0f`
  - `offset = 0`

## 3) Tare Reset Command (ESP32 -> Device)

### Purpose
Request a tare/reset action from the weight bridge.

### Command byte

- `I2C_CMD_TARE_RESET = 0xA5`

### Payload

```text
Byte 0: 0xA5
```

### Used by firmware

- `resetWeightSensor()` sends this command as a single-byte I2C transmission.

## Error Handling Expectations

The device should:

- Respond to read requests with exactly 4 bytes when data is available.
- Gracefully handle malformed/unexpected calibration writes.
- Accept and process `0xA5` tare command idempotently when possible.

The ESP32 firmware currently:

- Validates response length and float sanity (`isfinite`).
- Emits error events on communication/payload failures.
- Uses last-known-value fallback to avoid UI/stream glitches when reads fail intermittently.

## Reference Implementation Mapping

- Protocol constants and declarations: `include/sensors.h`
- I2C send/read logic: `src/sensors.cpp`
  - `sendWeightCalibration(float, int32_t)`
  - `readWeightSensor()`
  - `resetWeightSensor()`

## Suggested Device-Side Contract

For best interoperability, implement the slave-side behavior as:

1. On write of 8 bytes: parse as `{scale, offset}` and apply calibration.
2. On write of 1 byte `0xA5`: perform tare/reset.
3. On master read request of 4 bytes: return latest weight as float grams.
4. Ensure response generation is quick and non-blocking.

## Versioning Note

If protocol changes are introduced later (new commands, CRC, sequence numbers), add a version byte or command frame format to maintain backward compatibility.
