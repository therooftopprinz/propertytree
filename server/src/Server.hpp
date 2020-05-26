#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <map>

#include <bfc/EpollReactor.hpp>

#include <logless/Logger.hpp>

#include <IServer.hpp>
#include <ConnectionSession.hpp>
#include <ProtocolHandler.hpp>

namespace propertytree
{

class Server : public IServer
{
public:
    Server();
    void run();

private:

    void onDisconnect(int pFd);
    void handleServerRead();

    bfc::EpollReactor mReactor;
    int mServerFd;

    std::map<int, std::shared_ptr<ConnectionSession>> mConnections;
    std::mutex mConnectionsMutex;

    ProtocolHandler mProto;
};

} // propertytree

#endif // __SERVER_HPP__