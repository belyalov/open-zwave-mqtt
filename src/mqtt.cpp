
#include <unistd.h>
#include <mosquitto.h>
#include <algorithm>
#include <openzwave/Manager.h>
#include <openzwave/platform/Log.h>
#include "mqtt.h"
#include "command_classes.h"
#include "polling.h"

using namespace std;
using namespace OpenZWave;

// mapping between string and actual openzwave value
map<string, const OpenZWave::ValueID> endpoints;
// custom topic bindings
map<string, mqtt_custom_topic_callback> endpoints_custom;

// mosquitto mqtt client
mosquitto* mqtt_client = NULL;


void
mqtt_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* msg)
{
    string topic = msg->topic;
    string value(static_cast<const char*> (msg->payload), msg->payloadlen);

    // Custom endpoints (topic -> function)
    auto cit = endpoints_custom.find(topic);
    if (cit != endpoints_custom.end()) {
        cit->second(value);
        return;
    }

    // For value based payloads - ignore messages with empty value
    if(!msg->payloadlen){
        // Ignore empty messages
        return;
    }

    auto it = endpoints.find(msg->topic);
    if (it == endpoints.end()) {
        // Topic not found
        Log::Write(LogLevel_Error, "MQTT: Topic not found '%s'", msg->topic);
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

    // Ad hoc fix for GE Dimmers:
    // This is great series of dimmers with / without motion sensor,
    // however it has terrible bug - it does not report light status when updated through ZWave.
    // So this fix simply schedules 3 value POLL
    auto n = node_find_by_id(v.GetNodeId());
    // 26933 Smart Motion Dimmer
    if (n && n->product_type == "0x494d" && n->product_id == "0x3034") {
        polling_enable(v, 3);
    }
}

// Create MQTT client connect
void
mqtt_connect(const string& client_id, const string& host, const uint16_t port, const string& user, const string& passwd)
{
    // Init MQTT library - mosquitto
    mosquitto_lib_init();
    // Create MQTT client: id - openzwave, clean session
    mqtt_client = mosquitto_new(client_id.c_str(), true, 0);
    // Set user details
    mosquitto_username_pw_set(mqtt_client, user.c_str(), passwd.c_str());
    // Set message callback
    mosquitto_message_callback_set(mqtt_client, mqtt_message_callback);
    // Connect to broker
    int res = mosquitto_connect(mqtt_client, host.c_str(), port, 60);
    if (res != 0) {
        Log::Write(LogLevel_Error, "MQTT: Unable to connect to broker.");
        exit(1);
    }
    Log::Write(LogLevel_Info, "MQTT: Connected to broker.");
}

void mqtt_loop()
{
    while (1) {
        int res = mosquitto_loop(mqtt_client, -1, 1);
        if(res){
            Log::Write(LogLevel_Error, "MQTT: connection to broker lost, reconnecting...");
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
    if (!n->location.empty()) {
        name_path += n->location + "/";
    }
    name_path += n->name + "/" + command_class_str(cmd_class);
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
mqtt_publish(const options* opts, const OpenZWave::ValueID& v)
{
    int res;
    string value;

    if (!OpenZWave::Manager::Get()->GetValueAsString(v, &value)) {
        Log::Write(LogLevel_Error, v.GetNodeId(), "GetValueAsString() failed.");
        return;
    }

    // Publish value to MQTT
    if (value.empty()) {
        return;
    }

    auto topics = make_value_path(opts->mqtt_prefix, v);

    if (opts->mqtt_name_topics) {
        res = mosquitto_publish(mqtt_client, NULL, topics.first.c_str(),
                value.size(), value.c_str(), 0, true);
        if (res != 0) {
            Log::Write(LogLevel_Error, v.GetNodeId(),
                "Error while publishing message to MQTT topic '%s'", topics.first.c_str());
        } else {
            Log::Write(LogLevel_Debug, v.GetNodeId(), "MQTT PUBLISH: %s -> %s",
                topics.first.c_str(), value.c_str());
        }
    }

    if (opts->mqtt_id_topics) {
        res = mosquitto_publish(mqtt_client, NULL, topics.second.c_str(),
                value.size(), value.c_str(), 0, true);
        if (res != 0) {
            Log::Write(LogLevel_Error, v.GetNodeId(),
                "Error while publishing message to MQTT topic '%s'", topics.second.c_str());
        } else {
            Log::Write(LogLevel_Debug, v.GetNodeId(), "MQTT PUBLISH: %s -> %s",
                topics.second.c_str(), value.c_str());
        }
    }
}

void
mqtt_subscribe(const options* opts, const OpenZWave::ValueID& v)
{
    // Ignore read only values
    if (OpenZWave::Manager::Get()->IsValueReadOnly(v)) {
        return;
    }

    // Make string representation of changeable parameter
    auto paths = make_value_path(opts->mqtt_prefix, v);

    // Subscribe to name based topic, if enabled
    if (opts->mqtt_name_topics) {
        string ep = paths.first + "/set";
        int res = mosquitto_subscribe(mqtt_client, NULL, ep.c_str(), 0);
        if (res != 0) {
            throw runtime_error("mosquitto_subscribe failed");
        }
        endpoints.insert(make_pair(ep, v));
    }

    // Subscribe to id based topic, if enabled
    if (opts->mqtt_id_topics) {
        string ep = paths.second + "/set";
        int res = mosquitto_subscribe(mqtt_client, NULL, ep.c_str(), 0);
        if (res != 0) {
            throw runtime_error("mosquitto_subscribe failed");
        }
        endpoints.insert(make_pair(ep, v));
    }
}

// topic -> custom function subscriber
void
mqtt_subscribe(const string& pref, const string& topic, mqtt_custom_topic_callback cb)
{
    string top = pref;
    if (!top.empty()) {
        top += "/";
    }
    top += topic;

    // subscribe to topic / add to map
    int res = mosquitto_subscribe(mqtt_client, NULL, top.c_str(), 0);
    if (res != 0) {
        throw runtime_error("mosquitto_subscribe failed");
    }
    endpoints_custom.insert(make_pair(top, cb));
}

// Mostly for unittests

void
mqtt_unsubscribe_all()
{
    endpoints.clear();
    endpoints_custom.clear();
}

const map<string, const OpenZWave::ValueID>&
mqtt_get_endpoints()
{
    return endpoints;
}

const map<string, mqtt_custom_topic_callback>
mqtt_get_endpoints_custom()
{
    return endpoints_custom;
}
