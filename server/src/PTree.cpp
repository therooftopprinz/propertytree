#include <iostream>
#include <utility>
#include <cstring>
#include <algorithm>
#include "Types.hpp"
#include "PTree.hpp"
#include <common/src/Utils.hpp>

namespace ptree
{
namespace core
{

IProperty::IProperty(protocol::Uuid uuid, NodeWkPtr&& parent):
    uuid(uuid), parent(std::move(parent))
{
}
uint32_t IProperty::getUuid()
{
    return uuid;
}

NodeWkPtr& IProperty::getParent()
{
    return parent;
}

Value::Value(protocol::Uuid uuid, NodeWkPtr parent):
    IProperty(uuid, std::move(parent))
{
}

Value::~Value()
{
}

std::list<IdWatcherPair>::iterator Value::findWatcher(void* id)
{
    auto isFnEq = [&id](IdWatcherPair& tocompare)
    {
        return tocompare.first == id;
    };
    return std::find_if(watchers.begin(), watchers.end(), isFnEq);
}

bool Value::addWatcher(void* id, ValueWatcher& watcher)
{
    std::lock_guard<std::mutex> guard(watchersMutex);
    if (findWatcher(id) != watchers.end())
    {
        return false;
    }
    else
    {
        watchers.push_back(std::make_pair(id, watcher));
        return true;
    }
}

void Value::informValueWatcher()
{
    for (auto i = watchers.begin(); i != watchers.end(); i++)
    {
        /** Call watcher wrapper (UpdateNotificationHandler) for client server.
            False is when ClientServer is not running anymore.**/
        bool rv = i->second(shared_from_this());
        if (!rv)
        {
            watchers.erase(i);
        }
    }
}

bool Value::removeWatcher(void* id)
{
    std::lock_guard<std::mutex> guard(watchersMutex);
    auto found = findWatcher(id);
    if (found != watchers.end())
    {
        watchers.erase(found);
        return true;
    }
    else
    {
        return false;
    }
}

void Value::setValue(ValueContainer& valueToSet)
{
    value = valueToSet;
    informValueWatcher();
}

void Value::setValue(void* offset, uint32_t size)
{
    value = ValueContainer(size);
    std::memcpy(value.data(), offset, size);
    informValueWatcher();
}

ValueContainer& Value::getValue()
{
    return value;
}

Node::Node(protocol::Uuid uuid, NodeWkPtr parent):
    IProperty(uuid, std::move(parent)), properties(std::make_shared<PropertyMap>())
{
}

uint32_t Node::numberOfChildren()
{
    return properties->size();
}

void Node::deleteProperty(PropertyMap::iterator found)
{
    if (found != properties->end())
    {
        auto asNode = std::dynamic_pointer_cast<Node>(found->second);
        auto asValue = std::dynamic_pointer_cast<Value>(found->second);

        if (asValue)
        {
            properties->erase(found);
        }
        else if (asNode)
        {
            if (!asNode->numberOfChildren())
            {
                properties->erase(found);
            }
            else
            {
                log << logger::ERROR << "Delete: Node not empty!";
                throw NotEmpty();
            }
        }
    }
    else
    {
        log << logger::ERROR << "Delete: Not found!";
        throw ObjectNotFound();
    }
}

void Node::deleteProperty(std::string name)
{
    log << logger::ERROR << "Deleting: " << name;
    std::lock_guard<std::mutex> guard(propertiesMutex);
    auto found = properties->find(name);
    deleteProperty(found);
}

void Node::deleteProperty(protocol::Uuid uuid)
{
    log << logger::ERROR << "Deleting: " << uuid;
    std::lock_guard<std::mutex> guard(propertiesMutex);
    auto found = std::find_if(properties->begin(), properties->end(), [&uuid](const auto& i) {
            return i.second->getUuid() == uuid;
        });
    deleteProperty(found);
}

logger::Logger Node::log = logger::Logger("Node");

std::shared_ptr<const PropertyMap> Node::getProperties()
{
    return properties;
}

Rpc::Rpc(protocol::Uuid uuid, NodeWkPtr parent):
    IProperty(uuid, std::move(parent))
{
}

Rpc::~Rpc()
{
}

void Rpc::setWatcher(RpcWatcher& watcher)
{
    this->watcher = watcher;
}

void Rpc::operator()(uint64_t csid, uint32_t tid, ValueContainer&& parameter)
{
    watcher(csid, tid, std::move(parameter));
}

PTree::PTree(IIdGeneratorPtr idgen) :
    idgen(idgen),
    log("PTree")
{
    root = std::make_shared<Node>(idgen->getId(), NodePtr());
}

NodePtr PTree::getNodeByPath(const std::string& path)
{
    log << logger::DEBUG << "Get Node:" << path;

    if (path == "/")
    {
        return root;
    }

    char tname[path.length()];
    uint32_t cursor = 1;
    uint32_t curTname = 0;
    int32_t nAncestors = -1;
    NodePtr currentNode = root;

    for (auto c : path)
    {
        if (c == '/')
        {
            nAncestors++;
        }
    }

    if (nAncestors < 0)
    {
        log << logger::ERROR << "Mailformed path: " << path;
        throw MalformedPath();
    }

    while (true)
    {
        while (cursor < path.length())
        {
            char t = path[cursor++];
            if (t == '/')
            {
                break;
            }

            tname[curTname++] = t;
        }

        tname[curTname] = 0;
        log << logger::DEBUG << "get["<< curTname <<"]: " << tname;
        if (curTname <= 1)
        {
            log << logger::ERROR << "Malformed path: " << path;
            throw MalformedPath();
        }
        
        curTname = 0;
        currentNode = currentNode->getProperty<Node>(tname);
        
        if (!currentNode)
        {
            log << logger::ERROR << "Node not found(1): " << path;
            throw ObjectNotFound();
        }

        if (nAncestors-- == 0)
        {
            return currentNode;
        }
    }

    log << logger::ERROR << "Node not found(2): " << path;
    throw ObjectNotFound();
}


std::list<std::tuple<std::string, protocol::Uuid, protocol::PropertyType>> PTree::getPTreeInfo()
{
    std::lock_guard<std::mutex> guard(ptree);
    std::list<std::tuple<std::string, protocol::Uuid, protocol::PropertyType>> infolist;
    std::list<std::tuple<std::string, NodePtr, PropertyMap::const_iterator>> backTrack;
    backTrack.emplace_back("", root, root->getProperties()->begin());

    while(backTrack.begin() != backTrack.end())
    {
        bool enterANode = false;
        auto& current = backTrack.back();
        while (std::get<2>(current) != std::get<1>(current)->getProperties()->end())
        {
            // log << logger::DEBUG << "CURRENT " << std::get<0>(current) << " index " <<  std::distance(
            //         std::get<1>(current)->getProperties()->begin(), std::get<2>(current));
            if (auto next = std::dynamic_pointer_cast<Node>(std::get<2>(current)->second)) // a Node
            {
                backTrack.emplace_back(
                    std::get<0>(current) +"/"+ std::get<2>(current)->first,
                    next, next->getProperties()->begin());
                // log << logger::DEBUG << "FOUND NODE AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;
                infolist.emplace_back(
                    std::get<0>(current) +"/"+ std::get<2>(current)->first,
                    next->getUuid(),
                    protocol::PropertyType::Node);
                std::get<2>(current)++;

                enterANode = true;
                break;
            }
            else if (auto next = std::dynamic_pointer_cast<Value>(std::get<2>(current)->second)) // a Value
            {
                // log << logger::DEBUG << "FOUND VALUE AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;
                infolist.emplace_back(
                    std::get<0>(current) +"/"+ std::get<2>(current)->first,
                    next->getUuid(),
                    protocol::PropertyType::Value);
                std::get<2>(current)++;
            }
            else if (auto next = std::dynamic_pointer_cast<Rpc>(std::get<2>(current)->second)) // an Rpc
            {
                // add rpc to list;
                // log << logger::DEBUG << "FOUND RPC AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;
                infolist.emplace_back(
                    std::get<0>(current) +"/"+ std::get<2>(current)->first,
                    next->getUuid(),
                    protocol::PropertyType::Rpc);
                std::get<2>(current)++;
            }
        }
        if (enterANode) continue;
        // Done iterating the Node
        backTrack.pop_back();
    }
    return infolist;
}


uint32_t PTree::deleteProperty(std::string path)
{
    std::lock_guard<std::mutex> treeguard(ptree);
    log << logger::DEBUG << "PTree deleting: " << path;
    auto names = utils::getParentAndChildNames(path);
    NodePtr parentNode = getNodeByPath(names.first);

    auto property = getPropertyByPath<IProperty>(path);
    uint32_t uuid = props[property];
    log << logger::DEBUG << "uuid: " << uuid;

    parentNode->deleteProperty(names.second);

    std::lock_guard<std::mutex> guard(uuidpropMutex);
    auto deleteItProp = props.find(property);
    auto deleteItUuid = uuids.find(uuid);

    props.erase(deleteItProp);
    uuids.erase(deleteItUuid);

    return uuid;
}

bool PTree::deleteProperty(protocol::Uuid uuid)
{
    std::lock_guard<std::mutex> treeguard(ptree);
    log << logger::DEBUG << "PTree deleting: " << uuid;

    std::lock_guard<std::mutex> guard(uuidpropMutex);
    auto deleteItProp = uuids.find(uuid);
    if (deleteItProp != uuids.end())
    {
        // NOTE: should not happen to be null
        auto parent = deleteItProp->second->getParent().lock();
        parent->deleteProperty(uuid);
    }
    return false;
}

IdGenerator::IdGenerator():
    base(99),
    log("IdGenerator")
{
}

IdGenerator::~IdGenerator()
{
}


uint32_t IdGenerator::getId()
{
    if (ids.size() == 0)
    {
        return base++;
    }

    auto rv = *(ids.end()--);
    ids.pop_back();
    return rv;
}

void IdGenerator::returnId(uint32_t id)
{
    return ids.push_back(id);
}


} // namespace core
} // namspace ptree