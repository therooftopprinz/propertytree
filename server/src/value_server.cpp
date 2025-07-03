#include <value_server.hpp>
#include <logger.hpp>
#include <utils.hpp>

namespace propertytree
{

template <typename T=std::chrono::microseconds>
static uint64_t now()
{
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

value_server::value_server(
    const bfc::configuration_parser& config,
    reactor_t& reactor)
    : m_config(config)
    , m_reactor(reactor)
    , m_server_socket(socket(AF_INET, SOCK_DGRAM, 0))
{
    LOG_INF("value_server | construct %p;", this);

    auto port      = utils::get_config<unsigned>(   config, "service.value.port").value();
    auto interface = utils::get_config<std::string>(config, "service.value.interface").value_or(std::string("127.0.0.1"));

    auto bind_addr = bfc::ip4_port_to_sockaddr(interface, port);

    LOG_INF("value_server | bind_address=%s;", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str());
    m_server_socket.set_sock_opt(SOL_SOCKET, SO_REUSEADDR, 1);

    if (0 > m_server_socket.bind(bind_addr))
    {
        LOG_ERR("value_server | bind_address=%s; error='%s;'", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str(), strerror(errno));
        return;
    }

    m_server_rctx = m_reactor.make_context(m_server_socket.fd());
    m_reactor.add_read_rdy(m_server_rctx, [this](){
            read();
        });
}

void value_server::read()
{
    sockaddr client_addr;
    socklen_t client_addr_sz;

    // @todo: bfc latest can be rvalue in recv
    bfc::buffer_view bv(m_rcv_buff, sizeof(m_rcv_buff));
    auto rs = m_server_socket.recv(bv, 0, &client_addr, &client_addr_sz);

    if (rs < 0)
    {
        return;
    }

    auto addr = utils::get_addr(&client_addr);
    auto client_it = m_client_map.find(addr);
    cctx_ptr ctx;
    if (m_client_map.end() == client_it)
    {
        ctx = std::make_shared<client_context>();
        ctx->client_address = client_addr;
        m_client_map.emplace(addr, ctx);
    }

    handle(ctx, bfc::const_buffer_view(m_rcv_buff, rs));
}

void value_server::handle_set(cctx_ptr& c, const header_s& h, const key_s& msg, const bfc::const_buffer_view& data)
{
    if (h.flags & HEADER_FLAG_TRANSACTIONAL)
    {
        auto hdr = (header_s*)(m_snd_buff);
        auto ack = next_struct<acknowledge_s>(hdr);
        hdr->flags = HEADER_FLAG_TRANSACTIONAL;
        hdr->type = E_MSGT_ACKNOWLEDGE;
        hdr->transaction = h.transaction;
        ack->status = E_STATUS_OK;
        if (!set_value(msg.key, data))
        {
            ack->status = E_STATUS_DATA_TOO_BIG;
        }
        auto msz = sizeof(header_s) + sizeof(acknowledge_s);
        m_server_socket.send(bfc::const_buffer_view(m_snd_buff, msz), 0, &(c->client_address), sizeof(c->client_address));
        return;
    }

    if (data.size() > 1024*8)
    {
        return;
    }

    auto& v = get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto key = next_struct<key_sn_s>(hdr);
    auto dat = next_struct<std::byte>(key);
    hdr->flags = 0;
    hdr->type = E_MSGT_VALUE;
    key->key = msg.key;
    key->sn = v.ssn++;
    std::memcpy(dat, data.data(), data.size());
    auto msz = sizeof(header_s) + sizeof(key_sn_s) + data.size();
    for (auto& s : v.subscribers)
    {
        m_server_socket.send(bfc::const_buffer_view(m_snd_buff, msz), 0, &(s->client_address), sizeof(s->client_address));
    }
}

void value_server::handle_get(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto key = next_struct<key_s>(hdr);
    auto dat = next_struct<std::byte>(key);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_VALUE;
    key->key = msg.key;
    std::memcpy(dat, v.data.data(), v.data.size());
    auto msz = sizeof(header_s) + sizeof(key_sn_s) + v.data.size();
    m_server_socket.send(bfc::const_buffer_view(m_snd_buff, msz), 0, &(c->client_address), sizeof(c->client_address));
    return;
}

void value_server::handle_subscribe(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto ack = next_struct<acknowledge_s>(hdr);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_ACKNOWLEDGE;
    hdr->transaction = h.transaction;
    ack->status = E_STATUS_OK;
    v.subscribers.emplace(c);
    auto msz = sizeof(header_s) + sizeof(acknowledge_s);
    m_server_socket.send(bfc::const_buffer_view(m_snd_buff, msz), 0, &(c->client_address), sizeof(c->client_address));
    return;
}

void value_server::handle_unsubscribe(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto ack = next_struct<acknowledge_s>(hdr);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_ACKNOWLEDGE;
    hdr->transaction = h.transaction;
    ack->status = E_STATUS_OK;
    v.subscribers.erase(c);
    auto msz = sizeof(header_s) + sizeof(acknowledge_s);
    m_server_socket.send(bfc::const_buffer_view(m_snd_buff, msz), 0, &(c->client_address), sizeof(c->client_address));
    return;
}

void value_server::handle(cctx_ptr& c, const bfc::const_buffer_view& b)
{
    auto hdr = (const header_s*) b.data();
    auto key = next_struct<key_s>(hdr);
    switch (hdr->type)
    {
        case E_MSGT_SET:
        {
            auto pl = next_struct<std::byte>(key);
            auto sz = b.size();
            constexpr auto of = sizeof(header_s) + sizeof(key_s);
            sz = of >= sz ? 0 : (sz-of);
            handle_set(c, *hdr, *key, bfc::const_buffer_view(pl, sz));
            break;
        }
        case E_MSGT_GET:
            handle_get(c, *hdr, *key);
            break;
        case E_MSGT_SUBSCRIBE:
            handle_subscribe(c, *hdr, *key);
            break;
        case E_MSGT_UNSUBSCRIBE:
            handle_unsubscribe(c, *hdr, *key);
            break;
        default:
            break;
    }
}

value_server::value& value_server::get_value(uint16_t id)
{
    if (id >= m_value_map.size())
    {
        m_value_map.reserve(id*2);
        m_value_map.resize(id+1);
    }

    return m_value_map[id];
}

bool value_server::set_value(uint16_t id, const bfc::const_buffer_view& data)
{
    if (data.size() > 1024*8)
    {
        return false;
    }

    auto& value = get_value(id);
    value.data.resize(data.size());
    std::memcpy(value.data.data(), data.data(), data.size());
    value.vsn++;
}

} // namespace propertytree
