#include "HandleRpcRequestMessageHandler.hpp"

namespace ptree
{
namespace client
{

HandleRpcRequestMessageHandler::
    HandleRpcRequestMessageHandler(PTreeClient& pc, IEndPoint& ep):
        MessageHandler(pc, ep)
{}

void HandleRpcRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("HandleRpcRequestMessageHandler");

    protocol::HandleRpcRequest request;
    request.unpackFrom(*message);

    protocol::HandleRpcResponse response;

    response.callerId = request.callerId;
    response.callerTransactionId = request.callerTransactionId;
    response.returnValue = ptreeClient.callRpc(request.uuid, request.parameter);

    if (response.returnValue.size())
    {
        ptreeClient.messageSender(header->transactionId, protocol::MessageType::HandleRpcResponse, response);
    }
}

} // namespace client
} // namespace ptree
