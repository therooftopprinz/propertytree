#ifndef SERVER_PTREEOUTGOING_HPP_
#define SERVER_PTREEOUTGOING_HPP_

#include <mutex>
#include <common/src/IEndPoint.hpp>
#include <server/src/PTree.hpp>
#include <common/src/Logger.hpp>
#include "IPTreeOutgoing.hpp"

namespace ptree
{
namespace server
{   

class PTreeOutgoing: public IPTreeOutgoing
{
public:
    PTreeOutgoing(IEndPointPtr& endpoint);
    ~PTreeOutgoing();
    void start();
    void stop();

    void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path);
    void notifyDeletion(uint32_t uuid);
    void notifyValueUpdate(core::ValuePtr);
    void notifyRpcRequest(protocol::Uuid uuid, uint64_t clientServerId, uint32_t transactionId, Buffer&& parameter);
    void notifyRpcResponse(uint32_t transactionId, Buffer&& returnValue);

    struct ActionTypeAndPath
    {
        enum class UpdateType {CREATE_OBJECT, DELETE_OBJECT};
        UpdateType utype;
        protocol::PropertyType ptype;
        std::string path;
    };

    typedef std::map<uint32_t, ActionTypeAndPath> UuidActionTypeAndPathMap;

    private:
    UuidActionTypeAndPathMap metaUpdateNotification;
    std::mutex metaUpdateNotificationMutex;

    std::list<core::ValuePtr> valueUpdateNotification;
    std::mutex valueUpdateNotificationMutex;

    std::mutex sendLock;
    IEndPointPtr& endpoint;

    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_PTREEOUTGOING_HPP_