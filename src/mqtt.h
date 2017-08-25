
#ifndef MQTT_H
#define MQTT_H

#include "node_value.h"


void mqtt_connect(const std::string&, const std::string&, const uint16_t port);
void mqtt_loop();

void mqtt_subscribe(const std::string&, const OpenZWave::ValueID&);
void mqtt_publish(const std::string&, const OpenZWave::ValueID&);
void mqtt_unsubscribe_all();

// Mostly for tests
const std::map<std::string, const OpenZWave::ValueID>&
     mqtt_get_endpoints();

#endif
