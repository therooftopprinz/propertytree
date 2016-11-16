#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
#include "MessageHandlers/MessageHandler.hpp"
#include "MessageHandlers/SigninRequestMessageHandler.hpp"

namespace ptree
{
namespace server
{

struct MessageHandlerFactory
{
    static MessageHandler
        get(protocol::MessageType type, ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon);
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_