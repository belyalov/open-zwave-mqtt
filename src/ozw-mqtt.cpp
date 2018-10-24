
#include <signal.h>
#include <stdio.h>
#include <openzwave/Options.h>
#include <openzwave/Manager.h>
#include <openzwave/Driver.h>
#include <openzwave/platform/Log.h>

#include "node_value.h"
#include "mqtt.h"
#include "process_notification.h"
#include "options.h"

using namespace OpenZWave;


void
signal_handler(int s)
{
    printf("Caught signal %d\n",s);
    exit(1);
}

void
save_config(const string& value)
{
    Manager::Get()->WriteConfig(home_id);
    printf("OZW configuration saved.\n");
}

//-----------------------------------------------------------------------------
// <main>
//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
    // Parse command line options
    try {
        options opt;
        opt.parse_argv(argc, argv);

        // Parse topics filter list, if desired
        if (!opt.topics_file.empty()) {
            opt.parse_topics_file();
            printf("Loaded %lu topics to publish to.\n", opt.topic_overrides.size());
        }

        // Setup signal handling
        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = signal_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);

        // Make a connection to MQTT broker
        printf("Connecting to MQTT Broker %s:%d...", opt.mqtt_host.c_str(), opt.mqtt_port);
        mqtt_connect(opt.mqtt_client_id, opt.mqtt_host, opt.mqtt_port, opt.mqtt_user, opt.mqtt_passwd);

        // Create the OpenZWave Manager.
        // The first argument is the path to the config files (where the manufacturer_specific.xml file is located
        // The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL
        // the log file will appear in the program's working directory.
        printf("Starting OpenZWave...\n");
        Options::Create(opt.system_config, opt.user_config, "");
        Options::Get()->AddOptionInt("SaveLogLevel", opt.log_level);
        Options::Get()->AddOptionInt("QueueLogLevel", opt.log_level);
        Options::Get()->AddOptionInt("DumpTrigger", LogLevel_Error);
        Options::Get()->Lock();

        Manager::Create();

        // Add a callback handler to the manager.
        Manager::Get()->AddWatcher(process_notification, &opt);
        // Add a Z-Wave Driver
        Manager::Get()->AddDriver(opt.device);
        // Default poll interval is 2s.
        // NOTE: only devices explicitly enabled for polling will be polled.
        Manager::Get()->SetPollInterval(500, true);

        // Register save config mqtt topic
        mqtt_subscribe(opt.mqtt_prefix, "ozw/save_config", save_config);

        // Main loop
        mqtt_loop();
    } catch (exception e) {
        printf("Error: %s\n", e.what());
    }
    return 0;
}
