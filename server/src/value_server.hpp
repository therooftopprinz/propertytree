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
    using reactor_t = bfc::epoll_reactor<std::function<void()>>;
    value_server(const bfc::configuration_parser&, reactor_t&);
private:
    struct client_context
    {
        bfc::socket client_socket;
        reactor_t::context client_rctx;
        sockaddr client_address;

        std::byte read_buffer[512];
        uint16_t read_buffer_idx = 0;
        enum read_state_e {WAIT_HEADER, WAIT_REMAINING};
        read_state_e read_state = WAIT_HEADER;
        size_t expected_read_size = 0;
    };

    using client_context_ptr = std::shared_ptr<client_context>;

    struct value
    {
        bfc::buffer value;
        std::vector<client_context_ptr> subscribers; 
    };

    void on_accept_ready();

    void handle(std::shared_ptr<client_context>, cum::allocate_request&&);
    void handle(std::shared_ptr<client_context>, cum::set_value&&);
    void handle(std::shared_ptr<client_context>, cum::get_value_request&&);
    void handle(std::shared_ptr<client_context>, cum::subscribe&&);
    void handle(std::shared_ptr<client_context>, cum::unsubscribe&&);

    void handle(std::shared_ptr<client_context>, cum::allocate_response&&)      {}
    void handle(std::shared_ptr<client_context>, cum::get_value_response&&)     {}
    void handle(std::shared_ptr<client_context>, cum::update&&)                 {}

    void read_client(std::shared_ptr<client_context>&);
    void disconnect_client(std::shared_ptr<client_context>&);

    const bfc::configuration_parser& m_config;
    reactor_t& m_reactor;
    bfc::socket m_server_socket;
    reactor_t::context m_server_rctx;

    std::map<int, std::shared_ptr<client_context>> m_client_map;
};

} // propertytree

#endif // __VALUE_SERVER_HPP__
