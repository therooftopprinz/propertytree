#ifndef __VALUE_SERVER_HPP__
#define __VALUE_SERVER_HPP__

#include <map>
#include <set>
#include <stdexcept>

#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>
#include <bfc/socket.hpp>
#include <bfc/socket.hpp>
#include <propertytree/protocol_ng.hpp>

namespace propertytree
{

class value_server
{
public:
    static constexpr size_t ENCODE_SIZE = 1024*64;
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
        size_t expected_read_size = 2;
    };

    using client_context_ptr = std::shared_ptr<client_context>;

    struct value
    {
        std::vector<std::byte> data;
        std::set<client_context_ptr> subscribers;
        uint64_t sequence_number = 0;
    };

    void on_accept_ready();

    size_t encode(int, const cum::protocol_value_client& msg, std::byte* data, size_t size);

    value& get_value(uint32_t);
    void set_value(uint32_t, cum::bufferX&&);

    void send_ack(std::shared_ptr<client_context>& client, uint16_t transaction_id, cum::EStatus status);

    void handle(std::shared_ptr<client_context>&, cum::set_value&&);
    void handle(std::shared_ptr<client_context>&, cum::get_value_request&&);
    void handle(std::shared_ptr<client_context>&, cum::subscribe&&);
    void handle(std::shared_ptr<client_context>&, cum::unsubscribe&&);

    void read_client(std::shared_ptr<client_context>&);
    void disconnect_client(std::shared_ptr<client_context>&);

    const bfc::configuration_parser& m_config;
    reactor_t& m_reactor;
    bfc::socket m_server_socket;
    reactor_t::context m_server_rctx;

    std::map<int, std::shared_ptr<client_context>> m_client_map;
    std::vector<value> value_map;

    std::byte send_buffer[ENCODE_SIZE];
};

} // propertytree

#endif // __VALUE_SERVER_HPP__
