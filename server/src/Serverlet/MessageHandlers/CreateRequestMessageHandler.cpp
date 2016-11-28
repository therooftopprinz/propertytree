#include "CreateRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

RcpHandler::RcpHandler()
{
}

RcpHandler::RcpHandler(ClientServerWkPtr clientServer, protocol::Uuid uuid) :
    clientServer(clientServer), uuid(uuid)
{
}

RcpHandler::~RcpHandler()
{
}

void RcpHandler::handle(uint64_t csid, uint32_t tid, server::ClientServerWkPtr cswkptr, Buffer&& parameter)
{
    auto csshared = clientServer.lock();
    if(csshared)
    {
        csshared->notifyRpcRequest(uuid, csid, tid, cswkptr, std::move(parameter));
    }
}

CreateRequestMessageHandler::
    CreateRequestMessageHandler(ClientServerPtr& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(*cs.get(),ep,pt,csmon), cs(cs)
{
}

void CreateRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("CreateRequestMessageHandler");

    protocol::CreateRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    log << logger::DEBUG << "value sz: " << (*request.data).size();
    log << logger::DEBUG << "path: " << *request.path;
    core::NodePtr parentNode;
    bool created = true;
    protocol::Uuid id = 0;

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
        else if (*request.type == protocol::PropertyType::Rpc)
        {
            log << logger::DEBUG << "Rpc to be created.";
            auto val = ptree.createProperty<core::Rpc>(*request.path);
            id = val.first;
            std::shared_ptr<RcpHandler> rcphandler = std::make_shared<RcpHandler>(cs, id);
            using std::placeholders::_1;
            using std::placeholders::_2;
            using std::placeholders::_3;
            using std::placeholders::_4;
            core::RpcWatcher watcher = std::bind(&RcpHandler::handle, rcphandler, _1, _2, _3, _4);
            val.second->setWatcher(watcher);
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

    messageSender(header->transactionId, protocol::MessageType::CreateResponse, response);
    log << logger::DEBUG << "response size: " << response.size()+sizeof(protocol::MessageHeader);
}

} // namespace server
} // namespace ptree