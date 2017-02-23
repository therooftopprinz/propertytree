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


struct HandleRpcRequestMessageHandlerThread
{
    HandleRpcRequestMessageHandlerThread(protocol::MessageHeader& header, Buffer&& message,
        LocalPTree& ptree, IClientOutgoing& outgoing):
            header(header), message(std::move(message)), ptree(ptree), outgoing(outgoing)
    {}

    void handle()
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
    }
    protocol::MessageHeader header;
    Buffer message;
    LocalPTree& ptree;
    IClientOutgoing& outgoing;
};

void HandleRpcRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    auto& outgoing = this->outgoing;
    auto& ptree = this->ptree;

    std::thread(std::bind(&HandleRpcRequestMessageHandlerThread::handle,
        std::make_shared<HandleRpcRequestMessageHandlerThread>(header, std::move(message),ptree, outgoing))
    ).detach();

}

} // namespace client
} // namespace ptree
