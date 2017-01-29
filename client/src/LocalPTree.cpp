#include <algorithm>
#include <client/src/LocalPTree.hpp>

namespace ptree
{
namespace client
{

LocalPTree::LocalPTree(IClientOutgoing& outgoing, TransactionsCV& transactionsCV):
    outgoing(outgoing), transactionsCV(transactionsCV), log("LocalPTree")
{
}

void LocalPTree::addToPropertyMap(std::string& path, protocol::Uuid uuid, IPropertyPtr& property)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    uuidPropertyMap[uuid] = property;
    pathPropertyMap[path] = property;
}

void LocalPTree::removeFromPropertyMap(std::string& path)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    auto foundPathProp = pathPropertyMap.find(path);
    if (foundPathProp == pathPropertyMap.end())
    {
        return;
    }
    auto foundUuidProp = uuidPropertyMap.find(foundPathProp->second->getUuid());
    pathPropertyMap.erase(foundPathProp);
    uuidPropertyMap.erase(foundUuidProp);
}

void LocalPTree::removeFromPropertyMap(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    auto foundUuidProp = uuidPropertyMap.find(uuid);
    if (foundUuidProp == uuidPropertyMap.end())
    {
        return;
    }
    auto foundPathProp = pathPropertyMap.find(foundUuidProp->second->getPath());
    pathPropertyMap.erase(foundPathProp);
    uuidPropertyMap.erase(foundUuidProp);
}

IPropertyPtr LocalPTree::getPropertyByUuid(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    auto found = uuidPropertyMap.find(uuid);
    if (found == uuidPropertyMap.end())
    {
        return IPropertyPtr();
    }
    return found->second;
}

IPropertyPtr LocalPTree::getPropertyByPath(std::string path)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    auto found = pathPropertyMap.find(path);
    if (found == pathPropertyMap.end())
    {
        return IPropertyPtr();
    }
    return found->second;
}



ValueContainerPtr LocalPTree::createValue(std::string path, Buffer& value)
{
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Value, value);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "VALUE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<ValueContainer>(response.uuid, path, value, true);
            auto ivc = std::dynamic_pointer_cast<IProperty>(vc);
            addToPropertyMap(path, response.uuid, ivc);
            return vc;
        }
        else
        {
            log << logger::ERROR << "VALUE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "VALUE CREATE REQUEST TIMEOUT";
    }
    return ValueContainerPtr();
}

NodeContainerPtr LocalPTree::createNode(std::string path)
{
    Buffer empty;
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Node, empty);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "NODE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<NodeContainer>(response.uuid, path, true);
            auto ivc = std::dynamic_pointer_cast<IProperty>(vc);
            addToPropertyMap(path, response.uuid, ivc);
            return vc;
        }
        else
        {
            log << logger::ERROR << "NODE CREATE REQUEST NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "NODE CREATE REQUEST TIMEOUT";
    }
    return NodeContainerPtr();
}

RpcContainerPtr LocalPTree::createRpc(std::string path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler)
{
    Buffer empty;
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Rpc, empty);

    if (transactionsCV.waitTransactionCV(created.first))
    {
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "RPC CREATED WITH UUID " << response.uuid;
            auto rc = std::make_shared<RpcContainer>(response.uuid, path, handler, voidHandler, true);
            auto irc = std::dynamic_pointer_cast<IProperty>(rc);
            addToPropertyMap(path, response.uuid, irc);
            return rc;
        }
        else
        {
            log << logger::ERROR << "RPC CREATE RPC NOT OK";
        }
    }
    else
    {
        log << logger::ERROR << "RPC CREATE RPC TIMEOUT";
    }
    return RpcContainerPtr();
}

void LocalPTree::fillValue(ValueContainerPtr& value)
{
    auto getValue = outgoing.getValue(value->getUuid());
    if (transactionsCV.waitTransactionCV(getValue.first))
    {
        protocol::GetValueResponse response;
        response.unpackFrom(getValue.second->getBuffer());
        if (response.data.size())
        {
            value->updateValue(std::move(response.data), false);
        }
    }
    else
    {
        log << logger::ERROR << "GET VALUE REQUEST TIMEOUT";
    }
}

IPropertyPtr LocalPTree::fetchMeta(std::string& path)
{
    auto found = getPropertyByPath(path);
    if (found)
    {
        return found;
    }

    auto meta = outgoing.getSpecificMeta(path);
    if (transactionsCV.waitTransactionCV(meta.first))
    {
        protocol::GetSpecificMetaResponse response;
        response.unpackFrom(meta.second->getBuffer());
        log << logger::DEBUG << "UUID FOR " << path << " IS " << (uint32_t)response.meta.uuid;
        if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
        {
            switch (response.meta.propertyType)
            {
                case protocol::PropertyType::Value:
                {
                    log << logger::DEBUG << path << " is a Value";
                    auto value = std::make_shared<ValueContainer>(response.meta.uuid, path, Buffer(), false);
                    auto ivc = std::dynamic_pointer_cast<IProperty>(value);
                    addToPropertyMap(path, response.meta.uuid, ivc);
                    return value;
                }
                case protocol::PropertyType::Node:
                {
                    log << logger::DEBUG << path << " is a Node";
                    auto node = std::make_shared<NodeContainer>(response.meta.uuid, path, false);
                    auto inc = std::dynamic_pointer_cast<IProperty>(node);
                    addToPropertyMap(path, response.meta.uuid, inc);
                    return node;
                }
                case protocol::PropertyType::Rpc:
                {
                    log << logger::DEBUG << path << " is an Rpc";
                    auto rpc = std::make_shared<RpcContainer>(response.meta.uuid, path, false);
                    auto ipc = std::dynamic_pointer_cast<IProperty>(rpc);
                    addToPropertyMap(path, response.meta.uuid, ipc);
                    return rpc;
                }
            }
        }
        else
        {
            log << logger::DEBUG << "META FOR: " << path << " IS NOT FOUND!";
        }
    }
    else
    {
        log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT";
    }
    return IPropertyPtr();
}

ValueContainerPtr LocalPTree::getValue(std::string& path)
{
    auto value = std::dynamic_pointer_cast<ValueContainer>(fetchMeta(path));
    if (!value)
    {
        return value;
    }

    if (!value->isAutoUpdate())
    {
        fillValue(value);
    }
    return value;
}

RpcContainerPtr LocalPTree::getRpc(std::string& path)
{
    auto rpc = std::dynamic_pointer_cast<RpcContainer>(fetchMeta(path));
    if (!rpc)
    {
        return rpc;
    }

    log << logger::DEBUG << "RPC FETCHED WITH UUID " << rpc->getUuid();
    return rpc;
}

void LocalPTree::handleUpdaNotification(protocol::Uuid uuid, Buffer&& value)
{
    auto found = std::dynamic_pointer_cast<ValueContainer>(getPropertyByUuid(uuid));
    if (!found)
    {
        log << logger::WARNING << "Updated value not in local values. Not updating.";
        return;
    }
    found->updateValue(std::move(value), true);
}

bool LocalPTree::enableAutoUpdate(ValueContainerPtr& vc)
{
    auto uuid = vc->getUuid();
    auto subscribe = outgoing.subscribePropertyUpdate(uuid);
    if (transactionsCV.waitTransactionCV(subscribe.first))
    {
        protocol::SubscribePropertyUpdateResponse response;
        response.unpackFrom(subscribe.second->getBuffer());
        if (response.response  == protocol::SubscribePropertyUpdateResponse::Response::OK)
        {
            vc->setAutoUpdate(true);
            log << logger::DEBUG << "SUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "SUBSCRIBE: PLEASE CHECK PATH IS CORRECT AND A VALUE.";
        }
    }
    else
    {
        log << logger::ERROR << "SUBSCRIBE REQUEST TIMEOUT";
    }
    return false;
}

bool LocalPTree::disableAutoUpdate(ValueContainerPtr& vc)
{
    auto uuid = vc->getUuid();
    auto unsubscribe = outgoing.unsubscribePropertyUpdate(uuid);
    if (transactionsCV.waitTransactionCV(unsubscribe.first))
    {
        protocol::UnsubscribePropertyUpdateResponse response;
        response.unpackFrom(unsubscribe.second->getBuffer());
        if (response.response  == protocol::UnsubscribePropertyUpdateResponse::Response::OK)
        {
            vc->setAutoUpdate(false);
            log << logger::DEBUG << "UNSUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "UNSUBSCRIBE: PLEASE CHECK PATH IS CORRECT AND A VALUE.";
        }
    }
    else
    {
        log << logger::ERROR << "UNSUBSCRIBE REQUEST TIMEOUT";
    }
    return false;
}

void LocalPTree::addMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    auto i = std::find(metaUpdateHandlers.object.begin(), metaUpdateHandlers.object.end(), handler);
    if (i == metaUpdateHandlers.object.end())
    {
        metaUpdateHandlers.object.emplace_back(handler);
    }
}

void LocalPTree::deleteMetaWatcher(std::shared_ptr<IMetaUpdateHandler> handler)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    auto i = std::find(metaUpdateHandlers.object.begin(), metaUpdateHandlers.object.end(), handler);
    if (i == metaUpdateHandlers.object.end())
    {
        return;
    }
    metaUpdateHandlers.object.erase(i);
}

void LocalPTree::triggerMetaUpdateWatchersCreate(protocol::Uuid uuid, std::string& path, protocol::PropertyType propertyType)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    for (auto& i : metaUpdateHandlers.object)
    {
        i->handleCreation(uuid, path, propertyType);
    }
}

void LocalPTree::triggerMetaUpdateWatchersDelete(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    for (auto& i : metaUpdateHandlers.object)
    {
        i->handleDeletion(uuid);
    }
}

Buffer LocalPTree::handleIncomingRpc(protocol::Uuid uuid, Buffer& parameter)
{
    auto rpc = std::dynamic_pointer_cast<RpcContainer>(getPropertyByUuid(uuid));
    if (rpc && rpc->handler)
    {
        return rpc->handler(parameter);
    }
    return Buffer();
}

}
}
