#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class  GetSpecificMetaRequestMessageHandler : public MessageHandler
{
public:
    GetSpecificMetaRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    IPTreeOutgoing& outgoing;
    core::PTree& ptree;
};

} // namespace server
} // namespace ptree

#endif // SERVER_SERVERLET_MESSAGEHANDLERS_GETSPECIFICMETAREQUESTMESSAGEHANDLER_HPP_
