#include <tcp_value_server.hpp>
#include <logger.hpp>
#include <utils.hpp>

namespace propertytree
{

tcp_value_server::tcp_value_server(
    const bfc::configuration_parser* c,
    reactor_t* r,
    value_map* v)
    : m_config(c)
    , m_reactor(r)
    , m_value_map(v)
    , m_server_socket(bfc::create_tcp4())
{
    auto port      = utils::get_config<unsigned>(*c, "service.value.port").value();
    auto interface = utils::get_config<std::string>(*c, "service.value.interface").value_or("127.0.0.1");

    auto bind_addr = bfc::ip4_port_to_sockaddr(interface, port);

    LOG_INF("tcp_value_server | bind_address=%s;", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str());

    m_server_socket.set_sock_opt(SOL_SOCKET, SO_REUSEADDR, 1);

    if (0 > m_server_socket.bind(bind_addr))
    {
        LOG_ERR("tcp_value_server | bind_address=%s; error='%s;'", bfc::sockaddr_to_string((sockaddr*) &bind_addr).c_str(), strerror(errno));
        logger.flush();
        return;
    }

    m_server_rctx = r->make_context(m_server_socket.fd());
    m_reactor->add_read_rdy(m_server_rctx, [this](){
            on_accept_ready();
        });

    m_server_socket.listen();
}

void tcp_value_server::disconnect_client(std::shared_ptr<client_context>& ctx)
{
    auto& c =  *ctx;
    LOG_ERR("tcp_value_server | fd=%3d; | removing client...", c.client_socket.fd());
    m_reactor->rem_read_rdy(c.client_rctx);
    m_client_map.erase(c.client_socket.fd());
}

void tcp_value_server::read_client(std::shared_ptr<client_context>& ctx)
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
        LOG_ERR("tcp_value_server | fd=%3d; | Unexpected disconnect!", c.client_socket.fd());
        disconnect_client(ctx);
        return;
    }

    IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("tcp_value_server | fd=%3d; | read[%zu;]=%x;", c.client_socket.fd(), res, buffer_log_t(res, c.read_buffer + c.read_buffer_idx));

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

        IF_LB(LB_DUMP_MSG_SOCK) LOG_INF("tcp_value_server | fd=%3d; | expected_size=%zu;", c.client_socket.fd(), c.expected_read_size);
        return;
    }

    if (c.expected_read_size == c.read_buffer_idx)
    {
        IF_LB(LB_DUMP_MSG_RAW) LOG_INF("tcp_value_server | fd=%3d; | to_decode=%x;", c.client_socket.fd(), buffer_log_t(c.read_buffer_idx, c.read_buffer));

        auto cbuff = bfc::const_buffer_view(c.read_buffer, c.read_buffer_idx);

        cum::protocol_value_server message;
        cum::per_codec_ctx dec_ctx((std::byte*)cbuff.data(), cbuff.size());
        decode_per(message, dec_ctx);

        IF_LB(LB_DUMP_MSG_PROTO)
        {
            std::string stred;
            cum::str("root", message, stred, true);
            LOG_DBG("tcp_value_server | fd=%3d; | decoded=%s;", c.client_socket.fd(), stred.c_str());
        }

        std::visit([this, &ctx](auto&& msg) mutable {
                handle(ctx, std::move(msg));
            }, std::move(message));

        c.read_state = client_context::WAIT_HEADER;
        c.read_buffer_idx = 0;
    }
}

void tcp_value_server::on_accept_ready()
{
    sockaddr client_addr;
    auto client_socket = m_server_socket.accept(client_addr);
    if (-1 == client_socket.fd())
    {
        return;
    }

    LOG_INF("tcp_value_server | fd=%3d; | connected client=%s;", client_socket.fd(), bfc::sockaddr_to_string(&client_addr).c_str());

    auto rctx = m_reactor->make_context(client_socket.fd());
    auto client_ctx = std::make_shared<client_context>(std::move(client_socket), std::move(rctx), client_addr);
    std::memcpy(&client_ctx->client_address, &client_addr, sizeof(client_addr));
    m_client_map.emplace(client_ctx->client_socket.fd(), client_ctx);

    m_reactor->add_read_rdy(client_ctx->client_rctx, [this, client_ctx]() mutable {
            read_client(client_ctx);
        });
}

size_t tcp_value_server::encode(int fd, const cum::protocol_value_client& msg, std::byte* data, size_t size)
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
        LOG_INF("tcp_value_server | fd=%3d; | to_encode=%s;", fd, stred.c_str());
    }

    IF_LB(LB_DUMP_MSG_RAW)
    {
        LOG_INF("tcp_value_server | fd=%3d; | encoded[%u;]=%x;", fd, encode_size, buffer_log_t(encode_size, data));
    }

    return encode_size;
}

void tcp_value_server::handle(std::shared_ptr<client_context>& client, cum::subscribe&& msg)
{
    auto& value = m_value_map->get_value(msg.id);
    value.data_subscribers.emplace(client);

    cum::protocol_value_client smsg = cum::acknowledge{};
    auto& acknowledge = std::get<cum::acknowledge>(smsg);
    acknowledge.transaction_id = msg.transaction_id;
    acknowledge.status = 0;

    std::byte m_snd_buff[ENCODE_SIZE];
    auto size = encode(client->client_socket.fd(), smsg, m_snd_buff, sizeof(m_snd_buff));
    auto bv = bfc::const_buffer_view(m_snd_buff, size);
    client->send(bv);

}

void tcp_value_server::handle(std::shared_ptr<client_context>& client, cum::unsubscribe&& msg)
{
    auto& value = m_value_map->get_value(msg.id);
    value.data_subscribers.erase(client);
}

} // namespace propertytree
