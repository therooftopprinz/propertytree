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
    //     createTestResponseMatcher(createTestRequestTid),
    //     createValueResponseMatcher(createValueRequestTid),

        testCreationAction(std::bind(&ClientServerTests::propTestCreationAction, this)),
        valueCreationDeleteImmediatelyAction(std::bind(&ClientServerTests::propValueCreationActionValueDelete, this)),
    //     valueCreationSubscribeAction(std::bind(&ClientServerTests::propValueCreationActionSubscribe, this)),
        endpoint(std::make_shared<EndPointMock>()),
        idgen(std::make_shared<core::IdGenerator>()),
        monitor(std::make_shared<ClientServerMonitor>()),
        ptree(std::make_shared<core::PTree>(idgen)),
        server(std::make_shared<ClientServer>(endpoint, ptree, monitor)),
        log("TEST")
    {
        auto signinRequestMsg = createSigninRequestMessage(signinRqstTid, 1, 100);
        endpoint->queueToReceive(signinRequestMsg);

        signinRspMsgMatcher = std::make_shared<MessageMatcher>(createSigninResponseMessage(signinRqstTid, 1));
        testCreationMatcher = std::make_shared<CreateObjectMetaUpdateNotificationMatcher>("/Test");
        valueCreationMatcher = std::make_shared<CreateObjectMetaUpdateNotificationMatcher>("/Test/Value");
        valueDeletionMatcher = std::make_shared<DeleteObjectMetaUpdateNotificationMatcher>();

        createTestResponseFullMatcher = 
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createTestRequestTid, protocol::CreateResponse::Response::OK));
        createValueResponseFullMatcher =
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createValueRequestTid, protocol::CreateResponse::Response::OK));
        createValueResponseAlreadyExistFullMatcher =
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createValueRequest2Tid, protocol::CreateResponse::Response::ALREADY_EXIST));
        createValueResponseInvalidPathFullMatcher =
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createValueRequestTid, protocol::CreateResponse::Response::MALFORMED_PATH));
        createValueResponseInvalidParentFullMatcher =
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createValueRequestTid, protocol::CreateResponse::Response::PARENT_NOT_FOUND)); // nolint
        createValueResponseInvalidParentFullMatcher =
            std::make_shared<MessageMatcher>(
                createCreateResponseMessage(createValueRequestTid, protocol::CreateResponse::Response::PARENT_NOT_FOUND)); // nolint
        deleteValueResponseOkMatcher =
            std::make_shared<MessageMatcher>(
                createDeleteResponseMessage(deleteValueRequestTid, protocol::DeleteResponse::Response::OK));
        deleteValueResponseNotFoundMatcher =
            std::make_shared<MessageMatcher>(
                createDeleteResponseMessage(deleteValueRequestTid, protocol::DeleteResponse::Response::OBJECT_NOT_FOUND)); // nolint
        deleteTestResponseNotEmptyMatcher =
            std::make_shared<MessageMatcher>(
                createDeleteResponseMessage(deleteTestRequestTid, protocol::DeleteResponse::Response::NOT_EMPTY));

        auto testVal = valueToBuffer<uint32_t>(42);
        createTestRequestMessage = createCreateRequestMessage(
            createTestRequestTid, Buffer(), protocol::PropertyType::Node, "/Test");
        createValueRequestMessage = createCreateRequestMessage(
            createValueRequestTid, testVal, protocol::PropertyType::Value, "/Test/Value");
        createValueRequestMessageForAlreadyExist = createCreateRequestMessage(
            createValueRequest2Tid, testVal, protocol::PropertyType::Value, "/Test/Value");
        createValueRequestMessageForAlreadyInvalidPath = createCreateRequestMessage(
            createValueRequestTid, testVal, protocol::PropertyType::Value, "/Test//Value");
        deleteRequestMessageForValueMessage = createDeleteRequestMessage(deleteValueRequestTid, "/Test/Value");
        deleteRequestMessageForTestMessage = createDeleteRequestMessage(deleteTestRequestTid, "/Test");
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
        uint32_t sz = signin.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SigninRequest, sz, transactionId);
        Buffer enbuff(signin.size());
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
        uint32_t sz = signin.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::SigninResponse, sz, transactionId);
        Buffer enbuff(signin.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        signin >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }

    Buffer createCreateRequestMessage(uint32_t transactionId, Buffer valueContainer, protocol::PropertyType type,
        std::string path)
    {
        protocol::CreateRequest createReq;
        createReq.type = type;
        createReq.data = valueContainer;
        createReq.path = path;

        uint32_t sz = createReq.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::CreateRequest, sz, transactionId);
        Buffer enbuff(createReq.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        createReq >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }
    Buffer createCreateResponseMessage(uint32_t transactionId, protocol::CreateResponse::Response response)
    {
        protocol::CreateResponse createRsp;
        createRsp.response = response;

        uint32_t sz = createRsp.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::CreateResponse, sz, transactionId);
        Buffer enbuff(createRsp.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        createRsp >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }
    Buffer createDeleteRequestMessage(uint32_t transactionId, std::string path)
    {
        protocol::DeleteRequest deleteReq;
        deleteReq.path = path;

        uint32_t sz = deleteReq.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::DeleteRequest, sz, transactionId);
        Buffer enbuff(deleteReq.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        deleteReq >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

        return message;
    }
    Buffer createDeleteResponseMessage(uint32_t transactionId, protocol::DeleteResponse::Response response)
    {
        protocol::DeleteResponse deleteRsp;
        deleteRsp.response = response;

        uint32_t sz = deleteRsp.size() + sizeof(protocol::MessageHeader);

        Buffer message = createHeader(protocol::MessageType::DeleteResponse, sz, transactionId);
        Buffer enbuff(deleteRsp.size());
        protocol::BufferView enbuffv(enbuff);
        protocol::Encoder en(enbuffv);
        deleteRsp >> en;
        message.insert(message.end(), enbuff.begin(), enbuff.end());

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
    Buffer createTestRequestMessage;
    Buffer createValueRequestMessage;
    Buffer createValueRequestMessageForAlreadyExist;
    Buffer createValueRequestMessageForAlreadyInvalidPath;
    Buffer deleteRequestMessageForValueMessage;
    Buffer deleteRequestMessageForTestMessage;

    std::shared_ptr<MessageMatcher> signinRspMsgMatcher;
    std::shared_ptr<CreateObjectMetaUpdateNotificationMatcher> testCreationMatcher;
    std::shared_ptr<CreateObjectMetaUpdateNotificationMatcher> valueCreationMatcher;
    std::shared_ptr<DeleteObjectMetaUpdateNotificationMatcher> valueDeletionMatcher;
    // DeleteObjectMetaUpdateNotificationMatcher valueDeletionMatcher;
    // MessageCreateResponseCreator createTestResponseMatcher;
    // MessageCreateResponseCreator createValueResponseMatcher;
    std::shared_ptr<MessageMatcher> createTestResponseFullMatcher;
    std::shared_ptr<MessageMatcher> createValueResponseFullMatcher;
    std::shared_ptr<MessageMatcher> createValueResponseAlreadyExistFullMatcher;
    std::shared_ptr<MessageMatcher> createValueResponseInvalidPathFullMatcher;
    std::shared_ptr<MessageMatcher> createValueResponseInvalidParentFullMatcher;
    std::shared_ptr<MessageMatcher> deleteValueResponseOkMatcher;
    std::shared_ptr<MessageMatcher> deleteValueResponseNotFoundMatcher;
    std::shared_ptr<MessageMatcher> deleteTestResponseNotEmptyMatcher;
    std::function<void()> testCreationAction;
    std::function<void()> valueCreationDeleteImmediatelyAction;
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

TEST_F(ClientServerTests, shouldSigninRequestAndRespondSameVersionForOk)
{
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1ms);
    server->setup();
    endpoint->waitForAllSending(2500.0);
    server->teardown();
}

TEST_F(ClientServerTests, shouldCreateOnPTreeWhenCreateRequested)
{
    endpoint->queueToReceive(createTestRequestMessage);

    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "fetching /Test/Value";
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
        core::ValuePtr val;
        ASSERT_NO_THROW(val = this->ptree->getPropertyByPath<core::Value>("/Test/Value"));
        EXPECT_EQ(42u, val->getValue<uint32_t>());
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);

    server->setup();
    endpoint->waitForAllSending(2500.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateMessageCreateResponse)
{
    endpoint->queueToReceive(createTestRequestMessage);;

    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(0, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());

    server->setup();
    endpoint->waitForAllSending(2500.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenAlreadyExisting)
{
    endpoint->queueToReceive(createTestRequestMessage);

    std::function<void()> valueCreationAction = [this]()
    {
        log << logger::DEBUG << "/Test/Value is created with uuid: " <<
            this->valueCreationMatcher->getUuidOfLastMatched();
        endpoint->queueToReceive(this->createValueRequestMessageForAlreadyExist);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, createValueResponseAlreadyExistFullMatcher->get(), DefaultAction::get());

    server->setup();
    endpoint->waitForAllSending(2500.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenPathIsMalformed)
{
    endpoint->queueToReceive(createTestRequestMessage);

    std::function<void()> testCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher->getUuidOfLastMatched();  //nolint
        endpoint->queueToReceive(createValueRequestMessageForAlreadyInvalidPath);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(3, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidPathFullMatcher->get(), DefaultAction::get());

    server->setup();
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldNotCreateWhenParentObjectIsInvalid)
{
    endpoint->queueToReceive(createValueRequestMessage);

    endpoint->expectSend(2, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createValueResponseInvalidParentFullMatcher->get(), DefaultAction::get());

    server->setup();
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldDeleteOnPTree)
{
    endpoint->queueToReceive(createTestRequestMessage);

    std::function<void()> valueDeletionAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched(); // nolint
        ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationDeleteImmediatelyAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());

    server->setup();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldGenerateDeleteResponse)
{
    endpoint->queueToReceive(createTestRequestMessage);

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(2, 1, true, 1, valueCreationMatcher->get(), valueCreationDeleteImmediatelyAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteValueResponseOkMatcher->get(), DefaultAction::get());

    server->setup();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}


TEST_F(ClientServerTests, shouldDeleteResponseNotFound)
{
    endpoint->queueToReceive(createTestRequestMessage);

    std::function<void()> testCreationAction = [this]()
    {
        this->log << logger::DEBUG << "/Test is created with uuid: " << this->testCreationMatcher->getUuidOfLastMatched();
        endpoint->queueToReceive(deleteRequestMessageForValueMessage);
    };

    std::function<void()> valueDeletionAction = [this]()
    {
        this->log << logger::DEBUG << "/Test/Value is created with uuid: " << this->valueCreationMatcher->getUuidOfLastMatched();
        ASSERT_THROW(this->ptree->getPropertyByPath<core::Value>("/Test/Value"), core::ObjectNotFound);
    };

    endpoint->expectSend(1, 0, true, 1, testCreationMatcher->get(), testCreationAction);
    endpoint->expectSend(3, 0, false, 1, signinRspMsgMatcher->get(), DefaultAction::get());
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteValueResponseNotFoundMatcher->get(), valueDeletionAction);

    server->setup();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}


TEST_F(ClientServerTests, shouldDeleteResponseNotEmpty)
{
    endpoint->queueToReceive(createTestRequestMessage);

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
    endpoint->expectSend(4, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(5, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(6, 0, false, 1, deleteTestResponseNotEmptyMatcher->get(), DefaultAction::get());

    server->setup();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

TEST_F(ClientServerTests, shouldDeleteWithMetaUpdateNotification)
{
    endpoint->queueToReceive(createTestRequestMessage);

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
    endpoint->expectSend(0, 0, false, 1, createTestResponseFullMatcher->get(), DefaultAction::get());
    endpoint->expectSend(0, 0, false, 1, createValueResponseFullMatcher->get(), DefaultAction::get());

    server->setup();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    endpoint->waitForAllSending(10000.0);
    server->teardown();

    logger::loggerServer.waitEmpty();
}

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