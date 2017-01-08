#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <client/src/PTreeClient.hpp>
#include <common/src/Utils.hpp>
#include <common/TestingFramework/EndPointMock.hpp>
#include <common/TestingFramework/MessageMatcher.hpp>
#include <common/TestingFramework/MessageCreationHelper.hpp>
#include <client/src/ValueContainer.hpp>

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

struct MetaUpdateHandlerMock : public IMetaUpdateHandler
{
    MOCK_METHOD2(handleCreation,void(std::string, protocol::PropertyType));
    MOCK_METHOD1(handleDeletion,void(protocol::Uuid));
};


struct ClientTests : public common::MessageCreationHelper, public ::testing::Test
{
    ClientTests() :
        endpoint(std::make_shared<common::EndPointMock>()),
        handlerMock(std::make_shared<HandlerMock>()),
        metaHandlerMock(std::make_shared<MetaUpdateHandlerMock>()),
        log("TEST")
    {}

    MessageMatcher getSpecificMetaRequestMessageMatcher = createGetSpecificMetaRequestMessage(0, "/Value");
    MessageMatcher getValueRequestMessageMatcher = createGetValueRequestMessage(1, protocol::Uuid(100));
    MessageMatcher subscribeUpdateNotificationRequestMessageMatcher = createSubscribePropertyUpdateRequestMessage(2, protocol::Uuid(100));

    Buffer expectedVal = utils::buildBufferedValue<uint32_t>(42);

    void getSpecificMetaRequestActionFn()
    {
        this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Value, "/Value"));
    };

    void getValueRequestActionFn()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(1, expectedVal));
    };

    void subscribeUpdateNotificatioRequestActionFn()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::SubscribePropertyUpdateResponse,
            protocol::MessageType::SubscribePropertyUpdateResponse,
            protocol::SubscribePropertyUpdateResponse::Response>
            (2, protocol::SubscribePropertyUpdateResponse::Response::OK));
    };

    std::function<void()> getSpecificMetaRequestAction = std::bind(&ClientTests::getSpecificMetaRequestActionFn, this);
    std::function<void()> getValueRequestAction = std::bind(&ClientTests::getValueRequestActionFn, this);
    std::function<void()> subscribeUpdateNotificatioRequestAction = std::bind(&ClientTests::subscribeUpdateNotificatioRequestActionFn, this);

    std::shared_ptr<common::EndPointMock> endpoint;
    std::shared_ptr<HandlerMock> handlerMock;
    std::shared_ptr<MetaUpdateHandlerMock> metaHandlerMock;
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
    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(3, 2, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));

    ptc->enableAutoUpdate(value);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}


TEST_F(ClientTests, shouldReceiveUpdateNotification)
{
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(3, 2, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));
    EXPECT_EQ(value->get<uint32_t>(), 42u);
    ptc->enableAutoUpdate(value);

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
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);

    EXPECT_CALL(*handlerMock, handle(ValueContainerHas(newValue)));

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(3, 2, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));
    value->addWatcher(handlerMock);
    EXPECT_EQ(value->get<uint32_t>(), 42u);
    ptc->enableAutoUpdate(value);

    std::list<protocol::PropertyUpdateNotificationEntry> updates;
    updates.emplace_back(100, newValue);
    auto updateNotifMsg = createPropertyUpdateNotificationMessage(3, updates);
    this->endpoint->queueToReceive(updateNotifMsg);
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldUnsubscribe)
{
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);
    MessageMatcher unsubscribeUpdateNotificationRequestMessageMatcher = createUnsubscribePropertyUpdateRequestMessage(3, protocol::Uuid(100));
    MessageMatcher getValueRequestMessageMatcher2 = createGetValueRequestMessage(4, protocol::Uuid(100));

    std::function<void()> unsubscribeUpdateNotificatioRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::UnsubscribePropertyUpdateResponse,
            protocol::MessageType::UnsubscribePropertyUpdateResponse,
            protocol::UnsubscribePropertyUpdateResponse::Response>
            (3, protocol::UnsubscribePropertyUpdateResponse::Response::OK));
    };

    std::function<void()> getValueRequestAction2 = [this, &newValue]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(4, newValue));
    };

    EXPECT_CALL(*handlerMock, handle(ValueContainerHas(newValue))).Times(0);

    endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(2, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(3, 2, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);
    endpoint->expectSend(4, 3, false, 1, unsubscribeUpdateNotificationRequestMessageMatcher.get(), unsubscribeUpdateNotificatioRequestAction);
    endpoint->expectSend(5, 4, false, 1, getValueRequestMessageMatcher2.get(), getValueRequestAction2);

    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->getValue(std::string("/Value"));
    value->addWatcher(handlerMock);

    ptc->enableAutoUpdate(value);
    ptc->disableAutoUpdate(value);

    auto value2 = ptc->getValue(std::string("/Value"));
    EXPECT_EQ(value->get<uint32_t>(), 69u);
    EXPECT_EQ(value2->get<uint32_t>(), 69u);

    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldReceiveMetaUpdateNotificationAndRunHandler)
{
    using namespace std::chrono_literals;

    ptc = std::make_shared<PTreeClient>(endpoint);

    ptc->addMetaWatcher(metaHandlerMock);

    EXPECT_CALL(*metaHandlerMock, handleCreation("/Test", protocol::PropertyType::Node));
    EXPECT_CALL(*metaHandlerMock, handleCreation("/Test/Value", protocol::PropertyType::Value));
    EXPECT_CALL(*metaHandlerMock, handleDeletion(protocol::Uuid(100)));

    std::list<protocol::MetaCreate> createUpdates;
    std::list<protocol::MetaDelete> deleteUpdates;
    createUpdates.emplace_back(101, protocol::PropertyType::Node,"/Test");
    createUpdates.emplace_back(102, protocol::PropertyType::Value,"/Test/Value");
    deleteUpdates.emplace_back(100);

    auto updateNotifMsg = createMetaUpdateNotificationMessage(3, createUpdates, deleteUpdates);
    this->endpoint->queueToReceive(updateNotifMsg);
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldSendSetValueIndication)
{
    uint32_t newVal = 68;
    auto newValBuff = utils::buildBufferedValue<uint32_t>(68u);
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, expectedVal, protocol::PropertyType::Value,
        "/Value"));
    MessageMatcher setValueIndicationMessageMatcher(
        createSetValueIndicationMessage(1, 100, newValBuff));

    std::function<void()> createValueRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);
    endpoint->expectSend(1, 0, false, 1, setValueIndicationMessageMatcher.get(), DefaultAction::get());

    ptc = std::make_shared<PTreeClient>(endpoint);

    auto value = ptc->createValue("/Value", expectedVal);

    ptc->setValue(value, newVal);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldCreateRpc)
{
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, Buffer(), protocol::PropertyType::Rpc,
        "/Rpc"));

    std::function<void()> createRpcRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createRpcRequestAction);

    ptc = std::make_shared<PTreeClient>(endpoint);

    std::function<Buffer(Buffer&)> handler;
    std::function<void(Buffer&)> voidHandler;
  
    auto rpc = ptc->createRpc("/Rpc", handler, voidHandler);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

} // namespace client
} // namespace ptree
