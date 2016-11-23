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
    RcpHandler();
    RcpHandler(RcpHandler&);
    RcpHandler(ClientServerWkPtr clientServer, protocol::Uuid uuid);
    void handle(uint64_t csid, uint32_t tid, server::ClientServerWkPtr cswkptr, Buffer&& parameter);
    ~RcpHandler();
private:
    ClientServerWkPtr clientServer;
    protocol::Uuid uuid;
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