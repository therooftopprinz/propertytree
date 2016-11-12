#include "GetValueRequest.hpp"

namespace ptree
{
namespace server
{


GetValueRequestHandler::GetValueRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
    ClientServerPtr clientServer) :
    endpoint(endpoint),
    ptree(ptree),
    clientServer(clientServer)
{
}

void GetValueRequestHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("GetValueRequestHandler");

    protocol::GetValueRequest &rqst =
        *(protocol::GetValueRequest*)(message->data());
    uint32_t uuid = rqst.uuid;
    MessageGetValueResponseSender rsp(header->transactionId, endpoint);

    log << logger::DEBUG << "Requesting value for: " << uuid;
    try
    {
        auto value = ptree->getPropertyByUuid<core::Value>(uuid);
        if (value)
        {
            rsp.setValue(value);
        }
        else
        {
            log << logger::ERROR << "Object(uuid):" << uuid << " is not a Value";
        }
    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Object(uuid)" << uuid << " not found.";
    }

    rsp.send();
}

} // namespace server
} // namespace ptree