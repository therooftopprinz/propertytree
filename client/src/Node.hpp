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
    Node(std::weak_ptr<Node> pParent, uint64_t pUuid)
        : parent(pParent)
        , uuid(pUuid)
    {}

    bfc::Buffer data;
    std::map<std::string, std::shared_ptr<Node>> children;
    std::mutex mutex;

    std::weak_ptr<Node> parent;
    uint64_t uuid;
};

} // propertytree

#endif // __NODE_HPP__