#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <server/src/Messaging/MessageEssential.hpp>


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


struct MessagingTests : public ::testing::Test
{
    
};

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

} // namespace server
} // namespace ptree