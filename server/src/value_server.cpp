#include <value_server.hpp>
#include <netinet/tcp.h>
#include <logger.hpp>
#include <utils.hpp>

namespace propertytree
{

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

value_server::value& value_server::get_value(uint64_t id)
{
    if (id >= value_map.size())
    {
        value_map.reserve(id*2);
        value_map.resize(id+1);
    }

    return value_map[id];
}

void value_server::set_value(uint64_t id, std::vector<uint8_t>&& data)
{
    auto& value = get_value(id);
    value.data  = std::move(data);

    if (value.subscribers.size())
    {
        cum::protocol_value_client rsp = cum::update{};
        auto& update = std::get<cum::update>(rsp);
        update.data.id = id;
        update.data.data = value.data;
        update.sequence_number = value_sequence;

        std::byte buffer[ENCODE_SIZE];
        auto size = encode(-1, rsp, buffer, sizeof(buffer));
        auto bv = bfc::const_buffer_view(buffer, size);

        std::vector<client_context_ptr> to_delete;
        for (auto& client : value.subscribers)
        {
            auto res = client->client_socket.send(bv);
            if (0 > res)
            {
                LOG_DBG("value_server | fd=%3d; | (can't update)", client->client_socket.fd());
                to_delete.emplace_back(client);
            }
            else
            {
                LOG_DBG("value_server | fd=%3d; | (to update %" PRIu64 ";)", client->client_socket.fd(), value_sequence);
            }
        }

        for (auto& i : to_delete)
        {
            value.subscribers.erase(i);
        }
    }
}

void value_server::handle(std::shared_ptr<client_context>&, cum::set_value&& msg)
{
    set_value(msg.id, std::move(msg.data));
    value_sequence++;
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::get_value_request&& req)
{
    auto& value = get_value(req.id);

    cum::protocol_value_client rspu = cum::get_value_response{};
    auto& rsp = std::get<cum::get_value_response>(rspu);

    rsp.transaction_id  = req.transaction_id;
    rsp.data.data       = value.data;
    rsp.data.id         = req.id;
    rsp.sequence_number = value_sequence;

    std::byte buffer[ENCODE_SIZE];
    auto size = encode(client->client_socket.fd(), rspu, buffer, sizeof(buffer));
    auto bv = bfc::const_buffer_view(buffer, size);

    client->client_socket.send(bv);
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::subscribe&& msg)
{
    auto& value = get_value(msg.id);
    value.subscribers.emplace(client);
}

void value_server::handle(std::shared_ptr<client_context>& client, cum::unsubscribe&& msg)
{
    auto& value = get_value(msg.id);
    value.subscribers.erase(client);
}

} // namespace propertytree
