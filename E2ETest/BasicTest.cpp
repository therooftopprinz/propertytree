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

TEST_F(BasicTest, shouldCleanTree)
{
    auto root = sut.root();
    root.loadChildren(true);
    struct TraversalContext
    {
        std::vector<std::pair<std::string, Property>> children;
        size_t index = 0;
    };

    std::vector<TraversalContext> levels;

    levels.emplace_back(TraversalContext{root.children(), 0});

    while(true)
    {
        auto& curentLevel = levels.back();
        size_t& index = curentLevel.index;

        if (index >= curentLevel.children.size())
        {
            levels.pop_back();
            if (0 == levels.size())
            {
                break;
            }
        }

        auto& child = curentLevel.children[index].second;
        if (child.childrenSize())
        {
            levels.emplace_back(TraversalContext{child.children(), 0});
            continue;
        }
        else
        {
            child.destroy();
        }
        index++;
    }
}

TEST_F(BasicTest, shouldCreateAnSet)
{
    auto root = sut.root();
    auto child1 = root.create("child1");
    ASSERT_TRUE(child1);
    child1 = 42;
}

TEST_F(BasicTest, shouldGetValue)
{
    auto root = sut.root();
    auto child1 = root.get("child1");
    child1.fetch();
    EXPECT_EQ(child1.value<int>(), 42);
}

TEST_F(BasicTest, shouldGetValueWithoutTreeInfo)
{
    Client sut2 = Client(config);

    {
        auto root = sut.root();
        auto child2 = root.create("child2");
        ASSERT_TRUE(child2);
        child2 = 44;
    }
    {
        auto root = sut2.root();
        auto child2 = root.get("child2");
        ASSERT_TRUE(child2);
        child2.fetch();
        EXPECT_EQ(child2.value<int>(), 44);
    }
}

TEST_F(BasicTest, shouldSubscribeUpdateAndUnsubscribe)
{
    Client sut2 = Client(config);

    auto root = sut.root();
    auto child1 = root.get("child1");
    child1.subscribe();

    {
        auto root = sut2.root();
        auto child1 = root.get("child1");
        child1 = 43;
    }

    std::this_thread::sleep_for(std::chrono::microseconds(200));

    EXPECT_EQ(child1.value<int>(), 43);
    child1.unsubscribe();

    {
        auto root = sut2.root();
        auto child1 = root.get("child1");
        child1 = 44;
    }

    std::this_thread::sleep_for(std::chrono::microseconds(200));

    EXPECT_EQ(child1.value<int>(), 43);
}