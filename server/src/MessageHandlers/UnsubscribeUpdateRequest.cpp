#include "UnsubscribeUpdateRequest.hpp"

namespace ptree
{
namespace server
{

UnsubscribePropertyUpdateRequestHandler::UnsubscribePropertyUpdateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
    ClientServerPtr clientServer) :
    endpoint(endpoint),
    ptree(ptree),
    clientServer(clientServer)
{
}

void UnsubscribePropertyUpdateRequestHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("UnsubscribePropertyUpdateRequestHandler");

    protocol::SubscribePropertyUpdateRequest &rqst =
        *(protocol::SubscribePropertyUpdateRequest*)(message->data());
    uint32_t uuid = rqst.uuid;
    MessageUnsubscribeUpdateNotificationResponseSender rsp(header->transactionId, endpoint);
    rsp.setResponse(protocol::UnsubscribePropertyUpdateResponse::Response::OK);

    log << logger::DEBUG << "Unsubscribing for: " << uuid;
    try
    {
        auto value = ptree->getPropertyByUuid<core::Value>(uuid);
        if (value)
        {
            auto dr = value->removeWatcher(clientServer.get());
            if (!dr)
            {
                rsp.setResponse(protocol::UnsubscribePropertyUpdateResponse::Response::NOT_SUBSCRIBED);
            }
        }
        else
        {
            log << logger::ERROR << "Uuid not a value!";
            rsp.setResponse(protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND);
        }

    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
        rsp.setResponse(protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND);
    }
    rsp.send();
}

} // namespace server
} // namespace ptree