#ifndef SERVER_MESSAGEHANDLER_SIGNINREQUEST_HPP_
#define SERVER_MESSAGEHANDLER_SIGNINREQUEST_HPP_

#include <server/src/ClientServer.hpp>
#include <server/src/MessageHelpers/MessageSignInResponseSender.hpp>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{

class SigninRequestHandler
{
public:
    SigninRequestHandler(IEndPointPtr endpoint, ClientServerPtr clientServer);
    void handle(protocol::MessageHeaderPtr header, BufferPtr message);
private:
    IEndPointPtr endpoint;
    ClientServerPtr clientServer;
};

}
}
#endif // SERVER_MESSAGEHANDLER_SIGNINREQUEST_HPP_
