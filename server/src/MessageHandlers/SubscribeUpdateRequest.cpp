#include "SubscribeUpdateRequest.hpp"

namespace ptree
{
namespace server
{


UpdateNotificationHandler::UpdateNotificationHandler(ClientServerWkPtr clientServer):
    clientServer(clientServer)
{

}

bool UpdateNotificationHandler::handle(core::ValuePtr value)
{
    auto cs = clientServer.lock();
    if (cs)
    {
        cs->notifyValueUpdate(value);
        return true;
    }
    return false;
}


SubscribePropertyUpdateRequestHandler::SubscribePropertyUpdateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
    ClientServerPtr clientServer) :
    endpoint(endpoint),
    ptree(ptree),
    clientServer(clientServer)
{
}

void SubscribePropertyUpdateRequestHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SubscribePropertyUpdateRequestHandler");

    protocol::SubscribePropertyUpdateRequest &rqst =
        *(protocol::SubscribePropertyUpdateRequest*)(message->data());
    uint32_t uuid = rqst.uuid;
    MessageSubscribeUpdateNotificationResponseSender rsp(header->transactionId, endpoint);
    rsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::OK);

    log << logger::DEBUG << "subscribing for: " << uuid;
    try
    {
        auto value = ptree->getPropertyByUuid<core::Value>(uuid);
        if (value)
        {
            auto sbr = std::make_shared<UpdateNotificationHandler>(clientServer);
            using std::placeholders::_1;
            core::ValueWatcher fn = std::bind(&UpdateNotificationHandler::handle, sbr, _1);
            value->addWatcher(clientServer.get(), fn);
        }
        else
        {
            log << logger::ERROR << "Uuid not a value!";
            rsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::NOT_A_VALUE);
        }

    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
        rsp.setResponse(protocol::SubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND);
    }
    rsp.send();
}

} // namespace server
} // namespace ptree