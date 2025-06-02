#include <value_server.hpp>
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

void value_server::disconnect_client(std::shared_ptr<client_context>& context)
{
    auto& c = *context;
    LOG_ERR("value_server | fd=%d; | removing client...", c.client_socket.fd());
    m_reactor.rem_read_rdy(c.client_rctx);
    m_client_map.erase(c.client_socket.fd());
}

void value_server::read_client(std::shared_ptr<client_context>& context)
{
    auto& c = *context;
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
        LOG_ERR("value_server | fd=%d; | Unexpected disconnect!", c.client_socket.fd());
        disconnect_client(context);
        return;
    }

    c.read_buffer_idx += res;

    if (client_context::WAIT_HEADER == c.read_state)
    {
        std::memcpy(&c.expected_read_size, c.read_buffer, 2);
        c.read_buffer_idx = 0;
        c.read_state = client_context::WAIT_REMAINING;
        return;
    }

    if (c.expected_read_size == c.read_buffer_idx)
    {
        IF_LB(LB_DUMP_CLIENT_RAW) LOG_DBG("value_server | fd=%d; | data=%x;", c.client_socket.fd(), buffer_log_t(c.read_buffer_idx, c.read_buffer));

        auto cbuff = bfc::const_buffer_view(c.read_buffer, c.read_buffer_idx);

        // decode message and process
        cum::protocol_value message;
        cum::per_codec_ctx dec_ctx((std::byte*)cbuff.data(), cbuff.size());
        decode_per(message, dec_ctx);

        IF_LB(LB_DUMP_CLIENT_PM)
        {
            std::string stred;
            cum::str("root", message, stred, true);
            LOG_DBG("value_server | fd=%d; | message=%x;", c.client_socket.fd(), stred.c_str());
        }

        std::visit([this, context](auto&& msg) mutable {
                handle(context, std::move(msg));
            }, std::move(message));

        c.read_state = client_context::WAIT_HEADER;
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

    auto client_ctx = std::make_shared<client_context>();
    client_ctx->client_rctx = m_reactor.make_context(client_socket.fd());
    std::memcpy(&client_ctx->client_address, &client_addr, sizeof(client_addr));
    client_ctx->client_socket = std::move(client_socket);
    m_client_map.emplace(client_ctx->client_socket.fd(), client_ctx);

    m_reactor.add_read_rdy(client_ctx->client_rctx, [this, client_ctx]() mutable {
            read_client(client_ctx);
        });
}

} // namespace propertytree
