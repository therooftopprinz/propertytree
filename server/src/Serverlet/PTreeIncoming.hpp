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
    PTreeIncoming(uint64_t clientServerId, ClientServerConfig& config, IEndPoint& endpoint,
        IPTreeOutgoingPtr outgoing, core::PTreePtr& ptree, IPTreeServer& notifier);
    ~PTreeIncoming();
    void init(IPTreeOutgoingWkPtr o);

private:
    void handleIncoming();
    void processMessage(protocol::MessageHeader& header, Buffer& message);

    logger::Logger log;
    bool handleIncomingIsRunning = false;
    bool killHandleIncoming = false;
    uint64_t clientServerId;
    protocol::MessageType type;
    ClientServerConfig& config;
    IEndPoint& endpoint;
    IPTreeOutgoingWkPtr outgoingWkPtr;

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
};

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREEINCOMING_HPP_