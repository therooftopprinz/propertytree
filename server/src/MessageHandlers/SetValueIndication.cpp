#include "SetValueIndication.hpp"
namespace ptree
{
namespace server
{

SetValueIndicationHandler::SetValueIndicationHandler(IEndPointPtr endpoint, core::PTreePtr ptree, IClientServerMonitorPtr monitor,
        ClientServerPtr clientServer):
    endpoint(endpoint),
    ptree(ptree),
    monitor(monitor),
    clientServer(clientServer)
{

}

void SetValueIndicationHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("SetValueIndication");

    protocol::SetValueIndication &indt = *(protocol::SetValueIndication*)(message->data());
    uint32_t uuid = indt.uuid;
    uint32_t size = header->size - sizeof(protocol::MessageHeader) - sizeof(uint32_t);
    log << logger::DEBUG << "Setting value of (" << uuid << ")";
    try
    {
        core::ValuePtr value = ptree->getPropertyByUuid<core::Value>(uuid);
        value->setValue((void*)indt.data, size);        
    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Object not found! Will not send any reply! Object uuid: " << uuid;
    }
}

}
}