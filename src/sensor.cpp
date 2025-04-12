#include "sensor.h"

const int Sensor::SENSOR_PIN = A0;

//  This function returns the analog data to calling function
int Sensor::readSensor() {
    // Read the analog value from sensor
    const int sensorValue = analogRead(SENSOR_PIN);
    //const int outputValue = map(sensorValue, 0, 572, 100, 0);

    // Return analog moisture value
    return sensorValue;
}