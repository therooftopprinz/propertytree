#include "SubscribePropertyUpdateRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/Logger.hpp>
#include <server/src/Utils.hpp>

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

SubscribePropertyUpdateRequestMessageHandler::SubscribePropertyUpdateRequestMessageHandler
    (ClientServerPtr& cs,IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(*cs.get(),ep,pt,csmon)
        , cs(cs)
{

}

void SubscribePropertyUpdateRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SubscribePropertyUpdateRequest");

    protocol::SubscribePropertyUpdateRequest request;
    protocol::Decoder de(message->data(),message->data()+message->size());
    request << de;

    protocol::SubscribePropertyUpdateResponse response;
    response.response = protocol::SubscribePropertyUpdateResponse::Response::OK;

    log << logger::DEBUG << "subscribing for: " << *request.uuid;
    try
    {
        auto value = ptree.getPropertyByUuid<core::Value>(*request.uuid);
        if (value)
        {
            auto sbr = std::make_shared<UpdateNotificationHandler>(cs);
            using std::placeholders::_1;
            core::ValueWatcher fn = std::bind(&UpdateNotificationHandler::handle, sbr, _1);
            value->addWatcher(&clientServer, fn);
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

    messageSender(header->transactionId, protocol::MessageType::SubscribePropertyUpdateResponse, response);
}

}
}