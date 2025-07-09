#include <udp_value_server.hpp>
#include <logger.hpp>
#include <utils.hpp>

namespace propertytree
{

template <typename T=std::chrono::microseconds>
static uint64_t now()
{
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

udp_value_server::udp_value_server(
    const bfc::configuration_parser* c,
    reactor_t* r,
    value_map* v)
    : m_config(c)
    , m_reactor(r)
    , m_value_map(v)
    , m_server_socket(bfc::create_udp4())
{
    LOG_INF("udp_value_server | construct %p;", this);

    auto port      = utils::get_config<unsigned>(   *c, "service.value.port").value();
    auto interface = utils::get_config<std::string>(*c, "service.value.interface").value_or(std::string("127.0.0.1"));

    auto bind_addr = bfc::ip4_port_to_sockaddr(interface, port);

    LOG_INF("udp_value_server | bind_address=%s;", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str());
    m_server_socket.set_sock_opt(SOL_SOCKET, SO_REUSEADDR, 1);

    if (0 > m_server_socket.bind(bind_addr))
    {
        LOG_ERR("udp_value_server | bind_address=%s; error='%s;'", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str(), strerror(errno));
        return;
    }

    m_server_rctx = m_reactor->make_context(m_server_socket.fd());
    m_reactor->add_read_rdy(m_server_rctx, [this](){
            read();
        });
}

void udp_value_server::read()
{
    sockaddr client_addr;
    socklen_t client_addr_sz;

    auto res = m_server_socket.recv(bfc::buffer_view(m_rcv_buff, sizeof(m_rcv_buff)),
        0, &client_addr, &client_addr_sz);

    if (res < 0)
    {
        return;
    }

    IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("udp_value_server | from=%s; read[%zu;]=%x;",
        bfc::sockaddr_to_string(&client_addr).c_str(), res, buffer_log_t(res, m_rcv_buff));

    auto addr = utils::get_addr(&client_addr);
    auto client_it = m_client_map.find(addr);
    cctx_ptr ctx;
    if (m_client_map.end() == client_it)
    {
        ctx = std::make_shared<udp_client_context>(&m_server_socket, client_addr);
        m_client_map.emplace(addr, ctx);
    }
    else
    {
        ctx = client_it->second;
    }

    handle(ctx, bfc::const_buffer_view(m_rcv_buff, res));
}

void udp_value_server::handle_set(cctx_ptr& c, const header_s& h, const key_s& msg, const bfc::const_buffer_view& data)
{
    if (data.size() > 1024*8)
    {
        return;
    }

    auto& v = m_value_map->get_value(msg.key);

    if (h.flags & HEADER_FLAG_TRANSACTIONAL)
    {
        auto hdr = (header_s*)(m_snd_buff);
        auto ack = next_struct<acknowledge_s>(hdr);
        hdr->flags = HEADER_FLAG_TRANSACTIONAL;
        hdr->type = E_MSGT_ACKNOWLEDGE;
        hdr->transaction = h.transaction;
        ack->status = E_STATUS_OK;
        if (!m_value_map->set_value(msg.key, data))
        {
            ack->status = E_STATUS_DATA_TOO_BIG;
        }
        auto msz = sizeof(header_s) + sizeof(acknowledge_s);
        send(c, bfc::const_buffer_view(m_snd_buff, msz));

        if (c->last_transaction == h.transaction)
        {
            return;
        }

        c->last_transaction = h.transaction;

        cum::protocol_value_client smsg = cum::update{};
        auto& update = std::get<cum::update>(smsg);
        update.data.id = msg.key;
        update.data.data = v.data;
        update.sn = v.vsn;

        auto size = encode(smsg, m_snd_buff, sizeof(m_snd_buff));
        auto bv = bfc::const_buffer_view(m_snd_buff, size);

        for (auto& s : v.data_subscribers)
        {
            s->send(bv);
        }
        return;
    }

    auto hdr    = (header_s*)(m_snd_buff);
    auto key    = next_struct<key_sn_s>(hdr);
    auto dat    = next_struct<std::byte>(key);
    hdr->flags  = 0;
    hdr->type   = E_MSGT_VALUE;
    key->key    = msg.key;
    key->sn     = v.ssn++;
    std::memcpy(dat, data.data(), data.size());
    auto msz = sizeof(header_s) + sizeof(key_sn_s) + data.size();
    for (auto& s : v.stream_subscribers)
    {
        s->send(bfc::const_buffer_view(m_snd_buff, msz));
    }
}

void udp_value_server::handle_get(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = m_value_map->get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto key = next_struct<key_s>(hdr);
    auto dat = next_struct<std::byte>(key);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_VALUE;
    key->key = msg.key;
    std::memcpy(dat, v.data.data(), v.data.size());
    auto msz = sizeof(header_s) + sizeof(key_sn_s) + v.data.size();
    send(c, bfc::const_buffer_view(m_snd_buff, msz));
    return;
}

void udp_value_server::handle_subscribe(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = m_value_map->get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto ack = next_struct<acknowledge_s>(hdr);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_ACKNOWLEDGE;
    hdr->transaction = h.transaction;
    ack->status = E_STATUS_OK;
    v.stream_subscribers.emplace(c);
    auto msz = sizeof(header_s) + sizeof(acknowledge_s);
    send(c, bfc::const_buffer_view(m_snd_buff, msz));
    return;
}

void udp_value_server::handle_unsubscribe(cctx_ptr& c, const header_s& h, const key_s& msg)
{
    if (!(h.flags & HEADER_FLAG_TRANSACTIONAL))
    {
        return;
    }

    auto& v = m_value_map->get_value(msg.key);
    auto hdr = (header_s*)(m_snd_buff);
    auto ack = next_struct<acknowledge_s>(hdr);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_ACKNOWLEDGE;
    hdr->transaction = h.transaction;
    ack->status = E_STATUS_OK;
    v.stream_subscribers.erase(c);
    auto msz = sizeof(header_s) + sizeof(acknowledge_s);
    send(c, bfc::const_buffer_view(m_snd_buff, msz));
    return;
}

void udp_value_server::handle(cctx_ptr& c, const bfc::const_buffer_view& b)
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

size_t udp_value_server::encode(const cum::protocol_value_client& msg, std::byte* data, size_t size)
{
    auto& msg_size = *(new (data) uint16_t(0));
    cum::per_codec_ctx context(data+sizeof(msg_size), size-sizeof(msg_size));
    encode_per(msg, context);
    msg_size = size-context.size()-2;

    unsigned encode_size = msg_size + sizeof(msg_size);

    IF_LB(LB_DUMP_MSG_PROTO)
    {
        std::string stred;
        cum::str("root", msg, stred, true);
        LOG_INF("udp_value_server | ToTCP to_encode=%s;", stred.c_str());
    }

    IF_LB(LB_DUMP_MSG_RAW)
    {
        LOG_INF("udp_value_server | ToTCP encoded[%u;]=%x;", encode_size, buffer_log_t(encode_size, data));
    }

    return encode_size;
}

void udp_value_server::send(cctx_ptr& c, const bfc::const_buffer_view& buff)
{
    IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("udp_value_server | to=%s; write[%zu;]=%x;",
        bfc::sockaddr_to_string(&c->client_address).c_str(), buff.size(), buffer_log_t(buff.size(), buff.data()));

    m_server_socket.send(buff, 0, &(c->client_address), sizeof(c->client_address));
}


} // namespace propertytree
