#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
#include <memory>
#include "MessageHandlers/MessageHandler.hpp"
#include "MessageHandlers/SigninRequestMessageHandler.hpp"

namespace ptree
{
namespace server
{

struct MessageHandlerFactory
{
    static std::unique_ptr<MessageHandler>
        get(protocol::MessageType type, ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon);
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_