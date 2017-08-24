
#include <gtest/gtest.h>
#include "node_value.h"

#include "mock_manager.h"


using namespace std;
using namespace OpenZWave;


class value_tests: public ::testing::Test
{
protected:
    void SetUp() {
        n = node_add(1, 1);
        hid = n->home_id;
    }

    void TearDown() {
        node_remove_all();
        mock_manager_cleanup();
    }

    shared_ptr<const node> n;
    uint32_t hid;
};

TEST_F(value_tests, add)
{
    // create one more node
    auto n2 = node_add(1, 2);

    // add 2 values to each node
    // homeId, nodeId, genre, command_class, instance, index, type
    auto v1 = ValueID(hid, 1, ValueID::ValueGenre_User, 10, 1, 1, ValueID::ValueType_Int);
    auto v2 = ValueID(hid, 1, ValueID::ValueGenre_User, 11, 1, 2, ValueID::ValueType_Int);
    auto v3 = ValueID(hid, 2, ValueID::ValueGenre_User, 12, 1, 1, ValueID::ValueType_String);
    auto v4 = ValueID(hid, 2, ValueID::ValueGenre_User, 13, 1, 2, ValueID::ValueType_String);

    value_add(v1);
    value_add(v2);
    value_add(v3);
    value_add(v4);

    // Check
    ASSERT_EQ((vector<ValueID>{v1, v2}), n->values);
    ASSERT_EQ((vector<ValueID>{v3, v4}), n2->values);
}

TEST_F(value_tests, remove)
{
    auto& vals = n->values;
    // homeId, nodeId, genre, command_class, instance, index, type
    auto v1 = ValueID(hid, 1, ValueID::ValueGenre_User, 10, 1, 1, ValueID::ValueType_Int);
    auto v2 = ValueID(hid, 1, ValueID::ValueGenre_User, 11, 1, 2, ValueID::ValueType_Int);
    auto v3 = ValueID(hid, 1, ValueID::ValueGenre_User, 12, 1, 3, ValueID::ValueType_String);

    // add / check 3 values
    value_add(v1);
    value_add(v2);
    value_add(v3);
    ASSERT_EQ((vector<ValueID>{v1, v2, v3}), vals);

    // Delete from the middle, then check
    value_remove(v2);
    ASSERT_EQ((vector<ValueID>{v1, v3}), vals);

    // Delete last value
    value_remove(v3);
    ASSERT_EQ((vector<ValueID>{v1}), vals);

    // Finally delete all
    value_remove(v1);
    ASSERT_EQ((vector<ValueID>{}), vals);

    // One more time - negative case
    value_remove(v3);
    ASSERT_EQ((vector<ValueID>{}), vals);
}

TEST(value_utils, value_escape)
{
    map<string, string> runs = {
        {"VALUE", "value"},
        {"VaLue blah", "value_blah"},
        {"  ", "__"},
        {"//", "__"},
        {"VALUE//BLAH", "value__blah"},
        {"value_blah", "value_blah"},
        {"value/blah", "value_blah"},
    };

    for (auto it = runs.begin(); it != runs.end(); ++it) {
        ASSERT_EQ(it->second, value_escape_label(it->first));
    }
}
