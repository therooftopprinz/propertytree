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

    void clean()
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
            auto* curentLevel = &levels.back();
            size_t* index = &curentLevel->index;

            if (*index >= curentLevel->children.size())
            {
                levels.pop_back();
                if (0 == levels.size())
                {
                    break;
                }
                curentLevel = &levels.back();
                index = &curentLevel->index;
            }

            auto& child = curentLevel->children[*index].second;
            if (child.childrenSize())
            {
                levels.emplace_back(TraversalContext{child.children(), 0});
                continue;
            }
            else
            {
                child.destroy();
            }
            (*index)++;
        }
    }

    ClientConfig config = {"127.0.0.1", 12345};
    Client sut = Client(config);
};

TEST_F(BasicTest, shouldSignIn)
{
}

TEST_F(BasicTest, shouldCleanTree)
{
    clean();
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

TEST_F(BasicTest, shouldRetrieveTree)
{
    auto root = sut.root();
    auto child1 = root.get("child1");
    auto child2 = root.get("child2");
    auto child3 = root.create("child3");
    auto child4 = root.create("child4");

    child1 = 1;
    child2 = 2;
    child3 = 3;
    child4 = 4;

    auto child1_grandchild1 = child1.create("grandchild1");
    auto child1_grandchild2 = child1.create("grandchild2");

    child1_grandchild1 = 11;
    child1_grandchild2 = 12;

    auto child1_grandchild1_great1 = child1_grandchild1.create("great1");
    auto child1_grandchild1_great2 = child1_grandchild1.create("great2");

    child1_grandchild1_great1 = 111;
    child1_grandchild1_great2 = 112;

    {
        Client sut2 = Client(config);
        auto root = sut2.root();
        root.loadChildren(true);
        auto rootChildren = root.children();
        auto child1 = rootChildren[0].second;
        auto child2 = rootChildren[1].second;
        auto child3 = rootChildren[2].second;
        auto child4 = rootChildren[3].second;

        child1.fetch();
        child2.fetch();
        child3.fetch();
        child4.fetch();

        EXPECT_EQ(child1.value<int>(), 1);
        EXPECT_EQ(child2.value<int>(), 2);
        EXPECT_EQ(child3.value<int>(), 3);
        EXPECT_EQ(child4.value<int>(), 4);

        auto child1Children = child1.children();
        auto child1_grandchild1 = child1Children[0].second;
        auto child1_grandchild2 = child1Children[1].second;

        child1_grandchild1.fetch();
        child1_grandchild2.fetch();

        EXPECT_EQ(child1_grandchild1.value<int>(), 11);
        EXPECT_EQ(child1_grandchild2.value<int>(), 12);

        auto child1_grandchild1Children = child1_grandchild1.children();
        auto child1_grandchild1_great1 = child1_grandchild1Children[0].second;
        auto child1_grandchild1_great2 = child1_grandchild1Children[1].second;

        child1_grandchild1_great1.fetch();
        child1_grandchild1_great2.fetch();

        EXPECT_EQ(child1_grandchild1_great1.value<int>(), 111);
        EXPECT_EQ(child1_grandchild1_great2.value<int>(), 112);
    }
}