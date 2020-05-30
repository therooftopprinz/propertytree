#include <gtest/gtest.h>

#include <propertytree/Client.hpp>
#include <propertytree/Property.hpp>

#include<Utils.hpp>

using namespace testing;
using namespace propertytree;

struct ConcurrentOperationTest : Test
{
    void SetUp()
    {

    }

    void TearDown()
    {

    }

    ClientConfig config = {"127.0.0.1", 12345};
};

TEST_F(ConcurrentOperationTest, shouldCleanTree)
{
    Client sut = Client(config);
    clean(sut);
}


TEST_F(ConcurrentOperationTest, shouldConcurrentAddNodeOnSameParent)
{
    struct Runner
    {
        Runner(Property pParent, std::string pName)
            : parent(std::move(pParent))
            , name(pName)
        {}

        void operator()()
        {
            for (int i = 0u; i<500; i++)
            {
                auto child = parent.create(name);
                ASSERT_TRUE(child);
                child.destroy();
            }
        }
        Property parent;
        std::string name;
    };

    std::vector<std::thread> runners;
    std::map<uint32_t, Client> clients;
    for (auto i=0u; i<8; i++)
    {
        auto res = clients.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(config));
        auto& sut = res.first->second;
        std::string name = "runner" + std::to_string(i);
        runners.emplace_back([&sut, name](){Runner(sut.root(), name)();});
    }

    for (auto& i : runners)
    {
        i.join();
    }
}

