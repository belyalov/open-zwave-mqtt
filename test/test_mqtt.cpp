
#include <gtest/gtest.h>
#include "node_value.h"
#include "mqtt.h"
#include "mock_manager.h"
#include "mock_mosquitto.h"

using namespace std;
using namespace OpenZWave;


// Declared in mqtt.cpp, don't want to declare it in header just for unittests
extern void
mqtt_message_callback(struct mosquitto*, void*, const struct mosquitto_message*);


class mqtt_tests: public ::testing::Test
{
protected:
    void SetUp() {
        // create 2 nodes
        node_add(1, 1);
        node_add(1, 2);
    }

    void TearDown() {
        node_remove_all();
        mock_manager_cleanup();
        mqtt_unsubscribe_all();
        mock_mosquitto_cleanup();
    }

    template<typename T>
    void ASSERT_SUBSCRIPTIONS(T runs) {
        // Check created endpoints / mosquitto library subscribe history
        auto subs = mqtt_get_endpoints();
        auto hist = mock_mosquitto_subscribe_history();
        // endpoints equals created subscriptions
        ASSERT_EQ(subs.size(), hist.size());
        // Check each endpoint to match run
        for (auto it = runs.begin(); it != runs.end(); ++it) {
            auto it2 = subs.find(it->first.first);
            auto it3 = subs.find(it->first.second);
            ASSERT_TRUE(it2 != subs.end());
            ASSERT_TRUE(it3 != subs.end());
            ASSERT_EQ(it->second.GetId(), it2->second.GetId());
            ASSERT_EQ(it->second.GetId(), it3->second.GetId());
        }
        // One more cross check - subscription history to match endpoints
        for (size_t i = 0; i < hist.size(); i++) {
            ASSERT_TRUE(subs.find(hist[i]) != subs.end());
        }
    }

    template<typename T>
    void ASSERT_PUBLICATIONS(T runs) {
        // Check publication count
        auto hist = mock_mosquitto_publish_history();
        ASSERT_EQ(runs.size() * 2, hist.size());
        // Create temporary map of topics -> values.
        // This is limited to have only one message per topic, which is OK for tests
        map<string, string> topic_payload;
        for (auto it = runs.begin(); it != runs.end(); ++it) {
            string payload;
            OpenZWave::Manager::Get()->GetValueAsString(it->first, &payload);
            auto topics = it->second;
            topic_payload[topics.first] = payload;
            topic_payload[topics.second] = payload;
        }
        // Ensure that publication history is equal to runs
        for (auto it = hist.begin(); it != hist.end(); ++it) {
            const string& topic = (*it).first;
            const string& val = (*it).second;
            ASSERT_TRUE(topic_payload.find(topic) != topic_payload.end()) << "Not found: " << topic;
            ASSERT_EQ(val, topic_payload.find(topic)->second);
        }
    }

    // Dummy options
    options opts = {};
};


TEST_F(mqtt_tests, subscribe)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> valueID
    map<pair<string, string>, const ValueID> runs = {
        // regular value
        {
            {"location_h1_n1/name_h1_n1/basic/label1/set", "1/32/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n2/name_h1_n2/meter/label1/set", "2/50/1/set"},
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int)
        },
        // multi instance
        {
            {"location_h1_n1/name_h1_n1/switch_binary/1/label1/set", "1/37/1/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_binary/2/label1/set", "1/37/2/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 2, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_multilevel/1/label1/set", "1/38/1/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x26, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_multilevel/2/label1/set", "1/38/2/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x26, 2, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mqtt_subscribe(&opts, it->second);
    }

    // check subscriptions
    ASSERT_SUBSCRIPTIONS(runs);
}

TEST_F(mqtt_tests, subscribe_escape_value_label)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> ValueID
    map<pair<string, string>, pair<const ValueID, string> > runs = {
        {
            {"location_h1_n1/name_h1_n1/basic/somelabel/set", "1/32/1/set"},
            {ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int), "SoMeLAbeL"}
        },
        {
            {"location_h1_n2/name_h1_n2/meter/_space_dash-_smth/set", "2/50/1/set"},
            {ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int), " space dash- SMTH"}
        },
    };

    // subscribe
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        // Set mock label value
        mock_manager_set_value_label(it->second.first, it->second.second);
        mqtt_subscribe(&opts, it->second.first);
    }

    // Check history
    auto hist = mock_mosquitto_subscribe_history();
    size_t idx = 0;
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        ASSERT_EQ(hist[idx++], it->first.first);
        ASSERT_EQ(hist[idx++], it->first.second);
    }
}

TEST_F(mqtt_tests, subscribe_readonly)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> (homeId, nodeId, genre, command_class, instance, index, type)
    map<pair<string, string>, const ValueID> runs = {
        {
            {"location_h1_n1/name_h1_n1/basic/label1/set", "1/32/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n2/name_h1_n2/meter/label1/set", "2/50/1/set"},
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe to read only values
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mock_manager_set_value_readonly(it->second);
        mqtt_subscribe(&opts, it->second);
    }

    // there should be no subscriptions - all values are readonly
    ASSERT_TRUE(mqtt_get_endpoints().empty());
}

TEST_F(mqtt_tests, prefix)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> (homeId, nodeId, genre, command_class, instance, index, type)
    map<pair<string, string>, const ValueID> runs = {
        {
            {"prefix/location_h1_n1/name_h1_n1/basic/label1/set", "prefix/1/32/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"prefix/location_h1_n1/name_h1_n1/switch_binary/1/label1/set", "prefix/1/37/1/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 1, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe to read only values
    opts.mqtt_prefix = "prefix";
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mqtt_subscribe(&opts, it->second);
    }

    // Check subscriptions
    ASSERT_SUBSCRIPTIONS(runs);
}

TEST_F(mqtt_tests, publish)
{
    // Override default generated location for node10
    OpenZWave::Manager::Get()->SetNodeLocation(1, 10, "");
    node_add(1, 10);

    // valueID -> (<topic1, topic2> -> payload)
    map<const ValueID, pair<string, string> > runs = {
        // regular value
        {
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int),
            {"location_h1_n1/name_h1_n1/basic/label1", "1/32/1"}
        },
        {
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int),
            {"location_h1_n2/name_h1_n2/meter/label1", "2/50/1"}
        },
        // empty location
        {
            ValueID(1, 10, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int),
            {"name_h1_n10/basic/label1", "10/32/1"}
        },
        // multi instance
        {
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 1, 1, ValueID::ValueType_Int),
            {"location_h1_n1/name_h1_n1/switch_binary/1/label1", "1/37/1/1"}
        },
        {
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 2, 1, ValueID::ValueType_Int),
            {"location_h1_n1/name_h1_n1/switch_binary/2/label1", "1/37/2/1"}
        }
    };

    // Publish values
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mqtt_publish(&opts, it->first);
    }

    ASSERT_PUBLICATIONS(runs);
}

TEST_F(mqtt_tests, incoming_message)
{
    // mqtt_message_callback(struct mosquitto*, void*, const struct mosquitto_message*);
    map<pair<string, string>, const ValueID> runs = {
        {
            {"location_h1_n1/name_h1_n1/basic/label1/set", "1/32/1/set"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n2/name_h1_n2/meter/label1/set", "2/50/1/set"},
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int)
        },
    };

    // add values / subscribe to them
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        value_add(it->second);
        mqtt_subscribe(&opts, it->second);
    }

    // Emulate mqtt message callback
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        struct mosquitto_message m {};
        // first, named topic
        m.topic = (char*) it->first.first.c_str();
        m.payload = (void*) "1";
        m.payloadlen = 1;
        mqtt_message_callback(NULL, NULL, &m);
        // second, named topic
        m.topic = (char*) it->first.second.c_str();
        m.payload = (void*) "2";
        m.payloadlen = 1;
        mqtt_message_callback(NULL, NULL, &m);
    }

    // Check results
    auto hist = mock_manager_get_value_set_history();
    size_t idx = 0;
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        // first topic
        ASSERT_EQ(it->second.GetId(), hist[idx].first);
        ASSERT_EQ("1", hist[idx++].second);
        // second topic
        ASSERT_EQ(it->second.GetId(), hist[idx].first);
        ASSERT_EQ("2", hist[idx++].second);
    }
}

// Helper for custom subscriptions
bool custom_called1 = false;
bool custom_called2 = false;

void
custom_subscr_helper(const string& value)
{
    if (value == "1") {
        custom_called1 = true;
    } else {
        custom_called2 = true;
    }
}


TEST_F(mqtt_tests, subscribe_custom)
{
    // Make 2 subscriptions to custom topic / function
    mqtt_subscribe("", "some/custom/topic", custom_subscr_helper);
    mqtt_subscribe("pref", "some/custom/topic", custom_subscr_helper);

    // check custom subscriptions
    ASSERT_EQ(2, mqtt_get_endpoints_custom().size());

    // Reset flags
    custom_called1 = false;
    custom_called2 = false;

    // Emulate incoming message without prefix
    struct mosquitto_message m {};
    m.topic = (char*) "some/custom/topic";
    m.payload = (void*) "1";
    m.payloadlen = 1;
    mqtt_message_callback(NULL, NULL, &m);

    // Second one - with prefix
    m.topic = (char*) "pref/some/custom/topic";
    m.payload = (void*) "2";
    // "Send" message
    mqtt_message_callback(NULL, NULL, &m);

    // Check flags
    ASSERT_TRUE(custom_called1);
    ASSERT_TRUE(custom_called2);
}
