#include <Arduino.h>
#include <ArduinoJson.h>
#include <HC4051.h>
#include <HaBridge.h>
#include <IJson.h>
#include <MQTTRemote.h>
#include <TelnetStream.h>
#include <entities/HaEntityNumber.h>

#include "Every.h"
#include "haentitymoisture.h"
#include "lcd.h"
#include "mqtt.h"
#include "ntptime.h"
#include "ota.h"
#include "sensor.h"
#include "wifi.h"

#define LED_PIN LED_BUILTIN
#define SENSOR_PIN A0

#define SENSOR_HIGH_VALUE 560                     // The value of the sensor if dry (or sensor is missing)
#define WAKEUP_INTERVAL (10 * 1000000)            // Wake up interval in microseconds
#define UPDATE_INTERVAL (WAKEUP_INTERVAL / 1000)  // Update interval in miliseconds
#define SLEEP_ENABLED 0

// Analog demultiplexer object
HC4051 mp(D5, D6, D4);

// Timer for measuring interval
Every::Toggle tmeasure(10000);

// Time update
Every tset(500);

// Wifi client credentials
const char wifi_ssid[] = "LMParizka_8a";
const char wifi_password[] = "31415926535";

// Voltage calculated
int v_calculated = 0;

// Sensor measured values
int sensorValue[4] = {};

unsigned long last_publish_ms = 0;

bool continousMode = false;
uint8_t singleChannel = UINT8_MAX;

void remoteCheck() {
    MQTTHomeAssistantEntities::loop();
}

int mapSensorValue(const int sensorValue) {
    const int outputValue = map(sensorValue, 0, SENSOR_HIGH_VALUE, 100, 0);

    // Return analog moisture value
    return outputValue;
}

void log(String text) {
    static int i = 0;

    TelnetStream.print(i++);
    TelnetStream.print(": ");
    TelnetStream.println(text);
}

void switchChannel(const uint8_t channel) {
    mp.setChannel(channel);
    delay(100);
}

void measureChannel(const uint8_t channel) {
    const int analogValue = Sensor::readSensor();

    sensorValue[channel] = max(mapSensorValue(analogValue), 0);

    char sensorStr[100] = {};
    sprintf(sensorStr, "Sensor %d output: %d [adc: %d]", channel, sensorValue[channel], analogValue);

    Serial.printf("Sensor %d output: %d [adc: %d]\n", channel, sensorValue[channel], analogValue);
    log(sensorStr);
}

void measure() {
    // MTTQ handle incoming messages
    remoteCheck();

    Serial.println("Start sending data");
    digitalWrite(LED_PIN, LOW);

    // Publish the wakeup interval
    MQTTHomeAssistantEntities::ha_entity_check_interval.publishNumber(WAKEUP_INTERVAL / 1000 / 1000);

    for (uint8_t channel = 0; channel < sizeof(sensorValue) / sizeof(sensorValue[0]); channel++) {
        switchChannel(channel);
        measureChannel(channel);

        switch (channel) {
            case 0:
                MQTTHomeAssistantEntities::ha_entity_sensor0.publishNumber(sensorValue[0]);
                break;
            case 1:
                MQTTHomeAssistantEntities::ha_entity_sensor1.publishNumber(sensorValue[1]);
                break;
            case 2:
                MQTTHomeAssistantEntities::ha_entity_sensor2.publishNumber(sensorValue[2]);
                break;
            case 3:
                MQTTHomeAssistantEntities::ha_entity_sensor3.publishNumber(sensorValue[3]);
                break;
            case 7:
                MQTTHomeAssistantEntities::ha_entity_sensor7.publishNumber((float)v_calculated / 1000.0);
                break;
            default:
                break;
        }
    }
    mp.setChannel(0);
    log("");

    // Display the measured values
#if SLEEP_ENABLED == 1
    LCD::init();
#endif
    LCD::displayValues(sensorValue[0], sensorValue[1], sensorValue[2], sensorValue[3]);

    digitalWrite(LED_PIN, HIGH);
    Serial.println("Stop sending data");
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    // pinMode(D0, OUTPUT);
    // pinMode(SENSOR_PIN, INPUT);

    digitalWrite(LED_PIN, LOW);

    // Initialize the lcd display
    LCD::init();

    // Initialize the time class
    Time::init();

    // Initialize the MQTT connection
    MQTTHomeAssistantEntities::init();

#if SLEEP_ENABLED == 1
    // Measure the cell voltage (with wifi disabled)
    mp.setChannel(7);
    delay(50);
    const int m = analogRead(SENSOR_PIN);
    //  The voltage is divided by two
    const int v_measured = m;
    const int v_correction = 0;
    v_calculated = max((map(m, 0, 1023, 0, 2870) * 2) + v_correction, (long)0);
    Serial.printf("Sensor voltage: %d (measured: %d)\n", v_calculated, v_measured);
#endif

    // Setup wifi
    WifiManager::connect(wifi_ssid, wifi_password);

    // OTA initialization
    OTA::setup(wifi_ssid, wifi_password);

    // Telnet trace init
    TelnetStream.begin();

#if SLEEP_ENABLED == 1
    measure();

    // Configure deep sleep
    Serial.println("Going to deep sleep");
    ESP.deepSleep(WAKEUP_INTERVAL, WAKE_RF_DISABLED);
#else
    tmeasure.reset();
    tset.reset();
#endif
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
#if SLEEP_ENABLED != 1
    if (continousMode) {
        if (singleChannel < 8) {
            switchChannel(singleChannel);
            measureChannel(singleChannel);
            delay(1000);
        } else {
            measure();
            delay(1000);
        }
    } else {
        if (tmeasure()) {
            if (tmeasure.state) {
                measure();
            } else {
                // Change the time
                Time::loop();

                LCD::print("    " + Time::getTime(), 0, true);
            }
        }
        if (!tmeasure.state) {
            if (tset()) {
                // Change the time
                Time::loop();

                LCD::print("    " + Time::getTime(), 0, true);
            }
        }
    }
    // Check for OTA update
    OTA::loop();

    switch (TelnetStream.read()) {
        case 'r':
        case 'R':
            TelnetStream.stop();
            delay(100);
            ESP.reset();
            break;
        case 'q':
        case 'Q':
            if (continousMode) {
                if (singleChannel < 8) {
                    TelnetStream.println("Single channel off");
                    singleChannel = UINT8_MAX;
                } else {
                    TelnetStream.println("Continous mode off");
                    continousMode = false;
                }
            } else {
                TelnetStream.println("bye bye");
                TelnetStream.flush();
                TelnetStream.stop();
            }
            break;
        case 'c':
        case 'C':
            if (continousMode) {
                TelnetStream.println("Continous mode off");
                continousMode = false;
            } else {
                TelnetStream.println("Continous mode on");
                continousMode = true;
            }
            break;
        case '0':
            if (continousMode) {
                TelnetStream.println("Single channel 1 on");
                singleChannel = 0;
            }
            break;
        case '1':
            if (continousMode) {
                TelnetStream.println("Single channel 2 on");
                singleChannel = 1;
            }
            break;
        case '2':
            if (continousMode) {
                TelnetStream.println("Single channel 3 on");
                singleChannel = 2;
            }
            break;
        case '3':
            if (continousMode) {
                TelnetStream.println("Single channel 4 on");
                singleChannel = 3;
            }
            break;
        default:
            break;
    }
#endif
}