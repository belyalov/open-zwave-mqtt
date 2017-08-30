
#include <algorithm>
#include "options.h"

using namespace std;

void print_help()
{
    printf("\n");
    printf("OpenZwave to MQTT bridge.\n");
    printf("\n");
    printf("Usage: ozw-mqtt [-d device] [-h MQTT host] [-c ozw_config] [...]\n");
    printf("\n");
    printf("\t -d [--device]\t\t ZWave Device (default /dev/zwave)\n");
    printf("\t -c [--config]\t\t OpenZWave library config dir (default /usr/local/etc/openzwave)\n");
    printf("\t -h [--mqtt-host]\t MQTT Broker host to connect to (default localhost)\n");
    printf("\t -p [--mqtt-port]\t MQTT Broker port (default 1883)\n");
    printf("\t -C [--mqtt-client-id]\t MQTT Client-ID (default ozw-mqtt)\n");
    printf("\t -C [--mqtt-prefix]\t Add prefix to all ZWave subscriptions / publications (default none)\n");
    printf("\t -u [--mqtt-user]\t MQTT Username (no default value)\n");
    printf("\t -P [--mqtt-passwd]\t MQTT Password (no default value)\n");
    printf("\t -D [--debug]\t\t Enable debug logs (default off)\n");
    printf("\t -H [--help]\t\t Print this message\n");
    printf("\n");
}

options::options():
        openzwave_config("/usr/local/etc/openzwave"),
        device("/dev/zwave"),
        mqtt_host("127.0.0.1"),
        mqtt_client_id("ozw-mqtt"),
        mqtt_port(1883),
        debug(false)
{
}

bool
options::parse_argv(int argc, const char* argv[])
{
    for (int i = 1; i < argc; i++) {
        string k(argv[i]);
        // for convience replace _ with -
        std::replace(k.begin(), k.end(), '_', '-');

        // parameters without arguments
        if (k == "-H" || k == "--help") {
            print_help();
            return false;
        }
        if (k == "--debug" || k == "-D") {
            debug = true;
            continue;
        }

        // next parameters requires value
        if (i + 1 >= argc) {
            // no value provided
            printf("Value requred for '%s'\n", k.c_str());
            return false;
        }
        string v = argv[++i];
        if (v.size() > 2 && v.substr(0, 2) == "--") {
            printf("Value requred for '%s'\n", k.c_str());
            return false;
        }

        if (k == "--config" || k == "-c") {
            openzwave_config = v;
        } else if (k == "--device" || k == "-d") {
            device = v;
        } else if (k == "--mqtt-host" || k =="-h") {
            mqtt_host = v;
        } else if (k == "--mqtt-port" || k == "-p") {
            mqtt_port = stoi(v);
        } else if (k == "--mqtt-client-id" || k == "-C") {
            mqtt_client_id = v;
        } else if (k == "--mqtt-prefix") {
            mqtt_prefix = v;
        } else if (k == "--mqtt-user" || k == "-u") {
            mqtt_user = v;
        }  else if (k == "--mqtt-passwd" || k == "-P") {
            mqtt_passwd = v;
        } else {
            printf("Unknown option '%s'\n", k.c_str());
            return false;
        }
    }
    return true;
}
