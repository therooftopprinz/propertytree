#ifndef SERVER_MESSAGEHANDLER_CREATEREQUEST_HPP_
#define SERVER_MESSAGEHANDLER_CREATEREQUEST_HPP_

#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/Types.hpp>
#include <server/src/MessageHelpers/MessageCreateResponseSender.hpp>

namespace ptree
{
namespace server
{

class CreateRequestHandler
{
public:
    CreateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree, IClientServerMonitorPtr monitor,
        ClientServerPtr clientServer);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);

private:
    IEndPointPtr endpoint;
    core::PTreePtr ptree;
    IClientServerMonitorPtr monitor;
    ClientServerPtr clientServer;
};

}
}
#endif // SERVER_MESSAGEHANDLER_CREATEREQUEST_HPP_
