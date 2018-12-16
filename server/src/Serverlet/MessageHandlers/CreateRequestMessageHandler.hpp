#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include <server/src/IPTreeServer.hpp>
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

class CreateRequestMessageHandler
{
public:
    CreateRequestMessageHandler(core::PTree& ptree, IPTreeServer& notifier);
    void handle(IPTreeOutgoingPtr& outgoing, protocol::MessageHeader& header, Buffer& message);
private:
    core::PTree& ptree;
    IPTreeServer& notifier;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_CREATEREQUESTMESSAGEHANDLER_HPP_