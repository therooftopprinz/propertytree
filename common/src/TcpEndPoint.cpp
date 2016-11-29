#include "TcpEndPoint.hpp"

namespace ptree
{
namespace common
{


TcpEndPoint::TcpEndPoint(int sockfd) :
    sockfd(sockfd), flags(0)
{
}

TcpEndPoint::~TcpEndPoint()
{
    close(sockfd);
}

ssize_t TcpEndPoint::send(const void *buffer, uint32_t size)
{
    return ::send(sockfd, buffer, size, flags);
}

ssize_t TcpEndPoint::receive(void *buffer, uint32_t size)
{
    return recv(sockfd, buffer, size, flags);
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