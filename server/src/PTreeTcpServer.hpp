#ifndef SERVER_PTREETCPSERVER_HPP_
#define SERVER_PTREETCPSERVER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <server/src/Serverlet/IEndPoint.hpp>
#include <server/src/Serverlet/TcpEndPoint.hpp>
#include "Types.hpp"

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
    PTreeTcpServer()
    {
        
    }
    // void run()
    // {
    //     int sockfd, newsockfd, portno;
    //     socklen_t clilen;
    //     struct sockaddr_in serv_addr, cli_addr;
    //     int n;

    //     sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //     if (sockfd < 0)
    //     {
    //         error("ERROR opening socket");
    //     }

    //     bzero((char *) &serv_addr, sizeof(serv_addr));

    //     serv_addr.sin_family = AF_INET;
    //     serv_addr.sin_addr.s_addr = INADDR_ANY;
    //     serv_addr.sin_port = htons(portno);
    //     if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    //     {
    //         error("ERROR on binding");
    //     } 

    //     listen(sockfd, 5);
    //     clilen = sizeof(cli_addr);

    //     while (true)
    //     {
    //         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,&clilen);
    //         if (newsockfd < 0)
    //             continue;

    //         TcpEndPoint ep (newsockfd);
    //     }
    // }
private:
    int socketfd;
    struct sockaddr;
    int portno;
};


} // namespace server

} // namespace ptree

#endif  // SERVER_PTREETCPSERVER_HPP_