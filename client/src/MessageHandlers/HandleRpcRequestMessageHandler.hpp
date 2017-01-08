#ifndef CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct HandleRpcRequestMessageHandler : public MessageHandler
{
    HandleRpcRequestMessageHandler(PTreeClient& ps, IEndPoint& ep);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_