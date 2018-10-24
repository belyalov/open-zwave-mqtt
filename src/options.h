#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <map>

class param_error: public std::exception {
    std::string msg;
public:
    const char * what () const throw () {
        return msg.c_str();
    }
    param_error(const std::string& _msg, const std::string& _param) {
        msg = _msg + " for '" + _param + "'";
    };
};

struct options {
    options();

    void parse_argv(int argc, const char* argv[]);
    void parse_topics_file();

    std::string system_config;
    std::string user_config;
    std::string topics_file;
    std::string device;
    std::string mqtt_host;
    std::string mqtt_client_id;
    std::string mqtt_user;
    std::string mqtt_passwd;
    std::string mqtt_prefix;
    // List of topics allowed to publish to MQTT
    // By default - empty which means publish all
    // Second element is user-friendly name, if set, like:
    // home/switch/switch_multilevel/0/1 -> home/living
    std::map<std::string, std::string> topic_overrides;
    // MQTT connection parameters
    uint16_t mqtt_port;
    bool mqtt_name_topics;
    bool mqtt_id_topics;
    // Log level
    uint32_t log_level;
};

void print_help();

#endif
