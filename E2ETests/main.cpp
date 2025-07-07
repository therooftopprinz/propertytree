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
    propertytree::value_client::config_s config2;

    config1.logful = false;
    config2.logful = false;
    // config1.no_delay = true;
    // config2.no_delay = true;
    config1.log = "client1.log";
    config2.log = "client2.log";

    client1.emplace(config1, reactor);
    client2.emplace(config2, reactor);

    // client1->get_logger().set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
    // client2->get_logger().set_logbit(LB_DUMP_MSG_RAW | LB_DUMP_MSG_PROTO | LB_DUMP_MSG_SOCK);
    client1->get_logger().set_level(logless::FATAL);
    client2->get_logger().set_level(logless::FATAL);
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

void hot_delay(uint64_t n, uint64_t tt = now())
{
    while (tt+n > now());
}

void TEST_SET_UPDATE_LATENCY(uint64_t id=0, uint64_t period=50, uint64_t N = 100000u)
{
    auto c1v0 = client1->get(id);
    auto c2v0 = client2->get(id);

    double tt = 0;
    std::atomic_uint64_t n = 0;

    struct test_t
    {
        uint64_t n = 0;
        uint64_t t = 0;
    };

    std::vector<std::tuple<uint64_t,uint64_t>> lat;
    lat.reserve(N);

    c2v0->subscribe([&tt, &n, &lat](const propertytree::buffer& data){
            if (!data.size())
            {
                assert(false);
            }
            auto ct = now();
            auto st = as<test_t>(data);
            auto diff = (ct-st.t);
            tt += diff;

            // lat.emplace_back(std::make_tuple(st.n, ct-st.t));

            n++;
        });

    auto t0 = now();
    test_t t{};
    for (auto i=0u; i<N; i++)
    {
        t.t = now();
        t.n = i;
        *c1v0 = t;
        hot_delay(period, t.t);
    }

    while (n.load() < N);
    auto t1 = now();

    {
        std::ofstream ocsv("lat_" + std::to_string(id) + ".csv", std::ios::binary | std::ios::trunc);
        for (auto& i : lat)
        {
            ocsv << std::get<0>(i) << ", " << std::get<1>(i) << "\n";
        }
    }

    auto tdiff_s = double(t1-t0)/1000000;
    auto ave_lat = tt/N;

    LOG("TEST_SET_UPDATE_LATENCY | average_latency_ms=%lf\n", ave_lat/1000);
    LOG("TEST_SET_UPDATE_LATENCY | tdiff_s=%lf throughput=%.3lf\n", tdiff_s, double(N)/tdiff_s);
}

int main(int argc, const char* argv[])
{
    bfc::configuration_parser args;
    for (auto i=1; i<argc; i++)
    {
        args.load_line(argv[i]);
    }

    auto rth = std::thread([](){reactor.run();});

    init_clients();

    TEST_SET_UPDATE_LATENCY(
        args.as<uint64_t>("--id").value_or(0),
        args.as<uint64_t>("--period").value_or(50));

    reactor.stop();
    rth.join();
}