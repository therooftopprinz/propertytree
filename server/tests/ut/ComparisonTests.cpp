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

struct MessagingCodecComparisonTests : public ::testing::Test
{
    MessagingCodecComparisonTests():
        log("TEST")
    {}

    logger::Logger log;
};

} // namespace server
} // namespace ptree
