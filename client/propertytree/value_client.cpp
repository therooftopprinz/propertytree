#include "value_client.hpp"
#include "logger.hpp"

namespace propertytree
{

template <typename T=std::chrono::microseconds>
static uint64_t now()
{
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

buffer value::raw()
{
    return data;
}

value& value::operator=(buffer b)
{
    data = std::move(b);
    commit();
    return *this;
}

size_t value::size()
{
    return data.size();
}

void value::fetch()
{
    data = client.get_value(id);
}

unsigned value::subscribe(cb_t cb)
{
    if (0 == watchers.size())
    {
        client.subscribe(id);
    }

    auto wid = m_watchers_id.fetch_add(1);
    watchers.emplace(wid, std::move(cb));
    return wid;
}

void value::unsubscribe(unsigned wid)
{
    watchers.erase(wid);
    if (0 == watchers.size())
    {
        client.unsubscribe(id);
    }
}

void value::commit()
{
    client.set_value(id, data);
}

value::value(uint64_t id, value_client& client)
    : id(id)
    , client(client)
{}

value_client::value_client(const config_s* cfg, reactor_t* reactor)
    : m_logger(cfg->log.c_str())
    , m_reactor(reactor)
    , m_tcp_server_socket(bfc::create_tcp4())
    , m_udp_server_socket(bfc::create_udp4())
{
    if (cfg->logful)
    {
        m_logger.logful();
    }

    if (0 > m_tcp_server_socket.fd())
    {
        PTLOG_ERR("value_client | fd=%3d; | tcp_socket_error=%s;" PRIu64, m_tcp_server_socket.fd(), strerror(errno));
        return;
    }

    if (0 > m_udp_server_socket.fd())
    {
        PTLOG_ERR("value_client | fd=%3d; | ucp_socket_error=%s;" PRIu64, m_udp_server_socket.fd(), strerror(errno));
        return;
    }

    m_udp_server_addr = bfc::ip4_port_to_sockaddr(cfg->ip, cfg->port);

    m_tcp_server_socket_ctx = m_reactor->make_context(m_tcp_server_socket.fd());
    m_udp_server_socket_ctx = m_reactor->make_context(m_udp_server_socket.fd());

    m_reactor->add_read_rdy(m_tcp_server_socket_ctx, [this](){tcp_read_server();});
    m_reactor->add_read_rdy(m_udp_server_socket_ctx, [this](){udp_read_server();});

    if (0 > m_tcp_server_socket.connect(m_udp_server_addr))
    {
        PTLOG_ERR("value_client | fd=%3d; | tcp_connection_error=%s;" PRIu64, m_tcp_server_socket.fd(), strerror(errno));
        return;
    }

    PTLOG_INF("value_client | fd=%3d; | connected!", m_tcp_server_socket.fd());
}

logless::logger& value_client::get_logger()
{
    return m_logger;
}

value_ptr value_client::get(uint64_t id)
{
    std::unique_lock lg(m_value_map_mutex);
    auto it = m_value_map.find(id);
    std::shared_ptr<value> vp;
    if (m_value_map.end() == it)
    {
        vp = std::make_shared<value>(id, *this);
        m_value_map.emplace(id, vp);
    }
    else
    {
        vp = it->second;
    }
    return vp;
}

void value_client::tcp_disconnect()
{
    m_reactor->rem_read_rdy(m_tcp_server_socket_ctx);
    m_tcp_server_socket = {};
}

void value_client::tcp_handle(cum::acknowledge&& ack)
{
    std::unique_lock lg(m_tcp_transaction_map_mutex);
    auto it = m_tcp_transaction_map.find(ack.transaction_id);
    if (m_tcp_transaction_map.end() == it)
    {
        return;
    }
    auto transaction = it->second;
    m_tcp_transaction_map.erase(it);
    lg.unlock();
    transaction->rsp = std::move(ack);
    std::unique_lock lg2(transaction->mutex);
    transaction->satisfied = true;
    transaction->cv.notify_all();
}

void value_client::tcp_handle(cum::update&& rsp)
{
    auto value_ptr = get(rsp.data.id);

    std::unique_lock lg2(value_ptr->mutex);
    value_ptr->data = std::move(rsp.data.data);
    for (auto& cbit : value_ptr->watchers)
    {
        auto& cb = cbit.second;
        if (cb)
        {
            cb(value_ptr->data);
        }
    }
}

void value_client::tcp_read_server()
{
    int read_size = 0;
    read_size = m_tcp_expected_read_size - m_tcp_read_buffer_idx;

    if (m_tcp_read_buffer_idx + read_size > sizeof(m_tcp_read_buffer))
    {
        PTLOG_ERR("value_client | fd=%3d; | protocol_error=read_buffer_overrun", m_tcp_server_socket.fd());
        tcp_disconnect();
        return;
    }

    auto res = read(m_tcp_server_socket.fd(), m_tcp_read_buffer + m_tcp_read_buffer_idx, read_size);

    if (0 >= res)
    {
        PTLOG_ERR("value_client | fd=%3d; | connection_error=%s;", m_tcp_server_socket.fd(), strerror(errno));
        tcp_disconnect();
        return;
    }

    PTIF_LB(LB_DUMP_MSG_SOCK) PTLOG_INF("value_client | read[%zu;]=%x;",
        m_tcp_server_socket.fd(), res, buffer_log_t(res, m_tcp_read_buffer + m_tcp_read_buffer_idx));

    m_tcp_read_buffer_idx += res;

    if (WAIT_HEADER == m_tcp_read_state)
    {
        if (m_tcp_read_buffer_idx != m_tcp_expected_read_size)
        {
            return;
        }

        std::memcpy(&m_tcp_expected_read_size, m_tcp_read_buffer, 2);
        m_tcp_read_buffer_idx = 0;
        m_tcp_read_state = WAIT_REMAINING;

        PTIF_LB(LB_DUMP_MSG_SOCK) PTLOG_INF("value_client | fd=%3d; | expected_size=%zu;",
            m_tcp_server_socket.fd(), m_tcp_expected_read_size);
        return;
    }

    if (m_tcp_expected_read_size == m_tcp_read_buffer_idx)
    {
        PTIF_LB(LB_DUMP_MSG_RAW) PTLOG_INF("value_client | fd=%3d; | to_decode=%x;",
            m_tcp_server_socket.fd(), buffer_log_t(m_tcp_read_buffer_idx, m_tcp_read_buffer));

        auto cbuff = bfc::const_buffer_view(m_tcp_read_buffer, m_tcp_read_buffer_idx);

        uint64_t ts_decode_t0 = 0;
        uint64_t ts_decode_t1 = 0;

        PTIF_LB(LB_DUMP_PERF) ts_decode_t0 = now();
        cum::protocol_value_client message;
        cum::per_codec_ctx dec_ctx((std::byte*)cbuff.data(), cbuff.size());
        decode_per(message, dec_ctx);
        PTIF_LB(LB_DUMP_PERF) ts_decode_t1 = now();

        PTIF_LB(LB_DUMP_PERF) PTLOG_INF("value_client | decode_time=%" PRIu64 "; start=% " PRIu64 ";",
            uint64_t(ts_decode_t1 - ts_decode_t0), ts_decode_t0);

        PTIF_LB(LB_DUMP_MSG_PROTO)
        {
            std::string stred;
            cum::str("root", message, stred, true);
            PTLOG_INF("value_client | fd=%3d; | decoded=%s;", m_tcp_server_socket.fd(), stred.c_str());
        }

        uint64_t ts_process_t0 = 0;
        uint64_t ts_process_t1 = 0;

        PTIF_LB(LB_DUMP_PERF) ts_process_t0 = now();
        std::visit([this](auto&& msg) mutable {
                tcp_handle(std::move(msg));
            }, std::move(message));
        PTIF_LB(LB_DUMP_PERF) ts_process_t1 = now();

        PTIF_LB(LB_DUMP_PERF) PTLOG_INF("value_client | process_time=%" PRIu64 "; start=% " PRIu64 ";",
            uint64_t(ts_process_t1 - ts_process_t0), ts_process_t0);

        m_tcp_read_state = WAIT_HEADER;
        m_tcp_read_buffer_idx = 0;
        m_tcp_expected_read_size = 2;
    }
}

size_t value_client::encode(const cum::protocol_value_server& msg, std::byte* data, size_t size)
{
    auto& msg_size = *(new (data) uint16_t(0));
    cum::per_codec_ctx context(data+sizeof(msg_size), size-sizeof(msg_size));
    encode_per(msg, context);
    msg_size = size - context.size() - sizeof(msg_size);

    PTIF_LB(LB_DUMP_MSG_PROTO)
    {
        std::string stred;
        cum::str("root", msg, stred, true);
        PTLOG_INF("value_client | fd=%3d; | to_encode=%s;", m_tcp_server_socket.fd(), stred.c_str());
    }

    unsigned encode_size= msg_size+sizeof(msg_size);

    PTIF_LB(LB_DUMP_MSG_RAW) PTLOG_INF("value_client | fd=%3d; | encoded[%u;]=%x;", m_tcp_server_socket.fd(), encode_size, buffer_log_t(encode_size, data));

    return msg_size + sizeof(msg_size);
}

void value_client::stream(uint64_t key, const buffer& value)
{
    if (value.size() > 1024*8)
    {
        return;
    }

    std::byte send_buff[ENCODE_SIZE];
    auto hdr = (header_s*)(send_buff);
    auto msg = next_struct<key_s>(hdr);
    auto data = next_struct<std::byte>(msg);
    hdr->flags = 0;
    hdr->type = E_MSGT_SET;
    msg->key = key;
    std::memcpy(data, value.data(), value.size());

    auto msz = sizeof(header_s) + sizeof(key_s) + value.size();
    m_udp_server_socket.send(bfc::const_buffer_view(send_buff, msz),
        0, (sockaddr*)&(m_udp_server_addr), sizeof(m_udp_server_addr));
}

void value_client::set_value(uint64_t key, const buffer& value)
{
    if (value.size() > 1024*8)
    {
        return;
    }

    auto transaction = std::make_shared<udp_transaction_s>();
    auto transaction_id = m_udp_transaction_id.fetch_add(1);

    {
        std::unique_lock lg(m_udp_transaction_map_mutex);
        m_udp_transaction_map.emplace(transaction_id, transaction);
    }

    std::byte* send_buff = transaction->snd_msg;
    auto hdr = (header_s*)(send_buff);
    auto msg = next_struct<key_s>(hdr);
    auto data = next_struct<std::byte>(msg);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_SET;
    hdr->transaction = transaction_id;
    msg->key = key;
    std::memcpy(data, value.data(), value.size());

    auto msz = sizeof(header_s) + sizeof(key_s) + value.size();
    size_t i = 0;
    for (i=0; i<10 && !transaction->satisfied; i++)
    {
        m_udp_server_socket.send(bfc::const_buffer_view(send_buff, msz),
            0, (sockaddr*)&(m_udp_server_addr), sizeof(m_udp_server_addr));
        std::unique_lock lg(transaction->mutex);
        transaction->cv.wait_for(lg, std::chrono::milliseconds(50), [&transaction](){
                return transaction->satisfied;
            });
    }
    if (!transaction->satisfied)
        PTLOG_ERR("value_client | transaction failed! trid=%zu;", transaction_id);
    if (i>1)
        PTLOG_INF("value_client | transaction completed trid=%zu; in n=%zu;", transaction_id, i);

}

buffer value_client::get_value(uint64_t key)
{
    auto transaction = std::make_shared<udp_transaction_s>();
    auto transaction_id = m_udp_transaction_id.fetch_add(1);

    {
        std::unique_lock lg(m_udp_transaction_map_mutex);
        m_udp_transaction_map.emplace(transaction_id, transaction);
    }

    std::byte* send_buff = transaction->snd_msg;
    auto hdr = (header_s*)(send_buff);
    auto msg = next_struct<key_s>(hdr);
    hdr->flags = HEADER_FLAG_TRANSACTIONAL;
    hdr->type = E_MSGT_GET;
    hdr->transaction = transaction_id;
    msg->key = key;

    auto msz = sizeof(header_s) + sizeof(key_s);
    for (auto i=0; i<3 && !transaction->satisfied; i++)
    {
        m_udp_server_socket.send(bfc::const_buffer_view(send_buff, msz),
            0, (sockaddr*)&(m_udp_server_addr), sizeof(m_udp_server_addr));
        std::unique_lock lg(transaction->mutex);
        transaction->cv.wait_for(lg, std::chrono::microseconds(10), [&transaction](){
                return transaction->satisfied;
            });
    }

    return {};
}

void value_client::subscribe(uint64_t id)
{
    auto transaction = std::make_shared<tcp_transaction_s>();
    auto transaction_id = m_tcp_transaction_id.fetch_add(1);

    {
        std::unique_lock lg(m_udp_transaction_map_mutex);
        m_tcp_transaction_map.emplace(transaction_id, transaction);
    }

    cum::protocol_value_server msg = cum::subscribe{};
    auto& subscribe = std::get<cum::subscribe>(msg);
    subscribe.id = id;
    subscribe.transaction_id = transaction_id;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);
    m_tcp_server_socket.send(bv);

    std::unique_lock lg(transaction->mutex);
    transaction->cv.wait_for(lg, std::chrono::milliseconds(500), [&transaction](){
            return transaction->satisfied;
        });
}

void value_client::unsubscribe(uint64_t id)
{
    auto transaction = std::make_shared<tcp_transaction_s>();
    auto transaction_id = m_tcp_transaction_id.fetch_add(1);

    {
        std::unique_lock lg(m_udp_transaction_map_mutex);
        m_tcp_transaction_map.emplace(transaction_id, transaction);
    }

    cum::protocol_value_server msg = cum::unsubscribe{};
    auto& unsubscribe = std::get<cum::unsubscribe>(msg);
    unsubscribe.id = id;
    unsubscribe.transaction_id = transaction_id;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);
    m_tcp_server_socket.send(bv);

    std::unique_lock lg(transaction->mutex);
    transaction->cv.wait_for(lg, std::chrono::milliseconds(500), [&transaction](){
            return transaction->satisfied;
        });
}

void value_client::udp_read_server()
{
    auto res = m_udp_server_socket.recv(bfc::buffer_view(m_udp_read_buffer, sizeof(m_udp_read_buffer)), 0);

    if (0 > res)
    {
        return;
    }

    PTIF_LB(LB_DUMP_MSG_SOCK) PTLOG_INF("value_client | read[%zu;]=%x;", res, buffer_log_t(res, m_udp_read_buffer));

    udp_handle(bfc::const_buffer_view(m_udp_read_buffer, res));
}

void value_client::udp_handle_ack(const header_s& h, const acknowledge_s& m)
{
    std::unique_lock lg(m_udp_transaction_map_mutex);
    auto it = m_udp_transaction_map.find(h.transaction);
    if (m_udp_transaction_map.end() == it)
    {
        return;
    }
    auto transaction = it->second;
    m_udp_transaction_map.erase(it);
    lg.unlock();
    std::unique_lock lg2(transaction->mutex);
    transaction->satisfied = true;
    transaction->cv.notify_all();
}

void value_client::udp_handle_value(const header_s&, const key_sn_s& key, const bfc::const_buffer_view& data)
{
    auto vp = get(key.key);
    std::unique_lock lg(vp->mutex);
    for (auto& i : vp->stream_watchers)
    {
        i.second(data);
    }
}

void value_client::udp_handle(const bfc::const_buffer_view&& data)
{
    auto hdr = (header_s*)(data.data());
    switch(hdr->type)
    {
        case E_MSGT_ACKNOWLEDGE:
        {
            auto msg = next_struct<acknowledge_s>(hdr);
            udp_handle_ack(*hdr, *msg);
            break;
        }
        case E_MSGT_VALUE:
        {
            auto msg = next_struct<key_sn_s>(hdr);
            auto pl = next_struct<std::byte>(hdr);
            auto sz = data.size();
            constexpr auto of = sizeof(header_s) + sizeof(key_sn_s);
            sz = of >= sz ? 0 : (sz-of);
            udp_handle_value(*hdr, *msg, bfc::const_buffer_view(pl, sz));
            break;
        }
    }
}

} // namespace propertytree
