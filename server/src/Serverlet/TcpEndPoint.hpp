#ifndef SERVER_TCPENDPOINT_HPP_
#define SERVER_TCPENDPOINT_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <bits/time.h>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{

class TcpEndPoint : public IEndPoint
{
public:
    TcpEndPoint(int sockfd) :
        sockfd(sockfd), flags(0)
    {
    }

    ~TcpEndPoint()
    {
        // ::close(sockfd);
    }

    inline ssize_t send(const void *buffer, uint32_t size)
    {
        // return ::send(sockfd, buffer, size, flags);
        return 0;
    }

    inline ssize_t receive(void *buffer, uint32_t size)
    {
        // return ::receive(sockfd, buffer, size, flags);
        return 0;
    }

    inline void setBlockingTimeout(int secs, int microsecs)
    {
        struct timeval tv;
        tv.tv_sec = secs;
        tv.tv_usec = microsecs;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    }

private:
    int sockfd;
    int flags;
};

} // namespace server

} // namespace ptree

#endif  // SERVER_IENDPOINT_HPP_