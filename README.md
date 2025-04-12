# Soil moisture reader
The device that reads four mosture sensors put into the flower container

## Main components:
- WEMOS D1 mini Lite ESP8266
- 74HC4051 8-Channel analog multiplexer/demultiplexer module
- Sensor for measuring soil moisture HD-38 (4x)
- LCD display 2x16 characters blue + I2C LCM1602 converter
- 5V power supply moduke (LM7805)


## Features
- Connected to the Wifi
- Push the sensor values on MQTT broker (sensor values can be pulled to the homeassistant server)
- Debug via telnet connection
- Firmware upload via OTA enabled




