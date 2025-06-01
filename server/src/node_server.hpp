#ifndef __NODE_SERVER_HPP__
#define __NODE_SERVER_HPP___

#include <map>
#include <stdexcept>

#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>

namespace propertytree
{

class node_server
{
public:
    node_server(const bfc::configuration_parser& config, bfc::epoll_reactor<>& reactor);
private:
};

} // propertytree

#endif // __NODE_SERVER_HPP__
