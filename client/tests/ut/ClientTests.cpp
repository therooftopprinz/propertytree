#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <client/src/PTreeClient.hpp>
#include <common/src/Utils.hpp>
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
        log("TEST")
    {}

    std::shared_ptr<common::EndPointMock> endpoint;
    std::shared_ptr<PTreeClient> ptc;
    logger::Logger log;
};


TEST_F(ClientTests, shouldSendSignInRequestOnCreation)
{
    MessageMatcher signinRequestMessageMatcher(createSigninRequestMessage(0, 1, 300));

    std::function<void()> signinRequestAction = [this]()
    {
        std::list<std::tuple<std::string, protocol::Uuid, protocol::PropertyType>> metaList;
        metaList.emplace_back(std::make_tuple("/Test", protocol::Uuid(100), protocol::PropertyType::Node));
        metaList.emplace_back(std::make_tuple("/Test/Value", protocol::Uuid(101), protocol::PropertyType::Value));
        this->endpoint->queueToReceive(createSigninResponseMessage(0, 1, metaList));
    };

    endpoint->expectSend(0, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);
    ptc->signIn();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldCreateNode)
{
    auto expectedVal = Buffer();
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, expectedVal, protocol::PropertyType::Node,
        "/Test"));

    std::function<void()> createValueRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    EXPECT_TRUE(ptc->createNode("/Test"));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldCreateValue)
{
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42);
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, expectedVal, protocol::PropertyType::Value,
        "/Value"));

    std::function<void()> createValueRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->createValue("/Value", expectedVal);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}



} // namespace client
} // namespace ptree
