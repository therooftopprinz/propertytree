#include <value_server.hpp>

namespace propertytree
{

value_server::value_server(
    bfc::configuration_parser& config,
    bfc::epoll_reactor<>& reactor)
    : config(config)
    , reactor(reactor)
    , server(bfc::create_tcp4())
{
    auto port      = *config.as<unsigned>("service.value.port");
    auto interface = *config.arg("service.value.interface");

    server.bind(bfc::to_ip_port(0, port));
    server_ctx.emplace(reactor.make_context(server.fd()));
    reactor.add_read_rdy(*server_ctx, [this](){
            on_accept_ready();
        });
    server.listen();

}

void value_server::on_accept_ready()
{
    sockaddr client_addr;
    auto client = server.accept(client_addr);
    // client
}

} // namespace propertytree