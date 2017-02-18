#include <common/src/TcpEndPoint.hpp>
#include <common/src/Utils.hpp>
#include <client/src/PTreeClient.hpp>

#include <cstring>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write

int main()
{
    int sockfd;
    struct sockaddr_in server;
     
    //Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 6666 );
 
    //Connect to remote server
    if (connect(sockfd , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
     
    std::shared_ptr<ptree::common::TcpEndPoint> sock = std::make_shared<ptree::common::TcpEndPoint>(sockfd);
    std::shared_ptr<ptree::client::PTreeClient> ptc = std::make_shared<ptree::client::PTreeClient>(sock);

    auto ptree = ptc->getPTree();
    auto val = ptree::utils::buildBufferedValue(42);
    auto value = ptree->createValue("/Value", val);
    for(;;);


    return 0;
}