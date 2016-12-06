#ifndef SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLER_FACTORY_HPP_

#include <interface/protocol.hpp>
#include <memory>
#include "MessageHandlers/GenericResponseMessageHandler.hpp"

namespace ptree
{
namespace client
{

struct MessageHandlerFactory
{
    static std::unique_ptr<MessageHandler>
        get(protocol::MessageType type, PTreeClientPtr& pc, IEndPointPtr& ep);
};

} // namespace client
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_