#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
#include <memory>
#include "MessageHandlers/MessageHandler.hpp"
#include "MessageHandlers/GenericResponseMessageHandler.hpp"
#include "MessageHandlers/PropertyUpdateNotificationMessageHandler.hpp"
#include "MessageHandlers/MetaUpdateNotificationMessageHandler.hpp"
#include "MessageHandlers/HandleRpcRequestMessageHandler.hpp"

namespace ptree
{
namespace client
{

struct MessageHandlerFactory
{
    static std::unique_ptr<MessageHandler>
        get(protocol::MessageType, TransactionsCV&, IEndPoint&);
};

} // namespace client
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_