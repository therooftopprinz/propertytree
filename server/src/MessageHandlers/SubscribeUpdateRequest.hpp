#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/MessageHelpers/MessageSubscribeUpdateNotificationResponseSender.hpp>
#include <server/src/Types.hpp>

#ifndef SERVER_MESSAGEHANDLER_SUBSCRIBEUPDATEREQUEST_HPP_
#define SERVER_MESSAGEHANDLER_SUBSCRIBEUPDATEREQUEST_HPP_

namespace ptree
{
namespace server
{

class  UpdateNotificationHandler : public std::enable_shared_from_this<UpdateNotificationHandler>
{
public:
    UpdateNotificationHandler(ClientServerWkPtr clientServer);
    bool handle(core::ValuePtr value);
private:
    ClientServerWkPtr clientServer;
};


class  SubscribePropertyUpdateRequestHandler
{
public:
    SubscribePropertyUpdateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
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
