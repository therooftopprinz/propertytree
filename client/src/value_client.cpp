#include "value_client.hpp"
#include "logger.hpp"

namespace propertytree
{

template <typename T> std::optional<T> value::as()
{
    if (size() != sizeof(T))
    {
        return {};
    }

    T rv;
    std::memcpy(&rv, data.data(), size());
    return rv;
}

buffer value::raw()
{
    return data;
}

template <typename T> value& value::operator=(T& t)
{
    if (sizeof(T) != size())
    {
        data.resize(sizeof(T));
    }

    std::memcpy(data.data(), &t, sizeof(T));
    commit();
    return *this;
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
    : m_reactor(reactor)
    , m_server_socket(bfc::create_tcp4())
{
    if (0 > m_server_socket.fd())
    {
        return;
    }

    m_server_socket_ctx = m_reactor.make_context(m_server_socket.fd());
    m_server_socket.connect(bfc::ip4_port_to_sockaddr(cfg.ip, cfg.port));
    m_reactor.add_read_rdy(m_server_socket_ctx, [this](){});
}

value_ptr value_client::get(uint64_t id)
{
    return std::make_shared<value>(id, *this);
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
        LOG_ERR("value_client | protocol_error=unknown_response trid=%" PRIu64, rsp.sequence_number);
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
        LOG_ERR_S("value_client | protocol_error=read_buffer_overrun");
        disconnect();
        return;
    }

    auto res = read(m_server_socket.fd(), m_read_buffer + m_read_buffer_idx, read_size);

    if (0 >= res)
    {
        LOG_ERR("value_client | connection_error=%s", strerror(errno));
        disconnect();
        return;
    }

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
        return;
    }

    if (m_expected_read_size == m_read_buffer_idx)
    {
        IF_LB(LB_DUMP_MSG_RAW) LOG_DBG("value_client | data=%x;", buffer_log_t(m_read_buffer_idx, m_read_buffer));

        auto cbuff = bfc::const_buffer_view(m_read_buffer, m_read_buffer_idx);

        cum::protocol_value_client message;
        cum::per_codec_ctx dec_ctx((std::byte*)cbuff.data(), cbuff.size());
        decode_per(message, dec_ctx);

        IF_LB(LB_DUMP_MSG_PROTO)
        {
            std::string stred;
            cum::str("root", message, stred, true);
            LOG_DBG("value_client | message=%x;", stred.c_str());
        }

        std::visit([this](auto&& msg) mutable {
                handle(std::move(msg));
            }, std::move(message));

        m_read_state = WAIT_HEADER;
        m_read_buffer_idx = 0;
    }
}

size_t value_client::encode(const cum::protocol_value_server& msg, std::byte* data, size_t size)
{
    auto& msg_size = *(new (data) uint16_t(0));
    cum::per_codec_ctx context(data+sizeof(msg_size), size-sizeof(msg_size));
    encode_per(msg, context);
    msg_size = size-context.size()-2;

    IF_LB(LB_DUMP_MSG_PROTO)
    {
        std::string stred;
        cum::str("root", msg, stred, true);
        LOG_INF("value_server | to_encode=%s;", stred.c_str());
    }

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

    std::unique_lock lg2(transaction->mutex);
    transaction->cv.wait_for(lg2, std::chrono::milliseconds(1000), [&transaction](){
            return transaction->satisfied;
        });

    m_transaction_map.erase(get_value_request.transaction_id);
    if (!transaction->satisfied)
    {
        LOG_ERR("value_client | transaction_timeout=%" PRIu64, get_value_request.transaction_id);
        disconnect();
        return {};
    }

    if (transaction->message.index() != 0)
    {
        LOG_ERR("value_client | incorrect_response=%zu", transaction->message.index());
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
