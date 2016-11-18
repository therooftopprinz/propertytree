#include "DeleteRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/Logger.hpp>

namespace ptree
{
namespace server
{

DeleteRequestMessageHandler::
    DeleteRequestMessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
{}

inline void DeleteRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("DeleteRequestMessageHandler");

    protocol::DeleteRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    log << logger::DEBUG << "path: " << *request.path;
    bool deleted = true;

    protocol::Uuid uuid;
    protocol::DeleteResponse response;
    response.response = protocol::DeleteResponse::Response::OK;

    try
    {
        log << logger::DEBUG << "Deleting " << *request.path;
        auto property = ptree.getPropertyByPath<core::IProperty>(*request.path);
        if (property->getOwner() != &clientServer)
        {
            response.response = protocol::DeleteResponse::Response::NOT_PERMITTED;
            log << logger::ERROR << "Permission error: " << *request.path;
        }
        else
        {
            uuid = ptree.deleteProperty(*request.path);
            deleted = true;
        }
    }
    catch (core::ObjectNotFound)
    {
        response.response = protocol::DeleteResponse::Response::OBJECT_NOT_FOUND;
        log << logger::ERROR << "Object not found: " << *request.path;
    }
    catch (core::NotEmpty)
    {
        response.response = protocol::DeleteResponse::Response::NOT_EMPTY;
        log << logger::ERROR << "Node not empty found: " << *request.path;
    }

    if (deleted)
    {
        monitor.notifyDeletion(uuid);    
    }
    log << logger::DEBUG << "is deleted: " << deleted;  

    Buffer rspheader = createHeader(protocol::MessageType::DeleteResponse, response.size(), header->transactionId);
    endpoint.send(rspheader.data(), rspheader.size());
    
    Buffer responseMessageBuffer(response.size());
    protocol::BufferView responseMessageBufferView(responseMessageBuffer);
    protocol::Encoder en(responseMessageBufferView);
    response >> en;
    endpoint.send(responseMessageBuffer.data(), responseMessageBuffer.size());
    log << logger::DEBUG << "response size: " << response.size()+sizeof(protocol::MessageHeader);
}

} // namespace server
} // namespace ptree