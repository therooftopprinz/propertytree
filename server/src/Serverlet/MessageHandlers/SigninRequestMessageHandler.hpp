#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

struct SigninRequestMessageHandler : public MessageHandler
{
    SigninRequestMessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_