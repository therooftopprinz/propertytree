#include <memory>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/PTree.hpp>
#include <server/src/Logger.hpp>

#include <server/src/Utils.hpp>
#include "framework/EndPointMock.hpp"

#include "MessageMatchers/CreateObjectMetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/DeleteObjectMetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/MessageMatcher.hpp"
#include "MessageMatchers/MetaUpdateNotificationMatcher.hpp"
#include "MessageMatchers/PropertyUpdateNotificationMatcher.hpp"

using namespace testing;

namespace ptree
{
namespace server
{

struct ClientServerTests : public ::testing::Test
{
    ClientServerTests() :
        // signinRqst(signinRqstTid),
        // signinRsp(signinRqstTid),
        // createTestRequest(createMessageNodeCreateRequestCreator("/Test", createTestRequestTid)),
        // createValueRequest(createMessageValueCreateRequestCreator("/Test/Value", 42, createValueRequestTid)),
        // deleteValueRqst(deleteValueRequestTid),
        // deleteTestRqst(deleteTestRequestTid),
        // setValueInd(setValueInd1stTid),
        // subscribeTestRqst(subscribeTestRqstTid),
        // subscribeValueRqst(subscribeValueRqstTid),
        // setValueInd2nd(setValueInd2ndTid),
        // unsubscribeValueRqst(unsubscribeValueRqstTid),
        // setValueInd3rd(setValueInd3rdTid),
        // getValueRequest(getValueReqTid),
        // getValueResponse(getValueReqTid),

    //     signinRspMsgMatcher(signinRsp.create()),
    //     testCreationMatcher("/Test"),
    //     valueCreationMatcher("/Test/Value"),
    //     createTestResponseMatcher(createTestRequestTid),
    //     createValueResponseMatcher(createValueRequestTid),

    //     testCreationAction(std::bind(&ClientServerTests::propTestCreationAction, this)),
    //     valueCreationDeleteImmediatelyAction(std::bind(&ClientServerTests::propValueCreationActionValueDelete, this)),
    //     valueCreationSubscribeAction(std::bind(&ClientServerTests::propValueCreationActionSubscribe, this)),
        endpoint(std::make_shared<EndPointMock>()),
        idgen(std::make_shared<core::IdGenerator>()),
        monitor(std::make_shared<ClientServerMonitor>()),
        ptree(std::make_shared<core::PTree>(idgen)),
        server(std::make_shared<ClientServer>(endpoint, ptree, monitor)),
        log("TEST")
    {
    //     createTestResponseMatcher.setResponse(protocol::CreateResponse::Response::OK);
    //     createValueResponseMatcher.setResponse(protocol::CreateResponse::Response::OK);
    //     createTestResponseFullMatcher = MessageMatcher(createTestResponseMatcher.create());
    //     createValueResponseFullMatcher = MessageMatcher(createValueResponseMatcher.create());
    //     deleteTestRqst.setPath("/Test");
    //     deleteValueRqst.setPath("/Test/Value");
            auto signinRequestMsg = createSigninRequestMessage(signinRqstTid, 1, 100);
            endpoint->queueToReceive(signinRequestMsg);

            auto signinResponseMsg = createSigninResponseMessage(signinRqstTid, 1);
            endpoint->queueToReceive(signinRequestMsg);
            signinRspMsgMatcher = std::make_shared<MessageMatcher>(signinResponseMsg);
    //     endpoint->queueToReceive(createTestRequest.create());

    }


    Buffer createHeader(protocol::MessageType type, uint32_t size, uint32_t transactionId)
    {
        Buffer header(sizeof(protocol::MessageHeader));
        protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
        headerRaw.type = type;
        headerRaw.size = size;
        headerRaw.transactionId = transactionId;
        return header;
    }

    Buffer createSigninRequestMessage(uint32_t transactionId, uint32_t version, uint32_t refreshRate)
    {
        protocol::SigninRequest signin;
        signin.version = version;
        signin.refreshRate = refreshRate;
        uint32_t sz = signin.size();

        Buffer message = createHeader(protocol::MessageType::SigninRequest, sz, transactionId);
        Buffer enbuff(sz);
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        signin >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());
        return message;
    }

    Buffer createSigninResponseMessage(uint32_t transactionId, uint32_t version)
    {
        protocol::SigninResponse signin;
        signin.version = version;
        uint32_t sz = signin.size();

        Buffer message = createHeader(protocol::MessageType::SigninResponse, sz, transactionId);
        Buffer enbuff(sz);
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        signin >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        log << logger::DEBUG << "Message size:" << sz;
        return message;
    }
    // template <class T>
    // static MessageCreateRequestCreator createMessageValueCreateRequestCreator(std::string path,
    //     T value, uint32_t transactionId)
    // {
    //     MessageCreateRequestCreator createValueRequest(transactionId);
    //     createValueRequest.setPath(path);
    //     createValueRequest.setType(protocol::PropertyType::Value);
    //     createValueRequest.setValue(utils::buildSharedBufferedValue<T>(value));
    //     return createValueRequest;
    // }

    // static MessageCreateRequestCreator createMessageNodeCreateRequestCreator(std::string path,
    //     uint32_t transactionId)
    // {
    //     MessageCreateRequestCreator createValueRequest(transactionId);
    //     createValueRequest.setPath(path);
    //     createValueRequest.setType(protocol::PropertyType::Node);
    //     return createValueRequest;
    // }

    // void propTestCreationAction()
    // {
    //     this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher.getUuidOfLastMatched();
    //     this->endpoint->queueToReceive(this->createValueRequest.create());
    // }

    // void propValueCreationActionValueDelete()
    // {
    //     this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
    //     using namespace std::chrono_literals;
    //     std::this_thread::sleep_for(10ms);
    //     this->log << logger::DEBUG << "Requesting deletion of /Test/Value";
    //     endpoint->queueToReceive(this->deleteValueRqst.create());
    // }

    // void propValueCreationActionSubscribe()
    // {
    //     uuidOfValue = this->valueCreationMatcher.getUuidOfLastMatched(); 
    //     log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();

    //     this->subscribeValueRqst.setUuid(uuidOfValue);
    //     this->endpoint->queueToReceive(this->subscribeValueRqst.create());
    // };

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

    // MessageSignInRequestCreator signinRqst;
    // MessageSignInResponseCreator signinRsp;
    // MessageCreateRequestCreator createTestRequest;
    // MessageCreateRequestCreator createValueRequest;
    // MessageDeleteRequestCreator deleteValueRqst;
    // MessageDeleteRequestCreator deleteTestRqst;
    // MessageSetValueIndicationCreator setValueInd;
    // MessageSubscribeUpdateNotificationRequestCreator subscribeTestRqst;
    // MessageSubscribeUpdateNotificationRequestCreator subscribeValueRqst;
    // MessageSetValueIndicationCreator setValueInd2nd;
    // MessageUnsubscribeUpdateNotificationRequestCreator unsubscribeValueRqst;
    // MessageSetValueIndicationCreator setValueInd3rd;
    // MessageGetValueRequestCreator getValueRequest;
    // MessageGetValueResponseCreator getValueResponse;

    std::shared_ptr<MessageMatcher> signinRspMsgMatcher;
    // CreateObjectMetaUpdateNotificationMatcher testCreationMatcher;
    // CreateObjectMetaUpdateNotificationMatcher valueCreationMatcher;
    // DeleteObjectMetaUpdateNotificationMatcher valueDeletionMatcher;
    // MessageCreateResponseCreator createTestResponseMatcher;
    // MessageCreateResponseCreator createValueResponseMatcher;
    // MessageMatcher createTestResponseFullMatcher;
    // MessageMatcher createValueResponseFullMatcher;

    // std::function<void()> testCreationAction;
    // std::function<void()> valueCreationDeleteImmediatelyAction;
    // std::function<void()> valueCreationSubscribeAction;

    std::shared_ptr<EndPointMock> endpoint;
    core::IIdGeneratorPtr idgen;
    IClientServerMonitorPtr monitor;
    core::PTreePtr ptree;
    ClientServerPtr server;
    logger::Logger log;
};

class ClientServerMonitorMock : public IClientServerMonitor
{
public:
    ClientServerMonitorMock() {}
    ~ClientServerMonitorMock() {}
    MOCK_METHOD1(addClientServer, void(ClientServerPtr));
    MOCK_METHOD1(removeClientServer, void(ClientServerPtr));
    MOCK_METHOD3(notifyCreation, void(uint32_t, protocol::PropertyType, std::string));
    MOCK_METHOD1(notifyDeletion, void(uint32_t));
};

TEST_F(ClientServerTests, DISABLED_shouldSigninRequestAndRespondSameVersionForOk)
{
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());

    using namespace std::chrono_literals;

    server->setup();
    endpoint->waitForAllSending(2500.0);
    server->teardown();
}

// TEST_F(ClientServerTests, shouldCreateOnPTreeWhenCreateRequested)
// {
//     std::function<void()> valueCreationAction = [this]()
//     {
//         log << logger::DEBUG << "fetching /Test/Value";
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         core::ValuePtr val;
//         ASSERT_NO_THROW(val = this->ptree->getPropertyByPath<core::Value>("/Test/Value"));
//         EXPECT_EQ(42u, val->getValue<uint32_t>());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(2500.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldGenerateMessageCreateResponse)
// {
//     std::function<void()> valueCreationAction = [this]()
//     {
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(2500.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldNotCreateWhenAlreadyExisting)
// {
//     MessageCreateRequestCreator createValueRequest2 = createMessageValueCreateRequestCreator("/Test/Value", 42, createValueRequest2Tid);
//     MessageCreateResponseCreator createValueResponseInvalid(createValueRequest2Tid);
//     createValueResponseInvalid.setResponse(protocol::CreateResponse::Response::ALREADY_EXIST);
//     MessageMatcher createValueResponseInvalidFullMatcher(createValueResponseInvalid.create());

//     std::function<void()> valueCreationAction = [this, &createValueRequest2]()
//     {
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         endpoint->queueToReceive(createValueRequest2.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, createValueResponseInvalidFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(2500.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldNotCreateWhenPathIsMalformed)
// {
//     createValueRequest = createMessageValueCreateRequestCreator("/Test//Value", 42, createValueRequestTid);

//     MessageCreateResponseCreator createValueResponseInvalid(createValueRequestTid);
//     createValueResponseInvalid.setResponse(protocol::CreateResponse::Response::MALFORMED_PATH);
//     MessageMatcher createValueResponseInvalidFullMatcher(createValueResponseInvalid.create());

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(3, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldNotCreateWhenParentObjectIsInvalid)
// {
//     createValueRequest = createMessageValueCreateRequestCreator("/Tests/Value", 42, createValueRequestTid);
//     MessageCreateResponseCreator createValueResponseInvalid(createValueRequestTid);
//     createValueResponseInvalid.setResponse(protocol::CreateResponse::Response::PARENT_NOT_FOUND);
//     MessageMatcher createValueResponseInvalidFullMatcher(createValueResponseInvalid.create());

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(3, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldDeleteOnPTree)
// {
//     std::function<void()> valueDeletionAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationDeleteImmediatelyAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldGenerateDeleteResponse)
// {
//     MessageDeleteResponseCreator deleteRsp(deleteValueRequestTid);
//     deleteRsp.setResponse(protocol::DeleteResponse::Response::OK);
//     MessageMatcher deleteRspFullMatcher(deleteRsp.create());

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationDeleteImmediatelyAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, deleteRspFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldDeleteResponseNotFound)
// {
//     MessageDeleteResponseCreator deleteRsp(deleteValueRequestTid);
//     deleteRsp.setResponse(protocol::DeleteResponse::Response::OBJECT_NOT_FOUND);
//     MessageMatcher deleteRspFullMatcher(deleteRsp.create());

//     std::function<void()> testCreationAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher.getUuidOfLastMatched();
//         endpoint->queueToReceive(this->deleteValueRqst.create());
//     };

//     std::function<void()> valueDeletionAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);

//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, deleteRspFullMatcher.get(), valueDeletionAction);

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldDeleteResponseNotEmpty)
// {
//     MessageDeleteResponseCreator deleteRsp(deleteTestRequestTid);
//     deleteRsp.setResponse(protocol::DeleteResponse::Response::NOT_EMPTY);
//     MessageMatcher deleteRspFullMatcher(deleteRsp.create());

//     std::function<void()> valueCreationAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         using namespace std::chrono_literals;
//         std::this_thread::sleep_for(10ms);
//         this->log << logger::DEBUG << "Requesting deletion of /Test";
//         endpoint->queueToReceive(this->deleteTestRqst.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, deleteRspFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldDeleteWithMetaUpdateNotification)
// {
//     std::function<void()> valueCreationAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         this->valueDeletionMatcher.setUuid(valueCreationMatcher.getUuidOfLastMatched());
//         using namespace std::chrono_literals;
//         std::this_thread::sleep_for(10ms);
//         this->log << logger::DEBUG << "Requesting deletion of /testing/Value";
//         endpoint->queueToReceive(this->deleteValueRqst.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(3, 2, true, 1, valueDeletionMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldSetSetValueWhenSetValueIndIsValid)
// {
//     std::function<void()> valueCreationAction = [this]()
//     {
//         uint32_t uuid = this->valueCreationMatcher.getUuidOfLastMatched(); 
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher.getUuidOfLastMatched();
//         setValueInd.setUuid(uuid);
//         setValueInd.setValue(utils::buildBufferedValue<uint32_t>(41));
//         this->endpoint->queueToReceive(this->setValueInd.create());
//         using namespace std::chrono_literals;
//         std::this_thread::sleep_for(200ms);
//         core::ValuePtr val;
//         ASSERT_NO_THROW(val = this->ptree->getPropertyByPath<core::Value>("/Test/Value"));
//         EXPECT_EQ(41u, val->getValue<uint32_t>());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseOk)
// {
//     MessageSubscribeUpdateNotificationResponseCreator subscribeValueRsp(subscribeValueRqstTid);
//     subscribeValueRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::OK);
//     MessageMatcher subscribeValueRspFullMatcher(subscribeValueRsp.create());

//     std::function<void()> valueCreationAction = [this]()
//     {
//         uint32_t uuid = this->valueCreationMatcher.getUuidOfLastMatched();
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << uuid;
//         this->subscribeValueRqst.setUuid(uuid);
        
//         this->endpoint->queueToReceive(this->subscribeValueRqst.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());

//     endpoint->expectSend(0, 0, false, 1, subscribeValueRspFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseUuidNotFound)
// {
//     MessageSubscribeUpdateNotificationResponseCreator subscribeValueRsp(subscribeValueRqstTid);
//     subscribeValueRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND);
//     MessageMatcher subscribeValueRspFullMatcher(subscribeValueRsp.create());

//     std::function<void()> valueCreationAction = [this]()
//     {
//         uint32_t uuid = this->valueCreationMatcher.getUuidOfLastMatched() + 1;
//         log << logger::DEBUG << "/Test/Value is created with uuid: " << uuid;
//         this->subscribeValueRqst.setUuid((uint32_t)-1);
        
//         this->endpoint->queueToReceive(this->subscribeValueRqst.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationAction);
//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());

//     endpoint->expectSend(0, 0, false, 1, subscribeValueRspFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldGenerateMessageSubscribePropertyUpdateResponseUuidNotAValue)
// {
//     MessageSubscribeUpdateNotificationResponseCreator subscribeTestRsp(subscribeTestRqstTid);
//     subscribeTestRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::NOT_A_VALUE);
//     MessageMatcher subscribeTestRspFullMatcher(subscribeTestRsp.create());

//     std::function<void()> testCreationAction = [this]()
//     {
//         uint32_t uuid = this->testCreationMatcher.getUuidOfLastMatched();
//         this->log << logger::DEBUG << "/Test is created with uuid: " << uuid;
//         this->subscribeTestRqst.setUuid(uuid);
//         this->endpoint->queueToReceive(this->subscribeTestRqst.create());
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);

//     endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(0, 0, false, 1, subscribeTestRspFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldSendPropertyUpdateNotificationWhenChanged)
// {
//     auto expectedValue = utils::buildSharedBufferedValue(6969);
//     PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

//     MessageSubscribeUpdateNotificationResponseCreator subscribeValueRsp(subscribeValueRqstTid);
//     subscribeValueRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::OK);
//     MessageMatcher subscribeValueRspFullMatcher(subscribeValueRsp.create());

//     std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
//     {
//         setValueInd.setUuid(this->uuidOfValue);
//         setValueInd.setValue(*expectedValue);
//         const auto& msg = this->setValueInd.create();
//         log << logger::DEBUG << "Subscribed to uuid: " << this->uuidOfValue;
//         this->endpoint->queueToReceive(msg);
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationSubscribeAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, subscribeValueRspFullMatcher.get(), subscribeValueRspAction);
//     endpoint->expectSend(6, 0, false, 1, valueUpdateMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(10000.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }

// TEST_F(ClientServerTests, shouldNotSendPropertyUpdateNotificationWhenUnsubscribed)
// {
//     auto expectedValue = utils::buildSharedBufferedValue(6969);
//     PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

//     MessageSubscribeUpdateNotificationResponseCreator subscribeValueRsp(subscribeValueRqstTid);
//     subscribeValueRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::OK);
//     MessageMatcher subscribeValueRspFullMatcher(subscribeValueRsp.create());

//     MessageUnsubscribeUpdateNotificationResponseCreator unsubscribeValueRsp(unsubscribeValueRqstTid);
//     unsubscribeValueRsp.setResponse(protocol::UnsubscribePropertyUpdateResponse::Response::OK);
//     MessageMatcher unsubscribeValueRspFullMatcher(unsubscribeValueRsp.create());

//     std::function<void()> testCreationAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher.getUuidOfLastMatched();
//         this->endpoint->queueToReceive(this->createValueRequest.create());
//     };

//     std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
//     {
//         setValueInd.setUuid(this->uuidOfValue);
//         setValueInd.setValue(*expectedValue);
//         const auto& msg = this->setValueInd.create();
//         log << logger::DEBUG << "Subscribed to uuid: " << this->uuidOfValue;
//         this->unsubscribeValueRqst.setUuid(this->uuidOfValue);
//         this->endpoint->queueToReceive(msg);
//     };

//     std::function<void()> valueUpdateAction = [this, &expectedValue]()
//     {
//         log << logger::DEBUG << "Unsubscribing /Test/Value";
//         this->endpoint->queueToReceive(this->unsubscribeValueRqst.create());
//     };


//     std::function<void()> unsubscribeValueAction = [this, &expectedValue]()
//     {
//         setValueInd2nd.setUuid(this->uuidOfValue);
//         setValueInd2nd.setValue(*expectedValue);
//         const auto& msg = this->setValueInd2nd.create();
//         log << logger::DEBUG << "Setting value again";
//         this->endpoint->queueToReceive(msg);
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationSubscribeAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, subscribeValueRspFullMatcher.get(), subscribeValueRspAction);
//     endpoint->expectSend(7, 0, false, 1, valueUpdateMatcher.get(), valueUpdateAction);
//     endpoint->expectSend(8, 0, false, 1, unsubscribeValueRspFullMatcher.get(), unsubscribeValueAction);

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(500ms);
//     endpoint->waitForAllSending(500.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


// TEST_F(ClientServerTests, shouldGetValue)
// {
//     auto expectedValue = utils::buildSharedBufferedValue(6969);
//     getValueResponse.setValue(expectedValue);
//     MessageMatcher getValueResponseFullMatcher(getValueResponse.create());

//     PropertyUpdateNotificationMatcher valueUpdateMatcher("/Test/Value", expectedValue, ptree);

//     MessageSubscribeUpdateNotificationResponseCreator subscribeValueRsp(subscribeValueRqstTid);
//     subscribeValueRsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::OK);
//     MessageMatcher subscribeValueRspFullMatcher(subscribeValueRsp.create());


//     std::function<void()> testCreationAction = [this]()
//     {
//         this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher.getUuidOfLastMatched();
//         this->endpoint->queueToReceive(this->createValueRequest.create());
//     };

//     std::function<void()> subscribeValueRspAction = [this, &expectedValue]()
//     {
//         setValueInd.setUuid(this->uuidOfValue);
//         setValueInd.setValue(*expectedValue);
//         const auto& msg = this->setValueInd.create();
//         log << logger::DEBUG << "Subscribed to uuid: " << this->uuidOfValue;
//         this->endpoint->queueToReceive(msg);
//     };

//     std::function<void()> valueUpdateAction = [this]()
//     {
//         getValueRequest.setUuid(this->uuidOfValue);
//         const auto& msg = this->getValueRequest.create();
//         log << logger::DEBUG << "Getting value of uuid: " << this->uuidOfValue;
//         this->endpoint->queueToReceive(msg);
//     };

//     endpoint->expectSend(1, 0, true, 1, testCreationMatcher.get(), testCreationAction);
//     endpoint->expectSend(2, 1, true, 1, valueCreationMatcher.get(), valueCreationSubscribeAction);
//     endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher.get(), DefaultAction::get());
//     endpoint->expectSend(6, 0, false, 1, subscribeValueRspFullMatcher.get(), subscribeValueRspAction);
//     endpoint->expectSend(7, 0, false, 1, valueUpdateMatcher.get(), valueUpdateAction);
//     endpoint->expectSend(8, 0, false, 1, getValueResponseFullMatcher.get(), DefaultAction::get());

//     server->setup();
//     using namespace std::chrono_literals;
//     std::this_thread::sleep_for(1s);
//     endpoint->waitForAllSending(500.0);
//     server->teardown();

//     logger::loggerServer.waitEmpty();
// }


} // namespace server
} // namespace ptree