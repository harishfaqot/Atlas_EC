#include "Atlas_EC.h"

AtlasEC ecSensor;

void setup() {
    Serial.begin(9600);
    ecSensor.begin();
    ecSensor.setDebugMode(true);
    ecSensor.setECDebugMode(false);
}

void loop() {
    ecSensor.processCommands(); // Handle serial commands
    
    float ec = ecSensor.readEC();
    Serial.print("Conductivity: ");
    Serial.print(ec);
    Serial.println(" uS/cm");
    delay(1000);
}