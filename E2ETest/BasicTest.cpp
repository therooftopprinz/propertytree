#include <gtest/gtest.h>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

using namespace testing;
using namespace propertytree;

struct BasicTest : Test
{
    void SetUp()
    {

    }

    void TearDown()
    {

    }

    ClientConfig config = {"127.0.0.1", 12345};
    Client sut = Client(config);
};

TEST_F(BasicTest, shouldSignIn)
{
}

TEST_F(BasicTest, shouldCreateAnSet)
{
    auto root = sut.root();
    auto child1 = sut.create(root, "child1");
    ASSERT_TRUE(child1);
    child1 = 42;
}

TEST_F(BasicTest, shouldGetValue)
{
    auto root = sut.root();
    auto child1 = sut.get(root, "child1");
    child1.fetch();
    EXPECT_EQ(child1.value<int>(), 42);
}

TEST_F(BasicTest, shouldGetValueWithoutTreeInfo)
{
    Client sut2 = Client(config);

    {
        auto root = sut.root();
        auto child1 = sut.get(root, "child1");
        auto child11 = sut.create(child1, "grandchild1");
        child11 = 44;
    }
    {
        auto root = sut2.root();
        auto child1 = sut2.get(root, "child1");
        auto child11 = sut2.get(child1, "grandchild1");
        child11.fetch();
        EXPECT_EQ(child11.value<int>(), 44);
    }

}