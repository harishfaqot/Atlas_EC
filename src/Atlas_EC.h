#ifndef ATLAS_EC_H
#define ATLAS_EC_H

#include <Wire.h>
#include <Arduino.h>

class AtlasEC {
public:
    AtlasEC(uint8_t i2c_address = 0x64);
    
    void begin();
    float readEC();
    
    // Configuration methods
    void setActiveMode(bool active);
    void setLED(bool on);
    void setProbeKValue(float k_value);
    void setTemperatureCompensation(float temperature);
    
    // Calibration methods
    void clearCalibration();
    void dryCalibration();
    void singlePointCalibration(float value);
    void lowPointCalibration(float value);
    void highPointCalibration(float value);
    
    // Debug control
    void setDebugMode(bool enable);
    void setECDebugMode(bool enable);
    
    // Device information
    void getDeviceInfo();
    uint8_t getI2CAddress();
    
    // Main loop processing (for handling serial commands)
    void processCommands();

private:
    uint8_t _i2c_address;
    bool _debug_mode;
    bool _ec_debug_mode;
    float _last_conductivity;
    
    // I2C communication methods
    void i2cRead(uint8_t reg, uint8_t bytes_to_read);
    void i2cWriteByte(uint8_t reg, uint8_t data);
    void i2cWriteLong(uint8_t reg, unsigned long data);
    
    // Command parsing and execution
    void parseSerialData(const char* data);
    void executeCommand(const char* cmd, const char* data1, const char* data2);
    void explainCommands();
    
    // Command handlers
    void handleAddressCommand(const char* data1, const char* data2);
    void handleInterruptCommand(const char* data1);
    void handleLEDCommand(const char* data1);
    void handleNRACommand(const char* data1);
    void handleCalibrationCommand(const char* data1, const char* data2);
    void handleTemperatureCommand(const char* data1);
    void handleProbeCommand(const char* data1);
    
    // Union for data handling
    union SensorData {
        uint8_t i2c_data[4];
        unsigned long long_value;
        unsigned int int_value;
    };
    
    SensorData _sensor_data;
};

#endif