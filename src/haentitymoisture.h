#pragma once

#include <Arduino.h>
#include <HaBridge.h>
#include <HaUtilities.h>

#include <string>

class HaEntityMoisture {
   public:
#define COMPONENT "number"

    struct Configuration {
        /**
         * @brief The minimum value allowed.
         */
        float min_value = 1.0;

        /**
         * @brief The maximum value allowed.
         */
        float max_value = 100.0;

        /**
         * In Home Assistant, trigger events even if the sensor's state hasn't changed. Useful if you want
         * to have meaningful value graphs in history or want to create an automation that triggers on every incoming state
         * message (not only when the sensor’s new state is different to the current one).
         */
        bool force_update = false;

        /**
         * @brief If true, this tells Home Assistant to publish the message on the command topic with retain set to true.
         */
        bool retain = false;
    };

    inline static Configuration _default = {.min_value = 1.0, .max_value = 100.0, .force_update = false, .retain = false};

   private:
    std::string _name;
    HaBridge& _ha_bridge;
    std::string _object_id;
    Configuration _configuration;

    std::optional<float> _number;

   public:
    HaEntityMoisture(HaBridge& ha_bridge, std::string name, std::string object_id, Configuration configuration = _default);

    virtual void publishConfiguration();

    virtual void republishState();

    virtual void publishNumber(float number);

    virtual bool setOnNumber(std::function<void(float)> callback);
};