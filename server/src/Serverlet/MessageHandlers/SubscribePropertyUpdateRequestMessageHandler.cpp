#include "SubscribePropertyUpdateRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

UpdateNotificationHandler::UpdateNotificationHandler(IPTreeOutgoingWkPtr outgoing):
    outgoing(outgoing)
{

}

bool UpdateNotificationHandler::handle(core::ValuePtr value)
{
    auto o = outgoing.lock();
    if (o)
    {
        o->notifyValueUpdate(value);
        return true;
    }
    return false;
}

SubscribePropertyUpdateRequestMessageHandler::SubscribePropertyUpdateRequestMessageHandler
    (uint64_t clientServerId, IPTreeOutgoingPtr& outgoing, core::PTree& ptree, IClientNotifier& notifier):
    clientServerId(clientServerId), outgoing(outgoing), ptree(ptree), notifier(notifier)
{

}

void SubscribePropertyUpdateRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SubscribePropertyUpdateRequest");

    protocol::SubscribePropertyUpdateRequest request;
    request.unpackFrom(*message);

    protocol::SubscribePropertyUpdateResponse response;
    response.response = protocol::SubscribePropertyUpdateResponse::Response::OK;

    log << logger::DEBUG << "subscribing for: " << request.uuid;
    try
    {
        auto value = ptree.getPropertyByUuid<core::Value>(request.uuid);
        if (value)
        {
            auto sbr = std::make_shared<UpdateNotificationHandler>(outgoing);
            using std::placeholders::_1;
            core::ValueWatcher fn = std::bind(&UpdateNotificationHandler::handle, sbr, _1);

            if (!value->addWatcher(clientServerId, fn))
            {
                log << logger::WARNING << "Is value already subscribed?!";
            }
        }
        else
        {
            log << logger::ERROR << "Uuid not a value!";
            response.response = protocol::SubscribePropertyUpdateResponse::Response::NOT_A_VALUE;
        }

    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
        response.response = protocol::SubscribePropertyUpdateResponse::Response::UUID_NOT_FOUND;
    }

    outgoing->sendToClient(header->transactionId, protocol::MessageType::SubscribePropertyUpdateResponse, response);
}

}
}