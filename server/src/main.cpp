#include <functional>
#include <string>
#include <sstream>

#include <signal.h>

#include <logless/logger.hpp>
#include <bfc/epoll_reactor.hpp>
#include <bfc/configuration_parser.hpp>

#include <udp_value_server.hpp>
#include <tcp_value_server.hpp>
#include <node_server.hpp>
#include <logger.hpp>
#include <utils.hpp>
#include <value_map.hpp>

using logless::log;
using logless::INFO;
using logless::LOGALL;

logless::logger logger("server.log");

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

    std::stringstream lbss(utils::get_config<std::string>(args, "log.bit").value_or(""));

    uint64_t bit = 0;
    while(lbss.good())
    {
        std::string substr;
        std::getline(lbss, substr, ',' );

        for(;substr.size() && ' ' == substr.front(); substr.erase(0,1));
        for(;substr.size() && ' ' == substr.back();  substr.pop_back());

        auto bitit = lbmap.find(substr);
        if (lbmap.end() != bitit)
        {
            auto lbit = bitit->second;
            LOG_INF("main | log.bit: %-24s; %16lx;", substr.c_str(), lbit);
            bit |= lbit;
        }
    }

    logger.set_logbit(bit);

    LOG_INF("main | log.bit: %lx;", logger.get_logbit());

    LOG_INF("main | %s; - config:", config_file.c_str());
    for (auto i : args)
    {
        LOG_INF("main |  %s; = %s;", i.first.c_str(), i.second.c_str());
    }

    if (0 == utils::get_config<unsigned>(args, "log.ful").value_or(0))
    {
        logger.logless();
    }

    if (auto lvl = utils::get_config<unsigned>(args, "log.level"))
    {
        logger.set_level(lvl.value());
    }

    bfc::epoll_reactor<std::function<void()>> reactor;
    propertytree::value_map value_map;


    std::optional<propertytree::udp_value_server> udp_value_server;
    std::optional<propertytree::tcp_value_server> tcp_value_server;
    // std::optional<propertytree::node_server>  node_server;

    auto value_port = utils::get_config<unsigned>(args, "service.value.port");
    if (value_port)
    {
        udp_value_server.emplace(&args, &reactor, &value_map);
        tcp_value_server.emplace(&args, &reactor, &value_map);
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
