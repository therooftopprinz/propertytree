#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

struct RcpRequestMessageHandler : public MessageHandler
{
    RcpRequestMessageHandler(ClientServerPtr& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor& csmon);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    ClientServerPtr& cs;
};

}
}
#endif // SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_
