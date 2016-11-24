#include "HandleRpcResponseMessageHandler.hpp"

#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/Logger.hpp>
#include <server/src/Utils.hpp>

namespace ptree
{
namespace server
{

HandleRpcResponseMessageHandler::HandleRpcResponseMessageHandler
    (ClientServer& cs,IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
        MessageHandler(cs,ep,pt,csmon)
{

}

void HandleRpcResponseMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
{
    logger::Logger log("HandleRcpResponseMessageHandler");

    protocol::HandleRpcResponse response;
    protocol::Decoder de(message->data(),message->data()+message->size());
    response << de;

    log << logger::DEBUG << "requesting handle rpc for: " << *response.callerId << " at: "
        << *response.callerTransactionId;
    auto csTarget = monitor.getClientServerPtrById(*response.callerId);

    csTarget->notifyRpcResponse(*response.callerTransactionId, std::move(*response.returnValue));

    // try
    // {
    //     **

    //     Do something like:
    //     1. Get rcp object by uuid
    //     2. Call object's rpcWatcher (which is bound to the object creator ClientServer's rpcWatcherList)
    //         with transaction id and RpcRequest callee's ClientServer instances' raw pointer

    //     In the rpcWatcher:
    //     1. Push the args to the clientServerRpcList
        
    //     In the handleOutGoing of the object creator:
    //     1. Generate HandleRpcRequest for the object creator;

    //     In CreateRequest when type is RPC:
    //     1. Insert RPC on the ptree
    //     2. make rpcWatcher and add it to the clientServerRpcList
        
    //     **

    //     auto rpc = ptree.getPropertyByUuid<core::Rpc>(*request.uuid);
    //     if (rpc)
    //     {
    //         log << logger::DEBUG << "Calling RPC object ";
    //         (*rpc)((uintptr_t) cs.get(), header->transactionId, cs, std::move(*request.parameter));
    //     }
    //     else
    //     {
    //         log << logger::ERROR << "Uuid not a rpc!";
    //     }
    // }
    // catch (core::ObjectNotFound)
    // {
    //     log << logger::ERROR << "Uuid not found!";
    // }
}

} // server
} // ptree