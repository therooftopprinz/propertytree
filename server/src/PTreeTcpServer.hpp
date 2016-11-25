#ifndef SERVER_PTREETCPSERVER_HPP_
#define SERVER_PTREETCPSERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <server/src/Serverlet/IEndPoint.hpp>
#include <server/src/Serverlet/TcpEndPoint.hpp>
#include <server/src/Serverlet/ClientServer.hpp>
#include <server/src/PTree.hpp>
#include "Types.hpp"
#include "Logger.hpp"

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
    std::shared_ptr<IClientServerMonitor> monitor;
    logger::Logger log;
};


} // namespace server

} // namespace ptree

#endif  // SERVER_PTREETCPSERVER_HPP_