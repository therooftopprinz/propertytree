#ifndef SERVER_MESSAGEHANDLER_SETVALUEINDICATION_HPP_
#define SERVER_MESSAGEHANDLER_SETVALUEINDICATION_HPP_

#include <server/src/PTree.hpp>
#include <server/src/ClientServer.hpp>
#include <server/src/Types.hpp>

namespace ptree
{
namespace server
{

class SetValueIndicationHandler
{
public:
    SetValueIndicationHandler(IEndPointPtr endpoint, core::PTreePtr ptree, IClientServerMonitorPtr monitor,
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
#endif // SERVER_MESSAGEHANDLER_SETVALUEINDICATION_HPP_
