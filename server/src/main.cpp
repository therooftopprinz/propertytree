#include <functional>

#include <signal.h>

#include <logless/logger.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>

#include <value_server.hpp>
#include <node_server.hpp>
#include <logger.hpp>
#include <utils.hpp>


using logless::log;
using logless::INFO;
using logless::LOGALL;

logless::logger logger("server.bin");

int main(int argc, const char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    logger.logful();

    bfc::configuration_parser args;
    for (auto i=1; i<argc; i++)
    {
        args.load_line(argv[i]);
    }

    auto config_file = args.arg("--config").value_or("server.cfg");
    args.load(config_file);

    LOG_INF("main | %s; - config:", config_file.c_str());
    for (auto i : args)
    {
        LOG_INF("main |  %s; = %s;", i.first.c_str(), i.second.c_str());
    }

    bfc::epoll_reactor<std::function<void()>> reactor;

    std::optional<propertytree::value_server> value_server;
    // std::optional<propertytree::node_server>  node_server;

    auto value_port = utils::get_config<unsigned>(args, "service.value.port");
    if (value_port)
    {
        value_server.emplace(args, reactor);
    }

    auto node_port = utils::get_config<unsigned>(args, "service.node.port");
    if (node_port)
    {
        // args.emplace("service.node.port", node_port.value_or(*node_port2));
        // node_server.emplace(args, reactor);
    }

    reactor.run();

    return 0;
}
