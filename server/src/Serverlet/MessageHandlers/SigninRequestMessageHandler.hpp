#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include <server/src/Serverlet/ClientServerConfig.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class SigninRequestMessageHandler : public MessageHandler
{
public:
    SigninRequestMessageHandler(IPTreeOutgoing& outgoing, ClientServerConfig& config, core::PTree& ptree,
        IClientNotifier& notifier);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    IPTreeOutgoing& outgoing;
    ClientServerConfig& config;
    core::PTree& ptree;
    IClientNotifier& notifier;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_SIGNUPREQUESTMESSAGEHANDLER_HPP_