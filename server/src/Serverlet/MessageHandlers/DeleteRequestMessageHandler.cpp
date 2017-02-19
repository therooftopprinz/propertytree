#include "DeleteRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

DeleteRequestMessageHandler::
    DeleteRequestMessageHandler(IPTreeOutgoing& outgoing, core::PTree& ptree, IClientNotifier& notifier):
        outgoing(outgoing), ptree(ptree), notifier(notifier)
{}

inline void DeleteRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("DeleteRequestMessageHandler");

    protocol::DeleteRequest request;
    request.unpackFrom(message);

    log << logger::DEBUG << "DeleteRequest: " << request.toString();
    bool deleted = true;

    protocol::DeleteResponse response;
    response.response = protocol::DeleteResponse::Response::OK;

    try
    {
        log << logger::DEBUG << "Deleting " << request.uuid;
        // auto property = ptree.getPropertyByPath<core::IProperty>(request.path);
        /**TODO: check ownership if allowed to delete**/

        ptree.deleteProperty(request.uuid);
    }
    catch (core::ObjectNotFound)
    {
        deleted = false;
        response.response = protocol::DeleteResponse::Response::OBJECT_NOT_FOUND;
        log << logger::ERROR << "Object not found: " << request.uuid;
    }
    catch (core::NotEmpty)
    {
        deleted = false;
        response.response = protocol::DeleteResponse::Response::NOT_EMPTY;
        log << logger::ERROR << "Node not empty: " << request.uuid;
    }

    if (deleted)
    {
        notifier.notifyDeletion(request.uuid);
    }
    else
    {

    }

    outgoing.sendToClient(header.transactionId, protocol::MessageType::DeleteResponse, response);
}

} // namespace server
} // namespace ptree