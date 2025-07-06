#ifndef __TCP_VALUE_SERVER_HPP__
#define __TCP_VALUE_SERVER_HPP__

#include <stdexcept>
#include <map>
#include <set>

#include <bfc/configuration_parser.hpp>
#include <propertytree/protocol_ng.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>

#include <value_map.hpp>

namespace propertytree
{

template <typename reactor_t = bfc::epoll_reactor<std::function<void()>>>
struct tcp_client_context : client_context
{
    bfc::socket client_socket;
    typename reactor_t::context client_rctx;
    sockaddr client_address;

    std::byte read_buffer[512];
    uint16_t read_buffer_idx = 0;
    enum read_state_e {WAIT_HEADER, WAIT_REMAINING};
    read_state_e read_state = WAIT_HEADER;
    size_t expected_read_size = 2;

    tcp_client_context(bfc::socket&& s, typename reactor_t::context x, sockaddr c)
        : client_socket(std::move(s))
        , client_rctx(std::move(x))
        , client_address(c)
    {}

    int send(const bfc::const_buffer_view& b) override
    {
        return client_socket.send(b);
    }
};

class tcp_value_server
{
public:
    static constexpr size_t ENCODE_SIZE = 1024*64;
    using reactor_t = bfc::epoll_reactor<std::function<void()>>;
    tcp_value_server(const bfc::configuration_parser*, reactor_t*, value_map*);

private:
    using client_context = tcp_client_context<reactor_t>;
    using client_context_ptr = std::shared_ptr<client_context>;

    void on_accept_ready();

    size_t encode(int, const cum::protocol_value_client& msg, std::byte* data, size_t size);

    value& get_value(uint64_t);

    void handle(std::shared_ptr<client_context>&, cum::subscribe&&);
    void handle(std::shared_ptr<client_context>&, cum::unsubscribe&&);

    void read_client(std::shared_ptr<client_context>&);
    void disconnect_client(std::shared_ptr<client_context>&);

    const bfc::configuration_parser* m_config;
    reactor_t* m_reactor;
    value_map* m_value_map;

    bfc::socket m_server_socket;
    reactor_t::context m_server_rctx;

    std::map<int, std::shared_ptr<client_context>> m_client_map;

    uint64_t value_sequence = 0;
};

} // propertytree

#endif // __TCP_VALUE_SERVER_HPP__
