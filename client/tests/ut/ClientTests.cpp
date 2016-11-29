#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <client/src/PTreeClient.hpp>
#include <common/TestingFramework/EndPointMock.hpp>
#include "MessageMatchers/MessageMatcher.hpp"

using namespace testing;

namespace ptree
{
namespace client
{

struct ClientTests : public ::testing::Test
{
    ClientTests() :
        endpoint(std::make_shared<common::EndPointMock>())
    {}

    std::shared_ptr<common::EndPointMock> endpoint;
};


TEST_F(ClientTests, shouldSendSignInRequestOnCreation)
{
    PTreeClient ptc;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    logger::loggerServer.waitEmpty();
}

}
}