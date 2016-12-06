#ifndef CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct GenericResponseMessageHandler : public MessageHandler
{
    GenericResponseMessageHandler(PTreeClient& ps, IEndPoint& ep);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_GENERICRESPONSEMESSAGEHANDLER_HPP_