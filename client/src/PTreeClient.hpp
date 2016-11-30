#ifndef CLIENT_PTREECLIENT_HPP_
#define CLIENT_PTREECLIENT_HPP_

#include <map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <common/src/IEndPoint.hpp>

namespace ptree
{
namespace client
{

typedef std::vector<uint8_t> Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;


class PTreeClient
{
public:
    PTreeClient(common::IEndPointPtr endpoint);
    ~PTreeClient();

    void signIn(bool enableMetaUpdate, uint32_t updateRate);
    bool createValue(std::string path, BufferPtr value);
    bool createNode(std::string path);
    bool createRpc(std::string path);

    bool delete(std::string path);

    void subscribeUpdateNotification(std::string path);
    void unSubscribeUpdateNotification(std::string path);

    void setValue(std::string path, BufferPtr value);
    BufferPtr getValue(std::string path);

    BufferPtr rpcRequest(Buffer argument);
    void handleRpcResponse(BufferPtr returnType, uint64_t calee, uint32_t transactionId);

    void installUpdateHandler(uint64_t id, std::function<void()> handler);

private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);
    void handleIncoming();

    bool handleIncomingIsRunning;
    bool handleOutgoingIsRunning;
    bool killHandleIncoming;
    uint32_t processMessageRunning;

    server::IEndPointPtr endpoint;
    std::mutex sendLock;

    std::map<protocol::Uuid, std::string> uuidPathMap;
    std::map<std::string, protocol::Uuid> pathUuidMap;
    std::map<std::string, Buffer> pathValueMap;
    std::mutex uuidPathMapMutex;
    std::mutex pathValueMapMutex;

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

}
}

#endif  // CLIENT_PTREECLIENT_HPP_