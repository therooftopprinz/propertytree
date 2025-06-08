#include <propertytree/value_client.hpp>
#include <propertytree/logger.hpp>

using namespace propertytree;

template <typename... Ts>
void LOG(const char* m, Ts... ars)
{
    printf(m, ars...);
}

int main(int argc, const char* argv[])
{
    propertytree::reactor_t reactor1;
    propertytree::reactor_t reactor2;

    auto rth1 = std::thread([&reactor1](){reactor1.run();});
    auto rth2 = std::thread([&reactor2](){reactor2.run();});

    propertytree::value_client::config_s config1;
    config1.log = "client1.log";
    config1.logful = false;

    propertytree::value_client::config_s config2;
    config2.log = "client2.log";
    config2.logful = false;

    propertytree::value_client client1(config1, reactor1);
    propertytree::value_client client2(config2, reactor2);
    
    auto& logger1 = client1.get_logger();
    auto& logger2 = client2.get_logger();

    logger1.logful();
    logger2.logful();

    // logger1.set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
    // logger2.set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
 
    auto c1v0 = client1.get(0);
    auto c2v0 = client2.get(0);

    (*c1v0) = 123;

    c2v0->fetch();

    LOG("c2v0: %d\n", c2v0->as<int>().value_or(std::numeric_limits<int>::max()));

    reactor1.stop();
    reactor2.stop();

    rth1.join();
    rth2.join();
}