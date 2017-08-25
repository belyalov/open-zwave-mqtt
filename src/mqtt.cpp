
#include <unistd.h>
#include <mosquitto.h>
#include <algorithm>
#include <openzwave/Manager.h>
#include "mqtt.h"
#include "command_classes.h"

using namespace std;

// mapping between string and actual openzwave value
map<string, const OpenZWave::ValueID> endpoints;

// mosquitto mqtt client
mosquitto* mqtt_client = NULL;


void
mqtt_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* msg)
{
    if(!msg->payloadlen){
        // Ignore empty messages
        return;
    }

    string topic = msg->topic;
    string value(static_cast<const char*> (msg->payload), msg->payloadlen);

    auto it = endpoints.find(msg->topic);
    if (it == endpoints.end()) {
        // Topic not found
        printf("Topic not found '%s'\n", msg->topic);
        return;
    }
    const OpenZWave::ValueID& v = it->second;

    if (v.GetType() == OpenZWave::ValueID::ValueType::ValueType_Button) {
        // Buttons requires dedicated support
        transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value == "1" || value == "true" || value == "t") {
            OpenZWave::Manager::Get()->PressButton(v);
        } else {
            OpenZWave::Manager::Get()->ReleaseButton(v);
        }
    } else {
        // Regular value
        OpenZWave::Manager::Get()->SetValue(v, value);
    }
}

// Create MQTT client connect
void
mqtt_connect(const string& client_id, const string& host, const uint16_t port)
{
    // Init MQTT library - mosquitto
    mosquitto_lib_init();
    // Create MQTT client: id - openzwave, clean session
    mqtt_client = mosquitto_new(client_id.c_str(), true, 0);
    // Set message callback
    mosquitto_message_callback_set(mqtt_client, mqtt_message_callback);
    // Connect to broker
    int res = mosquitto_connect(mqtt_client, host.c_str(), port, 60);
    if (res != 0) {
        printf("Unable to connect to MQTT broker\n");
        exit(1);
    }
}

void mqtt_loop()
{
    while (1) {
        int res = mosquitto_loop(mqtt_client, -1, 1);
        if(res){
            printf("connection lost, reconnecting.\n");
            sleep(1);
            mosquitto_reconnect(mqtt_client);
        }
        // sleep(1);
    }
}


// Make string from OpenZwave value
pair<string, string>
make_value_path(const string& prefix, const OpenZWave::ValueID& v)
{
    auto n = node_find_by_id(v.GetNodeId());
    if (!n) {
        throw invalid_argument("Node not found");
    }

    uint8_t cmd_class = v.GetCommandClassId();

    // prefix/node_location/node_name/command_class_name
    // prefix/node_id/command_class_id
    string name_path;
    string id_path;

    if (!prefix.empty()) {
        name_path += prefix + "/";
        id_path += prefix + "/";
    }
    name_path += n->location + "/" + n->name + "/" + command_class_str(cmd_class);
    id_path += to_string(n->id) + "/" + to_string(cmd_class);

    // Several command types support multi instances (e.g. 2-relay binary switch), so, add instance as well
    // Today we've found only 2 types that support it: SWITCH_MULTILEVEL and SWITCH_BINARY
    if (cmd_class == 0x25 || cmd_class == 0x26) {
        name_path += "/" + to_string(v.GetInstance());
        id_path += "/" + to_string(v.GetInstance());
    }
    name_path += "/" + value_escape_label(OpenZWave::Manager::Get()->GetValueLabel(v));
    id_path += "/" + to_string(v.GetIndex());

    return make_pair(name_path, id_path);
}

void
mqtt_publish(const string& prefix, const OpenZWave::ValueID& v)
{
    string value;

    if (!OpenZWave::Manager::Get()->GetValueAsString(v, &value)) {
        printf("GetValueAsString failed\n");
        return;
    }

    // Publish value to MQTT
    if (value.empty()) {
        return;
    }

    auto topics = make_value_path(prefix, v);

    int res = mosquitto_publish(mqtt_client, NULL, topics.first.c_str(),
            value.size(), value.c_str(), 0, true);
    if (res != 0) {
        printf("Error while publishing message to MQTT topic %s\n", topics.first.c_str());
    } else {
        // syslog here
        // printf("PUBLISH: %s -> %s\n", topics.first.c_str(), value.c_str());
    }

    res = mosquitto_publish(mqtt_client, NULL, topics.second.c_str(),
            value.size(), value.c_str(), 0, true);
    if (res != 0) {
        printf("Error while publishing message to MQTT topic %s\n", topics.second.c_str());
    } else {
        // syslog ?
        // printf("PUBLISH: %s -> %s\n", topics.second.c_str(), value.c_str());
    }
}

void
mqtt_subscribe(const string& prefix, const OpenZWave::ValueID& v)
{
    // Ignore read only values
    if (OpenZWave::Manager::Get()->IsValueReadOnly(v)) {
        return;
    }

    // Make string representation of changeable parameter
    auto paths = make_value_path(prefix, v);
    string ep1 = paths.first + "/set";
    string ep2 = paths.second + "/set";

    // subscribe to both topics - name / id based
    int res = mosquitto_subscribe(mqtt_client, NULL, ep1.c_str(), 0);
    if (res != 0) {
        throw runtime_error("mosquitto_subscribe failed");
    }
    endpoints.insert(make_pair(ep1, v));

    res = mosquitto_subscribe(mqtt_client, NULL, ep2.c_str(), 0);
    if (res != 0) {
        throw runtime_error("mosquitto_subscribe failed");
    }
    endpoints.insert(make_pair(ep2, v));
}


// Mostly for unittests

void
mqtt_unsubscribe_all()
{
    endpoints.clear();
}

const map<string, const OpenZWave::ValueID>&
mqtt_get_endpoints()
{
    return endpoints;
}
