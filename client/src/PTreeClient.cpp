#include "PTreeClient.hpp"
#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace client
{

PTreeClient::PTreeClient(common::IEndPointPtr endpoint):
    endpoint(endpoint),
    outgoing(transactionsCV, *this->endpoint),
    ptree(outgoing, transactionsCV),
    incoming(transactionsCV, *this->endpoint, ptree),
    log("PTreeClient")
{
    // sign in here;
    std::list<protocol::SigninRequest::FeatureFlag> ft;
    ft.push_back(protocol::SigninRequest::FeatureFlag::ENABLE_METAUPDATE);
    auto trans = outgoing.signinRequest(200*1000, ft);

    if (transactionsCV.waitTransactionCV(trans.first))
    {
        log << logger::DEBUG << "signin response received.";

        protocol::SigninResponse response;
        response.unpackFrom(trans.second->getBuffer());
    }
    else
    {
        log << logger::ERROR << "SIGNIN TIMEOUT";
    }
}

PTreeClient::~PTreeClient()
{
    log << logger::DEBUG << "~PTreeClient";
}

LocalPTreePtr PTreeClient::getPTree()
{
    return std::shared_ptr<LocalPTree>(shared_from_this(), &ptree);
}

// RpcContainerPtr PTreeClient::createRpc(std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler)
// {
//     protocol::CreateRequest request;
//     request.path = path;
//     request.type = protocol::PropertyType::Rpc;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::CreateRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::CreateResponse response;
//         response.unpackFrom(tcv->value);
//         if ( response.response  == protocol::CreateResponse::Response::OK)
//         {
//             log << logger::DEBUG << "RPC CREATED WITH UUID " << response.uuid;
//             auto rc = std::make_shared<RpcContainer>(response.uuid, handler, voidHandler, true);
//             insertLocalRpc(response.uuid, rc);
//             return rc;
//         }
//         else
//         {
//             log << logger::ERROR << "VALUE CREATE RPC NOT OK";
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "VALUE CREATE RPC TIMEOUT";
//     }
//     return RpcContainerPtr();
// }

// RpcContainerPtr PTreeClient::getRpc(std::string& path)
// {
//     auto uuid = getUuid(path);

//     log << logger::DEBUG << "GET RPC (" << uuid << ")" << path;

//     if (uuid == static_cast<protocol::Uuid>(-1))
//     {
//         auto meta = fetchMeta(path);
//         uuid = std::get<0>(meta);
//         auto ptype = std::get<1>(meta);
//         if (uuid == static_cast<protocol::Uuid>(-1) || ptype != protocol::PropertyType::Rpc)
//         {
//             return RpcContainerPtr();
//         }
//     }

//     log << logger::DEBUG << "RPC FETCHED WITH UUID " << uuid;
//     auto rc = std::make_shared<RpcContainer>(uuid, std::function<Buffer(Buffer&)>(), std::function<Buffer(Buffer&)>(), false);
//     insertLocalRpc(uuid, rc);
//     return rc;
// }

// bool PTreeClient::createNode(std::string path)
// {
//     protocol::CreateRequest request;
//     request.path = path;
//     request.type = protocol::PropertyType::Node;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::CreateRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::CreateResponse response;
//         response.unpackFrom(tcv->value);
//         if (response.response  == protocol::CreateResponse::Response::OK)
//         {
//             log << logger::DEBUG << "NODE CREATED WITH UUID " << response.uuid;
//             return true;
//         }
//         else
//         {
//             log << logger::ERROR << "NODE CREATE REQUEST NOT OK";
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "NODE CREATE REQUEST TIMEOUT";
//     }
//     return false;
// }

// Buffer PTreeClient::callRpc(protocol::Uuid uuid, Buffer& parameter)
// {
//     auto rpc = getLocalRpc(uuid);
//     if (rpc && rpc->handler)
//     {
//         return rpc->handler(parameter);
//     }
//     return Buffer();
// }

// Buffer PTreeClient::rpcRequest(RpcContainerPtr& rpc, Buffer& parameter)
// {
//     auto uuid = rpc->getUuid();
//     protocol::RpcRequest request;
//     request.uuid = uuid;
//     request.parameter = parameter;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::RpcRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::RpcResponse response;
//         response.unpackFrom(tcv->value);
//         return response.returnValue;
//     }
//     else
//     {
//         log << logger::ERROR << "RPC REQUEST TIMEOUT";
//     }
//     return Buffer();
// }

}
}
