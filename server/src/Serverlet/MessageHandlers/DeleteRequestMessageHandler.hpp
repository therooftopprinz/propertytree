#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include <server/src/IPTreeServer.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class DeleteRequestMessageHandler : public MessageHandler
{
public:
    DeleteRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree, IPTreeServer& notifier);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    IPTreeOutgoing& outgoing;
    core::PTree& ptree;
    IPTreeServer& notifier;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_