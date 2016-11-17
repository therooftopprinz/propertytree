#include "CreateRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/Logger.hpp>

namespace ptree
{
namespace server
{

CreateRequestMessageHandler::
    CreateRequestMessageHandler(ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
{}

inline void CreateRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("CreateRequestMessageHandler");

    protocol::CreateRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    log << logger::DEBUG << "value sz: " << (*request.data).size();
    log << logger::DEBUG << "path: " << *request.path;
    core::NodePtr parentNode;
    bool created = true;
    uint32_t id;

    protocol::CreateResponse response;
    response.response = protocol::CreateResponse::Response::OK;

    try
    {
        if (*request.type == protocol::PropertyType::Node)
        {
            log << logger::DEBUG << "Node to be created.";
            auto node = ptree.createProperty<core::Node>(*request.path);
            node.second->setOwner(&clientServer);
            id = node.first;
        }
        else if (*request.type == protocol::PropertyType::Value)
        {
            log << logger::DEBUG << "Value to be created.";
            auto val = ptree.createProperty<core::Value>(*request.path);
            utils::printRaw((*request.data).data(), (*request.data).size());
            val.second->setValue(*request.data);
            val.second->setOwner(&clientServer);
            id = val.first;
        }
        else
        {
            log << logger::ERROR << "PropertyType Error! Rquested property type is inccorect!";
            response.response = protocol::CreateResponse::Response::TYPE_ERROR;
            created = false;
        }
    }
    catch (core::ObjectNotFound)
    {
        response.response = protocol::CreateResponse::Response::PARENT_NOT_FOUND;
        log << logger::ERROR << "Parent object not found!";;
        created = false;
    }
    catch (core::ObjectExisting)
    {
        response.response = protocol::CreateResponse::Response::ALREADY_EXIST;
        log << logger::ERROR << "Already existing thrown!";
        created = false;
    }
    catch (core::MalformedPath)
    {
        response.response = protocol::CreateResponse::Response::MALFORMED_PATH;
        log << logger::ERROR << "Malformed path thrown!";
        created = false;
    }

    if (created)
    {
        monitor.notifyCreation(id, static_cast<protocol::PropertyType>(*request.type), *request.path);
    }
    log << logger::DEBUG << "is created: " << created;  

    Buffer rspheader = createHeader(protocol::MessageType::SigninResponse, response.size(), header->transactionId);
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