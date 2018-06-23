
#include <gtest/gtest.h>
#include "polling.h"

#include "mock_manager.h"


using namespace std;
using namespace OpenZWave;


class polling_tests: public ::testing::Test
{
protected:
    void SetUp() {
        hid = 10;
    }

    void TearDown() {
        polling_disable_all();
        mock_manager_cleanup();
    }

    // Home ID
    uint32_t hid;
};


TEST_F(polling_tests, sanity)
{
    // Create dummy value
    auto v1 = ValueID(hid, 2, ValueID::ValueGenre_User, 13, 1, 2, ValueID::ValueType_String);

    // enable poll for value
    polling_enable(v1, 2);
    // Ensure that polling has been enabled
    ASSERT_TRUE(mock_manager_get_polling_state(v1));

    // disable polling, should be actually disabled on the second call
    polling_disable(v1);
    ASSERT_TRUE(mock_manager_get_polling_state(v1));
    polling_disable(v1);
    ASSERT_FALSE(mock_manager_get_polling_state(v1));
}

TEST_F(polling_tests, disable_on_not_enabled_value)
{
    // No impact expected when disabling on non-existing item
    auto v1 = ValueID(hid, 5, ValueID::ValueGenre_User, 13, 1, 2, ValueID::ValueType_String);
    polling_disable(v1);
    ASSERT_FALSE(mock_manager_get_polling_state(v1));
}
