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

value_client::value_client(const config_s& cfg, reactor_t& reactor)
    : m_logger(cfg.log.c_str())
    , m_reactor(reactor)
    , m_server_socket(bfc::create_tcp4())
{
    if (cfg.logful)
    {
        m_logger.logful();
    }

    if (0 > m_server_socket.fd())
    {
        PTLOG_ERR("value_client | fd=%3d; | connection_error=%s;" PRIu64, m_server_socket.fd(), strerror(errno));
        return;
    }

    m_server_socket_ctx = m_reactor.make_context(m_server_socket.fd());

    m_reactor.add_read_rdy(m_server_socket_ctx, [this](){read_server();});

    auto res = m_server_socket.connect(bfc::ip4_port_to_sockaddr(cfg.ip, cfg.port));
    if (res < 0)
    {
        PTLOG_ERR("value_client | fd=%3d; | connection_error=%s;" PRIu64, m_server_socket.fd(), strerror(errno));
        return;
    }

    PTLOG_INF("value_client | fd=%3d; | connected!", m_server_socket.fd());
}

logless::logger& value_client::get_logger()
{
    return m_logger;
}

value_ptr value_client::get(uint64_t id)
{
    auto vp = std::make_shared<value>(id, *this);
    m_value_map.emplace(id, vp);
    return vp;
}

void value_client::disconnect()
{
    m_reactor.rem_read_rdy(m_server_socket_ctx);
    m_server_socket = {};
}

void value_client::handle(cum::get_value_response&& rsp)
{
    std::unique_lock lg(m_transaction_map_mutex);
    auto it = m_transaction_map.find(rsp.transaction_id);
    if (m_transaction_map.end() == it)
    {
        PTLOG_ERR("value_client | fd=%3d; | protocol_error=unknown_response trid=%" PRIu64, m_server_socket.fd(), rsp.sequence_number);
        disconnect();
        return;
    }

    auto transaction = it->second;
    m_transaction_map.erase(it);
    lg.unlock();

    std::unique_lock lg2(transaction->mutex);
    transaction->message = std::move(rsp);
    transaction->satisfied = true;
    lg2.unlock();

    transaction->cv.notify_all();
}

void value_client::handle(cum::update&& rsp)
{
    std::unique_lock lg(m_value_map_mutex);
    auto value_ptr = m_value_map[rsp.data.id];
    if (!value_ptr)
    {
        value_ptr = std::make_shared<value>(rsp.data.id, *this);
        m_value_map.emplace(rsp.data.id, value_ptr);
    }
    lg.unlock();

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

void value_client::read_server()
{
    int read_size = 0;
    read_size = m_expected_read_size - m_read_buffer_idx;

    if (m_read_buffer_idx + read_size > sizeof(m_read_buffer))
    {
        PTLOG_ERR("value_client | fd=%3d; | protocol_error=read_buffer_overrun", m_server_socket.fd());
        disconnect();
        return;
    }

    auto res = read(m_server_socket.fd(), m_read_buffer + m_read_buffer_idx, read_size);

    if (0 >= res)
    {
        PTLOG_ERR("value_client | fd=%3d; | connection_error=%s;", m_server_socket.fd(), strerror(errno));
        disconnect();
        return;
    }

    PTIF_LB(LB_DUMP_MSG_SOCK) PTLOG_INF("value_client | fd=%3d; | read[%zu;]=%x;", m_server_socket.fd(), res, buffer_log_t(res, m_read_buffer + m_read_buffer_idx));

    m_read_buffer_idx += res;

    if (WAIT_HEADER == m_read_state)
    {
        if (m_read_buffer_idx != m_expected_read_size)
        {
            return;
        }

        std::memcpy(&m_expected_read_size, m_read_buffer, 2);
        m_read_buffer_idx = 0;
        m_read_state = WAIT_REMAINING;

        PTIF_LB(LB_DUMP_MSG_SOCK) PTLOG_INF("value_client | fd=%3d; | expected_size=%zu;", m_server_socket.fd(), m_expected_read_size);
        return;
    }

    if (m_expected_read_size == m_read_buffer_idx)
    {
        PTIF_LB(LB_DUMP_MSG_RAW) PTLOG_INF("value_client | fd=%3d; | to_decode=%x;", m_server_socket.fd(), buffer_log_t(m_read_buffer_idx, m_read_buffer));

        auto cbuff = bfc::const_buffer_view(m_read_buffer, m_read_buffer_idx);

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
            PTLOG_INF("value_client | fd=%3d; | decoded=%s;", m_server_socket.fd(), stred.c_str());
        }

        uint64_t ts_process_t0 = 0;
        uint64_t ts_process_t1 = 0;

        PTIF_LB(LB_DUMP_PERF) ts_process_t0 = now();
        std::visit([this](auto&& msg) mutable {
                handle(std::move(msg));
            }, std::move(message));
        PTIF_LB(LB_DUMP_PERF) ts_process_t1 = now();

        PTIF_LB(LB_DUMP_PERF) PTLOG_INF("value_client | process_time=%" PRIu64 "; start=% " PRIu64 ";",
            uint64_t(ts_process_t1 - ts_process_t0), ts_process_t0);

        m_read_state = WAIT_HEADER;
        m_read_buffer_idx = 0;
        m_expected_read_size = 2;
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
        PTLOG_INF("value_client | fd=%3d; | to_encode=%s;", m_server_socket.fd(), stred.c_str());
    }

    unsigned encode_size= msg_size+sizeof(msg_size);

    PTIF_LB(LB_DUMP_MSG_RAW) PTLOG_INF("value_client | fd=%3d; | encoded[%u;]=%x;", m_server_socket.fd(), encode_size, buffer_log_t(encode_size, data));

    return msg_size + sizeof(msg_size);
}

void value_client::set_value(uint64_t id, const buffer& data)
{
    cum::protocol_value_server msg = cum::set_value{};
    auto& set_value = std::get<cum::set_value>(msg);

    set_value.id = id;
    set_value.data = data;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);
    m_server_socket.send(bv);
}

buffer value_client::get_value(uint64_t id)
{
    cum::protocol_value_server msg = cum::get_value_request{};
    auto& get_value_request = std::get<cum::get_value_request>(msg);

    get_value_request.transaction_id = m_transaction_id.fetch_add(1);
    get_value_request.id = id;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);

    std::unique_lock lg(m_transaction_map_mutex);
    auto transaction = std::make_shared<transaction_s>();
    m_transaction_map.emplace(get_value_request.transaction_id, transaction);
    m_server_socket.send(bv);
    lg.unlock();

    std::unique_lock lg2(transaction->mutex);
    // transaction->cv.wait_for(lg2, std::chrono::milliseconds(1000), [&transaction](){
    transaction->cv.wait(lg2, [&transaction](){
            return transaction->satisfied;
        });


    lg.lock();
    m_transaction_map.erase(get_value_request.transaction_id);
    lg.unlock();

    if (!transaction->satisfied)
    {
        PTLOG_ERR("value_client | fd=%3d; | transaction_timeout=%" PRIu64 ";", m_server_socket.fd(), get_value_request.transaction_id);
        disconnect();
        return {};
    }

    if (transaction->message.index() != 0)
    {
        PTLOG_ERR("value_client | fd=%3d; | incorrect_response=%zu",  m_server_socket.fd(), transaction->message.index());
        disconnect();
        return {};
    }

    auto& rsp = std::get<0>(transaction->message);
    return std::move(rsp.data.data);
}

void value_client::subscribe(uint64_t id)
{
    cum::protocol_value_server msg = cum::subscribe{};
    auto& subscribe = std::get<cum::subscribe>(msg);

    subscribe.id = id;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);
    m_server_socket.send(bv);
}

void value_client::unsubscribe(uint64_t id)
{
    cum::protocol_value_server msg = cum::unsubscribe{};
    auto& unsubscribe = std::get<cum::unsubscribe>(msg);

    unsubscribe.id = id;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(msg, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);
    m_server_socket.send(bv);
}

} // namespace propertytree
