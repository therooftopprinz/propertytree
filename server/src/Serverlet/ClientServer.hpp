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

    inline static std::shared_ptr<ClientServer> create(IEndPointPtr& endpoint, core::PTreePtr& ptree, IPTreeServer& notifier)
    {
        auto cs = std::make_shared<ClientServer>(endpoint, ptree, notifier);
        cs->init();
        return cs;
    }
    /** TODO: Privatize **/
    ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IPTreeServer& notifirer);
    uint64_t getId() {return clientServerId;}
    PTreeOutgoing& getOutgoing() {return outgoing;}
private:
    void init();
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    uint64_t clientServerId;
    ClientServerConfig config;
    IEndPointPtr endpoint; /*TODO: to reference*/
    PTreeOutgoing outgoing;
    core::PTreePtr ptree; /*TODO: to reference*/
    IPTreeServer& notifier; /*TODO: to reference*/
    PTreeIncoming incoming;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_