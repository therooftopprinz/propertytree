#include "RpcRequestMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace server
{

RpcRequestMessageHandler::RpcRequestMessageHandler(uint64_t clientServerId, core::PTree& ptree):
    clientServerId(clientServerId), ptree(ptree)
{

}

void RpcRequestMessageHandler::handle(protocol::MessageHeader& header, Buffer& message)
{
    logger::Logger log("RcpRequestMessageHandler");

    protocol::RpcRequest request;
    request.unpackFrom(message);

    log << logger::DEBUG << "requesting rpc for: " << request.uuid;
    try
    {
        auto rpc = ptree.getPropertyByUuid<core::Rpc>(request.uuid);
        if (rpc)
        {
            log << logger::DEBUG << "Calling RPC object ";
            (*rpc)(clientServerId, header.transactionId, std::move(request.parameter));
        }
        else
        {
            log << logger::ERROR << "Uuid not a rpc!";
        }
    }
    catch (core::ObjectNotFound)
    {
        log << logger::ERROR << "Uuid not found!";
    }
}

}
}