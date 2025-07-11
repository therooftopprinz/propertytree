#include <value_server.hpp>
#include <netinet/tcp.h>
#include <logger.hpp>
#include <utils.hpp>

namespace propertytree
{

template <typename T>
void protocol_buffer_copy_detail(cum::bufferX& dst, std::vector<std::byte>& src)
{
    dst = T{};
    auto& dstt = std::get<T>(dst);
    if constexpr (std::is_same_v<T, cum::bufferD>)
    {
        dstt.resize(src.size());
        std::copy(src.begin(), src.end(), dstt.begin());
    }
    else
    {
        dstt.size = src.size();
        std::memcpy(dstt.data, src.data(), src.size());
    }
}

void protocol_buffer_copy(cum::bufferX& dst, std::vector<std::byte>& src)
{
    if (src.size()<=8)
        protocol_buffer_copy_detail<cum::buffer8>(dst, src);
    else if (src.size()<=16)
        protocol_buffer_copy_detail<cum::buffer16>(dst, src);
    else if (src.size()<=32)
        protocol_buffer_copy_detail<cum::buffer32>(dst, src);
    else if (src.size()<=64)
        protocol_buffer_copy_detail<cum::buffer64>(dst, src);
    else if (src.size()<=128)
        protocol_buffer_copy_detail<cum::buffer128>(dst, src);
    else
    {
        dst = cum::bufferD{};
        auto& dstt = std::get<cum::bufferD>(dst);
        dstt = src;
    }
}

size_t protocol_buffer_size(cum::bufferX& b)
{
    auto sizer = [](auto& bb){
        using T = std::remove_cv_t<std::remove_reference_t<decltype(bb)>>;
        if constexpr (std::is_same_v<T, cum::bufferD>)
        {
            return bb.size();
        }
        else
        {
            return bb.size;
        }

    };
    return std::visit(sizer, b);
}

void protocol_buffer_copy(std::vector<std::byte>& dst, cum::bufferX& src)
{
    dst.resize(protocol_buffer_size(src));
    auto copier = [&dst](auto& bb){
        using T = std::remove_cv_t<std::remove_reference_t<decltype(bb)>>;
        if constexpr (std::is_same_v<T, cum::bufferD>)
        {
            std::copy(bb.begin(), bb.end(), dst.begin());
        }
        else
        {
            memcpy(dst.data(), bb.data, bb.size);
        }
    };
    return std::visit(copier, src);
}

value_server::value_server(
    const bfc::configuration_parser& config,
    reactor_t& reactor)
    : m_config(config)
    , m_reactor(reactor)
    , m_server_socket(bfc::create_tcp4())
{
    auto port      = utils::get_config<unsigned>(config, "service.value.port").value();
    auto interface = utils::get_config<std::string>(config, "service.value.interface").value_or("127.0.0.1");

    auto bind_addr = bfc::ip4_port_to_sockaddr(interface, port);

    LOG_INF("value_server | bind_address=%s;", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str());

    if (utils::get_config<unsigned>(config, "service.value.no_delay").value_or(0))
    {
        m_server_socket.set_sock_opt(IPPROTO_TCP, TCP_NODELAY, 1);
    }

    m_server_socket.set_sock_opt(SOL_SOCKET, SO_REUSEADDR, 1);

    if (0 > m_server_socket.bind(bind_addr))
    {
        LOG_ERR("value_server | bind_address=%s; error='%s;'", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str(), strerror(errno));
        logger.flush();
        return;
    }

    m_server_rctx = m_reactor.make_context(m_server_socket.fd());
    m_reactor.add_read_rdy(m_server_rctx, [this](){
            on_accept_ready();
        });

    m_server_socket.listen();
}

void value_server::disconnect_client(std::shared_ptr<client_context>& ctx)
{
    auto& c =  *ctx;
    LOG_ERR("value_server | fd=%3d; | removing client...", c.client_socket.fd());
    m_reactor.rem_read_rdy(c.client_rctx);
    m_client_map.erase(c.client_socket.fd());
}

void value_server::read_client(std::shared_ptr<client_context>& ctx)
{
    auto& c =  *ctx;
    int read_size = 0;
    if (client_context::WAIT_HEADER == c.read_state)
    {
        read_size = 2;
    }
    else
    {
        read_size = c.expected_read_size - c.read_buffer_idx;
    }

    auto res = read(c.client_socket.fd(), c.read_buffer + c.read_buffer_idx, read_size);

    if (0 >= res)
    {
        LOG_ERR("value_server | fd=%3d; | Unexpected disconnect!", c.client_socket.fd());
        disconnect_client(ctx);
        return;
    }

    IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("value_server | fd=%3d; | read[%zu;]=%x;", c.client_socket.fd(), res, buffer_log_t(res, c.read_buffer + c.read_buffer_idx));

    c.read_buffer_idx += res;

    if (client_context::WAIT_HEADER == c.read_state)
    {
        if (c.read_buffer_idx != c.expected_read_size)
        {
            return;
        }

        std::memcpy(&c.expected_read_size, c.read_buffer, 2);
        c.read_buffer_idx = 0;
        c.read_state = client_context::WAIT_REMAINING;

        IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("value_server | fd=%3d; | expected_size=%zu;", c.client_socket.fd(), c.expected_read_size);
        return;
    }

    if (c.expected_read_size == c.read_buffer_idx)
    {
        IF_LB(LB_DUMP_MSG_RAW) LOG_INF("value_server | fd=%3d; | to_decode[%u;]=%x;",
            c.client_socket.fd(), unsigned(c.read_buffer_idx), buffer_log_t(c.read_buffer_idx, c.read_buffer));

        auto cbuff = bfc::const_buffer_view(c.read_buffer, c.read_buffer_idx);

        cum::protocol_value_server message;
        cum::per_codec_ctx dec_ctx((std::byte*)cbuff.data(), cbuff.size());
        decode_per(message, dec_ctx);

        IF_LB(LB_DUMP_MSG_PROTO)
        {
            std::string stred;
            cum::str("root", message, stred, true);
            LOG_DBG("value_server | fd=%3d; | decoded=%s;", c.client_socket.fd(), stred.c_str());
        }

        std::visit([this, &ctx](auto&& msg) mutable {
                handle(ctx, std::move(msg));
            }, std::move(message));

        c.read_state = client_context::WAIT_HEADER;
        c.expected_read_size = 2;
        c.read_buffer_idx = 0;
    }
}

void value_server::on_accept_ready()
{
    sockaddr client_addr;
    auto client_socket = m_server_socket.accept(client_addr);
    if (-1 == client_socket.fd())
    {
        return;
    }

    LOG_INF("value_server | fd=%3d; | connected client=%s;", client_socket.fd(), bfc::sockaddr_to_string(&client_addr).c_str());

    auto client_ctx = std::make_shared<client_context>();
    client_ctx->client_rctx = m_reactor.make_context(client_socket.fd());
    std::memcpy(&client_ctx->client_address, &client_addr, sizeof(client_addr));
    client_ctx->client_socket = std::move(client_socket);
    m_client_map.emplace(client_ctx->client_socket.fd(), client_ctx);

    m_reactor.add_read_rdy(client_ctx->client_rctx, [this, client_ctx]() mutable {
            read_client(client_ctx);
        });
}

size_t value_server::encode(int fd, const cum::protocol_value_client& msg, std::byte* data, size_t size)
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
        LOG_INF("value_server | fd=%3d; | to_encode=%s;", fd, stred.c_str());
    }

    IF_LB(LB_DUMP_MSG_RAW)
    {
        LOG_INF("value_server | fd=%3d; | encoded[%u;]=%x;", fd, encode_size, buffer_log_t(encode_size, data));
    }

    return encode_size;
}

value_server::value& value_server::get_value(uint32_t key)
{
    if (key >= value_map.size())
    {
        value_map.reserve(key*2);
        value_map.resize(key+1);
    }

    return value_map[key];
}

void value_server::set_value(uint32_t key, cum::bufferX&& data)
{
    auto& value = get_value(key);
    protocol_buffer_copy(value.data, data);

    if (value.subscribers.size())
    {
        cum::protocol_value_client rsp = cum::update{};
        auto& update = std::get<cum::update>(rsp);
        update.data.key = key;
        update.data.value = std::move(data);
        update.sequence_number = value.sequence_number++;

        auto size = encode(-1, rsp, send_buffer, sizeof(send_buffer));
        auto bv = bfc::const_buffer_view(send_buffer, size);

        std::vector<client_context_ptr> to_delete;
        for (auto& client : value.subscribers)
        {
            auto res = client->client_socket.send(bv);
            if (0 > res)
            {
                LOG_ERR("value_server | fd=%3d; | (can't update)", client->client_socket.fd());
                to_delete.emplace_back(client);
            }
        }

        for (auto& i : to_delete)
        {
            value.subscribers.erase(i);
        }
    }
}

void value_server::send_ack(std::shared_ptr<client_context>& client, uint16_t transaction_id, cum::EStatus status)
{
    cum::protocol_value_client rsp = cum::acknowledge{};
    auto& acknowledge = std::get<cum::acknowledge>(rsp);
    acknowledge.transaction_id = transaction_id;
    acknowledge.status = status;

    auto size = encode(-1, rsp, send_buffer, sizeof(send_buffer));
    auto bv = bfc::const_buffer_view(send_buffer, size);
    client->client_socket.send(bv);
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::set_value&& req)
{
    set_value(req.data.key, std::move(req.data.value));
    if (cum::NONTRANSACTIONAL != req.transaction_id)
    {
        send_ack(client, req.transaction_id, cum::EStatus::OK);
    }
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::get_value_request&& req)
{
    auto& value = get_value(req.key);

    cum::protocol_value_client rspu = cum::get_value_response{};
    auto& rsp = std::get<cum::get_value_response>(rspu);

    rsp.transaction_id  = req.transaction_id;
    rsp.data.value      = value.data;
    rsp.data.key        = req.key;
    rsp.sequence_number = value.sequence_number;

    auto size = encode(client->client_socket.fd(), rspu, send_buffer, sizeof(send_buffer));
    auto bv = bfc::const_buffer_view(send_buffer, size);

    client->client_socket.send(bv);
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::subscribe&& req)
{
    auto& value = get_value(req.key);
    value.subscribers.emplace(client);

    if (cum::NONTRANSACTIONAL != req.transaction_id)
    {
        send_ack(client, req.transaction_id, cum::EStatus::OK);
    }
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::unsubscribe&& req)
{
    auto& value = get_value(req.key);
    value.subscribers.erase(client);

    if (cum::NONTRANSACTIONAL != req.transaction_id)
    {
        send_ack(client, req.transaction_id, cum::EStatus::OK);
    }
}

} // namespace propertytree
