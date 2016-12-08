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
        this->endpoint->queueToReceive(createSigninResponseMessage(0, 1));
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

TEST_F(ClientTests, shouldFetchValueWithGetSpecificMetaWhenNotAutoUpdate)
{
    std::string path = "/Value";
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42);
    MessageMatcher getSpecificMetaRequestMessageMatcher(createGetSpecificMetaRequestMessage(0, path));
    MessageMatcher getValueRequestMessageMatcher(createGetValueRequestMessage(1, protocol::Uuid(100)));

    std::function<void()> getSpecificMetaRequestAction = [this, &expectedVal, &path]()
    {
        this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Value, path));
    };

    std::function<void()> getValueRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(1, expectedVal));
    };

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue("/Value");

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

} // namespace client
} // namespace ptree
