#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <Server.hpp>

namespace propertytree
{

Server::Server()
    : mProto([this](){mReactor.stop();})
{
    mServerFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == mServerFd)
    {
        throw std::runtime_error(strerror(errno));
    }

    const int one = 1;
    int res = setsockopt(mServerFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if (res)
    {
        throw std::runtime_error(strerror(errno));
    }

    uint16_t port = 12345;
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = ntohs(port);

    char loc[24];
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, loc, sizeof(loc));
    Logless("Server: binding _:_", loc, port);

    res = bind(mServerFd, (sockaddr*)&addr, sizeof(addr));

    if  (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }

    res = listen(mServerFd, 100);

    if  (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }

    if (!mReactor.addHandler(mServerFd, [this](){
            handleServerRead();
        }))
    {
        throw std::runtime_error("Server: Failed to register server to EpollReactor.");
    }
}

void Server::run()
{
    mReactor.run();
}

void Server::onDisconnect(int pFd)
{
    Logless("Server: client disconnected fd=_", pFd);
    mReactor.removeHandler(pFd);
    auto connectionRaw = mConnections.find(pFd)->second.get();
    mConnections.erase(pFd);
    mProto.onDisconnect(connectionRaw);
}

void Server::handleServerRead()
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    auto res = accept(mServerFd, (sockaddr*)&addr, &len);

    if  (-1 == res)
    {
        throw std::runtime_error(strerror(errno));
    }

    char loc[24];
    inet_ntop(AF_INET, &addr.sin_addr.s_addr, loc, sizeof(loc));
    Logless("Server: connected client fd=_ address=_:_", res, loc, ntohs(addr.sin_port));

    std::unique_lock<std::mutex> lg(mConnectionsMutex);
    auto session = std::make_shared<ConnectionSession>(res, *this, mProto);
    mConnections.emplace(res, session);

    lg.unlock();

    if (!mReactor.addHandler(res, [session](){
            session->handleRead();
        }))
    {
        Logless("Server: Failed to register connection fd=_ to EpollReactor, errno=\"_\"", res, strerror(errno));
        onDisconnect(res);
    }
}

} // propertytree
