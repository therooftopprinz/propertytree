#include <string.h>
#include "TcpEndPoint.hpp"
namespace ptree
{
namespace common
{


TcpEndPoint::TcpEndPoint(int sockfd) :
    sockfd(sockfd), flags(0), log("TcpEndPoint")
{
}

TcpEndPoint::~TcpEndPoint()
{
    close(sockfd);
}

ssize_t TcpEndPoint::send(const void *buffer, uint32_t size)
{
    auto rv = ::send(sockfd, buffer, size, flags);
    int errcode = errno;
    if (rv == -1)
    {
        log << logger::ERROR << "send error occured!! error: " << strerror(errcode);
        return 0;
    }
    return rv;
}

ssize_t TcpEndPoint::receive(void *buffer, uint32_t size)
{
    auto rv = recv(sockfd, buffer, size, flags);
    int errcode = errno;
    if (rv == -1 && (errcode != EAGAIN || errcode != EWOULDBLOCK))
    {
        log << logger::ERROR << "receiv error occured!! error: " << strerror(errcode);
        return 0;
    }
    else if (rv ==-1)
    {
        return 0;
    }
    return rv;
}

void TcpEndPoint::setBlockingTimeout(int secs, int microsecs)
{
    struct timeval tv;
    tv.tv_sec = secs;
    tv.tv_usec = microsecs;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
}

} // namespace server

} // namespace ptree