#include "CreateRequestMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

RcpHandler::RcpHandler(IPTreeOutgoingWkPtr outgoing, protocol::Uuid uuid) :
    outgoing(outgoing), uuid(uuid)
{
}

RcpHandler::~RcpHandler()
{
}

void RcpHandler::handle(uint64_t csid, uint32_t tid, Buffer&& parameter)
{
    auto outgoingsh = outgoing.lock();
    if(outgoingsh)
    {
        outgoingsh->notifyRpcRequest(uuid, csid, tid, std::move(parameter));
    }
}

CreateRequestMessageHandler::CreateRequestMessageHandler(IPTreeOutgoingPtr& outgoing, core::PTree& ptree, IClientNotifier& notifier):
    outgoing(outgoing), ptree(ptree), notifier(notifier)
{
}

void CreateRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("CreateRequestMessageHandler");

    protocol::CreateRequest request;
    request.unpackFrom(message);

    log << logger::DEBUG << "CreateRequest: " << request.toString();
    core::NodePtr parentNode;
    bool created = true;
    protocol::Uuid id = 0;

    protocol::CreateResponse response;
    response.response = protocol::CreateResponse::Response::OK;
    response.uuid = id;

    try
    {
        if (request.type == protocol::PropertyType::Node)
        {
            log << logger::DEBUG << "Node to be created.";
            auto node = ptree.createProperty<core::Node>(request.path);
            /**TODO: set owner**/
            id = node.first;
        }
        else if (request.type == protocol::PropertyType::Value)
        {
            log << logger::DEBUG << "Value to be created.";
            auto val = ptree.createProperty<core::Value>(request.path);
            utils::printRaw((request.data).data(), (request.data).size());
            val.second->setValue(request.data);
            /**TODO: set owner**/
            id = val.first;
        }
        else if (request.type == protocol::PropertyType::Rpc)
        {
            log << logger::DEBUG << "Rpc to be created.";
            auto val = ptree.createProperty<core::Rpc>(request.path);
            id = val.first;
            std::shared_ptr<RcpHandler> rcphandler = std::make_shared<RcpHandler>(outgoing, id);
            using std::placeholders::_1;
            using std::placeholders::_2;
            using std::placeholders::_3;
            using std::placeholders::_4;
            core::RpcWatcher watcher = std::bind(&RcpHandler::handle, rcphandler, _1, _2, _3);
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
        response.uuid = id;
        notifier.notifyCreation(id, static_cast<protocol::PropertyType>(request.type), request.path);
    }

    outgoing->sendToClient(header.transactionId, protocol::MessageType::CreateResponse, response);
}

} // namespace server
} // namespace ptree