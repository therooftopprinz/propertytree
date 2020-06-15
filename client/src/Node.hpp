#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <map>

#include <bfc/EpollReactor.hpp>

#include <logless/Logger.hpp>

namespace propertytree
{

struct Node
{
    Node() = delete;
    Node(const std::string& pName, std::weak_ptr<Node> pParent, uint64_t pUuid)
        : name(pName)
        , parent(pParent)
        , uuid(pUuid)
    {}

    std::string name;

    std::vector<uint8_t> data;
    std::map<std::string, std::shared_ptr<Node>> children;
    std::function<std::vector<uint8_t>(const bfc::BufferView&)> rcpHandler;
    std::function<void()> updateHandler;
 
    std::mutex dataMutex;
    std::mutex childrenMutex;
    std::mutex rcpHandlerMutex;
    std::mutex updateHandlerMutex;

    std::weak_ptr<Node> parent;
    uint64_t uuid;
};

} // propertytree

#endif // __NODE_HPP__