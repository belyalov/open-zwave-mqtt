
#ifndef MQTT_H
#define MQTT_H

#include "node_value.h"
#include "options.h"

// Callback function for custom topics
typedef void (*mqtt_custom_topic_callback)(const std::string& value);

void mqtt_connect(const std::string&, const std::string&, const uint16_t port, const std::string& user, const std::string& passwd);
void mqtt_loop();

// Subscription to OZW value
void mqtt_subscribe(const options*, const OpenZWave::ValueID&);
// Custom subscription: topic -> function
void mqtt_subscribe(const std::string&, const std::string&, mqtt_custom_topic_callback);
void mqtt_publish(const options*, const OpenZWave::ValueID&);
void mqtt_unsubscribe_all();

// Mostly for tests
const std::map<std::string, const OpenZWave::ValueID>&
     mqtt_get_endpoints();

const std::map<std::string, mqtt_custom_topic_callback>
     mqtt_get_endpoints_custom();

#endif
