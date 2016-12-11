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

struct HandlerMock : public IValueWatcher
{
    MOCK_METHOD1(handle, void(ValueContainerPtr));
};

struct ClientTests : public common::MessageCreationHelper, public ::testing::Test
{
    ClientTests() :
        endpoint(std::make_shared<common::EndPointMock>()),
        handlerMock(std::make_shared<HandlerMock>()),
        log("TEST")
    {}

    std::shared_ptr<common::EndPointMock> endpoint;
    std::shared_ptr<HandlerMock> handlerMock;
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
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42u);
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

    EXPECT_EQ(value->get<uint32_t>(), 42u);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldSubscribeUpdateNotification)
{
    std::string path = "/Value";
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42u);

    MessageMatcher getSpecificMetaRequestMessageMatcher(createGetSpecificMetaRequestMessage(0, path));
    MessageMatcher getValueRequestMessageMatcher(createGetValueRequestMessage(1, protocol::Uuid(100)));
    MessageMatcher subscribeUpdateNotificationRequestMessageMatcher(createSubscribePropertyUpdateRequestMessage(2, protocol::Uuid(100)));

    std::function<void()> getSpecificMetaRequestAction = [this, &expectedVal, &path]()
    {
        this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Value, path));
    };

    std::function<void()> getValueRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(1, expectedVal));
    };

    std::function<void()> subscribeUpdateNotificatioRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::SubscribePropertyUpdateResponse,
            protocol::MessageType::SubscribePropertyUpdateResponse,
            protocol::SubscribePropertyUpdateResponse::Response>
            (2, protocol::SubscribePropertyUpdateResponse::Response::OK));
    };

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(2, 1, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));

    ptc->enableAutoUpdate(std::string("/Value"));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}


TEST_F(ClientTests, shouldReceiveUpdateNotification)
{
    std::string path = "/Value";
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42u);
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);

    MessageMatcher getSpecificMetaRequestMessageMatcher(createGetSpecificMetaRequestMessage(0, path));
    MessageMatcher getValueRequestMessageMatcher(createGetValueRequestMessage(1, protocol::Uuid(100)));
    MessageMatcher subscribeUpdateNotificationRequestMessageMatcher(createSubscribePropertyUpdateRequestMessage(2, protocol::Uuid(100)));

    std::function<void()> getSpecificMetaRequestAction = [this, &expectedVal, &path]()
    {
        this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Value, path));
    };

    std::function<void()> getValueRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(1, expectedVal));
    };

    std::function<void()> subscribeUpdateNotificatioRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::SubscribePropertyUpdateResponse,
            protocol::MessageType::SubscribePropertyUpdateResponse,
            protocol::SubscribePropertyUpdateResponse::Response>
            (2, protocol::SubscribePropertyUpdateResponse::Response::OK));
    };

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(2, 1, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));
    EXPECT_EQ(value->get<uint32_t>(), 42u);
    ptc->enableAutoUpdate(std::string("/Value"));

    std::list<protocol::PropertyUpdateNotificationEntry> updates;
    updates.emplace_back(100, newValue);
    auto updateNotifMsg = createPropertyUpdateNotificationMessage(3, updates);
    this->endpoint->queueToReceive(updateNotifMsg);
    std::this_thread::sleep_for(10ms);
    auto value2 = ptc->getValue(std::string("/Value"));
    EXPECT_EQ(value->get<uint32_t>(), 69u);

    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

MATCHER_P(ValueContainerHas, value, "")
{
    return arg->getCopy() == value;
}

TEST_F(ClientTests, shouldReceiveUpdateNotificationAndRunHandler)
{
    std::string path = "/Value";
    auto expectedVal = utils::buildBufferedValue<uint32_t>(42u);
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);

    MessageMatcher getSpecificMetaRequestMessageMatcher(createGetSpecificMetaRequestMessage(0, path));
    MessageMatcher getValueRequestMessageMatcher(createGetValueRequestMessage(1, protocol::Uuid(100)));
    MessageMatcher subscribeUpdateNotificationRequestMessageMatcher(createSubscribePropertyUpdateRequestMessage(2, protocol::Uuid(100)));

    std::function<void()> getSpecificMetaRequestAction = [this, &expectedVal, &path]()
    {
        this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Value, path));
    };

    std::function<void()> getValueRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(1, expectedVal));
    };

    std::function<void()> subscribeUpdateNotificatioRequestAction = [this, &expectedVal]()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::SubscribePropertyUpdateResponse,
            protocol::MessageType::SubscribePropertyUpdateResponse,
            protocol::SubscribePropertyUpdateResponse::Response>
            (2, protocol::SubscribePropertyUpdateResponse::Response::OK));
    };

    EXPECT_CALL(*handlerMock, handle(ValueContainerHas(newValue)));

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(2, 1, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));
    value->addWatcher(handlerMock);
    EXPECT_EQ(value->get<uint32_t>(), 42u);
    ptc->enableAutoUpdate(std::string("/Value"));

    std::list<protocol::PropertyUpdateNotificationEntry> updates;
    updates.emplace_back(100, newValue);
    auto updateNotifMsg = createPropertyUpdateNotificationMessage(3, updates);
    this->endpoint->queueToReceive(updateNotifMsg);
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}


} // namespace client
} // namespace ptree
