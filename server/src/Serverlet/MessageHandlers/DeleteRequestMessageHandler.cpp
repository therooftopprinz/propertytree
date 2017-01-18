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

inline void DeleteRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("DeleteRequestMessageHandler");

    protocol::DeleteRequest request;
    request.unpackFrom(*message);

    log << logger::DEBUG << "path: " << request.path;
    bool deleted = true;

    protocol::Uuid uuid = static_cast<uint32_t>(-1);
    protocol::DeleteResponse response;
    response.response = protocol::DeleteResponse::Response::OK;

    try
    {
        log << logger::DEBUG << "Deleting " << request.path;
        auto property = ptree.getPropertyByPath<core::IProperty>(request.path);
        /**TODO: check ownership if allowed to delete**/

        uuid = ptree.deleteProperty(request.path);
        deleted = true;

    }
    catch (core::ObjectNotFound)
    {
        deleted = false;
        response.response = protocol::DeleteResponse::Response::OBJECT_NOT_FOUND;
        log << logger::ERROR << "Object not found: " << request.path;
    }
    catch (core::NotEmpty)
    {
        deleted = false;
        response.response = protocol::DeleteResponse::Response::NOT_EMPTY;
        log << logger::ERROR << "Node not empty: " << request.path;
    }
    catch (core::MalformedPath)
    {
        deleted = false;
        response.response = protocol::DeleteResponse::Response::MALFORMED_PATH;
        log << logger::ERROR << "Malformed path thrown!";
    }
    if (deleted)
    {
        notifier.notifyDeletion(uuid);
    }
    log << logger::DEBUG << "is deleted: " << deleted;  
    outgoing.sendToClient(header->transactionId, protocol::MessageType::DeleteResponse, response);
}

} // namespace server
} // namespace ptree