#include "mqtt.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HaBridge.h>
#include <IJson.h>
#include <MQTTRemote.h>
#include <entities/HaEntityNumber.h>

#include "haentitymoisture.h"

MQTTRemote MQTTHomeAssistantEntities::mqtt_remote(mqtt_client_id.c_str(), mqtt_host, mqtt_port, mqtt_username, mqtt_password, 2048, 0);

HaBridge MQTTHomeAssistantEntities::ha_bridge(mqtt_remote, "moisturereader", device_doc, false);

HaEntityMoisture MQTTHomeAssistantEntities::ha_entity_sensor0(ha_bridge, "Sensor 0", "s0");
HaEntityMoisture MQTTHomeAssistantEntities::ha_entity_sensor1(ha_bridge, "Sensor 1", "s1");
HaEntityMoisture MQTTHomeAssistantEntities::ha_entity_sensor2(ha_bridge, "Sensor 2", "s2");
HaEntityMoisture MQTTHomeAssistantEntities::ha_entity_sensor3(ha_bridge, "Sensor 3", "s3");

HaEntityNumber MQTTHomeAssistantEntities::ha_entity_sensor7(ha_bridge, "Sensor 7", "v0", {.min_value = 0.0, .max_value = 5.0, .force_update = false, .retain = false});

HaEntityNumber MQTTHomeAssistantEntities::ha_entity_check_interval(ha_bridge, "Check interval", "ch0", {.min_value = 1.0, .max_value = 86400.0, .force_update = false, .retain = false});

IJsonDocument MQTTHomeAssistantEntities::device_doc;

MQTTHomeAssistantEntities::MQTTHomeAssistantEntities() {
    init();
}

void MQTTHomeAssistantEntities::init() {
    // Populate json dictionary
    device_doc["identifiers"] = "hw_" + String(mqtt_client_id);
    device_doc["name"] = "Moisture reader";
    device_doc["sw_version"] = "1.0.0";
    device_doc["model"] = "d1mini";
    device_doc["manufacturer"] = "karel.hebik";

    mqtt_remote.setQoS(1);
    mqtt_remote.setOnConnectionChange(onConnected);
    ha_entity_check_interval.setOnNumber(onNumber);
    mqtt_remote.subscribe("moisturereader/number/ch0/command", onIntervalReceived);
}

void MQTTHomeAssistantEntities::loop() {
    // MTTQ handle incoming messages
    mqtt_remote.handle(false);
}

void MQTTHomeAssistantEntities::onConnected(bool status) {
    // Publish Home Assistant Configuration for the sensors once connected to MQTT.
    ha_entity_sensor0.publishConfiguration();
    ha_entity_sensor1.publishConfiguration();
    ha_entity_sensor2.publishConfiguration();
    ha_entity_sensor3.publishConfiguration();
    ha_entity_sensor7.publishConfiguration();
    ha_entity_check_interval.publishConfiguration();
}

void MQTTHomeAssistantEntities::onNumber(float num) {
    Serial.printf("Number %f received\n", num);
}

void MQTTHomeAssistantEntities::onIntervalReceived(std::string topic, std::string value) {
    Serial.printf("Topic: %s, value: %s\n", topic.c_str(), value.c_str());
}