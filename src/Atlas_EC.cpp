#include "Atlas_EC.h"

// Register definitions
const uint8_t DEVICE_TYPE_REGISTER = 0x00;
const uint8_t ADDRESS_LOCK_REGISTER = 0x02;
const uint8_t NEW_ADDRESS_REGISTER = 0x03;
const uint8_t INTERRUPT_CONTROL_REGISTER = 0x04;
const uint8_t LED_CONTROL_REGISTER = 0x05;
const uint8_t ACTIVE_HIBERNATE_REGISTER = 0x06;
const uint8_t NEW_READING_REGISTER = 0x07;
const uint8_t PROBE_TYPE_REGISTER = 0x08;
const uint8_t CALIBRATION_REGISTER = 0x0A;
const uint8_t CALIBRATION_REQUEST_REGISTER = 0x0E;
const uint8_t CALIBRATION_CONFIRM_REGISTER = 0x0F;
const uint8_t TEMP_COMPENSATION_REGISTER = 0x10;
const uint8_t TEMP_CONFIRM_REGISTER = 0x14;
const uint8_t CONDUCTIVITY_REGISTER = 0x18;

// Command constants
const uint8_t ACTIVE_MODE = 0x01;
const uint8_t HIBERNATE_MODE = 0x00;
const uint8_t LED_ON = 0x01;
const uint8_t LED_OFF = 0x00;
const uint8_t CAL_CLEAR = 0x01;
const uint8_t CAL_DRY = 0x02;
const uint8_t CAL_SINGLE = 0x03;
const uint8_t CAL_LOW = 0x04;
const uint8_t CAL_HIGH = 0x05;

AtlasEC::AtlasEC(uint8_t i2c_address) 
    : _i2c_address(i2c_address), _debug_mode(false), _ec_debug_mode(false), _last_conductivity(0.0) {
}

void AtlasEC::begin() {
    Wire.begin();
    setActiveMode(true);
    _last_conductivity = 0.0;
}

float AtlasEC::readEC() {
    i2cRead(CONDUCTIVITY_REGISTER, 4);
    _last_conductivity = _sensor_data.long_value / 100.0;
    
    if (_ec_debug_mode) {
        Serial.print("EC= ");
        Serial.println(_last_conductivity);
    }
    
    return _last_conductivity; // Convert to standard units (uS/cm)
}

void AtlasEC::setActiveMode(bool active) {
    i2cWriteByte(ACTIVE_HIBERNATE_REGISTER, active ? ACTIVE_MODE : HIBERNATE_MODE);
}

void AtlasEC::setLED(bool on) {
    i2cWriteByte(LED_CONTROL_REGISTER, on ? LED_ON : LED_OFF);
}

void AtlasEC::setProbeKValue(float k_value) {
    unsigned long k_val = k_value * 100;
    _sensor_data.long_value = k_val;
    i2cWriteByte(PROBE_TYPE_REGISTER, _sensor_data.i2c_data[1]);
    i2cWriteByte(PROBE_TYPE_REGISTER + 1, _sensor_data.i2c_data[0]);
}

void AtlasEC::setTemperatureCompensation(float temperature) {
    unsigned long temp_val = temperature * 100;
    _sensor_data.long_value = temp_val;
    i2cWriteLong(TEMP_COMPENSATION_REGISTER, _sensor_data.long_value);
}

void AtlasEC::clearCalibration() {
    i2cWriteByte(CALIBRATION_REQUEST_REGISTER, CAL_CLEAR);
    delay(10);
}

void AtlasEC::dryCalibration() {
    i2cWriteByte(CALIBRATION_REQUEST_REGISTER, CAL_DRY);
    delay(15);
}

void AtlasEC::singlePointCalibration(float value) {
    unsigned long cal_val = value * 100;
    _sensor_data.long_value = cal_val;
    i2cWriteLong(CALIBRATION_REGISTER, _sensor_data.long_value);
    i2cWriteByte(CALIBRATION_REQUEST_REGISTER, CAL_SINGLE);
    delay(100);
}

void AtlasEC::lowPointCalibration(float value) {
    unsigned long cal_val = value * 100;
    _sensor_data.long_value = cal_val;
    i2cWriteLong(CALIBRATION_REGISTER, _sensor_data.long_value);
    i2cWriteByte(CALIBRATION_REQUEST_REGISTER, CAL_LOW);
    delay(100);
}

void AtlasEC::highPointCalibration(float value) {
    unsigned long cal_val = value * 100;
    _sensor_data.long_value = cal_val;
    i2cWriteLong(CALIBRATION_REGISTER, _sensor_data.long_value);
    i2cWriteByte(CALIBRATION_REQUEST_REGISTER, CAL_HIGH);
    delay(100);
}

void AtlasEC::setDebugMode(bool enable) {
    _debug_mode = enable;
}

void AtlasEC::setECDebugMode(bool enable) {
    _ec_debug_mode = enable;
}

void AtlasEC::getDeviceInfo() {
    i2cRead(DEVICE_TYPE_REGISTER, 2);
    Serial.print("Device: ");
    Serial.println(_sensor_data.i2c_data[1]);
    Serial.print("Version: ");
    Serial.println(_sensor_data.i2c_data[0]);
}

uint8_t AtlasEC::getI2CAddress() {
    return _i2c_address;
}

void AtlasEC::processCommands() {
    if (Serial.available()) {
        char computerdata[20];
        uint8_t bytes_received = Serial.readBytesUntil(13, computerdata, 19);
        computerdata[bytes_received] = 0;
        
        if (_debug_mode) {
            Serial.print("Received: ");
            Serial.println(computerdata);
        }
        
        parseSerialData(computerdata);
    }
}

// Private methods implementation
void AtlasEC::i2cRead(uint8_t reg, uint8_t bytes_to_read) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(_i2c_address, bytes_to_read);
    
    for (uint8_t i = bytes_to_read; i > 0; i--) {
        _sensor_data.i2c_data[i - 1] = Wire.read();
    }
    Wire.endTransmission();
}

void AtlasEC::i2cWriteByte(uint8_t reg, uint8_t data) {
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void AtlasEC::i2cWriteLong(uint8_t reg, unsigned long data) {
    _sensor_data.long_value = data;
    Wire.beginTransmission(_i2c_address);
    Wire.write(reg);
    for (int i = 3; i >= 0; i--) {
        Wire.write(_sensor_data.i2c_data[i]);
    }
    Wire.endTransmission();
}

void AtlasEC::parseSerialData(const char* data) {
    char buffer[20];
    strncpy(buffer, data, 19);
    buffer[19] = 0;
    
    // Convert to lowercase
    for (char* p = buffer; *p; p++) *p = tolower(*p);
    
    char* cmd = strtok(buffer, ",");
    char* data1 = strtok(NULL, ",");
    char* data2 = strtok(NULL, ",");
    
    executeCommand(cmd, data1, data2);
}

void AtlasEC::explainCommands() {
    Serial.println(F("**commands are not case sensitive**"));
    Serial.println();
    Serial.println(F("i = device type and version number"));
    Serial.println();
    Serial.println(F("adr,? = what is the I2C ID number"));
    Serial.println(F("adr,unlock = unlock the I2C address change register"));
    Serial.println(F("adr,lock = Lock the address change register"));
    Serial.println(F("adr,new,[new i2c address number]"));
    Serial.println();
    Serial.println(F("int,? = read the state of the interrupt control register"));
    Serial.println(F("int,[high],[low],[inv],[off] = set the interrupt control register"));
    Serial.println();
    Serial.println(F("led,? = read the state of the LED control register"));
    Serial.println(F("led,[on],[off] = set the LED control register"));
    Serial.println();
    Serial.println(F("on = start taking readings"));
    Serial.println(F("off = stop taking readings; hibernate"));
    Serial.println();
    Serial.println(F("nra,? = read the state of the new reading available register"));
    Serial.println(F("nra,clr = clear the new reading available register"));
    Serial.println();
    Serial.println(F("k,? = read the K value of the conductivity probe"));
    Serial.println(F("k,xx.x = set a new K value for the conductivity probe"));
    Serial.println();
    Serial.println(F("cal,? = read the state of the calibration register"));
    Serial.println(F("cal,clr = clear the calibration"));
    Serial.println(F("cal,xxxx = single point calibration"));
    Serial.println(F("cal,dry = dry calibration"));
    Serial.println(F("cal,low,xxxx = low point calibration"));
    Serial.println(F("cal,high,xxxx = high point calibration"));
    Serial.println();
    Serial.println(F("t,? = read the temperature compensation value"));
    Serial.println(F("t,xx.x = set the temperature compensation in C"));
    Serial.println();
    Serial.println(F("r = take a single conductivity reading"));
    Serial.println(F("r,1 = take continuous readings"));
    Serial.println(F("r,0 = END continuous readings"));
}

void AtlasEC::handleAddressCommand(const char* data1, const char* data2) {
    const uint8_t ADDRESS_UNLOCK_A = 0x55;
    const uint8_t ADDRESS_UNLOCK_B = 0xAA;
    const uint8_t ADDRESS_LOCK = 0x00;

    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(NEW_ADDRESS_REGISTER, 1);
        Serial.print("SMBus/I2C bus_address:");
        Serial.println(_sensor_data.i2c_data[0]);
    } else if (data1 && strcmp(data1, "unlock") == 0) {
        i2cWriteByte(ADDRESS_LOCK_REGISTER, ADDRESS_UNLOCK_A);
        i2cWriteByte(ADDRESS_LOCK_REGISTER, ADDRESS_UNLOCK_B);
        i2cRead(ADDRESS_LOCK_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 0) Serial.println("unlocked");
    } else if (data1 && strcmp(data1, "lock") == 0) {
        i2cWriteByte(ADDRESS_LOCK_REGISTER, ADDRESS_LOCK);
        i2cRead(ADDRESS_LOCK_REGISTER, 1);
        if (_sensor_data.i2c_data[0]) Serial.println("locked");
    } else if (data1 && strcmp(data1, "new") == 0 && data2) {
        uint8_t new_address = atoi(data2);
        if (new_address < 1 || new_address > 127) {
            Serial.println("out of range");
        } else {
            i2cRead(ADDRESS_LOCK_REGISTER, 1);
            if (_sensor_data.i2c_data[0]) {
                Serial.println("unlock register first");
            } else {
                i2cWriteByte(NEW_ADDRESS_REGISTER, new_address);
                _i2c_address = new_address;
                Serial.print("address changed to:");
                Serial.println(_i2c_address);
            }
        }
    }
}

void AtlasEC::handleInterruptCommand(const char* data1) {
    const uint8_t INTERRUPT_OFF = 0x00;
    const uint8_t INTERRUPT_HIGH = 0x02;
    const uint8_t INTERRUPT_LOW = 0x04;
    const uint8_t INTERRUPT_INVERT = 0x08;

    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(INTERRUPT_CONTROL_REGISTER, 1);
        Serial.print("Interrupt control register:");
        Serial.println(_sensor_data.i2c_data[0]);
    } else if (data1 && strcmp(data1, "high") == 0) {
        i2cWriteByte(INTERRUPT_CONTROL_REGISTER, INTERRUPT_HIGH);
        i2cRead(INTERRUPT_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 2) Serial.println("*ok, pin high on new reading");
    } else if (data1 && strcmp(data1, "low") == 0) {
        i2cWriteByte(INTERRUPT_CONTROL_REGISTER, INTERRUPT_LOW);
        i2cRead(INTERRUPT_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 4) Serial.println("*ok, pin low on new reading");
    } else if (data1 && strcmp(data1, "inv") == 0) {
        i2cWriteByte(INTERRUPT_CONTROL_REGISTER, INTERRUPT_INVERT);
        i2cRead(INTERRUPT_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 8) Serial.println("*ok, pin invert on new reading");
    } else if (data1 && strcmp(data1, "off") == 0) {
        i2cWriteByte(INTERRUPT_CONTROL_REGISTER, INTERRUPT_OFF);
        i2cRead(INTERRUPT_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 0) Serial.println("*ok, interrupt off");
    }
}

void AtlasEC::handleLEDCommand(const char* data1) {
    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(LED_CONTROL_REGISTER, 1);
        Serial.print("LED= ");
        if (_sensor_data.i2c_data[0]) Serial.println("on");
        else Serial.println("off");
    } else if (data1 && strcmp(data1, "on") == 0) {
        setLED(true);
        i2cRead(LED_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 1) Serial.println("*LED ON");
    } else if (data1 && strcmp(data1, "off") == 0) {
        setLED(false);
        i2cRead(LED_CONTROL_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 0) Serial.println("*LED off");
    }
}

void AtlasEC::handleCalibrationCommand(const char* data1, const char* data2) {
    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        Serial.println("calibration status:");
        if (_sensor_data.i2c_data[0] == 0) Serial.println("no calibration");
        if (bitRead(_sensor_data.i2c_data[0], 0) == 1) Serial.println("dry calibration done");
        if (bitRead(_sensor_data.i2c_data[0], 1) == 1) Serial.println("single point calibration done");
        if (bitRead(_sensor_data.i2c_data[0], 2) == 1) Serial.println("low-point calibration done");
        if (bitRead(_sensor_data.i2c_data[0], 3) == 1) Serial.println("high-point calibration done");
    } else if (data1 && strcmp(data1, "clr") == 0) {
        clearCalibration();
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 0) Serial.println("calibration cleared");
    } else if (data1 && strcmp(data1, "dry") == 0) {
        dryCalibration();
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        if (bitRead(_sensor_data.i2c_data[0], 0) == 1) Serial.println("dry calibration done");
    } else if (data1 && strcmp(data1, "low") == 0 && data2) {
        lowPointCalibration(atof(data2));
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        if (bitRead(_sensor_data.i2c_data[0], 2) == 1) Serial.println("low-point calibration done");
    } else if (data1 && strcmp(data1, "high") == 0 && data2) {
        highPointCalibration(atof(data2));
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        if (bitRead(_sensor_data.i2c_data[0], 3) == 1) Serial.println("high-point calibration done");
    } else if (data1 && isdigit(data1[0])) {
        singlePointCalibration(atof(data1));
        i2cRead(CALIBRATION_CONFIRM_REGISTER, 1);
        if (bitRead(_sensor_data.i2c_data[0], 1) == 1) Serial.println("single point calibration done");
    }
}

void AtlasEC::handleTemperatureCommand(const char* data1) {
    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(ACTIVE_HIBERNATE_REGISTER, 1);
        if (_sensor_data.i2c_data[0] == 0) {
            Serial.println("device must be taking readings for compensation registers to update");
        }
        
        i2cRead(TEMP_CONFIRM_REGISTER, 4);
        float compensation = _sensor_data.long_value / 100.0;
        Serial.print("T= ");
        Serial.println(compensation);
    } else if (data1 && isdigit(data1[0])) {
        setTemperatureCompensation(atof(data1));
        Serial.println("done");
    }
}

void AtlasEC::handleProbeCommand(const char* data1) {
    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(PROBE_TYPE_REGISTER, 2);
        float k_value = _sensor_data.int_value / 100.0;
        Serial.print("K= ");
        Serial.println(k_value);
    } else if (data1 && isdigit(data1[0])) {
        setProbeKValue(atof(data1));
        Serial.println("done");
    }
}

// Handle new reading available commands
void AtlasEC::handleNRACommand(const char* data1) {
    const uint8_t REG_CLEAR = 0x00;

    if (data1 && strcmp(data1, "?") == 0) {
        i2cRead(NEW_READING_REGISTER, 1);
        Serial.print("new reading available: ");
        if (_sensor_data.i2c_data[0]) Serial.println("yes");
        else Serial.println("no");
    } else if (data1 && strcmp(data1, "clr") == 0) {
        i2cWriteByte(NEW_READING_REGISTER, REG_CLEAR);
    }
}

// Update the executeCommand method to handle all commands
void AtlasEC::executeCommand(const char* cmd, const char* data1, const char* data2) {
    if (strcmp(cmd, "?") == 0) {
        explainCommands();
    } else if (strcmp(cmd, "i") == 0) {
        getDeviceInfo();
    } else if (strcmp(cmd, "debug") == 0) {
        if (data1 && strcmp(data1, "0") == 0) setDebugMode(false);
        else if (data1 && strcmp(data1, "1") == 0) setDebugMode(true);
        else if (data1 && strcmp(data1, "ec1") == 0) setECDebugMode(true);
        else if (data1 && strcmp(data1, "ec0") == 0) setECDebugMode(false);
    } else if (strcmp(cmd, "adr") == 0) {
        handleAddressCommand(data1, data2);
    } else if (strcmp(cmd, "int") == 0) {
        handleInterruptCommand(data1);
    } else if (strcmp(cmd, "led") == 0) {
        handleLEDCommand(data1);
    } else if (strcmp(cmd, "on") == 0) {
        setActiveMode(true);
        Serial.println("active");
    } else if (strcmp(cmd, "off") == 0) {
        setActiveMode(false);
        Serial.println("hibernate");
    } else if (strcmp(cmd, "nra") == 0) {
        handleNRACommand(data1);
    } else if (strcmp(cmd, "k") == 0) {
        handleProbeCommand(data1);
    } else if (strcmp(cmd, "cal") == 0) {
        handleCalibrationCommand(data1, data2);
    } else if (strcmp(cmd, "t") == 0) {
        handleTemperatureCommand(data1);
    } else if (strcmp(cmd, "r") == 0) {
        if (data1 && strcmp(data1, "1") == 0) {
            Serial.println("continuous mode on");
            // Continuous mode handling would be implemented here
        } else if (data1 && strcmp(data1, "0") == 0) {
            Serial.println("continuous mode off");
            // Continuous mode handling would be implemented here
        } else {
            readEC();
        }
    }
}