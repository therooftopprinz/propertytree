#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class RcpHandler : public std::enable_shared_from_this<RcpHandler>
{
public:
    RcpHandler(ClientServerWkPtr clientServer);
    void handle(uint64_t csid, uint32_t tid, server::ClientServerWkPtr cswkptr);
private:
    ClientServerWkPtr clientServer;
};

class CreateRequestMessageHandler : public MessageHandler
{
public:
    CreateRequestMessageHandler(ClientServerPtr& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    ClientServerPtr& cs;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_