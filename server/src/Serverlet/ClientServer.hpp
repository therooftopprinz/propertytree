#ifndef SERVER_CLIENTSERVER_HPP_
#define SERVER_CLIENTSERVER_HPP_

#include <mutex>
#include <functional>

#include <server/src/PTreeTcpServer.hpp>
#include <server/src/PTree.hpp>
#include <common/src/Logger.hpp>
#include <server/src/Types.hpp>

#include "ClientServerConfig.hpp"
#include "PTreeOutgoing.hpp"
#include "PTreeIncoming.hpp"

namespace ptree
{
namespace server
{

struct MessageHandlerFactory;

class ClientServer : public std::enable_shared_from_this<ClientServer>
{
public:
    ClientServer(const ClientServer&) = delete;
    ClientServer(ClientServer&) = delete;
    void operator = (const ClientServer&) = delete;
    void operator = (ClientServer&) = delete;

    ~ClientServer();

    ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IPTreeServer& notifirer);
    PTreeOutgoing& getOutgoing() {return *outgoing;}
private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    ClientServerConfig config;
    IEndPointPtr endpoint;
    PTreeOutgoingPtr outgoing;
    core::PTreePtr ptree; /*TODO: to reference*/
    PTreeIncoming incoming;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_