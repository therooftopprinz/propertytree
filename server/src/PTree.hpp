#ifndef SERVER_PTREE_HPP_
#define SERVER_PTREE_HPP_

#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <list>
#include <tuple>
#include <interface/protocol.hpp>
#include <common/src/Logger.hpp>
#include <common/src/Utils.hpp>
#include "Types.hpp"


namespace ptree
{
namespace core
{

class ObjectNotFound {};
class ObjectExisting {};
class MalformedPath {};
class NotEmpty {};

class IIdGenerator
{
public:
    IIdGenerator() = default;
    virtual ~IIdGenerator() = default;
    virtual uint32_t getId() = 0;
    virtual void returnId(uint32_t id) = 0;
};

/*** TODO: mutex on list of ids ***/
class IdGenerator : public IIdGenerator
{
public:
    IdGenerator();
    ~IdGenerator();
    uint32_t getId();
    void returnId(uint32_t id);
private:
    uint32_t base;
    std::list<uint32_t> ids;
    logger::Logger log;
};

class IProperty
{
public:
    IProperty(protocol::Uuid uuid, NodeWkPtr&& parent);
    virtual ~IProperty() = default;
    /**TODO: implement ownership**/
    protocol::Uuid getUuid();
    NodeWkPtr& getParent();
protected:
    protocol::Uuid uuid;
    NodeWkPtr parent;
};

class ValueSizeIncompatible {};
class Value : public IProperty, public std::enable_shared_from_this<Value>
{
public:
    Value(protocol::Uuid, NodeWkPtr parent);

    Value(const Value& val) = delete;

    ~Value();

    bool addWatcher(uint64_t csid, ValueWatcher& watcher);
    bool removeWatcher(uint64_t id);

    void setValue(const ValueContainer& valueToSet);
    void setValue(const void* offset, uint32_t size);

    ValueContainer& getValue();

private:
    std::list<IdWatcherPair>::iterator findWatcher(uint64_t id);
    void informValueWatcher();
    ValueContainer value;
    std::list<IdWatcherPair> watchers;
    std::mutex watchersMutex;
};

class Rpc : public IProperty, public std::enable_shared_from_this<Rpc>
{
public:
    Rpc(protocol::Uuid uuid, NodeWkPtr parent);
    ~Rpc();

    void setWatcher(RpcWatcher& handler);
    void operator()(uint64_t csid, uint32_t tid, ValueContainer&& parameter);

private:
    RpcWatcher watcher;
};

class Node : public IProperty, public std::enable_shared_from_this<Node>
{
public:
    Node(protocol::Uuid uuid, NodeWkPtr parent);

    void deleteProperty(std::string name);
    void deleteProperty(protocol::Uuid uuid);

    template<class T>
    std::shared_ptr<T> getProperty(const std::string& name)
    {
        std::lock_guard<std::mutex> guard(propertiesMutex);
        if (properties->find(name) != properties->end())
        {
            return std::dynamic_pointer_cast<T>((*properties)[name]);
        }
        log << logger::ERROR << "Object not found!: " << name;
        throw ObjectNotFound();
    }

    template<class T>
    std::shared_ptr<T> createProperty(const std::string& name, protocol::Uuid uuid)
    {
        std::lock_guard<std::mutex> guard(propertiesMutex);
        if (properties->find(name) == properties->end())
        {
            auto rv = std::make_shared<T>(uuid, shared_from_this());
            (*properties)[name] = std::dynamic_pointer_cast<IProperty>(rv);
            return rv;
        }
        log << logger::ERROR << "Object already exist: " << name;
        throw ObjectExisting();
    }

    std::shared_ptr<const PropertyMap> getProperties();
    uint32_t numberOfChildren();
private:
    void deleteProperty(PropertyMap::iterator found);

    PropertyMapPtr properties;
    std::mutex propertiesMutex;
    static logger::Logger log;
};

typedef std::map<protocol::Uuid, IPropertyPtr> UuidPropertyMap;
typedef std::map<IPropertyPtr, protocol::Uuid> PropertyUuidMap;

class PTree
{
public:
    PTree() = delete;
    PTree(IIdGeneratorPtr idgen);
    NodePtr getNodeByPath(const std::string& path);

    template<class T>
    std::shared_ptr<T> getPropertyByUuid(protocol::Uuid uuid)
    {
        std::lock_guard<std::mutex> guard(uuidpropMutex);
        auto it = uuids.find(uuid);
        if (it == uuids.end())
        {
            throw ObjectNotFound();
        }
        return std::dynamic_pointer_cast<T>(it->second);
    }

    template<class T>
    std::pair<uint32_t, std::shared_ptr<T>> createProperty(const std::string& path)
    {
        auto parentChild = utils::getParentAndChildNames(path);

        log << logger::DEBUG << "Creating: " << parentChild.second << " under " <<
            parentChild.first;

        NodePtr p = getNodeByPath(parentChild.first);

        std::lock_guard<std::mutex> treeguard(ptree);
        uint32_t id = idgen->getId();
        auto rv = p->createProperty<T>(parentChild.second, id);

        log << logger::DEBUG << "uuid:" << id;
        std::lock_guard<std::mutex> guard(uuidpropMutex);
        uuids[id] = rv;
        props[rv] = id;

        return std::make_pair(id, rv);
    }

    template<class T>
    std::shared_ptr<T> getPropertyByPath(const std::string& path)
    {
        auto parentChild = utils::getParentAndChildNames(path);

        NodePtr p = getNodeByPath(parentChild.first);
        
        return p->getProperty<T>(parentChild.second);
    }

    std::list<std::tuple<std::string, protocol::Uuid, protocol::PropertyType>> getPTreeInfo();
    uint32_t deleteProperty(std::string path);
    bool deleteProperty(protocol::Uuid uuid);
private:
    NodePtr root;
    UuidPropertyMap uuids;
    PropertyUuidMap props;
    std::mutex uuidpropMutex;
    std::mutex ptree;
    IIdGeneratorPtr idgen;
    logger::Logger log;
};

} // namespace core
} // namspace ptree

#endif  // SERVER_PTREE_HPP_