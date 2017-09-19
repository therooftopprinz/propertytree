#include "PTreeOutgoing.hpp"

namespace ptree
{
namespace server
{   

PTreeOutgoing::PTreeOutgoing(ClientServerConfig& config, IEndPointPtr& endpoint):
    handleOutgoingIsRunning(true), killHandleOutgoing(false), config(config), endpoint(endpoint),
    outgoingThread(&PTreeOutgoing::handleOutgoing, this), log("PTreeOutgoing")
{
    log << logger::DEBUG << "construct";
    log << logger::DEBUG << "Created outgoingThread.";
}

PTreeOutgoing::~PTreeOutgoing()
{
    log << logger::DEBUG << "destruct";
    killHandleOutgoing = true;
    outgoingThread.join();
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
    ValueUpdateNotificationEntry toPush;
    toPush.data = value->getValue();
    toPush.uuid = value->getUuid();
    valueUpdateNotification.push_back(toPush);
}

void PTreeOutgoing::notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter)
{
    protocol::HandleRpcRequest request;
    request.callerId = clientServerId;
    request.callerTransactionId = transactionId;
    request.uuid = uuid;
    request.parameter = std::move(parameter);
    log << logger::DEBUG << "notifyRpcRequest";
    sendToClient(static_cast<uint32_t>(-1), protocol::MessageType::HandleRpcRequest, request);
}

void PTreeOutgoing::notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue)
{
    log << logger::DEBUG << "notifyRpcResponse for transactionId: " << transactionId << " and cs: " << (void*)this;
    protocol::RpcResponse response;
    response.returnValue = std::move(returnValue);
    sendToClient(transactionId, protocol::MessageType::RpcResponse, response);
}


/**TODO: single send for header and content**/
Buffer PTreeOutgoing::createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId)
{
    Buffer header(sizeof(protocol::MessageHeader));
    protocol::MessageHeader& headerRaw = *((protocol::MessageHeader*)header.data());
    headerRaw.type = type;
    headerRaw.size = payloadSize+sizeof(protocol::MessageHeader);
    headerRaw.transactionId = transactionId;
    return header;
}

void PTreeOutgoing::sendToClient(uint32_t tid, protocol::MessageType mtype, protocol::Message& msg)
{
    std::lock_guard<std::mutex> sendGuard(sendLock);

    Buffer header = createHeader(mtype, msg.size(), tid);
    endpoint->send(header.data(), header.size());
    log << logger::DEBUG << "sendToClient: " << msg.toString();
    Buffer responseMessageBuffer = msg.getPacked();
    endpoint->send(responseMessageBuffer.data(), responseMessageBuffer.size());
}

void PTreeOutgoing::handleOutgoing()
{
    handleOutgoingIsRunning = true;
    while (!killHandleOutgoing)
    {
        if (!config.enableOutgoing)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(config.updateInterval));
            continue;
        }

        {
            std::lock_guard<std::mutex> updatenotifGuard(metaUpdateNotificationMutex);
            if (metaUpdateNotification.size())
            {
                log << logger::DEBUG << "Meta Notifaction available!";

                protocol::MetaUpdateNotification metaUpdateNotif;
                for(const auto& i : metaUpdateNotification)
                {
                    if (i.second.utype == ActionTypeAndPath::UpdateType::CREATE_OBJECT)
                    {
                        metaUpdateNotif.creations.get().push_back(protocol::MetaCreate(i.first, i.second.ptype, i.second.path));
                    }
                    else
                    {
                        metaUpdateNotif.deletions.get().push_back(protocol::MetaDelete(i.first));
                    }
                }

                log << logger::DEBUG << "sending meta..";
                sendToClient(static_cast<uint32_t>(-1), protocol::MessageType::MetaUpdateNotification, metaUpdateNotif);
                log << logger::DEBUG << "MetaUpdateNotification sent!";
                metaUpdateNotification.clear();
            }
        }
        {
            std::lock_guard<std::mutex> updatenotifGuard(valueUpdateNotificationMutex);
            if (valueUpdateNotification.size())
            {
                log << logger::DEBUG << "Property Update Notifaction available!";

                protocol::PropertyUpdateNotification propertyUpdateNotifs;
                for(auto& i : valueUpdateNotification)
                {
                    propertyUpdateNotifs.propertyUpdateNotifications.get().push_back(
                        protocol::PropertyUpdateNotificationEntry(i.uuid, std::move(i.data)));
                    /** TODO: Optimize by only using the reference of value since value's lifetime is dependent to 
                        valueUpdateNotification which assures an instance of value.**/
                }

                sendToClient(static_cast<uint32_t>(-1), protocol::MessageType::PropertyUpdateNotification, propertyUpdateNotifs);
                log << logger::DEBUG << "PropertyUpdateNotification sent!";
                valueUpdateNotification.clear();
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(config.updateInterval));
    }
    log << logger::DEBUG << "handleOutgoing: exiting..";
    handleOutgoingIsRunning = false;
}

} // namespace server
} // namespace ptree
