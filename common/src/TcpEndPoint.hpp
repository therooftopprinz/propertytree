#ifndef SERVER_TCPENDPOINT_HPP_
#define SERVER_TCPENDPOINT_HPP_

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bits/time.h>
#include <server/src/Types.hpp>
#include <common/src/IEndPoint.hpp>

namespace ptree
{
namespace common
{

class TcpEndPoint : public IEndPoint
{
public:
    TcpEndPoint(int sockfd);
    ~TcpEndPoint();
    ssize_t send(const void *buffer, uint32_t size);
    ssize_t receive(void *buffer, uint32_t size);
    void setBlockingTimeout(int secs, int microsecs);

private:
    int sockfd;
    int flags;
};

} // namespace server

} // namespace ptree

#endif  // SERVER_IENDPOINT_HPP_