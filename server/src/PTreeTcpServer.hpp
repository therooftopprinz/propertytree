#ifndef SERVER_PTREETCPSERVER_HPP_
#define SERVER_PTREETCPSERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include "Types.hpp"

namespace ptree
{
namespace server
{

struct IEndPoint
{
    virtual ssize_t send(const void *buffer, uint32_t size) = 0;
    virtual ssize_t receive(void *buffer, uint32_t size) = 0;
    // virtual void setReceiveTimeout(uint32_t timeout);
};

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