
#include <gtest/gtest.h>
#include "node_value.h"
#include "mqtt.h"
#include "mock_manager.h"

using namespace std;
using namespace OpenZWave;


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
    }

    template<typename T>
    void ASSERT_SUBSCRIPTIONS(T runs) {
        auto subs = mqtt_get_endpoints();
        for (auto it = runs.begin(); it != runs.end(); ++it) {
            auto it2 = subs.find(it->first.first);
            auto it3 = subs.find(it->first.second);
            ASSERT_TRUE(it2 != subs.end());
            ASSERT_TRUE(it3 != subs.end());
            ASSERT_EQ(it->second.GetId(), it2->second.GetId());
            ASSERT_EQ(it->second.GetId(), it3->second.GetId());
        }
    }
};


TEST_F(mqtt_tests, subscribe)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> (homeId, nodeId, genre, command_class, instance, index, type)
    map<pair<string, string>, const ValueID> runs = {
        // regular value
        {
            {"location_h1_n1/name_h1_n1/basic/label1", "1/32/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n2/name_h1_n2/meter/label1", "2/50/1"},
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int)
        },
        // multi instance
        {
            {"location_h1_n1/name_h1_n1/switch_binary/1/label1", "1/37/1/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_binary/2/label1", "1/37/2/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 2, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_multilevel/1/label1", "1/38/1/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x26, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n1/name_h1_n1/switch_multilevel/2/label1", "1/38/2/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x26, 2, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mqtt_subscribe("", it->second);
    }

    // check subscriptions
    ASSERT_SUBSCRIPTIONS(runs);
}

TEST_F(mqtt_tests, subscribe_readonly)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> (homeId, nodeId, genre, command_class, instance, index, type)
    map<pair<string, string>, const ValueID> runs = {
        {
            {"location_h1_n1/name_h1_n1/basic/label1", "1/32/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"location_h1_n2/name_h1_n2/meter/label1", "2/50/1"},
            ValueID(1, 2, ValueID::ValueGenre_User, 0x32, 1, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe to read only values
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mock_manager_set_value_readonly(it->second);
        mqtt_subscribe("", it->second);
    }

    // there should be no subscriptions - all values are readonly
    ASSERT_EQ((std::map<std::string, const OpenZWave::ValueID>){}, mqtt_get_endpoints());
}

TEST_F(mqtt_tests, prefix)
{
    // path: prefix/node_location/node_name/command_class_name
    // path: prefix/node_id/command_class_id
    // path -> (homeId, nodeId, genre, command_class, instance, index, type)
    map<pair<string, string>, const ValueID> runs = {
        {
            {"prefix/location_h1_n1/name_h1_n1/basic/label1", "prefix/1/32/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x20, 1, 1, ValueID::ValueType_Int)
        },
        {
            {"prefix/location_h1_n1/name_h1_n1/switch_binary/1/label1", "prefix/1/37/1/1"},
            ValueID(1, 1, ValueID::ValueGenre_User, 0x25, 1, 1, ValueID::ValueType_Int)
        },
    };

    // subscribe to read only values
    for (auto it = runs.begin(); it != runs.end(); ++it) {
        mqtt_subscribe("prefix", it->second);
    }

    // Check subscriptions
    ASSERT_SUBSCRIPTIONS(runs);
}
