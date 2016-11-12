#include <cstring>
#include <server/src/Utils.hpp>
#include "CreateRequest.hpp"

namespace ptree
{
namespace server
{

CreateRequestHandler::CreateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
    IClientServerMonitorPtr monitor, ClientServerPtr clientServer) :
    endpoint(endpoint),
    ptree(ptree),
    monitor(monitor),
    clientServer(clientServer)
{

}

void CreateRequestHandler::handle(protocol::MessageHeaderPtr header,
        BufferPtr message)
{
    logger::Logger log("CreateRequest");

    protocol::CreateRequest &rqst = *(protocol::CreateRequest*)(message->data());
    log << logger::DEBUG << "value sz: " << rqst.size;
    core::ValueContainer value(rqst.size);
    std::string path((char*)(rqst.data+rqst.size));
    log << logger::DEBUG << "path: " << path;
    std::memcpy(value.data(), rqst.data, rqst.size);
    core::NodePtr parentNode;
    bool created = true;
    MessageCreateResponseSender rsp(header->transactionId, endpoint);
    uint32_t id;

    rsp.setResponse(protocol::CreateResponse::Response::OK);

    try
    {
        if (rqst.type == protocol::PropertyType::Node)
        {
            log << logger::DEBUG << "Node to be created.";
            auto node = ptree->createProperty<core::Node>(path);
            node.second->setOwner(clientServer.get());
            id = node.first;
        }
        else if (rqst.type == protocol::PropertyType::Value)
        {
            log << logger::DEBUG << "Value to be created.";
            auto val = ptree->createProperty<core::Value>(path);
            utils::printRaw(value.data(), value.size());
            val.second->setValue(value);
            val.second->setOwner(clientServer.get());
            id = val.first;
        }
        else
        {
            log << logger::ERROR << "PropertyType Error! Rquested property type is inccorect!";
            rsp.setResponse(protocol::CreateResponse::Response::TYPE_ERROR);
            created = false;
        }
    }
    catch (core::ObjectNotFound)
    {
        rsp.setResponse(protocol::CreateResponse::Response::PARENT_NOT_FOUND);
        log << logger::ERROR << "Parent object not found!";;
        created = false;
    }
    catch (core::ObjectExisting)
    {
        rsp.setResponse(protocol::CreateResponse::Response::ALREADY_EXIST);
        log << logger::ERROR << "Already existing thrown!";
        created = false;
    }
    catch (core::MalformedPath)
    {
        rsp.setResponse(protocol::CreateResponse::Response::MALFORMED_PATH);
        log << logger::ERROR << "Malformed path thrown!";
        created = false;
    }

    rsp.send();

    if (created)
    {
        monitor->notifyCreation(id, static_cast<protocol::PropertyType>(rqst.type), path);
    }
    log << logger::DEBUG << "is created: " << created;  
}

}
}