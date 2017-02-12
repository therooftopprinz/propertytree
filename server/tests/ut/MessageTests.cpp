#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ctime>
#include <thread>
#include <interface/protocol.hpp>
#include <common/src/Utils.hpp>
#include <common/src/Logger.hpp>

using namespace testing;

namespace ptree
{
namespace protocol
{

struct MessagingTests : public ::testing::Test
{
    MessagingTests():
        log("TEST")
    {}

    logger::Logger log;
};

struct PhoneEntry1
{
    std::string name;
    uint32_t number;
    std::string email;
    MESSAGE_FIELDS(name, number, email);
};

struct PhoneBook1
{
    BlockArray<PhoneEntry1> entries;
    MESSAGE_FIELDS(BLOCK entries);
};

struct PhoneEntry2
{
    String name;
    Simple<uint32_t> number;
    String email;

    MESSAGE_FIELDS_2(name, number, email);
};

struct PhoneBook2
{
    ComplexArray<PhoneEntry2> entries;
    MESSAGE_FIELDS_2(entries);
};


std::chrono::microseconds duration(std::chrono::time_point< std::chrono::high_resolution_clock> start)
{
    auto s = std::chrono::duration_cast<std::chrono::microseconds>(start.time_since_epoch());
    auto c = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
    return c-s;
}

TEST_F(MessagingTests, DISABLED_propertyUpdateNotificationPackingLeakTest)
{
    protocol::PropertyUpdateNotification propertyUpdateNotifs;
    std::vector<uint8_t> val = {0,0,0,69};
    propertyUpdateNotifs.propertyUpdateNotifications.get().push_back(protocol::PropertyUpdateNotificationEntry(100, val));
    propertyUpdateNotifs.propertyUpdateNotifications.get().push_back(protocol::PropertyUpdateNotificationEntry(102, val));
    propertyUpdateNotifs.propertyUpdateNotifications.get().push_back(protocol::PropertyUpdateNotificationEntry(103, val));
    propertyUpdateNotifs.propertyUpdateNotifications.get().push_back(protocol::PropertyUpdateNotificationEntry(104, val));

    Buffer enbuff = propertyUpdateNotifs.getPacked();

    utils::printRaw(enbuff.data(),enbuff.size());

    logger::loggerServer.waitEmpty();
}

TEST_F(MessagingTests, arrayConsistencyTest)
{

    const char entry0Name[] = "juan dela cruz";
    const char entry0Email[] = "h2c@gmail.com";
    const char entry1Name[] = "juan dela cruz";
    const char entry1Email[] = "h2c@gmail.com";
    Buffer packed1;
    Buffer packed2;
    {
        PhoneBook1 ddd;
        PhoneEntry1 entry;
        entry = {entry0Name, 424242, entry0Email};
        ddd.entries.get().push_back(entry);

        entry = {entry1Name, 424242, entry1Email};
        ddd.entries.get().push_back(entry);

        packed1 = ddd.getPacked();
    }

    {
        PhoneBook2 ddd;

        {
            ddd.entries.get().emplace_back();
            auto& e = ddd.entries.get().back();
            e.name.setSize(sizeof(entry0Name));
            e.email.setSize(sizeof(entry0Email));
        }
        {
            ddd.entries.get().emplace_back();
            auto& e = ddd.entries.get().back();
            e.name.setSize(sizeof(entry0Name));
            e.email.setSize(sizeof(entry0Email));
        }

        ddd.allocate();

        {
            auto& e = ddd.entries.get()[0];
            std::strcpy(e.name.get(), entry0Name);
            e.number.get() = 424242;
            std::strcpy(e.email.get(), entry0Email);
        }
        {
            auto& e = ddd.entries.get()[1];
            std::strcpy(e.name.get(), entry1Name);
            e.number.get() = 424242;
            std::strcpy(e.email.get(), entry1Email);
        }

        packed2.assign(ddd.data.data(), ddd.data.data()+ddd.data.size());
    }

    utils::printRaw(packed2.data(), packed2.size());
    EXPECT_EQ(packed1, packed2);
    logger::loggerServer.waitEmpty();
}


TEST_F(MessagingTests, arraySpeedTest)
{
    std::chrono::time_point< std::chrono::high_resolution_clock>  s;
    auto loopCount = 99999ul;
    // auto loopCountAcc = loopCount*5000;
    std::chrono::microseconds e;

    const char entry0Name[] = "juan dela cruz";
    const char entry0Email[] = "h2c@gmail.com";
    const char entry1Name[] = "juan dela cruz";
    const char entry1Email[] = "h2c@gmail.com";

    {
        log << logger::DEBUG << "testing normal encode...";
        s = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < loopCount; i++)
        {
            PhoneBook1 ddd;
            PhoneEntry1 entry;
            entry = {entry0Name, 424242, entry0Email};
            ddd.entries.get().push_back(entry);
            entry = {entry1Name, 424242, entry1Email};
            ddd.entries.get().push_back(entry);
            ddd.getPacked();
        }
        e = duration(s);
        log << logger::DEBUG << "time: " << e.count()*0.000001;
    }

    {
        log << logger::DEBUG << "testing pointer encode...";
        s = std::chrono::high_resolution_clock::now();
        for (uint64_t i = 0; i < loopCount; i++)
        {
            PhoneBook2 ddd;

            {
                ddd.entries.get().emplace_back();
                auto& e = ddd.entries.get().back();
                e.name.setSize(sizeof(entry0Name));
                e.email.setSize(sizeof(entry0Email));
            }
            {
                ddd.entries.get().emplace_back();
                auto& e = ddd.entries.get().back();
                e.name.setSize(sizeof(entry0Name));
                e.email.setSize(sizeof(entry0Email));
            }

            ddd.allocate();

            {
                auto& e = ddd.entries.get()[0];
                std::strcpy(e.name.get(), entry0Name);
                e.number.get() = 424242;
                std::strcpy(e.email.get(), entry0Email);
            }
            {
                auto& e = ddd.entries.get()[1];
                std::strcpy(e.name.get(), entry1Name);
                e.number.get() = 424242;
                std::strcpy(e.email.get(), entry1Email);
            }
        }
        e = duration(s);
        log << logger::DEBUG << "time: " << e.count()*0.000001;
    }

    logger::loggerServer.waitEmpty();
}


} // namespace server
} // namespace ptree