#include <signal.h>

#include <logless/logger.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>

#include <value_server.hpp>
#include <node_server.hpp>

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

    log(logger, INFO, LOGALL, "INFO | main | %s; - config:", config_file.c_str());
    for (auto i : args)
    {
        log(logger, INFO, LOGALL, "INFO | main |  %s; = %s;", i.first.c_str(), i.second.c_str());
    }

    bfc::epoll_reactor reactor;

    std::optional<propertytree::value_server> value_server;
    std::optional<propertytree::node_server>  node_server;

    auto value_port = args.arg("--service.value.port");
    auto value_port2 = args.arg("service.value.port");
    if (value_port || value_port2)
    {
        args.emplace("service.value.port", value_port.value_or(*value_port2));
        value_server.emplace(args, reactor);
    }

    auto node_port = args.arg("--service.node.port");
    auto node_port2 = args.arg("service.node.port");
    if (node_port || node_port2)
    {
        args.emplace("service.node.port", node_port.value_or(*node_port2));
        node_server.emplace(args, reactor);
    }

    reactor.run();

    return 0;
}
