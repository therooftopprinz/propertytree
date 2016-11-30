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
    virtual ~IIdGenerator();
    virtual uint32_t getId();
    virtual void returnId(uint32_t id);
};

class IdGenerator
{
public:
    IdGenerator();
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
    virtual ~IProperty() = 0;
    void setOwner(void* ptr);
    void* getOwner();
    void setUuid(uint32_t uuid);
    protocol::Uuid getUuid();
protected:
    void* owner;
    std::string path;
    protocol::Uuid uuid;
};

class ValueSizeIncompatible {};
class Value : public IProperty, public std::enable_shared_from_this<Value>
{
public:
    Value()
    {
    }

    Value(const Value& val) = delete;

    ~Value();

    bool addWatcher(void* id, ValueWatcher& watcher);
    bool removeWatcher(void* id);
    
    template<class T>
    T& getValue()
    {
        if (value.size() != sizeof(T))
            throw ValueSizeIncompatible();
        return *(reinterpret_cast<T*>(value.data()));
    }

    template<class T>
    void setValue(T valueToSet)
    {
        value.resize(sizeof(T));
        *(reinterpret_cast<T*>(value.data())) = valueToSet;
        informValueWatcher();
    }

    void setValue(ValueContainer& valueToSet);
    void setValue(void* offset, uint32_t size);

    ValueContainer& getValue();

private:
    std::list<IdWatcherPair>::iterator findWatcher(void* id);
    void informValueWatcher();
    ValueContainer value;
    std::list<IdWatcherPair> watchers;
    std::mutex watchersMutex;
};

class Rpc : public IProperty, public std::enable_shared_from_this<Rpc>
{
public:
    Rpc();
    ~Rpc();

    void setWatcher(RpcWatcher& handler);
    void operator()(uint64_t csid, uint32_t tid, server::ClientServerWkPtr cswkptr, ValueContainer&& parameter);

private:
    RpcWatcher watcher;
};

class Node : public IProperty, public std::enable_shared_from_this<Node>
{
public:
    Node():
     properties(std::make_shared<PropertyMap>())
    {
    }

    void deleteProperty(std::string name);

    template<class T>
    std::shared_ptr<T> getProperty(std::string name)
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
    std::shared_ptr<T> createProperty(std::string name)
    {
        std::lock_guard<std::mutex> guard(propertiesMutex);
        if (properties->find(name) == properties->end())
        {
            auto rv = std::make_shared<T>();
            (*properties)[name] = std::dynamic_pointer_cast<IProperty>(rv);
            return rv;
        }
        log << logger::ERROR << "Object already exist: " << name;
        throw ObjectExisting();
    }

    std::shared_ptr<const PropertyMap> getProperties();
    uint32_t numberOfChildren();
private:
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
    NodePtr getNodeByPath(std::string path);

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
    std::pair<uint32_t, std::shared_ptr<T>> createProperty(std::string path)
    {
        auto parentChild = utils::getParentAndChildNames(path);

        log << logger::DEBUG << "Creating: " << parentChild.second << " under " <<
            parentChild.first;

        NodePtr p = getNodeByPath(parentChild.first);

        std::lock_guard<std::mutex> treeguard(ptree);
        auto rv = p->createProperty<T>(parentChild.second);

        uint32_t id = idgen->getId();
        log << logger::DEBUG << "uuid:" << id;
        std::lock_guard<std::mutex> guard(uuidpropMutex);
        uuids[id] = rv;
        props[rv] = id;
        rv->setUuid(id);
        return std::make_pair(id, rv);
    }

    template<class T>
    std::shared_ptr<T> getPropertyByPath(std::string path)
    {
        auto parentChild = utils::getParentAndChildNames(path);

        NodePtr p = getNodeByPath(parentChild.first);
        
        return p->getProperty<T>(parentChild.second);
    }

    void getPTreeInfo();
    uint32_t deleteProperty(std::string path);
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