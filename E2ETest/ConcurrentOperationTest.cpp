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

constexpr size_t COUNT = 50;

struct CreateDelete
{
    CreateDelete(Property pParent, std::string pName)
        : parent(std::move(pParent))
        , name(pName)
    {}

    void operator()()
    {
        for (auto i = 0u; i<COUNT; i++)
        {
            auto createTp0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto child = parent.create(name);
            auto createTp1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto diff = createTp1 - createTp0;
            createSigma += diff;

            ASSERT_TRUE(child);

            auto deleteTp0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            child.destroy();
            auto deleteTp1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            diff = deleteTp1 - deleteTp0;
            deleteSigma += diff;
        }

        createSigma /= COUNT;
        deleteSigma /= COUNT;
        printf("CreateDelete completed: %s average_create_latency:%lf average_delete_latency: %lf\n", name.c_str(), createSigma, deleteSigma);
    }
    Property parent;
    std::string name;
    double createSigma = 0;
    double deleteSigma = 0;
};

struct SetGetBasic
{
    SetGetBasic(Property pParent, std::string pName)
        : parent(std::move(pParent))
        , name(pName)
    {}

    void operator()()
    {
        auto value = parent.create("value");
        ASSERT_TRUE(value);

        constexpr size_t COUNT = 1000;
        for (uint32_t i = 0; i<COUNT; i++)
        {
            auto setValTp0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            value = i;
            auto setValTp1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            auto diff = setValTp1 - setValTp0;
            setValSigma += diff;

            auto getValTp0 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            value.fetch();
            EXPECT_EQ(i, value.value<int>());
            auto getValTp1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            diff = getValTp1 - getValTp0;
            getValSigma += diff;
        }

        setValSigma /= COUNT;
        getValSigma /= COUNT;
        printf("SetGetBasic completed: %s average_set_latency:%lf average_get_latency: %lf\n", name.c_str(), setValSigma, getValSigma);
        value.destroy();
    }
    Property parent;
    std::string name;
    double setValSigma = 0;
    double getValSigma = 0;
};


TEST_F(ConcurrentOperationTest, shouldConcurrentAddNodeOnSameParent)
{
    std::vector<std::thread> runners;
    std::map<uint32_t, Client> clients;
    for (auto i=0u; i<8; i++)
    {
        auto res = clients.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(config));
        auto& sut = res.first->second;
        std::string name = "runner" + std::to_string(i);
        runners.emplace_back([&sut, name](){CreateDelete(sut.root(), name)();});
    }

    for (auto& i : runners)
    {
        i.join();
    }
}


TEST_F(ConcurrentOperationTest, shouldConcurrentAddNodeOnDifferentParent)
{
    std::vector<std::thread> runners;
    std::map<uint32_t, Client> clients;
    for (auto i=0u; i<8; i++)
    {
        auto res = clients.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(config));
        auto& sut = res.first->second;
        std::string name = "CreateDeleteThreaded" + std::to_string(i);
        runners.emplace_back([&sut, name](){CreateDelete(sut.root().create(name), name)();});
    }

    for (auto& i : runners)
    {
        i.join();
    }
}

TEST_F(ConcurrentOperationTest, shouldSingleAddDeleteNode)
{
    Client sut = Client(config);
    for (auto i=0u; i<8u; i++)
    {
        CreateDelete(sut.root(), "CreateDeleteSingle"+std::to_string(i))();
    }
}

TEST_F(ConcurrentOperationTest, shouldSingleSetGet)
{
    Client sut = Client(config);
    for (auto i=0u; i<8u; i++)
    {
        SetGetBasic(sut.root(), "SetGetBasicSingle"+std::to_string(i))();
    }
}

TEST_F(ConcurrentOperationTest, shouldSingleSetGetThreaded)
{
    std::vector<std::thread> runners;
    std::map<uint32_t, Client> clients;
    for (auto i=0u; i<32; i++)
    {
        auto res = clients.emplace(std::piecewise_construct, std::forward_as_tuple(i), std::forward_as_tuple(config));
        auto& sut = res.first->second;
        std::string name = "SetGetBasicThreaded" + std::to_string(i);
        runners.emplace_back([&sut, name](){SetGetBasic(sut.root().create(name), name)();});
    }

    for (auto& i : runners)
    {
        i.join();
    }
}


TEST_F(ConcurrentOperationTest, shouldCleanTree3)
{
    Client sut = Client(config);
    clean(sut);
}