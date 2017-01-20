#ifndef SERVER_CLIENTSERVER_HPP_
#define SERVER_CLIENTSERVER_HPP_

#include <mutex>
#include <functional>

#include <server/src/PTreeTcpServer.hpp>
#include <server/src/PTree.hpp>
#include <common/src/Logger.hpp>
#include <server/src/Types.hpp>

#include "IClientServer.hpp"
#include "ClientServerConfig.hpp"
#include "PTreeOutgoing.hpp"
#include "PTreeIncoming.hpp"

namespace ptree
{
namespace server
{

class IClientNotifier
{
public:
    IClientNotifier() {}
    virtual ~IClientNotifier() {}
    virtual void addClientServer(uint64_t clienServerId, IPTreeOutgoing& clientNotifier) = 0;
    virtual void removeClientServer(uint64_t clienServerId) = 0;
    virtual void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path) = 0;
    virtual void notifyDeletion(uint32_t uuid) = 0;
    virtual void notifyRpcResponse(uint64_t csId, uint32_t transactionId, Buffer&& returnValue) = 0;
};

/** TODO: UT for notifier **/
class ClientNotifier : public IClientNotifier
{
public:
    ClientNotifier();
    ~ClientNotifier();
    void addClientServer(uint64_t clienServerId, IPTreeOutgoing& clientNotifier);
    void removeClientServer(uint64_t clienServerId);
    void notifyCreation(uint32_t uuid, protocol::PropertyType type, std::string path);
    void notifyDeletion(uint32_t uuid);
    void notifyRpcResponse(uint64_t clientServerId, uint32_t transactionId, Buffer&& returnValue);

private:
    std::map<uint64_t, std::reference_wrapper<IPTreeOutgoing>> clientNotifiers;
    std::mutex clientNotifierMutex;
    logger::Logger log;
};


struct MessageHandlerFactory;

class ClientServer : public std::enable_shared_from_this<ClientServer>
{
public:
    ClientServer(IEndPointPtr endpoint, core::PTreePtr ptree, IClientNotifierPtr notifier);
    ClientServer(const ClientServer&) = delete;
    ClientServer(ClientServer&) = delete;
    void operator = (const ClientServer&) = delete;
    void operator = (ClientServer&) = delete;

    ~ClientServer();

    void setup();
    void teardown();

private:
    void processMessage(protocol::MessageHeaderPtr header, BufferPtr message);

    uint64_t clientServerId;
    ClientServerConfig config;
    IEndPointPtr endpoint;
    PTreeOutgoing outgoing;
    core::PTreePtr ptree;
    IClientNotifierPtr notifier;
    PTreeIncoming incoming;
    logger::Logger log;
};

} // namespace server
} // namespace ptree

#endif  // SERVER_CLIENTSERVER_HPP_