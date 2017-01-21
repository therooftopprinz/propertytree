#include "UnsubscribePropertyUpdateRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

UnsubscribePropertyUpdateRequestMessageHandler::UnsubscribePropertyUpdateRequestMessageHandler
    (uint64_t clientServerId, IPTreeOutgoing& outgoing, core::PTree& ptree):
        clientServerId(clientServerId), outgoing(outgoing), ptree(ptree)
{
}

void UnsubscribePropertyUpdateRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("UnsubscribePropertyUpdateRequestHandler");

    protocol::UnsubscribePropertyUpdateRequest request;
    request.unpackFrom(message);

    protocol::UnsubscribePropertyUpdateResponse response;
    response.response = protocol::UnsubscribePropertyUpdateResponse::Response::OK;

    log << logger::DEBUG << "Unsubscribing for: " << request.uuid;
    try
    {
        auto value = ptree.getPropertyByUuid<core::Value>(request.uuid);
        if (value)
        {
            auto dr = value->removeWatcher(clientServerId);
            if (!dr)
            {
                response.response = protocol::UnsubscribePropertyUpdateResponse::Response::NOT_SUBSCRIBED;
            }
        }
        else
        {
            log << logger::ERROR << "Uuid not a value!";
            response.response = protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND;
        }

    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
        response.response = protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND;
    }
    outgoing.sendToClient(header.transactionId, protocol::MessageType::UnsubscribePropertyUpdateResponse, response);
}

} // namespace server
} // namespace ptree