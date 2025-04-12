#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HC4051.h>
#include <PubSubClient.h>
#include <TM1637Display.h>

#include "ota.h"
#include "sensor.h"

#define LED_PIN LED_BUILTIN
#define SENSOR_PIN A0
#define BROKER_ADDR IPAddress(192, 168, 0, 17)
#define WAKEUP_INTERVAL (30 * 1000000)            // Wake up interval in microseconds
#define UPDATE_INTERVAL (WAKEUP_INTERVAL / 1000)  // Update interval in miliseconds
#define SLEEP_ENABLED 1
#define OTA_ENABLED 0

// Analog demultiplexer object
HC4051 mp(D5, D6, D7);

const char wifi_ssid[] = "LMParizka_8a";
const char wifi_password[] = "31415926535";

#ifdef ESP8266
const String mqtt_client_id = String(ESP.getChipId());
#else
const std::string mqtt_client_id = ESP.getSketchMD5();
#endif
const char mqtt_host[] = "192.168.2.222";
const short mqtt_port = 1883;
const char mqtt_username[] = "local";
const char mqtt_password[] = "mqtt";

WiFiClient espClient;
PubSubClient client(espClient);

// https://gitlab.com/diy_bloke/verydeepsleep_mqtt.ino/blob/master/VeryDeepSleep_MQTT.ino

void connectWiFi() {
    Serial.println();
    delay(500);
    Serial.print("Connecting to wifi...");
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        // delay(2000);
        // ESP.restart();
    }
    Serial.println("");
    Serial.print("Connected to wifi [");
    Serial.print(WiFi.localIP());
    Serial.println("]");
}

void reconnectMQTT() {
    while (!client.connected()) {
        if (client.connect(mqtt_client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Connected...");
            // client.publish("home/hok/stat/connection", "OK");
        } else {
            Serial.print("failed, rc= ");
            Serial.print(client.state());
            delay(1000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);


    // Measure the cell voltage (with wifi disabled)
    mp.setChannel(7);
    delay(50);
    int sensorValue = analogRead(SENSOR_PIN);
    //  The voltage is divided by two
    const int v_measured = sensorValue;
    const int v_correction = 0;
    const int v_calculated = max((map(sensorValue, 0, 1023, 0, 2880) * 2) + v_correction, (long)0);

    // Setup wifi
    connectWiFi();

    // Your MQTT server's IP. Mind you, these are separated by dots again
    client.setServer(mqtt_host, mqtt_port);

    Serial.println("Start sending data");
    digitalWrite(LED_PIN, HIGH);

    if (!client.connected()) {
        reconnectMQTT();
    }

    for (uint8_t channel = 0; channel < 8; channel++) {
        mp.setChannel(channel);
        delay(50);

        sensorValue = Sensor::readSensor();
        Serial.printf("Sensor %d output: %d\n", channel, sensorValue);

        switch (channel) {
            case 0:
                client.publish("moisturereader/number/s0/state", String(sensorValue).c_str(), false);
                break;
            case 1:
                client.publish("moisturereader/number/s1/state", String(sensorValue).c_str(), false);
                break;
            case 2:
                client.publish("moisturereader/number/s2/state", String(sensorValue).c_str(), false);
                break;
            case 3:
                client.publish("moisturereader/number/s3/state", String(sensorValue).c_str(), false);
                break;
            case 7:
                Serial.printf("Sensor voltage: %d (measured: %d)\n", v_calculated, v_measured);
                client.publish("moisturereader/number/v0/state", String((float) v_calculated / 1000.0, 3).c_str(), false);
                break;
            default:
                break;
        }
    }
    digitalWrite(LED_PIN, LOW);

    // Close MQTT client cleanly
    client.disconnect();

    Serial.println("Go back to sleep");
    WiFi.disconnect(true);
    delay(1);

#if SLEEP_ENABLED == 1
    // Configure deep sleep
    ESP.deepSleep(WAKEUP_INTERVAL, WAKE_RF_DISABLED);
#endif
}

void loop() {
}