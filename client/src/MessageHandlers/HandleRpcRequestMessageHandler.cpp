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
    auto& outgoing = this->outgoing;
    auto& ptree = this->ptree;
    std::thread([&ptree, &outgoing, &message, &header]()
    {
        logger::Logger log("HandleRpcRequestMessageHandler");

        protocol::HandleRpcRequest request;
        request.unpackFrom(message);
        log << logger::DEBUG << "HandleRpcRequest: " << request.toString();
        protocol::HandleRpcResponse response;

        response.callerId = request.callerId;
        response.callerTransactionId = request.callerTransactionId;
        response.returnValue = ptree.handleIncomingRpc(request.uuid, request.parameter);

        if (response.returnValue.size())
        {
            outgoing.handleRpcResponse(header.transactionId, response);
        }
    }).detach();
}

} // namespace client
} // namespace ptree
