#include <Arduino.h>

class Sensor {
   private:
    static const int SENSOR_PIN;

   public:
    static int readSensor();
};