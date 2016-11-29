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

private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);
    void handleIncoming();

    bool handleIncomingIsRunning;
    bool handleOutgoingIsRunning;
    bool killHandleIncoming;
    bool processMessageRunning;

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