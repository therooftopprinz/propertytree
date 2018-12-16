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
        IPTreeOutgoing& outgoing, core::PTreePtr& ptree, IPTreeServer& notifier);
    ~PTreeIncoming();
    void init(IPTreeOutgoingWkPtr o);

private:
    void handleIncoming();
    void processMessage(protocol::MessageHeader& header, Buffer& message);

    bool handleIncomingIsRunning;
    bool killHandleIncoming;
    uint64_t clientServerId;
    protocol::MessageType type;
    ClientServerConfig& config;
    IEndPointPtr& endpoint;
    IPTreeOutgoing& outgoing;
    IPTreeOutgoingWkPtr outgoingWkPtr;
    core::PTreePtr& ptree;
    IPTreeServer& notifier;

    CreateRequestMessageHandler createRequestMessageHandler;
    DeleteRequestMessageHandler deleteRequestMessageHandler;
    GetSpecificMetaRequestMessageHandler getSpecificMetaRequestMessageHandler;
    GetValueRequestMessageHandler getValueRequestMessageHandler;
    HandleRpcResponseMessageHandler handleRpcResponseMessageHandler;
    RpcRequestMessageHandler rpcRequestMessageHandler;
    SetValueIndicationMessageHandler setValueIndicationMessageHandler;
    SigninRequestMessageHandler signinRequestMessageHandler;
    SubscribePropertyUpdateRequestMessageHandler subscribePropertyUpdateRequestMessageHandler;
    UnsubscribePropertyUpdateRequestMessageHandler unsubscribePropertyUpdateRequestMessageHandler;

    std::thread incomingThread;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREEINCOMING_HPP_