#ifndef SERVER_MESSAGEHANDLER_DELETEREQUEST_HPP_
#define SERVER_MESSAGEHANDLER_DELETEREQUEST_HPP_

#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/MessageHelpers/MessageDeleteResponseSender.hpp>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{

class DeleteRequestHandler
{
public:
    DeleteRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
        IClientServerMonitorPtr monitor, ClientServerPtr clientServer);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);

private:
    IEndPointPtr endpoint;
    core::PTreePtr ptree;
    IClientServerMonitorPtr monitor;
    ClientServerPtr clientServer;
};

}
}
#endif // SERVER_MESSAGEHANDLER_DELETEREQUEST_HPP_
