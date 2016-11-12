#include <cstring>
#include <server/src/Utils.hpp>
#include "DeleteRequest.hpp"

namespace ptree
{
namespace server
{

DeleteRequestHandler::DeleteRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
    IClientServerMonitorPtr monitor, ClientServerPtr clientServer) :
    endpoint(endpoint),
    ptree(ptree),
    monitor(monitor),
    clientServer(clientServer)
{

}

void DeleteRequestHandler::handle(protocol::MessageHeaderPtr header,
    BufferPtr message)
{
    logger::Logger log("DeleteRequest");

    protocol::DeleteRequest &rqst = *(protocol::DeleteRequest*)(message->data());
    std::string path((char*)rqst.path);
    bool deleted = false;
    MessageDeleteResponseSender rsp(header->transactionId, endpoint);
    uint32_t uuid = static_cast<uint32_t>(-1);
    rsp.setResponse(protocol::DeleteResponse::Response::OK);
    try
    {
        log << logger::DEBUG << "Deleting " << path;
        auto property = ptree->getPropertyByPath<core::IProperty>(path);
        if (property->getOwner() != clientServer.get())
        {
            rsp.setResponse(protocol::DeleteResponse::Response::NOT_PERMITTED);
            log << logger::ERROR << "Permission error: " << path;
        }
        else
        {
            uuid = ptree->deleteProperty(path);
            deleted = true;
        }
    }
    catch (core::ObjectNotFound)
    {
        rsp.setResponse(protocol::DeleteResponse::Response::OBJECT_NOT_FOUND);
        log << logger::ERROR << "Object not found: " << path;
    }
    catch (core::NotEmpty)
    {
        rsp.setResponse(protocol::DeleteResponse::Response::NOT_EMPTY);
        log << logger::ERROR << "Node not empty found: " << path;
    }

    rsp.send();

    if (deleted)
    {
        monitor->notifyDeletion(uuid);    
    }
    log << logger::DEBUG << "is deleted: " << deleted;  
}

}
}