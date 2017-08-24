
#include <gtest/gtest.h>
#include "node_value.h"

#include "mock_manager.h"


using namespace std;

class node_tests: public ::testing::Test
{
protected:
    void SetUp() {
    }

    void TearDown() {
        node_remove_all();
        mock_manager_cleanup();
    }
};


TEST_F(node_tests, create)
{
    // Successful add
    // home_id, node_id
    shared_ptr<const node> n = node_add(1, 2);
    ASSERT_TRUE(n);
    ASSERT_EQ(1, n->home_id);
    ASSERT_EQ(2, n->id);
    ASSERT_EQ("name_h1_n2", n->name);
    ASSERT_EQ("location_h1_n2", n->location);

    ASSERT_EQ("manufacturer_id_h1_n2", n->manufacturer_id);
    ASSERT_EQ("product_id_h1_n2", n->product_id);
    ASSERT_EQ("product_type_h1_n2", n->product_type);

    // Node already exists
    ASSERT_THROW(node_add(1, 2), invalid_argument);
}

TEST_F(node_tests, find)
{
    // home_id, node_id
    node_add(5, 6);

    // Positive case
    shared_ptr<const node> n1 = node_find_by_id(6);
    ASSERT_TRUE(n1);
    ASSERT_EQ("name_h5_n6", n1->name);

    // Negative - no such node
    shared_ptr<const node> n2 = node_find_by_id(11);
    ASSERT_FALSE(n2);
}

TEST_F(node_tests, delete)
{
    // home_id, node_id
    node_add(11, 22);
    node_add(11, 33);

    // Check n delete
    ASSERT_TRUE(node_find_by_id(22));
    node_remove_by_id(22);

    // Check again
    ASSERT_FALSE(node_find_by_id(22));

    // Delete all
    node_remove_all();

    // Check
    ASSERT_FALSE(node_find_by_id(22));
    ASSERT_FALSE(node_find_by_id(33));
}

TEST_F(node_tests, get_all)
{
    // Add 3 nodes (home_id, node_id)
    vector<shared_ptr<const node> > created;
    created.push_back(node_add(1, 1));
    created.push_back(node_add(1, 2));
    created.push_back(node_add(1, 3));

    ASSERT_EQ(created, node_get_all());

}
