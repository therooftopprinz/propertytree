#include <algorithm>
#include <client/src/LocalPTree.hpp>
#include <common/src/Utils.hpp>

namespace ptree
{
namespace client
{

LocalPTree::LocalPTree(IClientOutgoing& outgoing, TransactionsCV& transactionsCV):
    outgoing(outgoing), transactionsCV(transactionsCV), log("LocalPTree")
{
}

void LocalPTree::addToPropertyMap(const std::string& path, protocol::Uuid uuid, IPropertyPtr& property)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    uuidPropertyMap[uuid] = property;
    pathPropertyMap[path] = property;
}

void LocalPTree::removeFromPropertyMap(const std::string& path)
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

IPropertyPtr LocalPTree::getPropertyByPath(const std::string& path)
{
    std::lock_guard<std::mutex> lock(propertyMapMutex);
    auto found = pathPropertyMap.find(path);
    if (found == pathPropertyMap.end())
    {
        return IPropertyPtr();
    }
    return found->second;
}



ValueContainerPtr LocalPTree::createValue(const std::string& path, Buffer& value)
{
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Value, value);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        log << logger::DEBUG << "createValue resp has data of:";
        utils::printRaw(created.second->getBuffer().data(), created.second->getBuffer().size());
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        log << logger::DEBUG << "CreateResponse: " << response.toString();
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "VALUE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<ValueContainer>(*this, response.uuid, path, value, true);
            auto ivc = std::static_pointer_cast<IProperty>(vc);
            addToPropertyMap(path, response.uuid, ivc);
            return vc;
        }
        else
        {
            log << logger::ERROR << "VALUE CREATE REQUEST NOT OK TID: " << created.first;
        }
    }
    else
    {
        log << logger::ERROR << "VALUE CREATE REQUEST TIMEOUT TID: " << created.first;
    }
    return ValueContainerPtr();
}

NodeContainerPtr LocalPTree::createNode(const std::string& path)
{
    Buffer empty;
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Node, empty);
    if (transactionsCV.waitTransactionCV(created.first))
    {
        log << logger::DEBUG << "createNode resp has data of:";
        utils::printRaw(created.second->getBuffer().data(), created.second->getBuffer().size());
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        log << logger::DEBUG << "CreateResponse: " << response.toString();
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "NODE CREATED WITH UUID " << response.uuid;
            auto vc = std::make_shared<NodeContainer>(response.uuid, path, true);
            auto ivc = std::static_pointer_cast<IProperty>(vc);
            addToPropertyMap(path, response.uuid, ivc);
            return vc;
        }
        else
        {
            log << logger::ERROR << "NODE CREATE REQUEST NOT OK TID:" << created.first;
        }
    }
    else
    {
        log << logger::ERROR << "NODE CREATE REQUEST TIMEOUT TID:" << created.first;
    }
    return NodeContainerPtr();
}

RpcContainerPtr LocalPTree::createRpc(const std::string& path, std::function<Buffer(Buffer&)> handler, std::function<void(Buffer&)> voidHandler)
{
    Buffer empty;
    auto created  = outgoing.createRequest(path, protocol::PropertyType::Rpc, empty);

    if (transactionsCV.waitTransactionCV(created.first))
    {
        log << logger::DEBUG << "createRpc resp has data of:";
        utils::printRaw(created.second->getBuffer().data(), created.second->getBuffer().size());
        protocol::CreateResponse response;
        response.unpackFrom(created.second->getBuffer());
        log << logger::DEBUG << "CreateResponse: " << response.toString();
        if ( response.response  == protocol::CreateResponse::Response::OK)
        {
            log << logger::DEBUG << "RPC CREATED WITH UUID " << response.uuid;
            auto rc = std::make_shared<RpcContainer>(*this, response.uuid, path, handler, voidHandler, true);
            auto irc = std::static_pointer_cast<IProperty>(rc);
            addToPropertyMap(path, response.uuid, irc);
            return rc;
        }
        else
        {
            log << logger::ERROR << "RPC CREATE RPC NOT OK TID: " << created.first;
        }
    }
    else
    {
        log << logger::ERROR << "RPC CREATE RPC TIMEOUT TID: " << created.first;
    }
    return RpcContainerPtr();
}

void LocalPTree::fillValue(ValueContainerPtr& value)
{
    auto getValue = outgoing.getValue(value->getUuid());
    if (transactionsCV.waitTransactionCV(getValue.first))
    {
        protocol::GetValueResponse response;
        log << logger::DEBUG << "fillValue has data of:";
        utils::printRaw(getValue.second->getBuffer().data(), getValue.second->getBuffer().size());
        response.unpackFrom(getValue.second->getBuffer());
        log << logger::DEBUG << "getValueResponse: " << response.toString();
        if (response.data.size())
        {
            value->updateValue(std::move(response.data), false);
        }
    }
    else
    {
        log << logger::ERROR << "GET VALUE REQUEST TIMEOUT TID: " << getValue.first;
    }
}

IPropertyPtr LocalPTree::fetchMeta(const std::string& path)
{
    auto found = getPropertyByPath(path);
    if (found)
    {
        return found;
    }

    auto meta = outgoing.getSpecificMeta(path);
    if (transactionsCV.waitTransactionCV(meta.first))
    {
        log << logger::DEBUG << "getSpecificMeta resp has data of:";
        utils::printRaw(meta.second->getBuffer().data(), meta.second->getBuffer().size());
        protocol::GetSpecificMetaResponse response;
        response.unpackFrom(meta.second->getBuffer());
        log << logger::DEBUG << "GetSpecificMetaResponse: " << response.toString();
        if (response.meta.uuid != static_cast<protocol::Uuid>(-1))
        {
            switch (response.meta.propertyType)
            {
                case protocol::PropertyType::Value:
                {
                    log << logger::DEBUG << path << " is a Value";
                    auto value = std::make_shared<ValueContainer>(*this, response.meta.uuid, path, Buffer(), false);
                    auto ivc = std::static_pointer_cast<IProperty>(value);
                    addToPropertyMap(path, response.meta.uuid, ivc);
                    return value;
                }
                case protocol::PropertyType::Node:
                {
                    log << logger::DEBUG << path << " is a Node";
                    auto node = std::make_shared<NodeContainer>(response.meta.uuid, path, false);
                    auto inc = std::static_pointer_cast<IProperty>(node);
                    addToPropertyMap(path, response.meta.uuid, inc);
                    return node;
                }
                case protocol::PropertyType::Rpc:
                {
                    log << logger::DEBUG << path << " is an Rpc";
                    auto rpc = std::make_shared<RpcContainer>(*this, response.meta.uuid, path, false);
                    auto ipc = std::static_pointer_cast<IProperty>(rpc);
                    addToPropertyMap(path, response.meta.uuid, ipc);
                    return rpc;
                }
            }
        }
        else
        {
            log << logger::DEBUG << "META FOR: " << path << " IS NOT FOUND! TID: " << meta.first;
        }
    }
    else
    {
        log << logger::ERROR << "GET SPECIFIC META REQUEST TIMEOUT TID: " << meta.first;
    }
    return IPropertyPtr();
}

ValueContainerPtr LocalPTree::getValue(const std::string& path)
{
    auto value = std::dynamic_pointer_cast<ValueContainer>(fetchMeta(path));
    if (!value)
    {
        return value;
    }

    log << logger::DEBUG << "GetValue path: " << path;
    if (!value->isAutoUpdate()&&!value->isOwned())
    {
        log << logger::ERROR << "GetValue fetching.";
        fillValue(value);
    }
    return value;
}

RpcContainerPtr LocalPTree::getRpc(const std::string& path)
{
    auto rpc = std::dynamic_pointer_cast<RpcContainer>(fetchMeta(path));
    if (!rpc)
    {
        return rpc;
    }

    log << logger::DEBUG << "RPC FETCHED WITH UUID " << rpc->getUuid();
    return rpc;
}

bool LocalPTree::deleteProperty(IPropertyPtr& property)
{
    auto deleted  = outgoing.deleteRequest(property->getUuid());

    if (transactionsCV.waitTransactionCV(deleted.first))
    {
        log << logger::DEBUG << "deleteProperty resp has data of:";
        utils::printRaw(deleted.second->getBuffer().data(), deleted.second->getBuffer().size());
        protocol::DeleteResponse response;
        response.unpackFrom(deleted.second->getBuffer());
        log << logger::DEBUG << "DeleteResponse: " << response.toString();
        if ( response.response  == protocol::DeleteResponse::Response::OK)
        {
            log << logger::DEBUG << "PROPERTY WITH UUID " << property->getUuid() << " DELETED.";
            removeFromPropertyMap(property->getUuid());
            return true;
        }
        else
        {
            log << logger::ERROR << "DELETE NOT OK TID: " << deleted.first;;
        }
    }
    else
    {
        log << logger::ERROR << "DELETE TIMEOUT TID: " << deleted.first;;
    }
    return false;
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

bool LocalPTree::enableAutoUpdate(protocol::Uuid uuid)
{
    auto found = std::dynamic_pointer_cast<ValueContainer>(getPropertyByUuid(uuid));
    if (!found)
    {
        log << logger::ERROR << "Value is not on LocalPTree.";
        return false;
    }
    auto subscribe = outgoing.subscribePropertyUpdate(uuid);
    if (transactionsCV.waitTransactionCV(subscribe.first))
    {
        log << logger::DEBUG << "enableAutoUpdate resp has data of:";
        utils::printRaw(subscribe.second->getBuffer().data(), subscribe.second->getBuffer().size());
        protocol::SubscribePropertyUpdateResponse response;
        response.unpackFrom(subscribe.second->getBuffer());
        log << logger::DEBUG << "SubscribePropertyUpdateResponse: " << response.toString();
        if (response.response  == protocol::SubscribePropertyUpdateResponse::Response::OK)
        {
            log << logger::DEBUG << "SUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "SUBSCRIBE: PLEASE CHECK PATH IS CORRECT AND A VALUE. TID: " << subscribe.first;;
        }
    }
    else
    {
        log << logger::ERROR << "SUBSCRIBE REQUEST TIMEOUT TID: " << subscribe.first;;
    }
    return false;
}

bool LocalPTree::disableAutoUpdate(protocol::Uuid uuid)
{
    auto unsubscribe = outgoing.unsubscribePropertyUpdate(uuid);
    if (transactionsCV.waitTransactionCV(unsubscribe.first))
    {
        log << logger::DEBUG << "disableAutoUpdate resp has data of:";
        utils::printRaw(unsubscribe.second->getBuffer().data(), unsubscribe.second->getBuffer().size());
        protocol::UnsubscribePropertyUpdateResponse response;
        response.unpackFrom(unsubscribe.second->getBuffer());
        log << logger::DEBUG << "UnsubscribePropertyUpdateResponse: " << response.toString();
        if (response.response  == protocol::UnsubscribePropertyUpdateResponse::Response::OK)
        {
            log << logger::DEBUG << "UNSUBSCRIBED!! " << uuid;
            return true;
        }
        else
        {
            log << logger::ERROR << "UNSUBSCRIBE: PLEASE CHECK UUID IS CORRECT AND A VALUE." << unsubscribe.first;;
        }
    }
    else
    {
        log << logger::ERROR << "UNSUBSCRIBE REQUEST TIMEOUT" << unsubscribe.first;;
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

void LocalPTree::triggerMetaUpdateWatchersCreate(protocol::Uuid uuid, const std::string& path, protocol::PropertyType propertyType)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    // TODO HIGH PRIO: addToPropertyMap
    for (auto& i : metaUpdateHandlers.object)
    {
        i->handleCreation(uuid, path, propertyType);
    }
}

void LocalPTree::triggerMetaUpdateWatchersDelete(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> lock(metaUpdateHandlers.mutex);
    // TODO HIGH PRIO: removeToPropertyMap
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

void LocalPTree::setValue(protocol::Uuid uuid, Buffer&& value)
{
    outgoing.setValueIndication(uuid, std::move(value));
}

void LocalPTree::setValue(protocol::Uuid uuid, Buffer& value)
{
    outgoing.setValueIndication(uuid, value);
}

Buffer LocalPTree::rpcRequest(protocol::Uuid uuid, Buffer&& parameter)
{
    auto rpcRequest = outgoing.rpcRequest(uuid, std::move(parameter));
    if (transactionsCV.waitTransactionCV(rpcRequest.first))
    {
        log << logger::DEBUG << "rpcRequest resp has data of:";
        utils::printRaw(rpcRequest.second->getBuffer().data(), rpcRequest.second->getBuffer().size());
        protocol::RpcResponse response;
        response.unpackFrom(rpcRequest.second->getBuffer());
        log << logger::DEBUG << "RpcResponse: " << response.toString();
        return response.returnValue;
    }
    else
    {
        log << logger::ERROR << "RPC REQUEST TIMEOUT TID: " << rpcRequest.first;;
    }
    return Buffer();
}

}
}
