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

#include <server/src/Serverlet/MessageHandlers/MessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/SigninRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/CreateRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/DeleteRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/SetValueIndicationMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/SubscribePropertyUpdateRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/UnsubscribePropertyUpdateRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/GetValueRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/RpcRequestMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/HandleRpcResponseMessageHandler.hpp>
#include <server/src/Serverlet/MessageHandlers/GetSpecificMetaRequestMessageHandler.hpp>

namespace ptree
{
namespace server
{

class PTreeIncoming
{
public:
    PTreeIncoming(uint64_t clientServerId, ClientServerConfig& config, IEndPoint& endpoint,
        IPTreeOutgoingPtr outgoing, core::PTree& ptree, IPTreeServer& notifier);
    ~PTreeIncoming();
    PTreeIncoming() = delete;
    PTreeIncoming(const PTreeIncoming&) = delete;
    void operator=(const PTreeIncoming&) = delete;

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