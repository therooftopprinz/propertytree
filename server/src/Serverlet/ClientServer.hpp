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
    ClientServer(IEndPointPtr endpoint, core::PTree& ptree, IPTreeServer& notifirer);
    ~ClientServer();

    ClientServer(const ClientServer&) = delete;
    void operator = (const ClientServer&) = delete;

    PTreeOutgoing& getOutgoing() {return *outgoing;} /** TODO: REFACTOR **/
private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    ClientServerConfig config;
    IEndPointPtr endpoint;
    PTreeOutgoingPtr outgoing;
    PTreeIncoming incoming;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_