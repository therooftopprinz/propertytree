#ifndef CLIENT_PTREECLIENT_HPP_
#define CLIENT_PTREECLIENT_HPP_

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <common/src/IEndPoint.hpp>

namespace ptree
{
namespace client
{

typedef std::vector<uint8_t> Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;


class TransactionIdGenerator
{
public:
    TransactionIdGenerator():
        id(0)
    {
    }
    uint32_t get()
    {
        return id++;
    }
private:
    std::atomic<uint32_t> id;
};


class PTreeClient : public std::enable_shared_from_this<PTreeClient>
{
public:
    PTreeClient(common::IEndPointPtr endpoint);
    ~PTreeClient();

    void setup();
    void signIn();
    void signIn(bool enableMetaUpdate, uint32_t updateRate);
    bool createValue(std::string path, BufferPtr value);
    bool createNode(std::string path);
    bool createRpc(std::string path);

    bool deleteProperty(std::string path);

    void subscribeUpdateNotification(std::string path);
    void unSubscribeUpdateNotification(std::string path);

    void setValue(std::string path, BufferPtr value);
    BufferPtr getValue(std::string path);

    BufferPtr rpcRequest(Buffer argument);
    void handleRpcResponse(BufferPtr returnType, uint64_t calee, uint32_t transactionId);

    void installUpdateHandler(uint64_t id, std::function<void()> handler);

    void notifyTransactionCV(uint32_t transactionId);

    struct PTreeMeta
    {
        PTreeMeta(){}
        PTreeMeta(std::string& path, protocol::PropertyType type):
            path(path), type(type)
        {}
        std::string path;
        protocol::PropertyType type;
    };
    /** TODO: common memory for string key and meta path **/
    std::map<protocol::Uuid, PTreeMeta> uuidMetaMap;
    std::map<std::string, protocol::Uuid> pathUuidMap;
    std::mutex uuidMetaMapMutex;

private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);
    void handleIncoming();
    void sendSignIn(int refreshRate, const std::list<protocol::SigninRequest::FeatureFlag> features);

    /*** TODO: Commonize these with message handler***/
    Buffer createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId);
    template<class T>
    void messageSender(uint32_t tid, protocol::MessageType mtype, T& msg)
    {
        Buffer header = createHeader(mtype, msg.size(), tid);
        endpoint->send(header.data(), header.size());
        Buffer responseMessageBuffer(msg.size());
        protocol::BufferView responseMessageBufferView(responseMessageBuffer);
        protocol::Encoder en(responseMessageBufferView);
        msg >> en;
        endpoint->send(responseMessageBuffer.data(), responseMessageBuffer.size());
    }

    void addTransactionCV(uint32_t transactionId);
    bool waitTransactionCV(uint32_t transactionId);

    struct TransactionCV
    {
        TransactionCV():
            condition(false)
        {}
        std::mutex mutex;
        std::condition_variable cv;
        std::atomic<bool> condition;
    };

    std::mutex transactionIdCVLock;
    typedef std::map<uint32_t, std::shared_ptr<TransactionCV>> TrCVMap;
    TrCVMap transactionIdCV;

    TransactionIdGenerator transactionIdGenerator;
    bool handleIncomingIsRunning;
    bool handleOutgoingIsRunning;
    bool killHandleIncoming;
    uint32_t processMessageRunning;

    server::IEndPointPtr endpoint;
    std::mutex sendLock;



    logger::Logger log;
    enum class EIncomingState
    {
        WAIT_FOR_HEADER_EMPTY,
        WAIT_FOR_HEADER,
        WAIT_FOR_MESSAGE_EMPTY,
        WAIT_FOR_MESSAGE,
        ERROR_HEADER_TIMEOUT,
        ERROR_MESSAGE_TIMEOUT
    };
    EIncomingState incomingState;

};

typedef std::shared_ptr<PTreeClient> PTreeClientPtr;
}
}

#endif  // CLIENT_PTREECLIENT_HPP_