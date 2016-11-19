#ifndef SERVER_PTREETCPSERVER_HPP_
#define SERVER_PTREETCPSERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <server/src/Serverlet/IEndPoint.hpp>
#include "Types.hpp"

namespace ptree
{
namespace server
{


class TcpEndPoint : public IEndPoint
{
public:
    ssize_t send(void *buffer, int size);
    ssize_t receive(void *buffer, int size);
private:
    int socketfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
};

class ClientServerService
{
public:
private:
};

class PTreeTcpServer
{
public:
    PTreeTcpServer()
    {
        
    }
private:
    int socketfd;
    struct sockaddr;
};


} // namespace server

} // namespace ptree

#endif  // SERVER_PTREETCPSERVER_HPP_