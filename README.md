# Atlas EC Sensor Library

## Overview

A comprehensive Arduino library for interfacing with Atlas Scientific conductivity (EC) sensors via I2C communication.  
This library provides both simple API calls for basic readings and a full command interface for advanced configuration and calibration.

## Features

- Read conductivity measurements from Atlas Scientific EC sensors
- Configure sensor settings through I2C protocol
- Support for multiple calibration procedures
- Temperature compensation functionality
- Probe K-value configuration
- LED status control
- Device information retrieval
- Debug mode for development and troubleshooting

## Installation

### Through Arduino Library Manager
1. Open Arduino IDE
2. Navigate to **Sketch > Include Library > Manage Libraries**
3. Search for **Atlas_EC**
4. Click **Install**

### Manual Installation
1. Download the latest release from [GitHub](https://github.com/harishfaqot/Atlas_EC)
2. Extract the ZIP file
3. Copy the `Atlas_EC` folder to your Arduino **libraries** directory
4. Restart Arduino IDE

## Hardware Setup

### Connections
- **SDA** → Arduino SDA pin
- **SCL** → Arduino SCL pin  
- **VCC** → 5V
- **GND** → GND

### Default I2C Address
The default I2C address for the EC sensor is **0x64** (100 decimal).

## Quick Start

```cpp
#include <Atlas_EC.h>

AtlasEC ecSensor;

void setup() {
    Serial.begin(9600);
    ecSensor.begin();
}

void loop() {
    float ecValue = ecSensor.readEC();
    Serial.print("Conductivity: ");
    Serial.print(ecValue, 4);
    Serial.println(" mS/cm");
    delay(2000);
}
```

## API Reference

### Basic Methods
- `begin()` – Initialize sensor and set to active mode  
- `readEC()` – Read current EC value in standard units  

### Configuration Methods
- `setActiveMode(bool active)` – Enable/disable sensor readings  
- `setLED(bool on)` – Control status LED  
- `setProbeKValue(float k_value)` – Set probe K-value  
- `setTemperatureCompensation(float temperature)` – Apply temperature compensation  

### Calibration Methods
- `clearCalibration()` – Clear all calibration data  
- `dryCalibration()` – Perform dry calibration  
- `singlePointCalibration(float value)` – Single-point calibration  
- `lowPointCalibration(float value)` – Low-point calibration  
- `highPointCalibration(float value)` – High-point calibration  

### Utility Methods
- `setDebugMode(bool enable)` – Enable/disable debug output  
- `setECDebugMode(bool enable)` – Enable/disable EC-specific debug  
- `getDeviceInfo()` – Print device information  
- `getI2CAddress()` – Get current I2C address  
- `processCommands()` – Process serial commands  

## Serial Command Interface

The library supports an extensive set of serial commands for interactive configuration:

### Device Information
- `i` – Display device type and version  

### Address Configuration
- `adr,?` – Read current I2C address  
- `adr,unlock` – Unlock address change register  
- `adr,lock` – Lock address change register  
- `adr,new,[address]` – Change I2C address  

### Calibration Commands
- `cal,?` – Read calibration status  
- `cal,clr` – Clear calibration  
- `cal,dry` – Dry calibration  
- `cal,[value]` – Single-point calibration  
- `cal,low,[value]` – Low-point calibration  
- `cal,high,[value]` – High-point calibration  

### Other Commands
- `led,on/off` – Control LED  
- `on/off` – Start/stop readings  
- `t,[value]` – Set temperature compensation  
- `k,[value]` – Set probe K-value  
- `r` – Take reading  

## Examples

### Basic Reading Example
```cpp
#include <Atlas_EC.h>

AtlasEC ecSensor;

void setup() {
    Serial.begin(9600);
    ecSensor.begin();
}

void loop() {
    float ec = ecSensor.readEC();
    Serial.print("EC Value: ");
    Serial.println(ec, 4);
    delay(1000);
}
```

### Interactive Example
```cpp
#include <Atlas_EC.h>

AtlasEC ecSensor;

void setup() {
    Serial.begin(9600);
    ecSensor.begin();
    ecSensor.setDebugMode(true);
    Serial.println("Type '?' for command list");
}

void loop() {
    ecSensor.processCommands();
    delay(100);
}
```

## Calibration Procedure

1. Perform dry calibration:  
   ```
   cal,dry
   ```
2. Calibrate with known solution (example: 1413 µS/cm):  
   ```
   cal,1413
   ```
3. Verify calibration:  
   ```
   cal,?
   ```

## Troubleshooting

- Ensure proper I2C connections and pull-up resistors  
- Verify sensor power supply stability  
- Check I2C address configuration  
- Confirm calibration procedure is followed  

### Debug Mode
Enable debug mode for detailed communication feedback:
```cpp
ecSensor.setDebugMode(true);
```

## Compatibility

### Supported Architectures
- AVR (Uno, Mega, Leonardo)  
- SAMD (Zero, MKR series)  
- ESP32  
- ESP8266  
- STM32  

### Tested Sensors
- Atlas Scientific EZO-EC Circuit  
- Atlas Scientific EC sensor modules  

## Version History

- **v1.0.0** – Initial release  
  - Basic EC reading functionality  
  - Full command interface support  
  - Calibration procedures  
  - Configuration options  

## Contributing

Contributions are welcome!  
Please ensure:
- Code follows Arduino library guidelines  
- Proper documentation is included  
- Examples are tested and functional  

## License

This library is released under the **MIT License**. See [LICENSE](LICENSE) file for details.

## Support

If you encounter issues:
1. Check provided examples  
2. Verify hardware connections  
3. Ensure proper calibration  
4. Enable debug mode for logs  

## Documentation

Full documentation is available in the library header files and example sketches. Each method includes detailed comments explaining functionality and parameters.

## References

- Atlas Scientific EZO-EC Datasheet  
- I2C Communication Protocol  
- Arduino Library Specification  
