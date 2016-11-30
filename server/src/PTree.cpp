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

IProperty::~IProperty()
{
    
}

void IProperty::setOwner(void* ptr)
{
    owner = ptr;
}

void* IProperty::getOwner()
{
    return owner;
}

void IProperty::setUuid(uint32_t uuid)
{
    this->uuid = uuid;
}

uint32_t IProperty::getUuid()
{
    return uuid;
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

uint32_t Node::numberOfChildren()
{
    return properties->size();
}

void Node::deleteProperty(std::string name)
{
    std::lock_guard<std::mutex> guard(propertiesMutex);
    auto found = properties->find(name);
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
                log << logger::ERROR << "Delete: Node not empty:" << name;
                throw NotEmpty();
            }
        }
    }
    else
    {
        log << logger::ERROR << "Delete: Node not found:" << name;
        throw ObjectNotFound();
    }
}

logger::Logger Node::log = logger::Logger("Node");

std::shared_ptr<const PropertyMap> Node::getProperties()
{
    return properties;
}

Rpc::Rpc()
{
}

Rpc::~Rpc()
{
}

void Rpc::setWatcher(RpcWatcher& watcher)
{
    this->watcher = watcher;
}

void Rpc::operator()(uint64_t csid, uint32_t tid, server::ClientServerWkPtr cswkptr, ValueContainer&& parameter)
{
    watcher(csid, tid, cswkptr, std::move(parameter));
}

PTree::PTree(IIdGeneratorPtr idgen) :
    root(std::make_shared<Node>()),
    idgen(idgen),
    log("PTree")
{
}

NodePtr PTree::getNodeByPath(std::string path)
{
    log << logger::DEBUG << "Get Node:" << path;

    if (path == "/")
    {
        return root;
    }

    char tname[path.length()];
    uint32_t cursor = 1;
    uint32_t curTname = 0;
    uint32_t nAncestors = -1;
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


void PTree::getPTreeInfo()
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
                // add node to list;
                backTrack.emplace_back(std::get<0>(current) +"/"+ std::get<2>(current)->first, next, next->getProperties()->begin());
                log << logger::DEBUG << "FOUND NODE AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;

                std::get<2>(current)++;

                // for (const auto& i : backTrack)
                // {
                //     log << logger::DEBUG << "BACKTRACT " << std::get<0>(i) << " index_pos " <<  std::distance(
                //         std::get<1>(i)->getProperties()->begin(), std::get<2>(i));
                // }

                enterANode = true;
                break;
            }
            else if (auto next = std::dynamic_pointer_cast<Value>(std::get<2>(current)->second)) // a Value
            {
                // add value to list;
                log << logger::DEBUG << "FOUND VALUE AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;
                std::get<2>(current)++;
            }
            else if (auto next = std::dynamic_pointer_cast<Rpc>(std::get<2>(current)->second)) // an Rpc
            {
                // add rpc to list;
                log << logger::DEBUG << "FOUND RPC AT " << std::get<0>(current) +"/"+ std::get<2>(current)->first;
                std::get<2>(current)++;
            }
        }
        if (enterANode) continue;
        // Done iterating the Node
        backTrack.pop_back();
    }
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

IdGenerator::IdGenerator():
    base(100),
    log("IdGenerator")
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