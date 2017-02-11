#ifndef CLIENT_LOCALPTREE_HPP_
#define CLIENT_LOCALPTREE_HPP_

#include <interface/protocol.hpp>
#include <string>
#include <client/src/IProperty.hpp>
#include <client/src/NodeContainer.hpp>
#include <client/src/ValueContainer.hpp>
#include <client/src/RpcContainer.hpp>
#include <client/src/IClientOutgoing.hpp>
#include <common/src/Logger.hpp>

namespace ptree
{
namespace client
{


class ValueContainer;
typedef std::shared_ptr<ValueContainer> ValueContainerPtr;
class RpcContainer;
typedef std::shared_ptr<RpcContainer> RpcContainerPtr;

struct IMetaUpdateHandler;
class PropertyUpdateNotificationMessageHandler;
class MetaUpdateNotificationMessageHandler;
class LocalPTree
{
public:
    LocalPTree(IClientOutgoing& outgoing, TransactionsCV& transactionsCV);
    ValueContainerPtr createValue(std::string path, Buffer& value);
    RpcContainerPtr createRpc(std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler);
    NodeContainerPtr createNode(std::string path);
    ValueContainerPtr getValue(std::string& path);
    RpcContainerPtr getRpc(std::string& path);
    bool deleteProperty(IPropertyPtr& property);
    void addMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler);
    void deleteMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler);

    Buffer handleIncomingRpc(protocol::Uuid uuid, Buffer& parameter);
private:
    IClientOutgoing& outgoing;
    TransactionsCV& transactionsCV;

    std::map<protocol::Uuid, IPropertyPtr> uuidPropertyMap;
    /** TODO: choose between completely path free client or low bandwith. **/
    std::map<std::string, IPropertyPtr> pathPropertyMap;
    std::mutex propertyMapMutex;

    void addToPropertyMap(std::string& path, protocol::Uuid uuid, IPropertyPtr& property);
    void removeFromPropertyMap(std::string& path);
    void removeFromPropertyMap(protocol::Uuid uuid);
    IPropertyPtr getPropertyByUuid(protocol::Uuid uuid);
    IPropertyPtr getPropertyByPath(std::string path);

    void fillValue(ValueContainerPtr& value);
    IPropertyPtr fetchMeta(std::string& path);

    void handleUpdaNotification(protocol::Uuid uuid, Buffer&& value);
    MutexedObject<std::list<std::shared_ptr<IMetaUpdateHandler>>> metaUpdateHandlers;
    void triggerMetaUpdateWatchersCreate(protocol::Uuid uuid, std::string& path, protocol::PropertyType propertyType);
    void triggerMetaUpdateWatchersDelete(protocol::Uuid path);
    bool enableAutoUpdate(protocol::Uuid);
    bool disableAutoUpdate(protocol::Uuid);

    void setValue(protocol::Uuid uuid, Buffer&& value);
    Buffer rpcRequest(protocol::Uuid uuid, Buffer&& parameter);

    logger::Logger log;
    friend PropertyUpdateNotificationMessageHandler;
    friend MetaUpdateNotificationMessageHandler;
    friend ValueContainer;
    friend RpcContainer;
};

using LocalPTreePtr = std::shared_ptr<LocalPTree>;

struct IMetaUpdateHandler
{
    IMetaUpdateHandler() = default;
    virtual ~IMetaUpdateHandler() = default;
    virtual void handleCreation(protocol::Uuid uuid, std::string path, protocol::PropertyType propertyType) = 0;
    virtual void handleDeletion(protocol::Uuid) = 0;
};

}
}
#endif  // CLIENT_LOCALPTREE_HPP_
