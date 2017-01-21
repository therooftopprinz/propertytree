#include "GetValueRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

GetValueRequestMessageHandler::GetValueRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree):
     outgoing(outgoing), ptree(ptree)
{
}

void GetValueRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("GetValueRequestMessageHandler");

    protocol::GetValueRequest request;
    request.unpackFrom(message);

    protocol::GetValueResponse response;

    log << logger::DEBUG << "Requesting value for: " << request.uuid;
    try
    {
        auto value = ptree.getPropertyByUuid<core::Value>(request.uuid);
        if (value)
        {
            response.data = value->getValue();
        }
        else
        {
            response.data = Buffer();
        }
    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Object(uuid)" << request.uuid << " not found.";
    }

    outgoing.sendToClient(header.transactionId, protocol::MessageType::GetValueResponse, response);
}

} // namespace server
} // namespace ptree