#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HaBridge.h>
#include <IJson.h>
#include <MQTTRemote.h>
#include <WString.h>
#include <entities/HaEntityNumber.h>

#include "haentitymoisture.h"

#ifdef ESP8266
const String mqtt_client_id = String(ESP.getChipId());
#else
const std::string mqtt_client_id = ESP.getSketchMD5();
#endif
const char mqtt_host[] = "192.168.2.222";
const short mqtt_port = 1883;
const char mqtt_username[] = "local";
const char mqtt_password[] = "mqtt";

class MQTTHomeAssistantEntities {
   public:
    static MQTTRemote mqtt_remote;

    // Create the Home Assistant bridge. This is shared across all entities.
    // We only have one per device/hardware. In our example, the name of our device is "livingroom".
    // See constructor of HaBridge for more documentation.
    static HaBridge ha_bridge;

    // Moisture sensors
    static HaEntityMoisture ha_entity_sensor0;
    static HaEntityMoisture ha_entity_sensor1;
    static HaEntityMoisture ha_entity_sensor2;
    static HaEntityMoisture ha_entity_sensor3;

    // Voltage sensor
    static HaEntityNumber ha_entity_sensor7;

    // Check interval
    static HaEntityNumber ha_entity_check_interval;

    // Information about this device.
    // All these keys will be added to a "device" key in the Home Assistant configuration for each entity.
    // Only a flat layout structure is supported, no nesting.
    static IJsonDocument device_doc;

    static std::vector<std::reference_wrapper<HaEntity>> entities;

   public:
    MQTTHomeAssistantEntities();

    static void init();
    static void loop();

    static void onConnected(bool status);
    static void onNumber(float num);
    static void onIntervalReceived(std::string topic, std::string value);
};
