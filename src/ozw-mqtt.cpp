
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


//-----------------------------------------------------------------------------
// <main>
//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
    // Parse command line options
    options opt;
    bool opt_res = opt.parse_argv(argc, argv);
    if (!opt_res) {
        exit(1);
    }

    // Setup signal handling
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    printf("Starting OpenZWave to MQTT bridge.\n");

    // Make a connection to MQTT broker
    mqtt_connect(opt.mqtt_client_id, opt.mqtt_host, opt.mqtt_port);

    // Create the OpenZWave Manager.
    // The first argument is the path to the config files (where the manufacturer_specific.xml file is located
    // The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL
    // the log file will appear in the program's working directory.
    Options::Create(opt.system_config, opt.user_config, "");
    if (opt.debug) {
        Options::Get()->AddOptionInt("SaveLogLevel", LogLevel_Debug);
        Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Debug);
    } else {
        Options::Get()->AddOptionInt("SaveLogLevel", LogLevel_Warning);
        Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Warning);
    }
    Options::Get()->AddOptionInt("DumpTrigger", LogLevel_Error);
    Options::Get()->Lock();

    Manager::Create();

    // Add a callback handler to the manager.
    Manager::Get()->AddWatcher(process_notification, &opt);
    // Add a Z-Wave Driver
    Manager::Get()->AddDriver(opt.device);

    // Main loop
    mqtt_loop();

    return 0;
}
