#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>
#include <strings.h>
#include <server/src/IPTreeServer.hpp>
#include <server/src/PTreeTcpServer.hpp>

namespace ptree
{
namespace server
{

PTreeTcpServer::PTreeTcpServer()
    : portno(6666)
    , ptree(std::make_shared<core::PTree>(std::make_shared<core::IdGenerator>()))
    , log("PTreeTcpServer")
{}

void PTreeTcpServer::run()
{
    log << logger::DEBUG << "RUNNING ON localhost:" << portno;

    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        log << logger::ERROR << "ERROR opening socket";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        log << logger::ERROR << "ERROR binding socket";
        return;
    } 

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (true)
    {
        log << logger::DEBUG << "WATING FOR NEW CONNECTION";
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen);

        if (newsockfd < 0)
        {
            log << logger::DEBUG << "ACCEPT FAILED";
            continue;
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

        log << logger::DEBUG << "ACCEPTING NEW CONNECTION";
        IEndPointPtr ep = std::make_shared<TcpEndPoint>(newsockfd);
        ptreeServer.create(ep, ptree);
    }
}

} // namespace server
} // namespace ptree
