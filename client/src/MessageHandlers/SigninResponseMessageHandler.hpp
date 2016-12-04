#ifndef CLIENT_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace client
{

struct SigninResponseMessageHandler : public MessageHandler
{
    SigninResponseMessageHandler(PTreeClient& ps, IEndPoint& ep);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace client
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_