#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class RcpHandler : public std::enable_shared_from_this<RcpHandler>
{
public:
    RcpHandler() = delete;
    RcpHandler(IPTreeOutgoingWkPtr outgoing, protocol::Uuid uuid);
    void handle(uint64_t csid, uint32_t tid, Buffer&& parameter);
    ~RcpHandler();
private:
    IPTreeOutgoingWkPtr outgoing;
    protocol::Uuid uuid;
};

class CreateRequestMessageHandler : public MessageHandler
{
public:
    CreateRequestMessageHandler(IPTreeOutgoingPtr& outgoing, core::PTree& ptree, IClientNotifier& notifier);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    IPTreeOutgoingPtr& outgoing;
    core::PTree& ptree;
    IClientNotifier& notifier;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_