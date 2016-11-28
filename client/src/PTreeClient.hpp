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

class PTreeClient
{
public:
    PTreeClient(std::string host, int port);
private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);
    void handleIncoming();
    void handleOutgoing();

    bool handleIncomingIsRunning;
    bool handleOutgoingIsRunning;
    bool killHandleIncoming;
    bool killHandleOutgoing;

    IEndPointPtr endpoint;

    std::map<protocol::Uuid, std::string> uuidPathMap;
    std::map<std::string, protocol::Uuid> pathUuidMap;
    std::map<std::string, Buffer> pathValueMap;
    std::mutex uuidPathMapMutex;
    std::mutex pathValueMapMutex;

    enum class EIncomingState
    {
        WAIT_FOR_HEADER_EMPTY,
        WAIT_FOR_HEADER,
        WAIT_FOR_MESSAGE_EMPTY,
        WAIT_FOR_MESSAGE,
        ERROR_HEADER_TIMEOUT,
        ERROR_MESSAGE_TIMEOUT
    };
};

}
}

#endif  // CLIENT_PTREECLIENT_HPP_