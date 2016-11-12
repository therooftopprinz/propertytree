#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/MessageHelpers/MessageUnsubscribeUpdateNotificationResponseSender.hpp>
#include <server/src/Types.hpp>

#ifndef SERVER_MESSAGEHANDLER_UNSUBSCRIBEUPDATEREQUEST_HPP_
#define SERVER_MESSAGEHANDLER_UNSUBSCRIBEUPDATEREQUEST_HPP_

namespace ptree
{
namespace server
{

class  UnsubscribePropertyUpdateRequestHandler
{
public:
    UnsubscribePropertyUpdateRequestHandler(IEndPointPtr endpoint, core::PTreePtr ptree,
        ClientServerPtr clientServer);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    IEndPointPtr endpoint;
    core::PTreePtr ptree;
    ClientServerPtr clientServer;
};

} // namespace server
} // namespace ptree

#endif // SERVER_MESSAGEHANDLER_UNSUBSCRIBEUPDATEREQUEST_HPP_
