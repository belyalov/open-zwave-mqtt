
#include <signal.h>
#include <stdio.h>
#include <openzwave/Options.h>
#include <openzwave/Manager.h>
#include <openzwave/Driver.h>
#include <openzwave/platform/Log.h>

#include "node_value.h"
#include "mqtt.h"
#include "process_notification.h"

using namespace OpenZWave;


void
signal_handler(int s)
{
    // Manager::Get()->WriteConfig(home_id);
    printf("Caught signal %d\n",s);
    exit(1);
}


//-----------------------------------------------------------------------------
// <main>
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    // Setup signal handling
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signal_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    printf("Starting OpenZWave to MQTT bridge.\n");

    // Make a connection to MQTT broker
    mqtt_connect("ozw-mqtt", "192.168.1.1", 1883);

    // Create the OpenZWave Manager.
    // The first argument is the path to the config files (where the manufacturer_specific.xml file is located
    // The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL
    // the log file will appear in the program's working directory.
    Options::Create("/usr/local/etc/openzwave", "./", "./" );
    Options::Get()->AddOptionInt("SaveLogLevel", LogLevel_Warning);
    Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Warning);
    // Options::Get()->AddOptionInt("SaveLogLevel", LogLevel_Debug);
    // Options::Get()->AddOptionInt("QueueLogLevel", LogLevel_Debug);
    Options::Get()->AddOptionInt("DumpTrigger", LogLevel_Error);
    Options::Get()->Lock();

    Manager::Create();

    // Add a callback handler to the manager.  The second argument is a context that
    // is passed to the OnNotification method.  If the OnNotification is a method of
    // a class, the context would usually be a pointer to that class object, to
    // avoid the need for the notification handler to be a static.
    Manager::Get()->AddWatcher(process_notification, NULL);

    // Add a Z-Wave Driver
    // Modify this line to set the correct serial port for your PC interface.

#ifdef __APPLE__
    string port = "/dev/cu.usbmodem1411141";
#else
    string port = "/dev/zwave";
#endif
    Manager::Get()->AddDriver(port);

    // Main loop
    mqtt_loop();

    return 0;
}
