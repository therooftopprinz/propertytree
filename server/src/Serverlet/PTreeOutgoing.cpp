#include "PTreeOutgoing.hpp"

namespace ptree
{
namespace server
{   

PTreeOutgoing::PTreeOutgoing(IEndPointPtr& endpoint):
    endpoint(endpoint), log("PTreeOutgoing")
{
}

PTreeOutgoing::~PTreeOutgoing()
{
}

void PTreeOutgoing::notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path)
{
    std::lock_guard<std::mutex> guard(metaUpdateNotificationMutex);
    log << logger::DEBUG << "notifyCreation for: " << uuid;
    auto it = metaUpdateNotification.find(uuid);
    if (metaUpdateNotification.find(uuid) == metaUpdateNotification.end())
    {
        log << logger::DEBUG << "notification queued!";
        ActionTypeAndPath a;
        a.utype = ActionTypeAndPath::UpdateType::CREATE_OBJECT;
        a.ptype = type;
        a.path = path;
        metaUpdateNotification[uuid] = a;
    }
    else if (metaUpdateNotification[uuid].utype ==
        ActionTypeAndPath::UpdateType::DELETE_OBJECT)
    {
        log << logger::DEBUG << "Delete was already queued! Canceling.";
        metaUpdateNotification.erase(it);
    }
    else
    {
        log << logger::ERROR << "Error queued again!";
    }
}

void PTreeOutgoing::notifyDeletion(uint32_t uuid)
{
    log << logger::DEBUG << "notifyDeletion for:" << uuid;
    std::lock_guard<std::mutex> guard(metaUpdateNotificationMutex);
    if (metaUpdateNotification.find(uuid) == metaUpdateNotification.end())
    {
        log << logger::DEBUG << "notifaction queued!";
        ActionTypeAndPath a;
        a.utype = ActionTypeAndPath::UpdateType::DELETE_OBJECT;
        metaUpdateNotification[uuid] = a;
    }
    else
    {
        log << logger::ERROR << "Error queued again!";
    }
}

void PTreeOutgoing::notifyValueUpdate(core::ValuePtr value)
{
    std::lock_guard<std::mutex> guard(valueUpdateNotificationMutex);
    valueUpdateNotification.push_back(value);
}

void PTreeOutgoing::notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter)
{
    protocol::HandleRpcRequest request;
    request.callerId = clientServerId;
    request.callerTransactionId = transactionId;
    request.uuid = uuid;
    request.parameter = std::move(parameter);

    Buffer enbuff = request.getPacked();

    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = protocol::MessageType::HandleRpcRequest;
    headerRaw.size = enbuff.size()+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = static_cast<uint32_t>(-1);
    /** NOTE: sendLock not needing since calling function is from the MessageHandler which already locks sendLock.**/
    endpoint->send(header.data(), header.size());
    endpoint->send(enbuff.data(), enbuff.size());
}

void PTreeOutgoing::notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue)
{
    log << logger::DEBUG << "notifyRpcResponse for transactionId: " << transactionId << " and cs: " << (void*)this;
    protocol::RpcResponse response;
    response.returnValue = std::move(returnValue);

    Buffer enbuff = response.getPacked();

    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = protocol::MessageType::RpcResponse;
    headerRaw.size = enbuff.size()+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = transactionId;
    /** NOTE: sendLock not needing since calling function is from the MessageHandler which already locks sendLock.**/
    endpoint->send(header.data(), header.size());
    endpoint->send(enbuff.data(), enbuff.size());
}

} // namespace server
} // namespace ptree
