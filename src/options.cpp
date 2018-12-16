
#include <openzwave/platform/Log.h>
#include <algorithm>
#include <fstream>
#include "options.h"

using namespace std;


void print_help()
{
    printf("\n");
    printf("OpenZwave to MQTT bridge.\n");
    printf("\n");
    printf("Usage: ozw-mqtt [-d device] [-h MQTT host] [...]\n");
    printf("\n");
    printf("Common options:\n");
    printf("\t -d [--device]\t\t ZWave Device (default /dev/zwave)\n");
    printf("\t -c [--config]\t\t User configuration for OpenZWave library (default ./)\n");
    printf("\t -h [--mqtt-host]\t MQTT Broker host to connect to (default localhost)\n");
    printf("\t -u [--mqtt-user]\t MQTT Username (default empty)\n");
    printf("\t -p [--mqtt-passwd]\t MQTT Password (default empty)\n");
    printf("\n");
    printf("Other options:\n");
    printf("\t --mqtt-port\t\t MQTT Broker port (default 1883)\n");
    printf("\t --mqtt-client-id\t MQTT Client-ID (default ozw-mqtt)\n");
    printf("\t --mqtt-prefix\t\t Add prefix to all ZWave subscriptions / publications (default empty)\n");
    printf("\t --mqtt-no-name-topics\t Do not subscribe/publish to name-based topics\n");
    printf("\t --mqtt-no-id-topics\t Do not subscribe/publish to id-based topics\n");
    printf("\t --system-config\t OpenZWave library system config dir (default /usr/local/etc/openzwave)\n");
    printf("\t --topic-filter-file\t Publish only to topics from file separated by new line\n");
    printf("\t --log-level\t\t Set log level (error, warning, info, debug) (default info)\n");
    printf("\t --help\t\t\t Print this message\n");
    printf("\n");
}

options::options():
        system_config("/usr/local/etc/openzwave"),
        user_config("./"),
        device("/dev/zwave"),
        mqtt_host("127.0.0.1"),
        mqtt_client_id("ozw-mqtt"),
        mqtt_port(1883),
        mqtt_name_topics(true),
        mqtt_id_topics(true),
        log_level(OpenZWave::LogLevel_Info)
{
}

void
options::parse_argv(int argc, const char* argv[])
{
    for (int i = 1; i < argc; i++) {
        string k(argv[i]);
        // for convenience - replace '_' with '-''
        std::replace(k.begin(), k.end(), '_', '-');

        // Parameters without arguments
        if (k == "--help") {
            print_help();
            exit(1);
        } else if (k == "--mqtt-no-name-topics") {
            mqtt_name_topics = false;
            continue;
        } else if (k == "--mqtt-no-id-topics") {
            mqtt_id_topics = false;
            continue;
        }

        // Next parameters requires value
        if (i + 1 >= argc) {
            // no value provided
            throw param_error("Value required", k);
        }
        string v = argv[++i];
        if (v.size() > 2 && v.substr(0, 2) == "--") {
            throw param_error("Value required", k);
        }

        if (k == "--system-config") {
            system_config = v;
        } else if (k == "--config" || k == "-c") {
            user_config = v;
        } else if (k == "--topic-filter-file") {
            topics_file = v;
        } else if (k == "--device" || k == "-d") {
            device = v;
        } else if (k == "--mqtt-host" || k =="-h") {
            mqtt_host = v;
        } else if (k == "--mqtt-port") {
            mqtt_port = stoi(v);
        } else if (k == "--mqtt-client-id") {
            mqtt_client_id = v;
        } else if (k == "--mqtt-prefix") {
            mqtt_prefix = v;
        } else if (k == "--mqtt-user" || k == "-u") {
            mqtt_user = v;
        }  else if (k == "--mqtt-passwd" || k == "-p") {
            mqtt_passwd = v;
        } else if (k == "--log-level") {
            // error, warning, info, debug
            if (v == "error") log_level = OpenZWave::LogLevel_Error;
            else if (v == "warning") log_level = OpenZWave::LogLevel_Warning;
            else if (v == "info") log_level = OpenZWave::LogLevel_Info;
            else if (v == "debug") log_level = OpenZWave::LogLevel_Debug;
            else {
                throw param_error("Unknown log level", v);
            }
        } else {
            throw param_error("Unknown option", k);
        }
    }
}

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t")
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void
options::parse_topics_file()
{
    ifstream infile(topics_file);

    for(std::string line; getline(infile, line); ) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        // And comments - stats with '#'
        if (trim(line)[0] == '#') {
            continue;
        }
        size_t pos = line.find("=");
        if (pos != string::npos) {
            string t1 = trim(line.substr(0, pos));
            string t2 = trim(line.substr(pos + 1));
            topic_overrides.insert(make_pair(t1, t2));
        } else {
            // No user friendly topic specified, use the same
            topic_overrides.insert(make_pair(line, line));
        }
    }
}

