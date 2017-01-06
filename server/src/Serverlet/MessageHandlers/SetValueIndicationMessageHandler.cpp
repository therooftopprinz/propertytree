#include "SetValueIndicationMessageHandler.hpp"
#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

SetValueIndicationMessageHandler::SetValueIndicationMessageHandler
    (ClientServer& cs, IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
{
}

void SetValueIndicationMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SetValueIndication");

    protocol_x::SetValueIndication request;
    request.unpackFrom(*message);

    log << logger::DEBUG << "Setting value of (" << request.uuid << ") with size " << request.data.size();
    utils::printRaw(request.data.data(), request.data.size());
    try
    {
        core::ValuePtr value = ptree.getPropertyByUuid<core::Value>(request.uuid);
        value->setValue(request.data.data(), request.data.size());        
    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Object not found! Will not send any reply! Object uuid: " << request.uuid;
    }
}

}
}