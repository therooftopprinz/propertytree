#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <client/src/PTreeClient.hpp>
#include <common/TestingFramework/EndPointMock.hpp>
#include <common/TestingFramework/MessageMatcher.hpp>
#include <common/TestingFramework/MessageCreationHelper.hpp>

using namespace testing;


namespace ptree
{
namespace client
{

typedef common::MatcherFunctor MatcherFunctor;
typedef common::ActionFunctor ActionFunctor;
typedef common::DefaultAction DefaultAction;
typedef common::MessageMatcher MessageMatcher;

struct ClientTests : public common::MessageCreationHelper, public ::testing::Test
{
    ClientTests() :
        endpoint(std::make_shared<common::EndPointMock>()),
        ptc(endpoint)
    {}

    std::shared_ptr<common::EndPointMock> endpoint;
    PTreeClient ptc;
};


TEST_F(ClientTests, shouldSendSignInRequestOnCreation)
{
    MessageMatcher signinRequestMessageMatcher(createSigninRequestMessage(0, 1, 300));

    endpoint->expectSend(0, 0, false, 1, signinRequestMessageMatcher.get(), DefaultAction::get());
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

} // namespace client
} // namespace ptree