#include <memory>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/src/Logger.hpp>
#include <server/src/PTree.hpp>
#include <server/src/PTreeServer.hpp>
#include <common/src/Logger.hpp>
#include <server/src/Serverlet/ClientServer.hpp>

#include <common/src/Utils.hpp>
#include "MessageMatchers/CreateObjectMetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/DeleteObjectMetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/MetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/PropertyUpdateNotificationMatcher.hpp"
#include <common/TestingFramework/MessageMatcher.hpp>
#include <common/TestingFramework/MessageCreationHelper.hpp>

using namespace testing;

namespace ptree
{
namespace server
{

typedef common::MatcherFunctor MatcherFunctor;
typedef common::ActionFunctor ActionFunctor;
typedef common::DefaultAction DefaultAction;
typedef common::MessageMatcher MessageMatcher;

struct ClientServerTests : public common::MessageCreationHelper, public ::testing::Test
{
    ClientServerTests() :

        testCreationAction(std::bind(&ClientServerTests::propTestCreationAction, this)),
        valueCreationDeleteImmediatelyAction(std::bind(&ClientServerTests::propValueCreationActionValueDelete, this)),
        valueCreationSubscribeAction(std::bind(&ClientServerTests::propValueCreationActionSubscribe, this)),
        endpoint(std::make_shared<common::EndPointMock>()),
        idgen(std::make_shared<core::IdGenerator>()),
        monitor(std::make_shared<PTreeServer>()),
        ptree(std::make_shared<core::PTree>(idgen)),
        log("TEST")
    {
        signinRspMsgMatcher = std::make_shared<MessageMatcher>(createSigninResponseMessage(signinRqstTid, 1));
        testCreationMatcher = std::make_shared<CreateObjectMetaUpdateNotificationMatcher>("/Test");
        valueCreationMatcher = std::make_shared<CreateObjectMetaUpdateNotificationMatcher>("/Test/Value");
        valueDeletionMatcher = std::make_shared<DeleteObjectMetaUpdateNotificationMatcher>();
        rpcCreationMatcher = std::make_shared<CreateObjectMetaUpdateNotificationMatcher>("/RpcTest");

        using protocol::CreateResponse;
        using protocol::DeleteResponse;
        using protocol::SubscribePropertyUpdateResponse;
        using protocol::UnsubscribePropertyUpdateResponse;
        using protocol::MessageType;

        createTestResponseFullMatcher = createCreateResponseMessage(createTestRequestTid,
            CreateResponse::Response::OK, protocol::Uuid(100));
        createValueResponseFullMatcher = createCreateResponseMessage(createValueRequestTid,
            CreateResponse::Response::OK, protocol::Uuid(101));
        createValueResponseAlreadyExistFullMatcher = createCreateResponseMessage(createValueRequest2Tid,
            CreateResponse::Response::ALREADY_EXIST, protocol::Uuid(0));
        createValueResponseInvalidPathFullMatcher = createCreateResponseMessage(createValueRequestTid,
            CreateResponse::Response::MALFORMED_PATH, protocol::Uuid(0));
        createValueResponseInvalidParentFullMatcher = createCreateResponseMessage(createValueRequestTid,
            CreateResponse::Response::PARENT_NOT_FOUND, protocol::Uuid(0));

        deleteValueResponseOkMatcher = createCommonResponse<DeleteResponse, MessageType::DeleteResponse>
                (deleteValueRequestTid, DeleteResponse::Response::OK);
        deleteValueResponseNotFoundMatcher = createCommonResponse<DeleteResponse, MessageType::DeleteResponse>
                (deleteValueRequestTid, DeleteResponse::Response::OBJECT_NOT_FOUND);
        deleteTestResponseNotEmptyMatcher = createCommonResponse<DeleteResponse, MessageType::DeleteResponse>
                (deleteTestRequestTid, DeleteResponse::Response::NOT_EMPTY);
        subscribeValueResponseOkMatcher =  createCommonResponse<SubscribePropertyUpdateResponse, MessageType::SubscribePropertyUpdateResponse>
                (subscribeValueRqstTid, SubscribePropertyUpdateResponse::Response::OK);
        subscribeValueResponseUuidNotFoundMatcher = createCommonResponse<SubscribePropertyUpdateResponse, MessageType::SubscribePropertyUpdateResponse>
                (subscribeValueRqstTid, SubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND);
        subscribeTestResponseNotAValueMatcher = createCommonResponse<SubscribePropertyUpdateResponse, MessageType::SubscribePropertyUpdateResponse>
                (subscribeTestRqstTid, SubscribePropertyUpdateResponse::Response::NOT_A_VALUE);
        subscribeTestResponseNotAValueMatcher = createCommonResponse<SubscribePropertyUpdateResponse, MessageType::SubscribePropertyUpdateResponse>
                (subscribeTestRqstTid, SubscribePropertyUpdateResponse::Response::NOT_A_VALUE);
        unsubscribeValueResponseOkMatcher = createCommonResponse<UnsubscribePropertyUpdateResponse, MessageType::UnsubscribePropertyUpdateResponse>
                (unsubscribeValueRqstTid, UnsubscribePropertyUpdateResponse::Response::OK);

        auto testVal = valueToBuffer<uint32_t>(42);
        createTestRequestMessage = createCreateRequestMessage(
            createTestRequestTid, Buffer(), protocol::PropertyType::Node, "/Test");
        createValueRequestMessage = createCreateRequestMessage(
            createValueRequestTid, testVal, protocol::PropertyType::Value, "/Test/Value");
        createValueRequestMessageForAlreadyExist = createCreateRequestMessage(
            createValueRequest2Tid, testVal, protocol::PropertyType::Value, "/Test/Value");
        createValueRequestMessageForAlreadyInvalidPath = createCreateRequestMessage(
            createValueRequestTid, testVal, protocol::PropertyType::Value, "/Test//Value");
        deleteRequestMessageForValueMessage = createDeleteRequestMessage(deleteValueRequestTid, 101);
        deleteRequestMessageForTestMessage = createDeleteRequestMessage(deleteTestRequestTid, 100);
        createRpcTestMessage = createCreateRequestMessage(
            createRpcRequestTid, Buffer(), protocol::PropertyType::Rpc, "/RpcTest");
    }

    void sendSignIn()
    {
        auto signinRequestMsg = createSigninRequestMessage(signinRqstTid, 1, 100);
        endpoint->queueToReceive(signinRequestMsg);
    }

    void SetUp()
    {
        auto ep = std::dynamic_pointer_cast<IEndPoint>(endpoint);
        server = monitor->create(ep, ptree);
    }

    void TearDown()
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    template<typename T>
    Buffer valueToBuffer(T in)
    {
        Buffer bf(sizeof(in));
        *((T*)bf.data()) = in;
        return bf;
    }

    void propTestCreationAction()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " <<
            this->testCreationMatcher->getUuidOfLastMatched();

        createValueRequestMessage = createCreateRequestMessage(createValueRequestTid, valueToBuffer<uint32_t>(42),
            protocol::PropertyType::Value, "/Test/Value");

        this->endpoint->queueToReceive(createValueRequestMessage);
    }

    void propValueCreationActionValueDelete()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched(); // nolint
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
        this->log << logger::DEBUG << "Requesting deletion of /Test/Value";
        endpoint->queueToReceive(deleteRequestMessageForValueMessage);
    }

    void propValueCreationActionSubscribe()
    {
        uuidOfValue = this->valueCreationMatcher->getUuidOfLastMatched(); 
        log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched();
        this->endpoint->queueToReceive(createSubscribePropertyUpdateRequestMessage(subscribeValueRqstTid, uuidOfValue));
    };

    uint32_t uuidOfValue = static_cast<uint32_t>(-1);
    const uint32_t signinRqstTid = 0;
    const uint32_t createTestRequestTid = 1;
    const uint32_t createValueRequestTid = 2;
    const uint32_t createValueRequest2Tid = 3;
    const uint32_t deleteValueRequestTid = 4;
    const uint32_t deleteTestRequestTid = 5;
    const uint32_t setValueInd1stTid = 6;
    const uint32_t subscribeTestRqstTid = 7;
    const uint32_t subscribeValueRqstTid = 8;
    const uint32_t setValueInd2ndTid = 9;
    const uint32_t unsubscribeValueRqstTid = 10;
    const uint32_t setValueInd3rdTid = 11;
    const uint32_t getValueReqTid = 12;
    const uint32_t createRpcRequestTid = 13;

/*****

Test common timeline

0) SignIn
1) create /Test
2) create /Test/Value
3) create /Test/Value again
4) delete /Test/Value
5) delete /Test
6) set /Test/Value
7) subscribe /Test
8) subscribe /Test/Value
9) set /Test/Value
10) unsubscribe /Test/Value
11) set /Test/Value
12) get /Test/Value

******/


    Buffer createTestRequestMessage;
    Buffer createValueRequestMessage;
    Buffer createValueRequestMessageForAlreadyExist;
    Buffer createValueRequestMessageForAlreadyInvalidPath;
    Buffer deleteRequestMessageForValueMessage;
    Buffer deleteRequestMessageForTestMessage;
    Buffer createRpcTestMessage;

    std::shared_ptr<MessageMatcher> signinRspMsgMatcher;
    std::shared_ptr<CreateObjectMetaUpdateNotificationMatcher> testCreationMatcher;
    std::shared_ptr<CreateObjectMetaUpdateNotificationMatcher> valueCreationMatcher;
    std::shared_ptr<DeleteObjectMetaUpdateNotificationMatcher> valueDeletionMatcher;
    std::shared_ptr<CreateObjectMetaUpdateNotificationMatcher> rpcCreationMatcher;

    MessageMatcher createTestResponseFullMatcher;
    MessageMatcher createValueResponseFullMatcher;
    MessageMatcher createValueResponseAlreadyExistFullMatcher;
    MessageMatcher createValueResponseInvalidPathFullMatcher;
    MessageMatcher createValueResponseInvalidParentFullMatcher;
    MessageMatcher deleteValueResponseOkMatcher;
    MessageMatcher deleteValueResponseNotFoundMatcher;
    MessageMatcher deleteTestResponseNotEmptyMatcher;
    MessageMatcher subscribeValueResponseOkMatcher;
    MessageMatcher subscribeValueResponseUuidNotFoundMatcher;
    MessageMatcher subscribeTestResponseNotAValueMatcher;
    MessageMatcher unsubscribeValueResponseOkMatcher;
    MessageMatcher handleRpcRequestMatcher;
    MessageMatcher rpcResponseMatcher;

    std::function<void()> testCreationAction;
    std::function<void()> valueCreationDeleteImmediatelyAction;
    std::function<void()> valueCreationSubscribeAction;

    std::shared_ptr<common::EndPointMock> endpoint;
    core::IIdGeneratorPtr idgen;
    std::shared_ptr<PTreeServer> monitor;
    core::PTreePtr ptree;
    ClientServerPtr server;
    logger::Logger log;
};

TEST_F(ClientServerTests, shouldSigninRequestAndRespondSameVersionForOk)
{
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());

    sendSignIn();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
}

TEST_F(ClientServerTests, shouldCreateOnPTreeWhenCreateRequested)
{

    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "fetching /Test/Value";
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
        core::ValuePtr val;
        ASSERT_NO_THROW(val = this->ptree->getPropertyByPath<core::Value>("/Test/Value"));
        EXPECT_EQ(42u, *(uint32_t*)(val->getValue().data()));
    };

    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    endpoint->waitForAllSending(2500.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateMessageCreateResponse)
{
    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(15000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenAlreadyExisting)
{
    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
        endpoint->queueToReceive(this->createValueRequestMessageForAlreadyExist);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, createValueResponseAlreadyExistFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(2500.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenPathIsMalformed)
{
    std::function<void()> testCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher->getUuidOfLastMatched();  //nolint
        endpoint->queueToReceive(createValueRequestMessageForAlreadyInvalidPath);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(3, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidPathFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenParentObjectIsInvalid)
{
    endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidParentFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createValueRequestMessage);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldDeleteOnPTree)
{
    std::function<void()> valueDeletionAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched(); // nolint
        ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationDeleteImmediatelyAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateDeleteResponse)
{
    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationDeleteImmediatelyAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteValueResponseOkMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldDeleteResponseNotFound) // TIMEOUT
{
    std::function<void()> testCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher->getUuidOfLastMatched();
        endpoint->queueToReceive(deleteRequestMessageForValueMessage);
        // createDeleteRequestMessage(deleteValueRequestTid, 101);
    };

    std::function<void()> valueDeletionAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched();
        ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteValueResponseNotFoundMatcher.get(), valueDeletionAction);

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}


TEST_F(ClientServerTests, shouldDeleteResponseNotEmpty)
{
    std::function<void()> valueCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched(); // nolint
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
        this->log << logger::DEBUG << "Requesting deletion of /Test";
        endpoint->queueToReceive(deleteRequestMessageForTestMessage);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteTestResponseNotEmptyMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldDeleteWithMetaUpdateNotification)
{
    std::function<void()> valueCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched(); // nolint
        this->valueDeletionMatcher->setUuid(valueCreationMatcher->getUuidOfLastMatched());
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
        this->log << logger::DEBUG << "Requesting deletion of /testing/Value";
        endpoint->queueToReceive(deleteRequestMessageForValueMessage);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(3, 2, true, 1, valueDeletionMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldSetSetValueWhenSetValueIndIsValid)
{
    std::function<void()> valueCreationAction = [this]()
    {
        uint32_t uuid = this->valueCreationMatcher->getUuidOfLastMatched(); 
        log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched();
        auto data = utils::buildBufferedValue<uint32_t>(41);

        this->endpoint->queueToReceive(createSetValueIndicationMessage(setValueInd1stTid, uuid, data));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    log << logger::DEBUG << "Waiting for setval processing...";
    using namespace std::chrono_literals;
    /** TODO: use the value update notification matcher for this checking to avoid waiting **/
    std::this_thread::sleep_for(6s);
    core::ValuePtr val;
    ASSERT_NO_THROW(val = this->ptree->getPropertyByPath<core::Value>("/Test/Value"));
    EXPECT_EQ(41u, *(uint32_t*)(val->getValue().data()));
    std::this_thread::sleep_for(500ms);
    endpoint->waitForAllSending(15000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseOk)
{
    std::function<void()> valueCreationAction = [this]()
    {
        uint32_t uuid = this->valueCreationMatcher->getUuidOfLastMatched();
        log << logger::DEBUG << "/Test/Value is created with uuid: " << uuid;
        this->endpoint->queueToReceive(createSubscribePropertyUpdateRequestMessage(subscribeValueRqstTid, uuid));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, subscribeValueResponseOkMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseUuidNotFound)
{
    std::function<void()> valueCreationAction = [this]()
    {
        uint32_t uuid = static_cast<uint32_t>(-1);
        this->endpoint->queueToReceive(createSubscribePropertyUpdateRequestMessage(subscribeValueRqstTid, uuid));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, subscribeValueResponseUuidNotFoundMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}


TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseUuidNotAValue)
{
    std::function<void()> testCreationAction = [this]()
    {
        uint32_t uuid = this->testCreationMatcher->getUuidOfLastMatched();
        this->log << logger::DEBUG << "/Test is created with uuid: " << uuid;
        this->endpoint->queueToReceive(createSubscribePropertyUpdateRequestMessage(subscribeTestRqstTid, uuid));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, subscribeTestResponseNotAValueMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldSendPropertyUpdateNotificationWhenChanged)
{
    auto expectedValue = utils::buildSharedBufferedValue(6969);
    PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

    std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
    {
        log << logger::DEBUG << "Subscribed to uuid: " << this->uuidOfValue;
        this->endpoint->queueToReceive(createSetValueIndicationMessage(setValueInd1stTid, this->uuidOfValue, *expectedValue));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationSubscribeAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, subscribeValueResponseOkMatcher.get(), subscribeValueRspAction);
    endpoint->expectSend(6, 0, false, 1, valueUpdateMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotSendPropertyUpdateNotificationWhenUnsubscribed)
{
    auto expectedValue = utils::buildSharedBufferedValue(6969);
    PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

    std::function<void()> testCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher->getUuidOfLastMatched();
        this->endpoint->queueToReceive(createValueRequestMessage);
    };

    std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
    {
        uint32_t uuid = this->valueCreationMatcher->getUuidOfLastMatched();
        log << logger::DEBUG << "/Test/Value is created with uuid: " << uuid;
        this->endpoint->queueToReceive(createSetValueIndicationMessage(setValueInd1stTid, this->uuidOfValue, *expectedValue));
    };

    std::function<void()> valueUpdateAction = [this, &expectedValue]()
    {
        log << logger::DEBUG << "Unsubscribing /Test/Value";
        this->endpoint->queueToReceive(createUnsubscribePropertyUpdateRequestMessage(unsubscribeValueRqstTid, this->uuidOfValue));
    };

    std::function<void()> unsubscribeValueAction = [this, &expectedValue]()
    {
        log << logger::DEBUG << "Setting value again";
        this->endpoint->queueToReceive(createSetValueIndicationMessage(setValueInd2ndTid, this->uuidOfValue, *expectedValue));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationSubscribeAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, subscribeValueResponseOkMatcher.get(), subscribeValueRspAction);
    endpoint->expectSend(7, 0, false, 1, valueUpdateMatcher.get(), valueUpdateAction);
    endpoint->expectSend(8, 0, false, 1, unsubscribeValueResponseOkMatcher.get(), unsubscribeValueAction);

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGetValue)
{
    auto expectedValue = utils::buildSharedBufferedValue(6969);
    MessageMatcher getValueResponseFullMatcher(createGetValueResponseMessage(getValueReqTid, *expectedValue));

    PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

    std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
    {
        log << logger::DEBUG << "Subscribed to uuid: " << this->uuidOfValue;
        /** TODO: setValueReqTid **/
        this->endpoint->queueToReceive(createSetValueIndicationMessage(getValueReqTid, this->uuidOfValue, *expectedValue));
    };

    std::function<void()> valueUpdateAction = [this]()
    {
        log << logger::DEBUG << "Getting value of uuid: " << this->uuidOfValue;
        this->endpoint->queueToReceive(createGetValueRequestMessage(getValueReqTid, this->uuidOfValue));
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationSubscribeAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, subscribeValueResponseOkMatcher.get(), subscribeValueRspAction);
    endpoint->expectSend(7, 0, false, 1, valueUpdateMatcher.get(), valueUpdateAction);
    endpoint->expectSend(8, 0, false, 1, getValueResponseFullMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createTestRequestMessage);
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
    endpoint->waitForAllSending(10000.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldForwardRcpRequestToExecutor)
{
    auto expectedParam = utils::buildSharedBufferedValue(6969);

    std::function<void()> rpcCreationAction = [this, &expectedParam]()
    {
        auto uuid = this->rpcCreationMatcher->getUuidOfLastMatched(); 
        log << logger::DEBUG << "Requesting Rpc to uuid: " << uuid;
        this->endpoint->queueToReceive(createRpcRequestMessage(createRpcRequestTid, uuid, *expectedParam));
        handleRpcRequestMatcher.set(
            createHandleRpcRequestMessage(static_cast<uint32_t>(-1), (uintptr_t)server.get(), createRpcRequestTid,
                uuid, *expectedParam));
    };

    endpoint->expectSend(1, 0, true, 1, rpcCreationMatcher->get(), rpcCreationAction);
    endpoint->expectSend(2, 0, true, 1, handleRpcRequestMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createRpcTestMessage);
    endpoint->waitForAllSending(500.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldForwardRcpResponseToCaller)
{
    auto expectedParam = utils::buildSharedBufferedValue(6969);
    rpcResponseMatcher.set(createRpcResponseMessage(createRpcRequestTid, *expectedParam));
    protocol::Uuid uuid = 0;

    std::function<void()> rpcCreationAction = [this, &expectedParam, &uuid]()
    {
        uuid = this->rpcCreationMatcher->getUuidOfLastMatched(); 
        log << logger::DEBUG << "Requesting Rpc to uuid: " << uuid;
        this->endpoint->queueToReceive(createRpcRequestMessage(createRpcRequestTid, uuid, *expectedParam));
        handleRpcRequestMatcher.set(
            createHandleRpcRequestMessage(static_cast<uint32_t>(-1), (uintptr_t)server.get(), createRpcRequestTid,
                uuid, *expectedParam));
    };

    std::function<void()> handleRpcRequestAction = [this, &expectedParam]()
    {
        log << logger::DEBUG << "Sending HandleRpcResponse ";
        this->endpoint->queueToReceive(
            createHandleRpcResponseMessage(static_cast<uint32_t>(-1), (uintptr_t)server.get(), createRpcRequestTid, *expectedParam));
    };

    endpoint->expectSend(1, 0, true, 1, rpcCreationMatcher->get(), rpcCreationAction);
    endpoint->expectSend(2, 0, true, 1, handleRpcRequestMatcher.get(), handleRpcRequestAction);
    endpoint->expectSend(3, 0, true, 1, rpcResponseMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createRpcTestMessage);
    endpoint->waitForAllSending(500.0);

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGetSpecificMetaRequestAndRespondTheCorrectMeta)
{
    ptree->createProperty<core::Node>("/FCS"); // 100
    ptree->createProperty<core::Node>("/FCS/AILERON"); // 101
    ptree->createProperty<core::Value>("/FCS/AILERON/CURRENT_DEFLECTION"); // 102

    std::string path = "/FCS/AILERON/CURRENT_DEFLECTION";

    MessageMatcher getSpecifiMetaResponsetMessageMatcher(
        createGetSpecificMetaResponseMessage(1, 102, protocol::PropertyType::Value, path));

    endpoint->expectSend(0, 0, false, 1, getSpecifiMetaResponsetMessageMatcher.get(), DefaultAction::get());

    sendSignIn();
    endpoint->queueToReceive(createGetSpecificMetaRequestMessage(1, path));
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
}

TEST_F(ClientServerTests, errorScenario_BadDataOnReceive)
{
    const char data[] = 
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"\
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    using namespace std::chrono_literals;
    Buffer bogusData((uint8_t*)data, (uint8_t*)data+sizeof(data));
    endpoint->queueToReceive(bogusData);
    std::this_thread::sleep_for(2s);
    endpoint->queueToReceive(bogusData);
    std::this_thread::sleep_for(1ms);
    endpoint->waitForAllSending(2500.0);
}

} // namespace server
} // namespace ptree
