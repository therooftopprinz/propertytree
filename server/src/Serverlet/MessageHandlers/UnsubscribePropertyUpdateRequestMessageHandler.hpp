#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_UNSUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_UNSUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class  UnsubscribePropertyUpdateRequestMessageHandler : public MessageHandler
{
public:
    UnsubscribePropertyUpdateRequestMessageHandler(uint64_t clientServerId, IPTreeOutgoing& outgoing, core::PTree& ptree);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    uint64_t clientServerId;
    IPTreeOutgoing& outgoing;
    core::PTree& ptree;
};

} // namespace server
} // namespace ptree

#endif // SERVER_SERVERLET_MESSAGEHANDLERS_UNSUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
