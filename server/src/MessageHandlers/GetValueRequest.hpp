#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/MessageHelpers/MessageGetValueResponseSender.hpp>
#include <server/src/Types.hpp>

#ifndef SERVER_MESSAGEHANDLER_GETVALUEEQUEST_HPP_
#define SERVER_MESSAGEHANDLER_GETVALUEEQUEST_HPP_

namespace ptree
{
namespace server
{

class  GetValueRequestHandler
{
public:
    GetValueRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
        ClientServerPtr clientServer);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    IEndPointPtr endpoint;
    core::PTreePtr ptree;
    ClientServerPtr clientServer;
};

} // namespace server
} // namespace ptree

#endif // SERVER_MESSAGEHANDLER_SUBSCRIBEUPDATEREQUEST_HPP_
