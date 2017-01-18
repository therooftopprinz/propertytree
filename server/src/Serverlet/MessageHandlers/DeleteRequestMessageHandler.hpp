#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class DeleteRequestMessageHandler : public MessageHandler
{
public:
    DeleteRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree, IClientNotifier& notifier);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    IPTreeOutgoing& outgoing;
    core::PTree& ptree;
    IClientNotifier& notifier;
};


} // namespace server
} // namespace ptree

#endif  // SERVER_SERVERLET_MESSAGEHANDLERS_DELETEREQUESTMESSAGEHANDLER_HPP_