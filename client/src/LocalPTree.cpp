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

// RpcContainerPtr LocalPTree::createRpc(std::string, std::function<Buffer(Buffer&)>, std::function<void(Buffer&)>)
// {
// }

// bool LocalPTree::createNode(std::string)
// {
// }


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

// RpcContainerPtr LocalPTree::getRpc(std::string&)
// {
// }

}
}
