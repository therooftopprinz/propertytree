#ifndef CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_
#define CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_

#include "MessageHandler.hpp"
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

class HandleRpcRequestMessageHandler : public MessageHandler
{
public:
    HandleRpcRequestMessageHandler(TransactionsCV& transactionsCV, LocalPTree& ptree, IClientOutgoing& outgoing);
    void handle(protocol::MessageHeader& header, Buffer& message);

private:
    TransactionsCV& transactionsCV;
    LocalPTree& ptree;
    IClientOutgoing& outgoing;
};


} // namespace client
} // namespace ptree

#endif  // CLIENT_MESSAGEHANDLERS_HANDLERPCREQUESTMESSAGEHANDLER_HPP_