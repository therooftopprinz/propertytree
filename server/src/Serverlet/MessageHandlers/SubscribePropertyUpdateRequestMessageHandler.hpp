#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class  UpdateNotificationHandler : public std::enable_shared_from_this<UpdateNotificationHandler>
{
public:
    UpdateNotificationHandler(IPTreeOutgoingWkPtr outgoing);
    bool handle(core::ValuePtr value);
private:
    IPTreeOutgoingWkPtr outgoing;
};

struct SubscribePropertyUpdateRequestMessageHandler
{
    SubscribePropertyUpdateRequestMessageHandler(uint64_t clientServerId, core::PTree& ptree, IClientNotifier& notifier);
    void handle(IPTreeOutgoingPtr& outgoing, protocol::MessageHeader& header, Buffer& message);
private:
    uint64_t clientServerId;
    core::PTree& ptree;
    IClientNotifier& notifier;
};

}
}
#endif // SERVER_SERVERLET_MESSAGEHANDLERS_SUBSCRIBEPROPERTYUPDATEREQUESTMESSAGEHANDLER_HPP_
