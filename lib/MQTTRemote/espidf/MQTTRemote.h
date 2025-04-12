#ifndef __MQTT_REMOTE_H__
#define __MQTT_REMOTE_H__

#include "IMQTTRemote.h"

#include <functional>
#include <map>
#include <mqtt_client.h>
#include <string>

namespace MQTTRemoteLog {
const char TAG[] = "MQTTRemote";
} // namespace MQTTRemoteLog

namespace MQTTRemoteDefaults {
const uint32_t DEFAULT_STACK_SIZE = 8192;
const uint32_t DEFAULT_TASK_PRIORITY = 7;
} // namespace MQTTRemoteDefaults

/**
 * @brief MQTT wrapper for setting up MQTT connection (and will) and provide API for sending and subscribing to
 * messages.
 */
class MQTTRemote : public IMQTTRemote {
public:
  /**
   * @brief Construct a new Remote object
   *
   * To set log level for this object, use: esp_log_level_set(MQTTRemoteLog::TAG, ESP_LOG_*);
   *
   * A call to start() most follow.
   *
   * @param client_id Base ID for this device. This is used for the last will / status
   * topic.Example, if this is "esp_now_router", then the status/last will topic will be "esp_now_router/status". This
   * is also used as client ID for the MQTT connection. This has to be [a-zA-Z0-9_] only and unique among all MQTT
   * clients on the server. It should also be stable across connections.
   * @param host MQTT hostname or IP for MQTT server.
   * @param port MQTT port number.
   * @param username MQTT username.
   * @param password MQTT password.
   * @param max_message_size the max message size one can send. The larger to more memory/RAM is needed. Default: 2048
   * bytes
   * @param keep_alive keep alive interval in seconds. Default: 10 seconds
   */
  MQTTRemote(std::string client_id, std::string host, int port, std::string username, std::string password,
             uint16_t max_message_size = 2048, uint32_t keep_alive = 10);

  /**
   * @brief Call once there is a WIFI connection on which the host can be reached.
   * Will connect to the server and setup any subscriptions as well as start the MQTT loop.
   * @param on_connection_change optional callback on connect state change. Will be called when the client is connected
   * to server (every time, so expect calls on reconnection), and on disconnect. The parameter will be true on new
   * connection and false on disconnection. This callback will run from a dedicated task. Task size and priority can be
   * set.
   * @param task_size the stack size for the task that will call the on_connection_change callback, if set. Default:
   * 4096
   * @param task_priority the priority for the task that will call the on_connection_change callback, if set. Default: 7
   *
   * NOTE: Can only be called once WIFI has been setup! ESP-IDF will assert otherwise.
   */
  void start(std::function<void(bool)> on_connection_change = {},
             unsigned long task_size = MQTTRemoteDefaults::DEFAULT_STACK_SIZE,
             uint8_t task_priority = MQTTRemoteDefaults::DEFAULT_TASK_PRIORITY);
  void setup() { start(); }

  /**
   * @brief Publish a message.
   *
   * @param topic the topic to publish to.
   * @param message The message to send. This cannot be larger than the value set for max_message_size in the
   * constructor.
   * @param retain True to set this message as retained.
   * @param qos quality of service for published message (0 (default), 1 or 2)
   * @returns true on success, or false on failure.
   */
  bool publishMessage(std::string topic, std::string message, bool retain = false, uint8_t qos = 0) override;

  /**
   * Same as publishMessage(), but will print the message and topic and the result on serial.
   */
  bool publishMessageVerbose(std::string topic, std::string message, bool retain = false, uint8_t qos = 0) override;

  /**
   * @brief returns if there is a connection to the MQTT server.
   */
  bool connected() override { return _connected; }

  /**
   * @brief Subscribe to a topic. The callback will be invoked on every new message.
   * There can only be one callback per topic. If trying to subscribe to an already subscribe topic, it will be ignored.
   * Don't do heavy operations in the callback or delays as this will block the MQTT callback.
   *
   * Can be called before being connected. All subscriptions will be (re-)subscribed to once a connection is
   * (re-)established.
   *
   * @param message_callback a message callback with the topic and the message. The topic is repeated for convinience,
   * but it will always be for the subscribed topic.
   * @return true if an subcription was successul. Will return false if there is no active MQTT connection. In this
   * case, the subscription will be performed once connected. Will retun false if this subscription is already
   * subscribed to.
   */
  bool subscribe(std::string topic, IMQTTRemote::SubscriptionCallback message_callback) override;

  /**
   * @brief Unsubscribe a topic.
   */
  bool unsubscribe(std::string topic) override;

  /**
   * @brief The client ID for this device. This is used for the last will / status
   * topic.Example, if this is "esp_now_router", then the status/last will topic will be "esp_now_router/status". This
   * has to be [a-zA-Z0-9_] only.
   */
  std::string &clientId() override { return _client_id; }

private:
  /*
   * @brief Event handler registered to receive MQTT events
   *
   *  This function is called by the MQTT client event loop.
   *
   * @param handler_args user data registered to the event.
   * @param base Event base for the handler(always MQTT Base in this example).
   * @param event_id The id for the received event.
   * @param event_data The data for the event, esp_mqtt_event_handle_t.
   */
  static void onMqttEvent(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

  static void runTask(void *pvParams);

private:
  bool _started;
  bool _connected;
  bool _was_connected;
  std::string _client_id;
  std::string _last_will_topic;
  esp_mqtt_client_handle_t _mqtt_client;
  std::function<void(bool)> _on_connection_change;
  std::map<std::string, SubscriptionCallback> _subscriptions;
};

#endif // __MQTT_REMOTE_H__