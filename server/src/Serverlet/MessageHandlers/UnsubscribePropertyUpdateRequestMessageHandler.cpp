#include "UnsubscribePropertyUpdateRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/Logger.hpp>
#include <server/src/Utils.hpp>

namespace ptree
{
namespace server
{

UnsubscribePropertyUpdateRequestMessageHandler::UnsubscribePropertyUpdateRequestMessageHandler
    (ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
{
}

void UnsubscribePropertyUpdateRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("UnsubscribePropertyUpdateRequestHandler");

    protocol::UnsubscribePropertyUpdateRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    protocol::UnsubscribePropertyUpdateResponse response;
    response.response = protocol::UnsubscribePropertyUpdateResponse::Response::OK;

    log << logger::DEBUG << "Unsubscribing for: " << *request.uuid;
    try
    {
        auto value = ptree.getPropertyByUuid<core::Value>(*request.uuid);
        if (value)
        {
            auto dr = value->removeWatcher(&clientServer);
            if (!dr)
            {
                response.response = protocol::UnsubscribePropertyUpdateResponse::Response::NOT_SUBSCRIBED;
            }
        }
        else
        {
            log << logger::ERROR << "Uuid not a value!";
            response.response = protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND;
        }

    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
        response.response = protocol::UnsubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND;
    }
    messageSender(header->transactionId, protocol::MessageType::UnsubscribePropertyUpdateResponse, response);
}

} // namespace server
} // namespace ptree