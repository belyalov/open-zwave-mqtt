
#ifndef MOCK_MOSQUITTO_H
#define MOCK_MOSQUITTO_H

#include <vector>
#include <string>
#include <mosquitto.h>

const std::vector<std::string> mock_mosquitto_subscribe_history();
const std::vector<std::pair<std::string, std::string > >
                               mock_mosquitto_publish_history();
void                           mock_mosquitto_cleanup();

#endif
