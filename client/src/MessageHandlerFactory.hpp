#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
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

namespace ptree
{
namespace client
{

struct MessageHandlerFactory
{
    static std::unique_ptr<MessageHandler>
        get(protocol::MessageType type, PTreeClientPtr& cs, IEndPointPtr& ep);
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_