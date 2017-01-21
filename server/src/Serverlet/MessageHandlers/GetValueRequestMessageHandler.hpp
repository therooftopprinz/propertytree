#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_GETVALUEREQUESTHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_GETVALUEREQUESTHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class  GetValueRequestMessageHandler : public MessageHandler
{
public:
    GetValueRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    IPTreeOutgoing& outgoing;
    core::PTree& ptree;
};

} // namespace server
} // namespace ptree

#endif // SERVER_SERVERLET_MESSAGEHANDLERS_GETVALUEREQUESTHANDLER_HPP_
