#include <cassert>

#include <propertytree/value_client.hpp>
#include <propertytree/logger.hpp>

using namespace propertytree;

template <typename... Ts>
void LOG(const char* m, Ts... ars)
{
    printf(m, ars...);
}

propertytree::reactor_t reactor;

std::optional<propertytree::value_client> client1;
std::optional<propertytree::value_client> client2;

void init_clients()
{
    propertytree::value_client::config_s config1;
    config1.log = "client1.log";

    propertytree::value_client::config_s config2;
    config2.log = "client2.log";

    config1.logful = true;
    config2.logful = true;

    client1.emplace(config1, reactor);
    client2.emplace(config2, reactor);

    client1->get_logger().set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
    client2->get_logger().set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
}

template <typename T>
T as(const propertytree::buffer& b)
{
    return *(T*)(b.data());
}

template <typename T=std::chrono::microseconds>
uint64_t now()
{
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

constexpr auto N = 100000u;

void TEST_SET_UPDATE_LATENCY()
{
    auto c1v0 = client1->get(0);
    auto c2v0 = client2->get(0);

    double tt = 0;
    std::atomic_uint64_t n = 0;

    c2v0->subscribe([&tt, &n](const propertytree::buffer& data){
            if (!data.size())
            {
                assert(false);
            }
            auto st = as<uint64_t>(data);
            auto ct = now();
            double diff = (ct-st);
            tt += diff;
            n++;
        });

    auto t0 = now();
    for (auto i=0u; i<N; i++)
    {
        *c1v0 = now();
    }

    while (n.load() < N);
    auto t1 = now();

    auto tdiff_s = double(t1-t0)/1000000;
    auto ave_lat = tt/N;

    LOG("TEST_SET_UPDATE_LATENCY | average_latency_ms=%lf\n", ave_lat/1000);
    LOG("TEST_SET_UPDATE_LATENCY | tdiff_s=%lf throughput=%.3lf\n", tdiff_s, double(N)/tdiff_s);
}

int main(int argc, const char* argv[])
{
    auto rth = std::thread([](){reactor.run();});

    init_clients();

    TEST_SET_UPDATE_LATENCY();

    reactor.stop();
    rth.join();
}