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
    MOCK_METHOD3(handleCreation,void(protocol::Uuid, std::string, protocol::PropertyType));
    MOCK_METHOD1(handleDeletion,void(protocol::Uuid));
};


struct RpcHandler
{
    virtual Buffer handle(Buffer&) = 0;
};

struct RpcHandlerMock : public RpcHandler
{
    MOCK_METHOD1(handle, Buffer(Buffer&));
};


struct ClientTests : public common::MessageCreationHelper, public ::testing::Test
{
    ClientTests() :
        endpoint(std::make_shared<common::EndPointMock>()),
        handlerMock(std::make_shared<HandlerMock>()),
        metaHandlerMock(std::make_shared<MetaUpdateHandlerMock>()),
        signinRequestMessageMatcher(createSigninRequestMessage(0, 1, 200*1000)),
        log("TEST")
    {}

    MessageMatcher getSpecificMetaRequestMessageMatcher = createGetSpecificMetaRequestMessage(1, "/Value");
    MessageMatcher getValueRequestMessageMatcher = createGetValueRequestMessage(2, protocol::Uuid(100));
    MessageMatcher subscribeUpdateNotificationRequestMessageMatcher = createSubscribePropertyUpdateRequestMessage(3, protocol::Uuid(100));

    Buffer expectedVal = utils::buildBufferedValue<uint32_t>(42);

    void getSpecificMetaRequestActionFn()
    {
        endpoint->queueToReceive(createGetSpecificMetaResponseMessage(1, 100, protocol::PropertyType::Value, "/Value"));
    };

    void getValueRequestActionFn()
    {
        endpoint->queueToReceive(createGetValueResponseMessage(2, expectedVal));
    };

    void subscribeUpdateNotificatioRequestActionFn()
    {
        endpoint->queueToReceive(createCommonResponse<
            protocol::SubscribePropertyUpdateResponse,
            protocol::MessageType::SubscribePropertyUpdateResponse,
            protocol::SubscribePropertyUpdateResponse::Response>
            (3, protocol::SubscribePropertyUpdateResponse::Response::OK));
    };

    void signinRequestActionFn()
    {
        endpoint->queueToReceive(createSigninResponseMessage(0, 1));
    };

    std::function<void()> signinRequestAction = std::bind(&ClientTests::signinRequestActionFn, this);
    std::function<void()> getSpecificMetaRequestAction = std::bind(&ClientTests::getSpecificMetaRequestActionFn, this);
    std::function<void()> getValueRequestAction = std::bind(&ClientTests::getValueRequestActionFn, this);
    std::function<void()> subscribeUpdateNotificatioRequestAction = std::bind(&ClientTests::subscribeUpdateNotificatioRequestActionFn, this);
    std::shared_ptr<common::EndPointMock> endpoint;
    std::shared_ptr<HandlerMock> handlerMock;
    std::shared_ptr<MetaUpdateHandlerMock> metaHandlerMock;
    MessageMatcher signinRequestMessageMatcher;
    logger::Logger log;
};


TEST_F(ClientTests, shouldSendSignInRequestOnCreation)
{
    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
    }
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldCreateNode)
{
    auto expectedVal = Buffer();
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(1, expectedVal, protocol::PropertyType::Node,
        "/Test"));

    std::function<void()> createValueRequestAction = [this]()
    {
        endpoint->queueToReceive(createCreateResponseMessage(1, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 0, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        EXPECT_TRUE(ptree->createNode("/Test"));
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldCreateValue)
{
    MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(1, expectedVal, protocol::PropertyType::Value,
        "/Value"));

    std::function<void()> createValueRequestAction = [this]()
    {
        endpoint->queueToReceive(createCreateResponseMessage(1, protocol::CreateResponse::Response::OK,
            protocol::Uuid(100)));
    };

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 1, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        auto value = ptree->createValue("/Value", expectedVal);
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldFetchValueWithGetSpecificMetaWhenNotAutoUpdate)
{
    MessageMatcher getValueRequestMessageMatcher2 = createGetValueRequestMessage(3, protocol::Uuid(100));
    auto getValueRequestAction2 = [this]()
    {
        endpoint->queueToReceive(createGetValueResponseMessage(3, expectedVal));
    };

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 1, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(3, 2, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(4, 2, false, 1, getValueRequestMessageMatcher2.get(), getValueRequestAction2);

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        std::string valuePath = "/Value";
        auto value = ptree->getValue(valuePath);
        ASSERT_TRUE(value);
        auto value2 = ptree->getValue(valuePath);
        EXPECT_EQ(value, value2);
        EXPECT_EQ(value->get<uint32_t>(), 42u);
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldSubscribeUpdateNotification)
{
    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(3, 1, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(4, 2, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        std::string valuePath = "/Value";
        auto value = ptree->getValue(valuePath);
        EXPECT_TRUE(ptree->enableAutoUpdate(value));
        auto value2 = ptree->getValue(valuePath);
        EXPECT_EQ(value, value2);
        EXPECT_EQ(value->get<uint32_t>(), 42u);
    }

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}


TEST_F(ClientTests, shouldReceiveUpdateNotification)
{
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 1, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(3, 2, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(4, 3, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;
    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        std::string valuePath = "/Value";
        auto value = ptree->getValue(valuePath);
        EXPECT_EQ(value->get<uint32_t>(), 42u);
        EXPECT_TRUE(ptree->enableAutoUpdate(value));

        std::list<protocol::PropertyUpdateNotificationEntry> updates;
        updates.emplace_back(100, newValue);
        auto updateNotifMsg = createPropertyUpdateNotificationMessage(3, updates);
        this->endpoint->queueToReceive(updateNotifMsg);
        std::this_thread::sleep_for(10ms);
        EXPECT_EQ(value->get<uint32_t>(), 69u);
    }

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

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 1, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(3, 2, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(4, 3, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);

    using namespace std::chrono_literals;
    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        std::string valuePath = "/Value";
        auto value = ptree->getValue(valuePath);
        value->addWatcher(handlerMock);
        EXPECT_EQ(value->get<uint32_t>(), 42u);
        EXPECT_TRUE(ptree->enableAutoUpdate(value));

        std::list<protocol::PropertyUpdateNotificationEntry> updates;
        updates.emplace_back(100, newValue);
        auto updateNotifMsg = createPropertyUpdateNotificationMessage(3, updates);
        this->endpoint->queueToReceive(updateNotifMsg);
        std::this_thread::sleep_for(10ms);
    }

    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldUnsubscribe)
{
    auto newValue = utils::buildBufferedValue<uint32_t>(69u);
    MessageMatcher unsubscribeUpdateNotificationRequestMessageMatcher = createUnsubscribePropertyUpdateRequestMessage(4, protocol::Uuid(100));
    MessageMatcher getValueRequestMessageMatcher2 = createGetValueRequestMessage(5, protocol::Uuid(100));

    std::function<void()> unsubscribeUpdateNotificatioRequestAction = [this]()
    {
        this->endpoint->queueToReceive(createCommonResponse<
            protocol::UnsubscribePropertyUpdateResponse,
            protocol::MessageType::UnsubscribePropertyUpdateResponse,
            protocol::UnsubscribePropertyUpdateResponse::Response>
            (4, protocol::UnsubscribePropertyUpdateResponse::Response::OK));
    };

    std::function<void()> getValueRequestAction2 = [this, &newValue]()
    {
        this->endpoint->queueToReceive(createGetValueResponseMessage(5, newValue));
    };

    EXPECT_CALL(*handlerMock, handle(ValueContainerHas(newValue))).Times(0);

    endpoint->expectSend(1, 0, false, 1, signinRequestMessageMatcher.get(), signinRequestAction);
    endpoint->expectSend(2, 1, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
    endpoint->expectSend(3, 2, false, 1, getValueRequestMessageMatcher.get(), getValueRequestAction);
    endpoint->expectSend(4, 3, false, 1, subscribeUpdateNotificationRequestMessageMatcher.get(), subscribeUpdateNotificatioRequestAction);
    endpoint->expectSend(5, 4, false, 1, unsubscribeUpdateNotificationRequestMessageMatcher.get(), unsubscribeUpdateNotificatioRequestAction);
    endpoint->expectSend(6, 5, false, 1, getValueRequestMessageMatcher2.get(), getValueRequestAction2);

    using namespace std::chrono_literals;
    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        std::string valuePath = "/Value";
        auto value = ptree->getValue(valuePath);
        value->addWatcher(handlerMock);

        ptree->enableAutoUpdate(value);
        ptree->disableAutoUpdate(value);

        auto value2 = ptree->getValue(valuePath);
        EXPECT_EQ(value->get<uint32_t>(), 69u);
        EXPECT_EQ(value2->get<uint32_t>(), 69u);
    }

    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

TEST_F(ClientTests, shouldReceiveMetaUpdateNotificationAndRunHandler)
{
    using namespace std::chrono_literals;

    EXPECT_CALL(*metaHandlerMock, handleCreation(_, "/Test", protocol::PropertyType::Node));
    EXPECT_CALL(*metaHandlerMock, handleCreation(_, "/Test/Value", protocol::PropertyType::Value));
    EXPECT_CALL(*metaHandlerMock, handleDeletion(protocol::Uuid(100)));

    {
        auto ptc = std::make_shared<PTreeClient>(endpoint);
        auto ptree = ptc->getPTree();
        ptree->addMetaWatcher(metaHandlerMock);
        std::list<protocol::MetaCreate> createUpdates;
        std::list<protocol::MetaDelete> deleteUpdates;
        createUpdates.emplace_back(101, protocol::PropertyType::Node,"/Test");
        createUpdates.emplace_back(102, protocol::PropertyType::Value,"/Test/Value");
        deleteUpdates.emplace_back(100);

        auto updateNotifMsg = createMetaUpdateNotificationMessage(3, createUpdates, deleteUpdates);
        this->endpoint->queueToReceive(updateNotifMsg);
    }
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(2500.0);
    logger::loggerServer.waitEmpty();
}

// TEST_F(ClientTests, shouldSendSetValueIndication)
// {
//     uint32_t newVal = 68;
//     auto newValBuff = utils::buildBufferedValue<uint32_t>(68u);
//     MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, expectedVal, protocol::PropertyType::Value,
//         "/Value"));
//     MessageMatcher setValueIndicationMessageMatcher(
//         createSetValueIndicationMessage(1, 100, newValBuff));

//     std::function<void()> createValueRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
//             protocol::Uuid(100)));
//     };

//     endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createValueRequestAction);
//     endpoint->expectSend(1, 0, false, 1, setValueIndicationMessageMatcher.get(), DefaultAction::get());

//     ptc = std::make_shared<PTreeClient>(endpoint);

//     auto value = ptc->createValue("/Value", expectedVal);

//     ptc->setValue(value, newVal);

//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1ms);
//     endpoint->waitForAllSending(2500.0);
//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientTests, shouldCreateRpc)
// {
//     MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, Buffer(), protocol::PropertyType::Rpc,
//         "/Rpc"));

//     std::function<void()> createRpcRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
//             protocol::Uuid(100)));
//     };

//     endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createRpcRequestAction);

//     ptc = std::make_shared<PTreeClient>(endpoint);

//     std::function<Buffer(Buffer&)> handler;
//     std::function<void(Buffer&)> voidHandler;
  
//     auto rpc = ptc->createRpc("/Rpc", handler, voidHandler);

//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1ms);
//     endpoint->waitForAllSending(2500.0);
//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientTests, shouldCallRpcHandler)
// {
//     Buffer param = {69,0,0,0};
//     Buffer retVal = {0};

//     MessageMatcher createRequestMessageMatcher(createCreateRequestMessage(0, Buffer(), protocol::PropertyType::Rpc, "/Rpc"));
//     MessageMatcher handleRpcRequestMessage(createHandleRpcResponseMessage(static_cast<uint32_t>(-1), 1234, 8, retVal));

//     std::function<void()> createRpcRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createCreateResponseMessage(0, protocol::CreateResponse::Response::OK,
//             protocol::Uuid(100)));
//     };

//     RpcHandlerMock handlerMock;
//     using std::placeholders::_1;
//     std::function<Buffer(Buffer&)> handler = std::bind(&RpcHandlerMock::handle, &handlerMock, _1);
//     std::function<void(Buffer&)> voidHandler;

//     EXPECT_CALL(handlerMock, handle(param)).WillOnce(Return(retVal));
//     endpoint->expectSend(0, 0, false, 1, createRequestMessageMatcher.get(), createRpcRequestAction);
//     endpoint->expectSend(1, 0, false, 1, handleRpcRequestMessage.get(), DefaultAction::get());

//     ptc = std::make_shared<PTreeClient>(endpoint);
  
//     auto rpc = ptc->createRpc("/Rpc", handler, voidHandler);
//     endpoint->queueToReceive(createHandleRpcRequestMessage(static_cast<uint32_t>(-1), 1234, 8, protocol::Uuid(100), param));

//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(100ms);
//     endpoint->waitForAllSending(10000.0);
//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientTests, shouldFetchRpcWithGetSpecificMetaWhenNotAutoUpdate)
// {
//     std::string rpcPath = "/Rpc";

//     MessageMatcher getSpecificMetaRequestMessageMatcher = createGetSpecificMetaRequestMessage(0, rpcPath);

//     std::function<void()> getSpecificMetaRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Rpc, "/Rpc"));
//     };

//     endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);

//     ptc = std::make_shared<PTreeClient>(endpoint);

//     auto rpc = ptc->getRpc(rpcPath);

//     EXPECT_TRUE(rpc);

//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(2500.0);
//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientTests, shouldRequestRpc)
// {
//     std::string rpcPath = "/Rpc";
//     Buffer param = {69};
//     MessageMatcher getSpecificMetaRequestMessageMatcher = createGetSpecificMetaRequestMessage(0, rpcPath);
//     MessageMatcher rpcRequestMessageMatcher = createRpcRequestMessage(1, 100, param);

//     std::function<void()> getSpecificMetaRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createGetSpecificMetaResponseMessage(0, 100, protocol::PropertyType::Rpc, "/Rpc"));
//     };

//     std::function<void()> rpcRequestAction = [this]()
//     {
//         this->endpoint->queueToReceive(createRpcResponseMessage(1, Buffer({0})));
//     };

//     endpoint->expectSend(1, 0, false, 1, getSpecificMetaRequestMessageMatcher.get(), getSpecificMetaRequestAction);
//     endpoint->expectSend(1, 0, false, 1, rpcRequestMessageMatcher.get(), rpcRequestAction);

//     ptc = std::make_shared<PTreeClient>(endpoint);

//     auto rpc = ptc->getRpc(rpcPath);

//     ASSERT_TRUE(rpc);
//     auto rval = ptc->rpcRequest(rpc, param);
//     EXPECT_TRUE(rval.size()>0);

//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(2500.0);
//     logger::loggerServer.waitEmpty();
// }

} // namespace client
} // namespace ptree
