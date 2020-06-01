#include <signal.h>

#include <Server.hpp>

#include <bfc/Singleton.hpp>
#include <bfc/Timer.hpp>
#include <bfc/ThreadPool.hpp>

using namespace propertytree;

int main()
{
    signal(SIGPIPE, SIG_IGN);
    Logger::getInstance().logful();

    bfc::Singleton<bfc::ThreadPool<>>::instantiate();
    auto& timer = bfc::Singleton<bfc::Timer<>>::instantiate();

    std::thread timerThread([&timer]{
        timer.run();
    });

    Server server;
    server.run();

    timer.stop();
    timerThread.join();
}
