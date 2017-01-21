#ifndef SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_
#define SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_

#include <server/src/PTree.hpp>
#include <server/src/Serverlet/IPTreeOutgoing.hpp>
#include "MessageHandler.hpp"

namespace ptree
{
namespace server
{

class RpcRequestMessageHandler : public MessageHandler
{
public:
    RpcRequestMessageHandler(uint64_t clientServerId, core::PTree& ptree);
    void handle(protocol::MessageHeader& header, Buffer& message);
private:
    uint64_t clientServerId;
    core::PTree& ptree;
};

}
}
#endif // SERVER_SERVERLET_MESSAGEHANDLERS_RCPREQUESTMESSAGEHANDLER_HPP_
