// #include "RpcRequestMessageHandler.hpp"

// #include <server/src/Serverlet/ClientServer.hpp>
// #include <common/src/Logger.hpp>
// #include <common/src/Utils.hpp>

// namespace ptree
// {
// namespace server
// {

// RpcRequestMessageHandler::RpcRequestMessageHandler
//     (ClientServerPtr& cs,IEndPoint& ep, core::PTree& pt, IClientServerMonitor&  csmon):
//         MessageHandler(*cs.get(),ep,pt,csmon)
//         , cs(cs)
// {

// }

// void RpcRequestMessageHandler::handle(protocol::MessageHeaderPtr header, BufferPtr message)
// {
//     logger::Logger log("RcpRequestMessageHandler");

//     protocol::RpcRequest request;
//     request.unpackFrom(*message);

//     log << logger::DEBUG << "requesting rpc for: " << request.uuid;
//     try
//     {
//         auto rpc = ptree.getPropertyByUuid<core::Rpc>(request.uuid);
//         if (rpc)
//         {
//             log << logger::DEBUG << "Calling RPC object ";
//             (*rpc)((uintptr_t) cs.get(), header->transactionId, std::move(request.parameter));
//         }
//         else
//         {
//             log << logger::ERROR << "Uuid not a rpc!";
//         }
//     }
//     catch (core::ObjectNotFound)
//     {
//         log << logger::ERROR << "Uuid not found!";
//     }
// }

// }
// }