#include "HandleRpcResponseMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

HandleRpcResponseMessageHandler::HandleRpcResponseMessageHandler(IClientNotifier& notifier):
    notifier(notifier)
{
}

void HandleRpcResponseMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("HandleRcpResponseMessageHandler");

    protocol::HandleRpcResponse response;
    response.unpackFrom(*message);

    log << logger::DEBUG << "requesting handle rpc for: " << response.callerId << " at: "
        << response.callerTransactionId;
    notifier.notifyRpcResponse(response.callerId, response.callerTransactionId, std::move(response.returnValue));
}

} // server
} // ptree