#ifndef SERVER_PTREEOUTGOING_HPP_
#define SERVER_PTREEOUTGOING_HPP_

#include <mutex>
#include <memory>
#include <common/src/IEndPoint.hpp>
#include <server/src/PTree.hpp>
#include <server/src/Serverlet/MessageHandlers/MessageHandler.hpp>
#include <server/src/Serverlet/ClientServerConfig.hpp>
#include <common/src/Logger.hpp>
#include "IPTreeOutgoing.hpp"

namespace ptree
{
namespace server
{   

class PTreeOutgoing: public IPTreeOutgoing
{
public:
    PTreeOutgoing(ClientServerConfig& config, IEndPointPtr& endpoint);
    ~PTreeOutgoing();

    void notifyCreation(uint32_t uuid, protocol::PropertyType type, const std::string& path);
    void notifyDeletion(uint32_t uuid);
    void notifyValueUpdate(core::ValuePtr);
    void notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter);
    void notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue);

    void sendToClient(uint32_t tid, protocol::MessageType mtype, protocol::Message& msg);

    struct ActionTypeAndPath
    {
        enum class UpdateType {CREATE_OBJECT, DELETE_OBJECT};
        UpdateType utype;
        protocol::PropertyType ptype;
        std::string path;
    };

    typedef std::map<uint32_t, ActionTypeAndPath> UuidActionTypeAndPathMap;

private:
    void handleOutgoing();
    Buffer createHeader(protocol::MessageType type, uint32_t payloadSize, uint32_t transactionId);

    bool handleOutgoingIsRunning;
    bool killHandleOutgoing;

    UuidActionTypeAndPathMap metaUpdateNotification;
    std::mutex metaUpdateNotificationMutex;

    struct ValueUpdateNotificationEntry
    {
        protocol::Uuid uuid;
        Buffer data;
    };

    std::list<ValueUpdateNotificationEntry> valueUpdateNotification; // there's a bug here: when value is updated before the notification is sent, the data will be invalid!
    std::mutex valueUpdateNotificationMutex;

    ClientServerConfig& config;

    std::mutex sendLock;
    IEndPointPtr& endpoint;

    std::thread outgoingThread;
    logger::Logger log;
};


using PTreeOutgoingPtr = std::shared_ptr<PTreeOutgoing>;
using PTreeOutgoingWkPtr = std::weak_ptr<PTreeOutgoing>;

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREEOUTGOING_HPP_