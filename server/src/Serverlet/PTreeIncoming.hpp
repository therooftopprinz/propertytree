#ifndef SERVER_PTREEINCOMING_HPP_
#define SERVER_PTREEINCOMING_HPP_

#include <mutex>
#include <memory>
#include <common/src/IEndPoint.hpp>
#include <server/src/PTree.hpp>
#include <server/src/Serverlet/MessageHandlers/MessageHandler.hpp>
#include <server/src/Serverlet/ClientServerConfig.hpp>
#include <common/src/Logger.hpp>
#include "PTreeOutgoing.hpp"

#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace server
{
class PTreeIncoming
{
public:
    PTreeIncoming(uint64_t clientServerId, ClientServerConfig& config, IEndPointPtr& endpoint,
        core::PTreePtr& ptree, IClientNotifierPtr& notifier);
    ~PTreeIncoming();
    void init(IPTreeOutgoingWkPtr o);

private:
    void handleIncoming();
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    uint32_t processMessageRunning;
    bool handleIncomingIsRunning;
    bool killHandleIncoming;
    uint64_t clientServerId;
    protocol::MessageType type;
    ClientServerConfig& config;
    IEndPointPtr& endpoint;
    IPTreeOutgoingWkPtr outgoing;
    core::PTreePtr& ptree;
    IClientNotifierPtr& notifier;
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

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREEINCOMING_HPP_