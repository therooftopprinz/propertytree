#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ctime>
#include <thread>
#include <server/src/Messaging/MessageEssential.hpp>
#include <server/src/Utils.hpp>
#include <server/src/Logger.hpp>

using namespace testing;

namespace ptree
{
namespace server
{

struct SimpleBlock
{
    Simple<uint32_t> a;
    Simple<uint8_t> b;
    Simple<uint16_t> c;
    MESSAGE_FIELDS(a,b,c);
};

struct SimpleBlockWithString
{
    Simple<uint32_t> a;
    String b;
    Simple<uint8_t> c;

    MESSAGE_FIELDS(a,b,c);
};

struct Nested
{
    SimpleBlock a;
    SimpleBlockWithString b;

    MESSAGE_FIELDS(a,b);
};

struct Person
{
    String name;
    Simple<uint8_t> sex;
    Simple<uint32_t> favenumber;

    MESSAGE_FIELDS(name,sex,favenumber);
};


struct PersonArray
{
    BlockArray<Person> persons;
    MESSAGE_FIELDS(persons);
};


struct MessagingTests : public ::testing::Test
{
    MessagingTests():
        log("TEST")
    {}

    logger::Logger log;
};

// TEST_F(MessagingTests, shouldEncodeSimpleType)
// {
//     Simple<int> val;
// }

TEST_F(MessagingTests, shouldEncodeSimpleType)
{
    SimpleBlock val;
    val.a = 1;
    val.b = 2;
    val.c = 3;
    Buffer comval{01,00,00,00,02,03,00};
    Encoder en;
    val >> en;
    EXPECT_EQ(comval, en.data());
}

TEST_F(MessagingTests, shouldEncodeSimpleTypeWithString)
{
    SimpleBlockWithString val;
    val.a = 42;
    val.b = std::string("wow!");
    val.c = 'D';
    Buffer comval{42,00,00,00,'w','o','w','!',0,'D'};
    Encoder en;
    val >> en;
    EXPECT_EQ(comval, en.data());
}

TEST_F(MessagingTests, shouldEncodeNested)
{
    SimpleBlock val1;
    val1.a = 1;
    val1.b = 2;
    val1.c = 3;
    SimpleBlockWithString val2;
    val2.a = 42;
    val2.b = std::string("wow!");
    val2.c = 'D';
    Nested nes;

    nes.a = val1;
    nes.b = val2;

    Buffer comval{01,00,00,00,02,03,00,42,00,00,00,'w','o','w','!',0,'D'};
    Encoder en;
    nes >> en;
    EXPECT_EQ(comval, en.data());
}

TEST_F(MessagingTests, shouldEncodeArray)
{
    PersonArray plist;
    Person a, b, c;
    a.name = std::string("Prokorpyo");
    a.sex = 'M';
    a.favenumber = 69;
    b.name = std::string("Dinisa");
    b.sex = 'S';
    b.favenumber = 89;
    c.name = std::string("Rodi");
    c.sex = 'M';
    c.favenumber = 42;
    plist.persons->push_back(a);
    plist.persons->push_back(b);
    plist.persons->push_back(c);

    Encoder en;
    plist >> en;

    utils::printRaw(en.data().data(),en.data().size());
    utils::printRawAscii(en.data().data(),en.data().size());

    PersonArray deplist;
    Decoder de(en.data().data(),en.data().data()+en.data().size());
    deplist << de;

    for(auto& i : *(deplist.persons))
    {
        log << logger::DEBUG << "name "<< (std::string)i.name;
        log << logger::DEBUG << "sex  "<< i.sex;
        log << logger::DEBUG << "fav  "<< i.favenumber;
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
}

TEST_F(MessagingTests, shouldDecodeArray)
{
    Buffer en{
        3,0,0,0,
        'O','A','O','B','O','O','O','O','D',0,'M',1,0,0,0,
        'O',0,'M',2,0,0,0,
        'O','D',0,'M',3,0,0,0
    };
    utils::printRaw(en.data(),en.size());
    utils::printRawAscii(en.data(),en.size());

    PersonArray deplist;
    Decoder de(en.data(),en.data()+en.size());
    deplist << de;

    for(auto& i : *(deplist.persons))
    {
        log << logger::DEBUG << "name "<< (std::string)i.name;
        log << logger::DEBUG << "sex  "<< i.sex;
        log << logger::DEBUG << "fav  "<< i.favenumber;
    }
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
}

} // namespace server
} // namespace ptree