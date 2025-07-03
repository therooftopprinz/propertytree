#ifndef __VALUE_SERVER_HPP__
#define __VALUE_SERVER_HPP__

#include <map>
#include <set>
#include <stdexcept>

#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>
#include <bfc/socket.hpp>
#include <bfc/buffer.hpp>
#include <propertytree/protocol_udp.hpp>

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
        sockaddr client_address;
    };

    using cctx_ptr = std::shared_ptr<client_context>;

    struct value
    {
        std::vector<uint8_t> data;
        std::set<cctx_ptr> subscribers;
        int64_t vsn = 0;
        int64_t ssn = 0;
    };

    void read();

    void handle_set         (cctx_ptr&, const header_s&, const key_s&, const bfc::const_buffer_view&);
    void handle_get         (cctx_ptr&, const header_s&, const key_s&);
    void handle_subscribe   (cctx_ptr&, const header_s&, const key_s&);
    void handle_unsubscribe (cctx_ptr&, const header_s&, const key_s&);
    void handle             (cctx_ptr&, const bfc::const_buffer_view&);

    value& get_value(uint16_t);
    bool set_value(uint16_t, const bfc::const_buffer_view&);

    const bfc::configuration_parser& m_config;
    reactor_t& m_reactor;
    bfc::socket m_server_socket;
    reactor_t::context m_server_rctx;

    std::map<std::pair<uint32_t, uint16_t>, cctx_ptr> m_client_map;
    std::vector<value> m_value_map;

    std::byte m_rcv_buff[1024*64];
    std::byte m_snd_buff[1024*64];
};

} // propertytree

#endif // __VALUE_SERVER_HPP__
