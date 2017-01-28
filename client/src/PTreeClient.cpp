#include "PTreeClient.hpp"
#include "MessageHandlerFactory.hpp"

namespace ptree
{
namespace client
{

PTreeClient::PTreeClient(common::IEndPointPtr endpoint):
    endpoint(endpoint),
    outgoing(transactionsCV, *this->endpoint),
    incoming(transactionsCV, *this->endpoint),
    ptree(outgoing, transactionsCV),
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

LocalPTree& PTreeClient::getPTree()
{
    return ptree;
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

// ValueContainerPtr PTreeClient::sendGetValue(protocol::Uuid uuid, ValueContainerPtr& vc)
// {
//     protocol::GetValueRequest request;
//     request.uuid = uuid;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::GetValueRequest, request);
//     auto tcv = addTransactionCV(tid);

//     if (waitTransactionCV(tid))
//     {
//         protocol::GetValueResponse response;
//         response.unpackFrom(tcv->value);
//         if (response.data.size())
//         {
//             if (!vc)
//             {
//                 vc = std::make_shared<ValueContainer>(uuid, std::move(response.data), false);
//                 insertLocalValue(uuid, vc);
//             }
//             else
//             {
//                 vc->updateValue(std::move(response.data), false);
//             }

//             return vc;
//         }
//         else
//         {
//             return ValueContainerPtr();
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "GET VALUE REQUEST TIMEOUT";
//         return ValueContainerPtr();
//     }
// }


// /** TODO: reuse fetchMeta **/
// protocol::Uuid PTreeClient::fetchMetaAndAddToLocal(std::string& path)
// {
//     protocol::GetSpecificMetaRequest request;
//     request.path = path;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::GetSpecificMetaRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::GetSpecificMetaResponse response;
//         response.unpackFrom(tcv->value);
//         log << logger::DEBUG << "UUID FOR " << path << " IS " << (uint32_t)response.meta.uuid;
//         if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
//         {
//             addMeta(response.meta.uuid, response.meta.path, response.meta.propertyType);
//             return response.meta.uuid;
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT";
//     }
//     return static_cast<protocol::Uuid>(-1);
// }

// std::tuple<protocol::Uuid, protocol::PropertyType> PTreeClient::fetchMeta(std::string& path)
// {
//     protocol::GetSpecificMetaRequest request;
//     request.path = path;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::GetSpecificMetaRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::GetSpecificMetaResponse response;
//         response.unpackFrom(tcv->value);
//         log << logger::DEBUG << "UUID FOR " << path << " IS " << (uint32_t)response.meta.uuid;
//         if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
//         {
//             return std::make_tuple(response.meta.uuid, response.meta.propertyType);
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT";
//     }
//     return std::make_tuple(static_cast<protocol::Uuid>(-1), static_cast<protocol::PropertyType>(-1));
// }

// void PTreeClient::setValue(ValueContainerPtr& vc, Buffer& data)
// {
//     auto uuid = vc->getUuid();
//     log << logger::DEBUG << "SEND VALUE (" << uuid << ")";

//     Buffer tmv = data;
//     vc->updateValue(std::move(tmv), true);

//     sendSetValue(vc);
// }

// void PTreeClient::sendSetValue(ValueContainerPtr& vc)
// {
//     auto uuid = vc->getUuid();
//     log << logger::DEBUG << "SEND VALUE (" << uuid << ")";

//     protocol::SetValueIndication indication;
//     indication.uuid = uuid;
//     indication.data = vc->get();
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::SetValueIndication, indication);
// }


// ValueContainerPtr PTreeClient::getValue(std::string& path)
// {
//     auto uuid = getUuid(path);

//     log << logger::DEBUG << "GET VALUE (" << uuid << ")" << path;

//     if (uuid == static_cast<protocol::Uuid>(-1) && (uuid = fetchMetaAndAddToLocal(path)) == static_cast<protocol::Uuid>(-1))
//     {
//         return ValueContainerPtr();
//     }

//     auto vc = getLocalValue(uuid);
//     if (vc && ((vc->isAutoUpdate()) || vc->isOwned()))
//     {
//         return vc;
//     }

//     return sendGetValue(uuid, vc);
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


// void PTreeClient::addMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
// {
//     std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
//     auto i = std::find(metaUpdateHandlers.object.begin(), metaUpdateHandlers.object.end(), handler);
//     if (i == metaUpdateHandlers.object.end())
//     {
//         metaUpdateHandlers.object.emplace_back(handler);
//     }
// }

// void PTreeClient::deleteMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
// {
//     std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
//     auto i = std::find(metaUpdateHandlers.object.begin(), metaUpdateHandlers.object.end(), handler);
//     if (i == metaUpdateHandlers.object.end())
//     {
//         return;
//     }
//     metaUpdateHandlers.object.erase(i);
// }

// void PTreeClient::triggerMetaUpdateWatchersCreate(std::string& path, protocol::PropertyType propertyType)
// {
//     std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
//     for (auto& i : metaUpdateHandlers.object)
//     {
//         i->handleCreation(path, propertyType);
//     }
// }

// void PTreeClient::triggerMetaUpdateWatchersDelete(protocol::Uuid uuid)
// {
//     std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
//     for (auto& i : metaUpdateHandlers.object)
//     {
//         i->handleDeletion(uuid);
//     }
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

// bool PTreeClient::enableAutoUpdate(ValueContainerPtr& vc)
// {
//     auto uuid = vc->getUuid();
//     protocol::SubscribePropertyUpdateRequest request;
//     request.uuid = uuid;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::SubscribePropertyUpdateRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::SubscribePropertyUpdateResponse response;
//         response.unpackFrom(tcv->value);
//         if (response.response  == protocol::SubscribePropertyUpdateResponse::Response::OK)
//         {
//             vc->setAutoUpdate(true);
//             log << logger::DEBUG << "SUBSCRIBED!! " << uuid;
//             return true;
//         }
//         else
//         {
//             log << logger::ERROR << "PLEASE CHECK PATH IS CORRECT AND A VALUE.";
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "SUBSCRIBE REQUEST TIMEOUT";
//     }
//     return false;
// }

// bool PTreeClient::disableAutoUpdate(ValueContainerPtr& vc)
// {
//     auto uuid = vc->getUuid();
//     protocol::UnsubscribePropertyUpdateRequest request;
//     request.uuid = uuid;
//     auto tid = getTransactionId();
//     messageSender(tid, protocol::MessageType::UnsubscribePropertyUpdateRequest, request);
//     auto tcv = addTransactionCV(tid);
//     if (waitTransactionCV(tid))
//     {
//         protocol::UnsubscribePropertyUpdateResponse response;
//         response.unpackFrom(tcv->value);
//         if (response.response  == protocol::UnsubscribePropertyUpdateResponse::Response::OK)
//         {
//             vc->setAutoUpdate(false);
//             log << logger::DEBUG << "UNSUBSCRIBED!! " << uuid;
//             return true;
//         }
//         else
//         {
//             log << logger::ERROR << "PLEASE CHECK PATH IS CORRECT AND A VALUE.";
//         }
//     }
//     else
//     {
//         log << logger::ERROR << "UNSUBSCRIBE REQUEST TIMEOUT";
//     }
//     return false;
// }

// void PTreeClient::handleUpdaNotification(protocol::Uuid uuid, Buffer&& value)
// {
//     log << logger::DEBUG << "Handling update for " << (uint32_t)uuid;
//     std::lock_guard<std::mutex> lock(values.mutex);
//     auto i = values.object.find(uuid);
//     if (i == values.object.end())
//     {
//         log << logger::WARNING << "Updated value not in local values. Not updating.";
//         return;
//     }

//     i->second->updateValue(std::move(value), true);
// }

// Buffer PTreeClient::createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId)
// {
//     Buffer header(sizeof(protocol::MessageHeader));
//     protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
//     headerRaw.type = type;
//     headerRaw.size = payloadSize+sizeof(protocol::MessageHeader);
//     headerRaw.transactionId = transactionId;
//     return header;
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

// std::shared_ptr<PTreeClient::TransactionCV> PTreeClient::addTransactionCV(uint32_t transactionId)
// {
//     std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
//     // TODO: emplace
//     transactionIdCV.object[transactionId] = std::make_shared<PTreeClient::TransactionCV>();
//     return transactionIdCV.object[transactionId];
// }

// void PTreeClient::notifyTransactionCV(uint32_t transactionId, BufferPtr value)
// {
//     std::shared_ptr<TransactionCV> tcv;
//     {
//         std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
//         auto it = transactionIdCV.object.find(transactionId);
//         if (it == transactionIdCV.object.end())
//         {
//             log << logger::ERROR << "transactionId not found in CV list.";
//             return;
//         }
//         tcv = it->second;
//     }

//     tcv->condition = true;

//     {
//         std::lock_guard<std::mutex> guard(tcv->mutex);
//         tcv->value = std::move(*value);
//         tcv->cv.notify_all();
//     }

//     {
//         std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
//         auto it = transactionIdCV.object.find(transactionId);
//         if (it == transactionIdCV.object.end())
//         {
//             log << logger::ERROR << "transactionId not found in CV list.";
//             return;
//         }
//         transactionIdCV.object.erase(it);
//     }
// }

// bool PTreeClient::waitTransactionCV(uint32_t transactionId)
// {
//     std::shared_ptr<TransactionCV> tcv;
//     {
//         std::lock_guard<std::mutex> guard(transactionIdCV.mutex);
//         auto it = transactionIdCV.object.find(transactionId);
//         if (it == transactionIdCV.object.end())
//         {
//             log << logger::ERROR << "transactionId not found in CV list.";
//             return false;
//         }
//         tcv = it->second;
//     }

//     {
//         std::unique_lock<std::mutex> guard(tcv->mutex);
//         using namespace std::chrono_literals;
//         tcv->cv.wait_for(guard, 1s,[&tcv](){return bool(tcv->condition);});
//         return tcv->condition;
//     }
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
