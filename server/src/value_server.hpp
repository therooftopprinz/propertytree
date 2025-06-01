#ifndef __VALUE_SERVER_HPP__
#define __VALUE_SERVER_HPP__

#include <map>
#include <stdexcept>

#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>
#include <protocol_ng.hpp>

namespace propertytree
{

class value_server
{
public:
    using reactor_t = bfc::epoll_reactor<>;
    value_server(bfc::configuration_parser& config, reactor_t& reactor);
private:
    struct client_context
    {
        bfc::socket client;
        sockaddr client_address;
        reactor_t::context client_context;
    };

    using client_context_ptr = std::shared_ptr<client_context>;

    struct value
    {
        bfc::buffer value;
        std::vector<client_context_ptr> subscribers; 
    };

    void on_accept_ready();

    void handle(cum::set_value& msg);
    void handle(cum::get_value_request& msg);
    void handle(cum::subscribe& msg);
    void handle(cum::unsubscribe& msg);

    const bfc::configuration_parser& config;
    reactor_t& reactor;
    bfc::socket server;
    std::optional<reactor_t::context> server_ctx;
};

} // propertytree

#endif // __VALUE_SERVER_HPP__
