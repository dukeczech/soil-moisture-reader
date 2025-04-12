#include "haentitymoisture.h"

HaEntityMoisture::HaEntityMoisture(HaBridge& ha_bridge, std::string name, std::string object_id, Configuration configuration)
    : _name(homeassistantentities::trim(name)), _ha_bridge(ha_bridge), _object_id(object_id), _configuration(configuration) {}

void HaEntityMoisture::publishConfiguration() {
    IJsonDocument doc;

    if (!_name.empty()) {
        doc["name"] = _name;
    } else {
        doc["name"] = nullptr;
    }

    // More info https://www.home-assistant.io/integrations/sensor/
    doc["platform"] = "sensor";
    doc["min"] = _configuration.min_value;
    doc["max"] = _configuration.max_value;
    doc["force_update"] = _configuration.force_update;
    doc["retain"] = _configuration.retain;
    doc["device_class"] = "moisture";
    doc["mode"] = "box";
    doc["unit_of_measurement"] = "%";
    doc["state_class"] = "measurement";
    doc["state_topic"] = _ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _object_id);
    doc["command_topic"] = _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _object_id);
    _ha_bridge.publishConfiguration(COMPONENT, _object_id, "", doc);
}

void HaEntityMoisture::republishState() {
    if (_number) {
        publishNumber(*_number);
    }
}

void HaEntityMoisture::publishNumber(float number) {
    // numbered == OFF
    _ha_bridge.publishMessage(_ha_bridge.getTopic(HaBridge::TopicType::State, COMPONENT, _object_id), std::to_string(number));
    _number = number;
}

bool HaEntityMoisture::setOnNumber(std::function<void(float)> callback) {
    return _ha_bridge.remote().subscribe(
        _ha_bridge.getTopic(HaBridge::TopicType::Command, COMPONENT, _object_id), [callback](std::string topic, std::string message) {
            callback(std::stof(message));
        });
}
