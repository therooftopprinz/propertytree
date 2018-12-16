#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
#include <server/src/IPTreeServer.hpp>
#include <memory>
#include "MessageHandlers/MessageHandler.hpp"
#include "MessageHandlers/SigninRequestMessageHandler.hpp"
#include "MessageHandlers/CreateRequestMessageHandler.hpp"
#include "MessageHandlers/DeleteRequestMessageHandler.hpp"
#include "MessageHandlers/SetValueIndicationMessageHandler.hpp"
#include "MessageHandlers/SubscribePropertyUpdateRequestMessageHandler.hpp"
#include "MessageHandlers/UnsubscribePropertyUpdateRequestMessageHandler.hpp"
#include "MessageHandlers/GetValueRequestMessageHandler.hpp"
#include "MessageHandlers/RpcRequestMessageHandler.hpp"
#include "MessageHandlers/HandleRpcResponseMessageHandler.hpp"
#include "MessageHandlers/GetSpecificMetaRequestMessageHandler.hpp"
#include "ClientServerConfig.hpp"

namespace ptree
{
namespace server
{

struct MessageHandlerDummy: public MessageHandler
{
    void handle(protocol::MessageHeader& header, Buffer& message);
};

struct MessageHandlerFactory
{
    /** TODO: avoid heap allocation!! **/
    static std::unique_ptr<MessageHandler>
        get(uint64_t clientServerId, protocol::MessageType type, ClientServerConfig& config, IPTreeOutgoingPtr& outgoing, core::PTreePtr& pt, IPTreeServer& notifier);
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_