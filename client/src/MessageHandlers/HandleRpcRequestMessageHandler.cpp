#include <client/src/IClientOutgoing.hpp>
#include "HandleRpcRequestMessageHandler.hpp"

namespace ptree
{
namespace client
{

HandleRpcRequestMessageHandler::
    HandleRpcRequestMessageHandler(TransactionsCV& transactionsCV, LocalPTree& ptree, IClientOutgoing& outgoing):
        transactionsCV(transactionsCV), ptree(ptree), outgoing(outgoing)
{}

void HandleRpcRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("HandleRpcRequestMessageHandler");

    protocol::HandleRpcRequest request;
    request.unpackFrom(message);

    protocol::HandleRpcResponse response;

    response.callerId = request.callerId;
    response.callerTransactionId = request.callerTransactionId;
    response.returnValue = ptree.handleIncomingRpc(request.uuid, request.parameter);

    if (response.returnValue.size())
    {
        outgoing.handleRpcResponse(header.transactionId, response);
    }
}

} // namespace client
} // namespace ptree
