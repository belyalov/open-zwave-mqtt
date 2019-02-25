
#include <unistd.h>
#include <mosquitto.h>
#include <algorithm>
#include <openzwave/Manager.h>
#include <openzwave/platform/Log.h>
#include "mqtt.h"
#include "command_classes.h"
#include "polling.h"

using namespace std;

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
        OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
            "MQTT: Topic not found '%s'", msg->topic);
        return;
    }
    const OpenZWave::ValueID& v = it->second;

    // Handle value type
    switch (v.GetType()) {
    case OpenZWave::ValueID::ValueType_Button:
        // Buttons requires dedicated support
        transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value == "1" || value == "true" || value == "t") {
            OpenZWave::Manager::Get()->PressButton(v);
        } else {
            OpenZWave::Manager::Get()->ReleaseButton(v);
        }
        break;
    case OpenZWave::ValueID::ValueType_Bool:
        transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value == "true") {
            OpenZWave::Manager::Get()->SetValue(v, true);
        } else if (value == "false") {
            OpenZWave::Manager::Get()->SetValue(v, false);
        } else {
            // Support int values as well
            OpenZWave::Manager::Get()->SetValue(v, atoi(value.c_str()) > 0);
        }
        break;
    default:
        // Regular value
        OpenZWave::Manager::Get()->SetValue(v, value);
    }

    // Ad hoc fix for GE Dimmers:
    // This is great series of dimmers with / without motion sensor,
    // however they have terrible bug - they don't report light status when updated through ZWave.
    // So this fix simply schedules value to be polled by openzwave intentionally.
    auto n = node_find_by_id(v.GetNodeId());
    // 26933 Smart Motion Dimmer
    if (n && n->product_type == "0x494d" && n->product_id == "0x3034") {
        polling_enable(v, 3);
    }
}

// Create MQTT client connect
void
mqtt_connect(const string& client_id, const string& host, const uint16_t port,
        const string& user, const string& passwd)
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
        OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
            "MQTT: Unable to connect to broker.");
        exit(1);
    }
    OpenZWave::Log::Write(OpenZWave::LogLevel_Info,
        "MQTT: Connected to broker.");
}

void mqtt_loop()
{
    while (1) {
        int res = mosquitto_loop(mqtt_client, -1, 1);
        if(res){
            OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
                "MQTT: connection to broker lost, reconnecting...");
            sleep(1);
            mosquitto_reconnect(mqtt_client);
        }
        // sleep(1);
    }
}

void publish_impl(const string& topic, const string& value)
{
    int res = mosquitto_publish(mqtt_client, NULL, topic.c_str(),
            value.size(), value.c_str(), 0, true);
    if (res != 0) {
        OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
            "MQTT publish to '%s' FAILED (%d)", topic.c_str(), res);
    } else {
        OpenZWave::Log::Write(OpenZWave::LogLevel_Info,
            "MQTT PUBLISH: %s -> %s", topic.c_str(), value.c_str());
    }
}

void
mqtt_publish(const options* opts, const OpenZWave::ValueID& v)
{
    string value;

    if (!OpenZWave::Manager::Get()->GetValueAsString(v, &value)) {
        OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
            v.GetNodeId(), "GetValueAsString() failed.");
        return;
    }

    // By default GetValueAsString() for Boolean types returns
    // True/False instead of 0/1 - which makes it difficult to
    // process messages from switches (true/false) and dimmers (0-99)
    // So convert this value type to 0/1
    if (v.GetType() == OpenZWave::ValueID::ValueType_Bool) {
        bool b;
        if (!OpenZWave::Manager::Get()->GetValueAsBool(v, &b)) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
                v.GetNodeId(), "GetValueAsBool() failed.");
            return;
        };
        value = b ? "1" : "0";
    } else {
        if (!OpenZWave::Manager::Get()->GetValueAsString(v, &value)) {
            OpenZWave::Log::Write(OpenZWave::LogLevel_Error,
                v.GetNodeId(), "GetValueAsString() failed.");
            return;
        }
    }

    // Do not publish empty messages
    if (value.empty()) {
        return;
    }

    // Make 2 topic variations:
    // 1. Name based
    // 2. ID based
    auto topics = value_make_paths(opts->mqtt_prefix, v);

    // If name/id topic found in the filter list - publish
    // only to overridden destination
    auto override = opts->topic_overrides.find(topics.first);
    if (override != opts->topic_overrides.end()) {
        publish_impl(override->second, value);
        return;
    }
    override = opts->topic_overrides.find(topics.second);
    if (override != opts->topic_overrides.end()) {
        publish_impl(override->second, value);
        return;
    }

    // Publish to auto-generated topic name(s)
    if (opts->mqtt_name_topics) {
        publish_impl(topics.first, value);
    }

    if (opts->mqtt_id_topics) {
        publish_impl(topics.second, value);
    }
}

void subscribe_impl(const string& topic, const OpenZWave::ValueID& v)
{
    string ep = topic + "/set";
    int res = mosquitto_subscribe(mqtt_client, NULL, ep.c_str(), 0);
    if (res != 0) {
        throw runtime_error("mosquitto_subscribe() failed");
    }
    endpoints.insert(make_pair(ep, v));
}

void
mqtt_subscribe(const options* opts, const OpenZWave::ValueID& v)
{
    // Ignore read only values
    if (OpenZWave::Manager::Get()->IsValueReadOnly(v)) {
        return;
    }

    // Make string representation of changeable parameter
    auto topics = value_make_paths(opts->mqtt_prefix, v);

    // If name/id topic found in the filter list - publish
    // only to overridden destination
    auto override = opts->topic_overrides.find(topics.first);
    if (override != opts->topic_overrides.end()) {
        subscribe_impl(override->second, v);
        return;
    }
    override = opts->topic_overrides.find(topics.second);
    if (override != opts->topic_overrides.end()) {
        subscribe_impl(override->second, v);
        return;
    }

    // Subscribe to name based topic, if enabled
    if (opts->mqtt_name_topics) {
        subscribe_impl(topics.first, v);
    }

    // Subscribe to id based topic, if enabled
    if (opts->mqtt_id_topics) {
        subscribe_impl(topics.second, v);
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
