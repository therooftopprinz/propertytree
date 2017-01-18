#ifndef SERVER_PTREETCPSERVER_HPP_
#define SERVER_PTREETCPSERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <common/src/IEndPoint.hpp>
#include <common/src/TcpEndPoint.hpp>
#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/PTree.hpp>
#include "Types.hpp"
#include <common/src/Logger.hpp>

namespace ptree
{
namespace server
{

class ClientServerService
{
public:
private:
};

class PTreeTcpServer
{
public:
    PTreeTcpServer();
    void run();
private:
    void serverlet(ClientServerPtr cs);
    int socketfd;
    int portno;
    std::shared_ptr<core::PTree> ptree;
    std::shared_ptr<IClientNotifier> monitor;
    logger::Logger log;
};


} // namespace server

} // namespace ptree

#endif  // SERVER_PTREETCPSERVER_HPP_