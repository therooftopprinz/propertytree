#ifndef __VALUE_SERVER_HPP__
#define __VALUE_SERVER_HPP__

#include <map>
#include <set>
#include <stdexcept>

#include <propertytree/protocol_udp.hpp>
#include <bfc/configuration_parser.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/socket.hpp>
#include <bfc/buffer.hpp>

#include <value_map.hpp>

namespace propertytree
{

struct udp_client_context : client_context
{
    bfc::socket* server_sock;
    sockaddr client_address;

    udp_client_context(bfc::socket* s, sockaddr c)
        : server_sock(s)
        , client_address(c)
    {}

    int send(const bfc::const_buffer_view& b) override
    {
        return server_sock->send(b, 0, &client_address, sizeof(client_address));
    }
};

class udp_value_server
{
public:
    using reactor_t = bfc::epoll_reactor<std::function<void()>>;
    udp_value_server(const bfc::configuration_parser*, reactor_t*, value_map*);

private:
    using cctx_ptr = std::shared_ptr<udp_client_context>;

    void read();

    void handle_set         (cctx_ptr&, const header_s&, const key_s&, const bfc::const_buffer_view&);
    void handle_get         (cctx_ptr&, const header_s&, const key_s&);
    void handle_subscribe   (cctx_ptr&, const header_s&, const key_s&);
    void handle_unsubscribe (cctx_ptr&, const header_s&, const key_s&);
    void handle             (cctx_ptr&, const bfc::const_buffer_view&);

    const bfc::configuration_parser* m_config;
    reactor_t* m_reactor;
    value_map* m_value_map;

    std::map<std::pair<uint32_t,uint16_t>, cctx_ptr> m_client_map;

    bfc::socket m_server_socket;
    reactor_t::context m_server_rctx;

    std::byte m_rcv_buff[1024*64];
    std::byte m_snd_buff[1024*64];
};

} // propertytree

#endif // __VALUE_SERVER_HPP__
